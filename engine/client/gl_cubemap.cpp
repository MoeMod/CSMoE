
#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "gl_cubemap.h"
#include "gl_texlru.h"

namespace xe {

#define MAX_SHADER_ENV 10
#define MAX_SHADER_CUBEMAP 10

class CCubeMapManager
{
public:
    CCubeMapManager();
    ~CCubeMapManager();
    void Reset();
    bool CheckTexture(const mstudiotexture_t *current);
    void SetupTexture();
    void UnloadTexture();
    float GetBrightMul() const;
    float GetBrightAdd() const;
    void SetEnabled(bool x) { m_bEnabled = x; }
    bool GetEnabled() { return m_bEnabled; }
    void LoadTexture();

private:
    void ClearData();
    void LoadData();

public:
    std::string m_pszEnvName[MAX_SHADER_ENV];
    GLuint m_EnvTextureId[MAX_SHADER_ENV];

    struct CubeMapData
    {
        int iEnvID;
        float mulBright;
        float addBright;
    };
    std::unique_ptr<CubeMapData> m_pCubeMapData[MAX_SHADER_CUBEMAP];
    int m_iCubeMapID;
    bool m_bEnabled;
    bool m_bRendering;
};

CCubeMapManager *gpCubeMapManager = nullptr;

CCubeMapManager::CCubeMapManager() : m_pszEnvName{}, m_EnvTextureId{}, m_pCubeMapData{}, m_iCubeMapID(0)
{
    m_bRendering = false;
    m_bEnabled = true;
}

CCubeMapManager::~CCubeMapManager()
{
    ClearData();
}

void CCubeMapManager::Reset()
{
    ClearData();
    LoadData();
}

void CCubeMapManager::ClearData()
{
    for (int i = 0; i < MAX_SHADER_CUBEMAP; i++)
    {
        m_pCubeMapData[i] = nullptr;
    }
}

void CCubeMapManager::LoadData()
{
    byte *afile;
    fs_offset_t filesize;

    if (!(afile = FS_LoadFile("shader/env.txt", &filesize, true)))
        return;
    string token;
    int col_idx;

    char *pfile = (char *)afile;
    struct
    {
        int id;
        string skyname;
    } cache_env_line;

    col_idx = 0;
    cache_env_line = {};

    while(pfile = COM_ParseFile(pfile, token))
    {
        switch(col_idx++)
        {
            case 0:
                cache_env_line.id = Q_atoi(token);
                continue;
            case 1:
                strncpy(cache_env_line.skyname, token, MAX_STRING);
        }

        m_pszEnvName[cache_env_line.id] = cache_env_line.skyname;

        col_idx = 0;
        cache_env_line = {};
    }
    FS_MapFree(afile, filesize);

    search_t *t = FS_Search( "shader/@*.txt", true, true );
    for( int i = 0; t && i < t->numfilenames; i++ )
    {
        int iCachedID = -1;
        sscanf(t->filenames[i], "shader/@%3d.txt", &iCachedID);
        if (iCachedID >= MAX_SHADER_CUBEMAP || iCachedID < 0)
            continue;
        if (!(afile = FS_LoadFile(t->filenames[i], &filesize, true)))
            continue;

        m_pCubeMapData[iCachedID] = std::make_unique<CubeMapData>();

        char *pfile = (char *)afile;
        int consume = 0;

        struct
        {
            int id;
            string name;
            float value;
        } cache_at_line;
        col_idx = 0;

        while(pfile = COM_ParseFile(pfile, token))
        {
            if(consume)
            {
                --consume;
                continue;
            }
            if(!strcmp(token, "type"))
            {
                consume += 1;
                continue;
            }

            switch(col_idx++)
            {
            case 0:
                sscanf(token, "%d", &cache_at_line.id);
                continue;
            case 1:
                sscanf(token, "%s", &cache_at_line.name);
                continue;
            case 2:
                sscanf(token, "%f", &cache_at_line.value);
            }
            col_idx = 0;

            if(!strcmp(cache_at_line.name, "env"))
                m_pCubeMapData[iCachedID]->iEnvID = cache_at_line.value;
            else if(!strcmp(cache_at_line.name, "mulBright"))
                m_pCubeMapData[iCachedID]->mulBright = cache_at_line.value;
            else if(!strcmp(cache_at_line.name, "addBright"))
                m_pCubeMapData[iCachedID]->addBright = cache_at_line.value;

            continue;
        }

        Mem_Free(afile);
        //gEngfuncs.pfnConsolePrint(va("Add cubemap [%d] %d %.1f %.1f \n", iCachedID, m_pCubeMapData[iCachedID]->iEnvID, m_pCubeMapData[iCachedID]->mulBright, m_pCubeMapData[iCachedID]->addBright));

    }
    Mem_Free(t);
}

void CCubeMapManager::LoadTexture()
{
    for(int iCachedID = 0; iCachedID < MAX_SHADER_ENV; ++iCachedID)
    {
        if(m_pszEnvName[iCachedID].empty())
            continue;
        m_EnvTextureId[iCachedID] = xe::TexLru_LoadTextureSkybox(m_pszEnvName[iCachedID].c_str(), 0, nullptr);
    }
}

bool CCubeMapManager::CheckTexture(const mstudiotexture_t *ptexture)
{
    if (!m_bEnabled)
        return false;
    if(ptexture->name[3]!= '@')
        return false;
    char szNum[4];
    strncpy(szNum, ptexture->name + 4, 3);
    m_iCubeMapID = atoi(szNum);

    if (m_iCubeMapID >=10)
        return false;

    return true;
}

void CCubeMapManager::SetupTexture()
{
    m_bRendering = true;
    auto textureID = m_EnvTextureId[m_iCubeMapID];
    xe::TexLru_Upload(textureID);

    pglActiveTextureARB(GL_TEXTURE5_ARB);
    pglEnable(GL_TEXTURE_2D);
    pglEnable(GL_TEXTURE_CUBE_MAP_ARB);
    pglBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureID);
    pglActiveTextureARB(GL_TEXTURE0_ARB);
}

void CCubeMapManager::UnloadTexture()
{
    if (!m_bEnabled || !m_bRendering)
        return;

    pglActiveTextureARB(GL_TEXTURE5_ARB);
    pglBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);
    pglDisable(GL_TEXTURE_2D);
    pglDisable(GL_TEXTURE_CUBE_MAP_ARB);
    pglActiveTextureARB(GL_TEXTURE0_ARB);
}

float CCubeMapManager::GetBrightMul() const
{
    return m_pCubeMapData[m_iCubeMapID]->mulBright;
}

float CCubeMapManager::GetBrightAdd() const
{
    return m_pCubeMapData[m_iCubeMapID]->addBright;
}

void CubeMap_Init()
{
    gpCubeMapManager = new CCubeMapManager();
    gpCubeMapManager->Reset();
}

void CubeMap_Shutdown()
{
    delete gpCubeMapManager;
    gpCubeMapManager = nullptr;
}

void CubeMap_LoadTexture()
{
    gpCubeMapManager->LoadTexture();
}

bool CubeMap_Bind(const mstudiotexture_t *ptexture)
{
    if(RI.currententity == &clgame.viewent && ptexture && gpCubeMapManager->CheckTexture(ptexture))
    {
        gpCubeMapManager->SetupTexture();
        return true;
    }
    else
    {
        gpCubeMapManager->UnloadTexture();
        return false;
    }
}

float CubeMap_envBrightMul()
{
    return gpCubeMapManager->GetBrightMul();
}

float CubeMap_envBrightAdd()
{
    return gpCubeMapManager->GetBrightAdd();
}

}