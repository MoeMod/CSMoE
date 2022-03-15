#ifndef MULTIPLAYERADVANCEDDIALOG_H
#define MULTIPLAYERADVANCEDDIALOG_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include "ScriptObject.h"
#include <vgui/KeyCode.h>

class CMultiplayerAdvancedDialog : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CMultiplayerAdvancedDialog, vgui2::Frame);

public:
	CMultiplayerAdvancedDialog(vgui2::Panel *parent);
	~CMultiplayerAdvancedDialog(void);

public:
	virtual void Activate(void);

private:

	void CreateControls(void);
	void DestroyControls(void);
	void GatherCurrentValues(void);
	void SaveValues(void);

public:
	CInfoDescription *m_pDescription;
	mpcontrol_t *m_pList;
	CPanelListPanel *m_pListPanel;

public:
	virtual void OnCommand(const char *command);
	virtual void OnClose(void);
	virtual void OnKeyCodeTyped(vgui2::KeyCode code);
};

#endif
