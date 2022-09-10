#ifndef CSCLASSMENU_H
#define CSCLASSMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls/classmenu.h"
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "vgui_controls/imagepanel.h"
#include "cso_controls/NewTabButton.h"
#include "shared_util.h"
#include "newmouseoverpanelbutton.h"

#define PANEL_CLASS "ClassMenu"

using namespace vgui2;

class CCSClassMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSClassMenu, CClassMenu);

public:
	CCSClassMenu(IViewport* pViewPort);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(IScheme *pScheme);

protected:
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(EditablePanel *panel);

public:
	const char *GetName(void);
	void ShowPanel(bool bShow);
	void Update(void);
	void Reset(void);
	void SetVisible(bool state);
	bool NeedsUpdate(void) { return true; }
	void OnCommand(const char *command);
	void OnSelectClass(const char* name);

	void SetTeam(TeamName team);
	void UpdateGameMode();

public:
	// Left Column
	NewTabButton* m_pShowCTWeapon;
	NewTabButton* m_pShowTERWeapon;
	NewMouseOverPanelButton* m_pSlotButtons[10]; // slot0 ... slot10
	vgui2::Button* m_pPrevBtn; // prevpage
	vgui2::Button* m_pNextBtn; // nextpage
};

#endif