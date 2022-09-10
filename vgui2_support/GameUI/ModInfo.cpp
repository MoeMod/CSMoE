//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include <string.h>

#include "ModInfo.h"
#include <KeyValues.h>
#include <vgui_controls/Controls.h>
#include <FileSystem.h>
#include "EngineInterface.h"

//-----------------------------------------------------------------------------
// Purpose: singleton accessor
//-----------------------------------------------------------------------------
CModInfo &ModInfo()
{
	static CModInfo s_ModInfo;
	return s_ModInfo;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CModInfo::CModInfo()
{
	m_pModData = new KeyValues("ModData");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CModInfo::~CModInfo()
{
	FreeModInfo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModInfo::FreeModInfo()
{
	if (m_pModData)
	{
		m_pModData->deleteThis();
		m_pModData = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetStartMap()
{
	return m_pModData->GetString("startmap", "c0a0");
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetTrainMap()
{
	return m_pModData->GetString("trainmap", "t0a0");
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::IsMultiplayerOnly()
{
	return (stricmp(m_pModData->GetString("type", ""), "multiplayer_only") == 0);
}

bool CModInfo::IsSinglePlayerOnly()
{
	return (stricmp(m_pModData->GetString("type", ""), "singleplayer_only") == 0);
}

bool CModInfo::BShowSimpleLoadingDialog()
{
	if (IsSinglePlayerOnly())
		return true;

	return (stricmp(m_pModData->GetString("game", "0"), "Opposing Force") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::NoModels()
{
	return (stricmp(m_pModData->GetString("nomodels", "0"), "1") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::NoHiModel()
{
	return (stricmp(m_pModData->GetString("nohimodel", "0"), "1") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetGameDescription()
{
	return m_pModData->GetString("game", "Half-Life");
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetFallbackDir()
{
	return m_pModData->GetString("fallback_dir", "");
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::GetDetailedTexture()
{
	return (stricmp(m_pModData->GetString("detailed_textures", "0"), "1") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::UseFallbackDirMaps()
{
	return (stricmp(m_pModData->GetString("fallback_maps", "0"), "1") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetMPFilter()
{
	return m_pModData->GetString("mpfilter", "");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModInfo::LoadCurrentGameInfo()
{
	// Load up liblist.gam for the current mod
	char const *filename = "liblist.gam";

	// walk through and add the key/value pairs to the keyvalues object
	FileHandle_t fh = vgui2::filesystem()->Open(filename, "rb");
	if (fh != FILESYSTEM_INVALID_HANDLE)
	{
		int len = vgui2::filesystem()->Size(fh);
		if (len > 0)
		{
			char *buf = (char *)_alloca(len + 1);
			vgui2::filesystem()->Read(buf, len, fh);
			buf[len] = 0;
			LoadGameInfoFromBuffer(buf, len);
		}

		vgui2::filesystem()->Close(fh);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModInfo::LoadGameInfoFromBuffer(const char *buffer, int bufferSize)
{
	char token[1024];
	bool done = false;
	char *p = (char *)buffer;
	while (!done && p)
	{
		char key[256];
		char value[256];

		p = engine->COM_ParseFile(p, token);
		if (strlen(token) <= 0)
			break;
		strcpy(key, token);

		p = engine->COM_ParseFile(p, token);
		strcpy(value, token);

		m_pModData->SetString(key, value);
	}
}

