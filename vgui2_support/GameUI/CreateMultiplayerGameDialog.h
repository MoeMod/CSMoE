//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CREATEMULTIPLAYERGAMEDIALOG_H
#define CREATEMULTIPLAYERGAMEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyDialog.h>

class CCreateMultiplayerGameServerPage;
class CCreateMultiplayerGameGameplayPage;
class CCreateMultiplayerGameBotPage;

//-----------------------------------------------------------------------------
// Purpose: dialog for launching a listenserver
//-----------------------------------------------------------------------------
class CCreateMultiplayerGameDialog : public vgui2::PropertyDialog
{
public:
	CCreateMultiplayerGameDialog(vgui2::Panel *parent);
	~CCreateMultiplayerGameDialog();

protected:
	virtual bool OnOK(bool applyOnly);
	virtual void OnClose();
	virtual void SetTitle(const char *title, bool surfaceTitle);

private:
	typedef vgui2::PropertyDialog BaseClass;

	CCreateMultiplayerGameServerPage *m_pServerPage;
	CCreateMultiplayerGameGameplayPage *m_pGameplayPage;
	CCreateMultiplayerGameBotPage *m_pBotPage;

	bool m_bBotsEnabled;

	// for loading/saving game config
	KeyValues *m_pBotSavedData;
};


#endif // CREATEMULTIPLAYERGAMEDIALOG_H
