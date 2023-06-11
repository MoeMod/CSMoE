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



#include "csmoe/newmouseoverpanelbutton.h"

#define PANEL_CLASS "ClassMenu"

using namespace vgui2;

class CCSClassMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSClassMenu, CClassMenu);

public:
	CCSClassMenu(IViewport* pViewPort);
	~CCSClassMenu(void);
	MESSAGE_FUNC_CHARPTR(OnUpdateClass, "UpdateClass", name);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(IScheme* pScheme);

protected:
	void SetupControlSettings();
	virtual MouseOverPanelButton* CreateNewMouseOverPanelButton(EditablePanel* panel);
	int m_pAfterUpdated;

public:
	const char* GetName(void);
	void ShowPanel(bool bShow);
	void Update(void);
	void Reset(void);
	void SetVisible(bool state);
	bool NeedsUpdate(void) { return true; }
	void OnCommand(const char* command);
	void OnSelectClass(TeamName team, const char* name);
	void OnSelectClassZombie(const char* name);

	void SetTeam(TeamName team);
	void SetupTeamPage(TeamName team, size_t iPage);
	void SetupPage(size_t iPage);
	void UpdateGameMode();
	void UpdateClass(int i);
	bool CheckShowType();
public:
	// Left Column
	NewTabButton* m_pShowCT;
	NewTabButton* m_pShowTER;
	NewMouseOverPanelButton* m_pSlotButtons[10]; // slot0 ... slot10
	vgui2::Button* m_pPrevBtn; // prevpage
	vgui2::Button* m_pNextBtn; // nextpage

	vgui2::Label* m_pTitleLabel;
	vgui2::ImagePanel* m_pClassImage;
	vgui2::ImagePanel* m_pSkillInfo[2];
	vgui2::ImagePanel* m_pSkillInfoImage[2];

	vgui2::Label* m_pClassDesc;
	vgui2::Label* m_pTipText;
	vgui2::Label* m_pSkillInfoText[2];
	vgui2::Label* m_pSkillInfoText_Desc[2];

	int m_pCurrentGameMode;
	size_t m_iCurrentPage;
	TeamName m_iCurrentTeamPage;
};

#endif