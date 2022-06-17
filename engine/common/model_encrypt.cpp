#include "common.h"
#include "model_encrypt.h"
#include "mod_local.h"
#include "studio.h"
#include "mod_extend_seq.h"
#include "port.h"
#include "mathlib/IceKey.H"
#include "studio.h"

#include <boost/asio.hpp>
#include <optional>

int matchpattern( const char *in, const char *pattern, qboolean caseinsensitive );

namespace xe {

    static const byte g_pEncryptorKey_22[32] =
    {
        0x73, 0x62, 0x74, 0x69, 0x74, 0x61, 0x6E, 0xE0,
	    0x8C, 0xC6, 0xF1, 0x96, 0xFB, 0x38, 0x75, 0x68,
	    0x88, 0x7A, 0x78, 0x86, 0x78, 0x86, 0x67, 0x70,
	    0xD9, 0x91, 0x07, 0x3A, 0x14, 0x74, 0xFE, 0x22
    };

    IceKey g_Encryptor(4);

    void EncryptChunk(byte* pData, size_t uDataSize)
    {
        if (!uDataSize)
            return;

        size_t uCount = (uDataSize + 7) >> 3;

        while (uCount)
        {
            g_Encryptor.encrypt(pData, pData);
            pData += 8;
            uCount--;
        }
    }

    void EncryptData(byte* pData, size_t uDataSize)
    {
        if (!uDataSize)
            return;

        do
        {
            size_t uTempSize = uDataSize;

            if (uTempSize > 1024)
                uTempSize = 1024;

            if (uTempSize & 7)
                return;

            EncryptChunk(pData, uTempSize);
            pData += uTempSize;
            uDataSize -= uTempSize;
        } while (uDataSize);
    }

    void Mod_EncryptModel(const char* model_name, byte* buffer)
    {
        studiohdr_t* studiohdr = reinterpret_cast<studiohdr_t*>(buffer);
        if (studiohdr->version != STUDIO_VERSION)
            return;

        studiohdr->version = 22;

        g_Encryptor.set(g_pEncryptorKey_22);

        mstudiotexture_t* ptexture = (mstudiotexture_t*)(buffer + studiohdr->textureindex);

        for (int i = 0; i < studiohdr->numtextures; i++)
            EncryptData(buffer + ptexture[i].index, (ptexture[i].width * ptexture[i].height) + (256 * 3));

        mstudiobodyparts_t* pbodypart = (mstudiobodyparts_t*)(buffer + studiohdr->bodypartindex);

        for (int i = 0; i < studiohdr->numbodyparts; i++)
        {
            mstudiomodel_t* pmodel = (mstudiomodel_t*)(buffer + pbodypart[i].modelindex);

            for (int j = 0; j < pbodypart[i].nummodels; j++)
            {
                if (pmodel[j].numverts > 0)
                    EncryptData(buffer + pmodel[j].vertindex, pmodel[j].numverts * sizeof(vec3_t));
            }
        }
    }

    struct ModelEncryptContext{
        boost::asio::io_context ioc_modelencrypt;
        boost::asio::thread_pool ioc_modelencrypt_worker;
        std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> ioc_modelencrypt_work_guard;
        std::optional<boost::asio::executor_work_guard<boost::asio::thread_pool::executor_type>> ioc_modelencrypt_worker_work_guard;
        int cur_num = 0;
        int total_num = 0;
        std::chrono::high_resolution_clock::time_point start_time;
    };
    std::shared_ptr<ModelEncryptContext> g_modelencrypt_ctx;

    void ModelEncrypt_End()
    {
        auto used_time = std::chrono::high_resolution_clock::now() - g_modelencrypt_ctx->start_time;

        using namespace std::chrono_literals;
        Msg(  "ModelEncrypt: finish model encrypting %d files for %d min %02d sec\n", g_modelencrypt_ctx->total_num, (int)(used_time / 1min), (int)(used_time / 1s) - (int)(used_time / 1min) * 60 );

        g_modelencrypt_ctx = nullptr;
    }

    void ModelEncrypt_Run()
    {
        if(g_modelencrypt_ctx)
        {
            g_modelencrypt_ctx->ioc_modelencrypt.poll();
            if(g_modelencrypt_ctx->cur_num >= g_modelencrypt_ctx->total_num)
            {
                ModelEncrypt_End();
            }
        }
    }

