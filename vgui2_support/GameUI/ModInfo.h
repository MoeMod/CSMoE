//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef MODINFO_H
#define MODINFO_H
#ifdef _WIN32
#pragma once
#endif

class KeyValues;

//-----------------------------------------------------------------------------
// Purpose: contains all the data entered about a mod in liblist.gam
//-----------------------------------------------------------------------------
class CModInfo
{
public:
	CModInfo();
	~CModInfo();
	void FreeModInfo();

	// loads mod info from liblist.gam
	void LoadCurrentGameInfo();
	// loads mod info from previously loaded liblist.gam
	void LoadGameInfoFromBuffer(const char *buffer, int bufferSize);

	// data accessors
	const char *GetStartMap();
	const char *GetTrainMap();
	bool IsMultiplayerOnly();
	bool IsSinglePlayerOnly();
	bool BShowSimpleLoadingDialog();
	bool NoModels();
	bool NoHiModel();
	const char *GetGameDescription();
   	const char *GetFallbackDir();
	bool GetDetailedTexture();
	bool UseFallbackDirMaps();

	const char *GetMPFilter();

private:
	KeyValues *m_pModData;
};


// singleton accessor
extern CModInfo &ModInfo();

#endif // MODINFO_H
