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

	// CS Bots
	void SetBotQuota( int quota );
	void SetBotsEnabled( bool enabled );
	int GetBotQuota( void );
	bool GetBotsEnabled( void );

protected:
	virtual void OnApplyChanges();
	MESSAGE_FUNC( OnCheckButtonChecked, "CheckButtonChecked" );

private:
	void LoadMapList();
	void LoadMaps( const char *pszPathID );

	vgui2::ComboBox *m_pMapList;
	vgui2::TextEntry *m_pBotQuotaCombo;
	vgui2::CheckButton *m_pEnableBotsCheck;
	CCvarToggleCheckButton *m_pEnableTutorCheck;

	enum { DATA_STR_LENGTH = 64 };
	char m_szHostName[DATA_STR_LENGTH];
	char m_szPassword[DATA_STR_LENGTH];
	char m_szMapName[DATA_STR_LENGTH];
	int m_iMaxPlayers;
	CUtlRBTree<CUtlSymbol> m_MapNames;
};


#endif // CREATEMULTIPLAYERGAMESERVERPAGE_H
