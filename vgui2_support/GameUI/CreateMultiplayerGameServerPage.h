//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CREATEMULTIPLAYERGAMESERVERPAGE_H
#define CREATEMULTIPLAYERGAMESERVERPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include "cvartogglecheckbutton.h"
#include <UtlRBTree.h>
#include <UtlSymbol.h>

namespace vgui2
{
class ListPanel;
}

#define MAX_GAMEMODES 7
static const char* g_szGameModeCode[MAX_GAMEMODES] = {
	"none", "dm", "tdm", "zb1", "zb3", "zb3f2p", "zbs"
}; 
static const wchar_t* g_szGameModeNames[MAX_GAMEMODES] = {
	L"竞技模式", L"个人竞技模式", L"团队竞技模式", L"生化模式(感染)", L"生化模式(英雄)", L"生化模式(英雄)-炮房", L"大灾变"
};

//-----------------------------------------------------------------------------
// Purpose: server options page of the create game server dialog
//-----------------------------------------------------------------------------
class CCreateMultiplayerGameServerPage : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CCreateMultiplayerGameServerPage, vgui2::PropertyPage );

public:
	CCreateMultiplayerGameServerPage(vgui2::Panel *parent, const char *name);
	~CCreateMultiplayerGameServerPage();

	// returns currently entered information about the server
	void SetMap(const char *name);
	bool IsRandomMapSelected();
	const char *GetMapName();

	void SetGameMode(const char* name);
	const char* GetGameMode();

	// CS Bots
	void SetBotQuota( int quota );
	void SetBotsEnabled( bool enabled );
	int GetBotQuota( void );
	bool GetBotsEnabled( void );

protected:
	virtual void OnApplyChanges();
    MESSAGE_FUNC_INT(OnCheckButtonChecked, "CheckButtonChecked", state);

private:
	void LoadMapList();
	void LoadModeList();
	void LoadMaps( const char *pszPathID );

	vgui2::ComboBox *m_pMapList;
	vgui2::ComboBox* m_pModeList;
	vgui2::TextEntry *m_pBotQuotaCombo;
	vgui2::CheckButton *m_pEnableBotsCheck;
	CCvarToggleCheckButton *m_pEnableTutorCheck;

	enum { DATA_STR_LENGTH = 64 };
	char m_szHostName[DATA_STR_LENGTH];
	char m_szPassword[DATA_STR_LENGTH];
	char m_szMapName[DATA_STR_LENGTH];
	char m_szModeName[DATA_STR_LENGTH];
	int m_iMaxPlayers;
	CUtlRBTree<CUtlSymbol> m_MapNames;
};


#endif // CREATEMULTIPLAYERGAMESERVERPAGE_H