    void ModelEncrypt_WriteResult(std::shared_ptr<ModelEncryptContext> modelencrypt_ctx, std::string destpath, byte* buffer, size_t size)
    {
        file_t *pfile = FS_Open( destpath.c_str(), "wb", false );
        if( pfile )
        {
            FS_Write(pfile, buffer, size);
            FS_Close(pfile);
        }
        ++modelencrypt_ctx->cur_num;
        Msg("ModelEncrypt: [%d/%d] %s\n", modelencrypt_ctx->cur_num, modelencrypt_ctx->total_num, destpath.c_str());
    }

    void ModelEncrypt_Process(std::shared_ptr<ModelEncryptContext> modelencrypt_ctx, std::string mdl, std::string destpath, byte* buffer, size_t size)
    {
        Mod_EncryptModel(mdl.c_str(), buffer);

        // switch to main thread
        boost::asio::dispatch(modelencrypt_ctx->ioc_modelencrypt, std::bind(ModelEncrypt_WriteResult, modelencrypt_ctx, destpath, buffer, size));
    }

    void ModelEncrypt_SubmitMDL(std::shared_ptr<ModelEncryptContext> modelencrypt_ctx, std::string mdl, std::string destpath)
    {
        fs_offset_t	size;
        auto buf = FS_LoadFile(mdl.c_str(), &size, true);
        if(buf)
        {
            std::shared_ptr<void> free_helper(buf, [](void *buf) { Mem_Free(buf); });
            boost::asio::dispatch(modelencrypt_ctx->ioc_modelencrypt_worker, std::bind(ModelEncrypt_Process, modelencrypt_ctx, std::move(mdl), std::move(destpath), buf, size));
            ++modelencrypt_ctx->total_num;
        }

    }

    void ModelEncrypt_SearchResursive(std::vector<std::string> &out, const char *start_dir, const char *pattern)
    {
        auto t = FS_Search( va("%s/*", start_dir), true, false );
        for( int i = 0; t && i < t->numfilenames; i++ )
        {
            auto filename = t->filenames[i];

            if(matchpattern(FS_FileWithoutPath(filename), pattern, true))
            {
                out.push_back(filename);
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
                ModelEncrypt_SearchResursive(out, t->filenames[i], pattern);
            }
        }
        if( t ) Mem_Free( t );
    }

    void ModelEncrypt_Initiate(const char* path)
    {
        try {
            Msg(  "ModelEncrypt: start\n" );

            std::vector<std::string> mdl_files;
            std::vector<std::string> tex_files;

            // search for files
           // ModelEncrypt_SearchResursive(mdl_files, "models", "*.mdl");
            mdl_files.push_back(path);

            Msg(  "ModelEncrypt: found %d mdl files\n", (int)mdl_files.size() );

            auto local_modelencrypt_ctx = g_modelencrypt_ctx;
            local_modelencrypt_ctx->start_time = std::chrono::high_resolution_clock::now();

            for(auto mdl : mdl_files)
            {
                ModelEncrypt_SubmitMDL(local_modelencrypt_ctx, mdl, mdl) ;
            }
        }
        catch(const std::exception &e)
        {
            Msg(  "ModelEncrypt error: %s\n", e.what() );
        }
    }

    void ModelEncrypt_Stop()
    {
        if(g_modelencrypt_ctx)
        {
            g_modelencrypt_ctx->ioc_modelencrypt_work_guard.reset();
            g_modelencrypt_ctx->ioc_modelencrypt_worker_work_guard.reset();
            g_modelencrypt_ctx->ioc_modelencrypt.stop();
            g_modelencrypt_ctx->ioc_modelencrypt_worker.stop();
        }
        g_modelencrypt_ctx = nullptr;
    }

    void ModelEncrypt_Start(const char* path)
    {
        ModelEncrypt_Stop();
        g_modelencrypt_ctx = std::make_shared<ModelEncryptContext>();
        g_modelencrypt_ctx->ioc_modelencrypt_work_guard.emplace(boost::asio::make_work_guard(g_modelencrypt_ctx->ioc_modelencrypt));
        g_modelencrypt_ctx->ioc_modelencrypt_worker_work_guard.emplace(boost::asio::make_work_guard(g_modelencrypt_ctx->ioc_modelencrypt_worker));
        ModelEncrypt_Initiate(path);
    }

    void Con_ModelEncrypt_f()
    {
        if(Cmd_Argc() == 2)
        {
            const char *path = Cmd_Argv(1);
            ModelEncrypt_Start(path);
            return;
        }

        Con_Print("Usage: model_encrypt <path>\n");
    }

    void ModelEncrypt_Init()
    {
        Cmd_AddCommand("model_encrypt", Con_ModelEncrypt_f, "model encrypt");
    }
}
