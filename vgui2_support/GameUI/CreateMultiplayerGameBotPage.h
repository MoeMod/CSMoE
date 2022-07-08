//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CREATEMULTIPLAYERGAMEBOTPAGE_H
#define CREATEMULTIPLAYERGAMEBOTPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CPanelListPanel;
class CDescription;
class mpcontrol_t;
class CCvarToggleCheckButton;

//-----------------------------------------------------------------------------
// Purpose: advanced bot properties page of the create game server dialog
//-----------------------------------------------------------------------------
class CCreateMultiplayerGameBotPage : public vgui2::PropertyPage
{
public:
	CCreateMultiplayerGameBotPage( vgui2::Panel *parent, const char *name, KeyValues *botKeys );
	~CCreateMultiplayerGameBotPage();

	// put all cvars and their current values into 'botKeys'
	void UpdateKeys( KeyValues *botKeys );

protected:
	virtual void OnResetChanges();
	virtual void OnApplyChanges();

private:
	CCvarToggleCheckButton *m_joinAfterPlayer;

	CCvarToggleCheckButton *m_allowRogues;

	CCvarToggleCheckButton *m_allowPistols;
	CCvarToggleCheckButton *m_allowShotguns;
	CCvarToggleCheckButton *m_allowSubmachineGuns;
	CCvarToggleCheckButton *m_allowMachineGuns;
	CCvarToggleCheckButton *m_allowRifles;
	CCvarToggleCheckButton *m_allowGrenades;
	CCvarToggleCheckButton *m_allowShields;
	CCvarToggleCheckButton *m_allowSnipers;

	CCvarToggleCheckButton *m_deferToHuman;

	vgui2::ComboBox *m_joinTeamCombo;
	void SetJoinTeamCombo( const char *team );

	vgui2::ComboBox *m_chatterCombo;
	void SetChatterCombo( const char *team );

	vgui2::TextEntry *m_quotaEntry;
	vgui2::TextEntry *m_prefixEntry;
};


#endif // CREATEMULTIPLAYERGAMEBOTPAGE_H
