#include "common.h"
#include "cook.h"
#include "mod_local.h"
#include "studio.h"
#include "mod_decryptor.h"
#include "mod_extend_seq.h"
#include "sprite.h"

#include <boost/asio.hpp>
#include <optional>

struct astcenc_context;
astcenc_context *Image_SaveASTC_CreateContext(int thread_num);
void Image_SaveASTC_DestroyContext(astcenc_context *context);
std::vector<byte> Image_SaveASTC_Worker(astcenc_context* context, byte *rgba, int width, int height, int size, int thread_id);
int matchpattern( const char *in, const char *pattern, qboolean caseinsensitive );

namespace xe {
    struct CookContext{
        boost::asio::io_context ioc_cook;
        boost::asio::thread_pool ioc_cook_worker;
        std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> ioc_cook_work_guard;
        std::optional<boost::asio::executor_work_guard<boost::asio::thread_pool::executor_type>> ioc_cook_worker_work_guard;
        int cur_num = 0;
        int total_num = 0;
        std::chrono::high_resolution_clock::time_point start_time;
    };
    std::shared_ptr<CookContext> g_cook_ctx;

    void Cook_End()
    {
        auto used_time = std::chrono::high_resolution_clock::now() - g_cook_ctx->start_time;

        using namespace std::chrono_literals;
        Msg(  "Cook: finish cooking %d files for %d min %02d sec\n", g_cook_ctx->total_num, (int)(used_time / 1min), (int)(used_time / 1s) - (int)(used_time / 1min) * 60 );

        g_cook_ctx = nullptr;
    }

