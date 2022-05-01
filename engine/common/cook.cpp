#include "common.h"
#include "cook.h"
#include "mod_local.h"
#include "studio.h"
#include "mod_decryptor.h"
#include "mod_extend_seq.h"

#include <boost/asio.hpp>
#include <optional>

struct astcenc_context;
astcenc_context *Image_SaveASTC_CreateContext(int thread_num);
void Image_SaveASTC_DestroyContext(astcenc_context *context);
std::vector<byte> Image_SaveASTC_Worker(astcenc_context *context, std::shared_ptr<byte[]> rgba, int width, int height, int size, int thread_id);

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
            g_cook_ctx->ioc_cook.poll();
            if(g_cook_ctx->cur_num >= g_cook_ctx->total_num)
            {
                Cook_End();
            }
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

    void Cook_ProcessTexture(std::shared_ptr<CookContext> cook_ctx, std::shared_ptr<byte[]> rgba, int width, int height, int size, std::string destpath)
    {
        std::shared_ptr<astcenc_context> context ( Image_SaveASTC_CreateContext(16), Image_SaveASTC_DestroyContext );
        auto astc_tex = Image_SaveASTC_Worker(context.get(), rgba, width, height, size, 0);

        // switch to main thread
        boost::asio::dispatch(cook_ctx->ioc_cook, std::bind(Cook_WriteResult, cook_ctx, destpath, std::move(astc_tex)));
    }

    void Cook_SubmitMDL(std::shared_ptr<CookContext> cook_ctx, std::string mdl)
    {
        auto buf = FS_LoadFile(mdl.c_str(), nullptr, true);
        if(buf)
        {
            std::shared_ptr<void> free_helper(buf, [](void *buf) { Mem_Free(buf); });

            Mod_DecryptModel(mdl.c_str(), buf);
            byte *buf2 = Mod_LoadExtendSeq(mdl.c_str(), buf);
            studiohdr_t *phdr = (studiohdr_t *)buf2;

            auto ptexture = (mstudiotexture_t *)(((byte *)phdr) + phdr->textureindex);
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
                    Image_SetMDLPointer((byte *)phdr + ptexture[i].index);
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

                    {
                        std::shared_ptr<byte[]> mt_buffer(new byte[pic->size]);
                        memcpy(mt_buffer.get(), pic->buffer, pic->size);

                        auto width = pic->width;
                        auto height = pic->height;
                        auto size = pic->size;

                        pic = nullptr;

                        boost::asio::dispatch(cook_ctx->ioc_cook_worker, std::bind(Cook_ProcessTexture, cook_ctx, mt_buffer, width, height, size, std::string(destpath)));
                        ++cook_ctx->total_num;
                    }
                }
            }
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

            {
                std::shared_ptr<byte[]> mt_buffer(new byte[pic->size]);
                memcpy(mt_buffer.get(), pic->buffer, pic->size);

                auto width = pic->width;
                auto height = pic->height;
                auto size = pic->size;

                pic = nullptr;

                boost::asio::dispatch(cook_ctx->ioc_cook_worker, std::bind(Cook_ProcessTexture, cook_ctx, mt_buffer, width, height, size, std::string(destpath)));
                ++cook_ctx->total_num;
            }
        }

    }

    void Cook_Initiate()
    {
        try {
            Msg(  "Cook: start\n" );

            std::vector<std::string> mdl_files;
            std::vector<std::string> tex_files;

            // search for files
            auto t = FS_Search( "models/*.mdl", true, true );
            for( int i = 0; t && i < t->numfilenames; i++ )
            {
                mdl_files.push_back(t->filenames[i]);
            }
            if( t ) Mem_Free( t );
            Msg(  "Cook: found %d mdl files\n", (int)mdl_files.size() );

            t = FS_Search( "models/texture/*.bmp", true, true );
            for( int i = 0; t && i < t->numfilenames; i++ )
            {
                tex_files.push_back(t->filenames[i]);
            }
            if( t ) Mem_Free( t );
            Msg(  "Cook: found %d texture files\n", (int)tex_files.size() );

            auto local_cook_ctx = g_cook_ctx;
            local_cook_ctx->start_time = std::chrono::high_resolution_clock::now();

            std::vector<boost::asio::awaitable<void>> task_list;

            for(auto mdl : mdl_files)
            {
                Cook_SubmitMDL(local_cook_ctx, mdl) ;
            }

            for(auto file : tex_files)
            {
                Cook_SubmitTex(local_cook_ctx, file);
            }
        }
        catch(const std::exception &e)
        {
            Msg(  "Cook error: %s\n", e.what() );
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
        Cook_Initiate();
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