    void Cook_Run()
    {
        if(g_cook_ctx)
        {
            g_cook_ctx->ioc_cook.run_for(duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(host.realframetime / 2)));
        }
    }

    void Cook_WriteResult(std::shared_ptr<CookContext> cook_ctx, std::string destpath, std::vector<byte> astc_tex)
    {
        file_t *pfile = FS_Open( destpath.c_str(), "wb", false );
        if( pfile )
        {
            FS_Write(pfile, astc_tex.data(), astc_tex.size());
            FS_Close(pfile);
        }
        ++cook_ctx->cur_num;
        Msg("Cook: [%d/%d] %s\n", cook_ctx->cur_num, cook_ctx->total_num, destpath.c_str());
    }

    void Cook_ProcessTexture(std::shared_ptr<CookContext> cook_ctx, std::vector<byte> rgba, int width, int height, std::string destpath)
    {
        std::shared_ptr<astcenc_context> context ( Image_SaveASTC_CreateContext(1), Image_SaveASTC_DestroyContext );
        auto astc_tex = Image_SaveASTC_Worker(context.get(), rgba.data(), width, height, rgba.size(), 0);

        // switch to main thread
        boost::asio::dispatch(cook_ctx->ioc_cook, std::bind(Cook_WriteResult, cook_ctx, destpath, std::move(astc_tex)));
    }

    void Cook_SubmitPic(std::shared_ptr<CookContext> cook_ctx, image_ref pic, std::string destpath)
    {
        std::vector<byte> mt_buffer(pic->size);
        memcpy(mt_buffer.data(), pic->buffer, pic->size);

        auto width = pic->width;
        auto height = pic->height;
        auto size = pic->size;

        boost::asio::dispatch(cook_ctx->ioc_cook_worker, std::bind(Cook_ProcessTexture, cook_ctx, std::move(mt_buffer), width, height, std::move(destpath)));
        ++cook_ctx->total_num;
    }

    void Cook_SubmitMDL(std::shared_ptr<CookContext> cook_ctx, std::string mdl)
    {
        fs_offset_t filesize;
        auto buf = FS_MapFile(mdl.c_str(), &filesize, true);
        if(buf)
        {
            if(Mod_IsModelEncrypted(mdl.c_str(), buf) || Mod_NumExtendSeq(mdl.c_str()) > 0)
            {
                // switch to COW mode
                FS_MapFree(buf, filesize);
                auto buf2 = FS_MapFileCOW( mdl.c_str(), &filesize, false );
                buf = buf2;
                Mod_DecryptModel(mdl.c_str(), buf2);

                buf2 = Mod_LoadExtendSeq(mdl.c_str(), buf2, &filesize);
                buf = buf2;
            }
            const studiohdr_t *phdr = (const studiohdr_t *)buf;

            auto ptexture = (const mstudiotexture_t *)(((const byte *)phdr) + phdr->textureindex);
            if( phdr->textureindex > 0 && phdr->numtextures <= MAXSTUDIOSKINS )
            {
                char mdlname[128];
                Q_strncpy( mdlname, mdl.c_str(), sizeof( mdlname ));
                FS_StripExtension( mdlname );

                for( int i = 0; i < phdr->numtextures; i++ )
                {
                    if(ptexture[i].width <= 4 || ptexture[i].height <= 1)
                    {
                        // too small, ignore
                        continue;
                    }
                    char name[128];
                    FS_FileBase( ptexture[i].name, name );

                    char texname[128];
                    Q_snprintf( texname, sizeof( texname ), "#%s/%s.mdl", mdlname, name );
                    auto size = sizeof( mstudiotexture_t ) + ptexture[i].width * ptexture[i].height + 768;
                    Image_SetMDLPointer((const byte *)phdr + ptexture[i].index);
                    auto pic = FS_LoadImage( texname, (byte *)&ptexture[i], size );
                    if(!pic)
                    {
                        Msg(  "Cook error: bad mdl format %s %s\n", mdl.c_str(), ptexture[i].name );
                        continue;
                    }

                    // save file as astc
                    char destpath[128];
                    Q_snprintf( destpath, sizeof( destpath ), "ddc/%s.mdl/%s", mdlname, name );
                    FS_StripExtension( destpath );
                    FS_DefaultExtension( destpath, ".astc" );

                    Cook_SubmitPic(cook_ctx, pic, destpath);
                }
            }
            FS_MapFree(buf, filesize);
        }

    }

    void Cook_SubmitTex(std::shared_ptr<CookContext> cook_ctx, std::string file)
    {
        auto pic = FS_LoadImage( file.c_str(), nullptr, 0 );
        if(pic)
        {
            char destpath[128];
            Q_snprintf( destpath, sizeof( destpath ), "ddc/%s", file.c_str() );
            FS_StripExtension( destpath );
            FS_DefaultExtension( destpath, ".astc" );

            Cook_SubmitPic(cook_ctx, pic, destpath);
        }

    }

    const byte *Cook_SubmitSPR_Frame(std::shared_ptr<CookContext> cook_ctx, const std::string &spr, const dsprite_t *psprite, const byte *pin, const char *group_suffix, int num)
    {
        auto pinframe = reinterpret_cast<const dspriteframe_t *>(pin);
        char		texname[128];
        Q_snprintf(texname, sizeof(texname), "#%s_%s_%i%i.spr", spr.c_str(), group_suffix, num / 10, num % 10);
        if(auto size = psprite->bounds[0] * psprite->bounds[1])
        {
            if(auto pic = FS_LoadImage(texname, pin, size))
            {
                char destpath[128];
                Q_snprintf( destpath, sizeof( destpath ), "ddc/%s/%s_%i%i.astc", spr.c_str(), group_suffix, num / 10, num % 10 );
                Cook_SubmitPic(cook_ctx, pic, destpath);
            }
        }
        return (pin + sizeof(dspriteframe_t) + pinframe->width * pinframe->height);
    }

    const byte *Cook_SubmitSPR_Group(std::shared_ptr<CookContext> cook_ctx, const std::string &spr, const dsprite_t *psprite, const byte *pin, const char *group_suffix, int framenum)
    {
        // TODO : not really used
        auto pingroup = reinterpret_cast<const dspritegroup_t *>(pin);
        pin += sizeof(dspritegroup_t);
        pin += sizeof(dspriteinterval_t) * pingroup->numframes;
        for( int i = 0; i < pingroup->numframes; i++ )
        {
            pin = Cook_SubmitSPR_Frame(cook_ctx, spr, psprite, pin, group_suffix, framenum * 10 + i);
        }
        return pin;
    }

    void Cook_SubmitSPR(std::shared_ptr<CookContext> cook_ctx, const std::string &spr)
    {
        fs_offset_t filesize;
        auto buffer = FS_MapFile(spr.c_str(), &filesize, true);
        if(buffer)
        {
            auto pin = reinterpret_cast<const dsprite_t *>(buffer);
            if( LittleLong(pin->ident) != IDSPRITEHEADER || LittleLong(pin->version) != SPRITE_VERSION )
            {
                // ignored invalid file
                FS_MapFree(buffer, filesize);
                return;
            }
            buffer += sizeof(dsprite_t);
            short numi = *reinterpret_cast<const short *>(buffer);

            if (numi == 256)
            {
                image_ref pal;

                buffer += sizeof(short);

                // install palette
                switch (pin->texFormat)
                {
                    case SPR_ADDITIVE:
                        pal = FS_LoadImage("#normal.pal", buffer, 768);
                        break;
                    case SPR_INDEXALPHA:
                        pal = FS_LoadImage("#decal.pal", buffer, 768);
                        break;
                    case SPR_ALPHTEST:
                        pal = FS_LoadImage("#transparent.pal", buffer, 768);
                        break;
                    case SPR_NORMAL:
                    default:
                        pal = FS_LoadImage("#normal.pal", buffer, 768);
                        break;
                }

                buffer += 768; //读下一个数据块
                pal = nullptr; // palette installed, no reason to keep this data
            }
            else
            {
                MsgDev(D_ERROR, "%s has wrong number of palette colors %i (should be 256)\n", spr.c_str(), numi);
                FS_MapFree( buffer, filesize );
                return;
            }

            for( int i = 0; i < pin->numframes; i++ ) //依次读取每一帧的数据
            {
                frametype_t frametype = (frametype_t)*buffer;
                char		group_suffix[8];

                switch (frametype)
                {
                    case FRAME_SINGLE:
                        Q_strncpy(group_suffix, "one", sizeof(group_suffix));
                        buffer = Cook_SubmitSPR_Frame(cook_ctx, spr, pin, buffer + sizeof(int), group_suffix, i);
                        break;
                    case FRAME_GROUP:
                        Q_strncpy(group_suffix, "grp", sizeof(group_suffix));
                        buffer = Cook_SubmitSPR_Group(cook_ctx, spr, pin, buffer + sizeof(int), group_suffix, i);
                        break;
                    case FRAME_ANGLED:
                        Q_strncpy(group_suffix, "ang", sizeof(group_suffix));
                        buffer = Cook_SubmitSPR_Group(cook_ctx, spr, pin, buffer + sizeof(int), group_suffix, i);
                        break;
                }
            }
            FS_MapFree(buffer, filesize);
        }
    }

    boost::asio::awaitable<void> Cook_YieldCurrentFrame()
    {
        co_await boost::asio::post(g_cook_ctx->ioc_cook, boost::asio::use_awaitable);
    }

    boost::asio::awaitable<void> Cook_CheckFrameSmooth()
    {
        static std::chrono::high_resolution_clock::time_point last_frame_time;
        auto now = std::chrono::high_resolution_clock::now();
        if(now - last_frame_time > std::chrono::duration<double>(host.realframetime))
        {
            last_frame_time = now;
            // yield for next frame
            co_await Cook_YieldCurrentFrame();
        }
        co_return;
    }

    template<class OutFn = void(*)(const char *)>
    boost::asio::awaitable<void> Cook_SearchResursive(const char *start_dir, const char *pattern, OutFn out)
    {
        std::unique_ptr<search_t, void(*)(void*)> t(FS_Search( va("%s/*", start_dir), true, false ), Mem_Free);
        for( int i = 0; t && i < t->numfilenames; i++ )
        {
            const char *filename = t->filenames[i];

            if(matchpattern(FS_FileWithoutPath(filename), pattern, true))
            {
                out(filename);
            }
            else if(!Q_stricmp(t->filenames[i], va("%s/.", start_dir)))
            {
                // ignored
            }
            else if(!Q_stricmp(t->filenames[i], va("%s/..", start_dir)))
            {
                // ignored
            }
            else
            {
                co_await Cook_SearchResursive(t->filenames[i], pattern, out);
            }
            co_await Cook_CheckFrameSmooth();
        }
    }

    boost::asio::awaitable<void> Cook_Initiate()
    {
        try {
            Msg(  "Cook: loading cook context\n" );

            auto local_cook_ctx = g_cook_ctx;
            local_cook_ctx->start_time = std::chrono::high_resolution_clock::now();
            co_await boost::asio::post(g_cook_ctx->ioc_cook, boost::asio::use_awaitable);

            // search for files
            co_await Cook_SearchResursive( "models", "*.mdl", std::bind(Cook_SubmitMDL, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("models/texture", "*.bmp", std::bind(Cook_SubmitTex, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("models/texture", "*.tga", std::bind(Cook_SubmitTex, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("gfx/env", "*.bmp", std::bind(Cook_SubmitTex, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("gfx/env", "*.tga", std::bind(Cook_SubmitTex, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("overviews", "*.bmp", std::bind(Cook_SubmitTex, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("overviews", "*.tga", std::bind(Cook_SubmitTex, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("resource", "*.tga", std::bind(Cook_SubmitTex, local_cook_ctx, std::placeholders::_1));
            co_await Cook_SearchResursive("sprites", "*.spr", std::bind(Cook_SubmitSPR, local_cook_ctx, std::placeholders::_1));

            while(g_cook_ctx->cur_num < g_cook_ctx->total_num)
            {
                co_await Cook_YieldCurrentFrame();
            }
            Cook_End();
        }
        catch(const std::exception &e)
        {
            Msg(  "Cook error: %s\n", e.what() );
            Cook_Stop();
        }
    }

    void Cook_Stop()
    {
        if(g_cook_ctx)
        {
            g_cook_ctx->ioc_cook_work_guard.reset();
            g_cook_ctx->ioc_cook_worker_work_guard.reset();
            g_cook_ctx->ioc_cook.stop();
            g_cook_ctx->ioc_cook_worker.stop();
        }
        g_cook_ctx = nullptr;
    }

    void Cook_Start()
    {
        Cook_Stop();
        g_cook_ctx = std::make_shared<CookContext>();
        g_cook_ctx->ioc_cook_work_guard.emplace(boost::asio::make_work_guard(g_cook_ctx->ioc_cook));
        g_cook_ctx->ioc_cook_worker_work_guard.emplace(boost::asio::make_work_guard(g_cook_ctx->ioc_cook_worker));
        boost::asio::co_spawn(g_cook_ctx->ioc_cook, Cook_Initiate(), boost::asio::detached);
    }

    void Con_Cook_f()
    {
        if(Cmd_Argc() >= 2)
        {
            const char *act = Cmd_Argv(1);
            if(!Q_stricmp(act, "start"))
            {
                Cook_Start();
                return;
            }
            else if(!Q_stricmp(act, "stop"))
            {
                Cook_Stop();
                return;
            }
        }

        Con_Print("Usage: cook <start|stop>");
    }

    void Cook_Init()
    {
        Cmd_AddCommand("cook", Con_Cook_f, "cook start / stop");
    }
}
