#ifndef BUYSUBMENU_H
#define BUYSUBMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/WizardSubPanel.h"
#include "vgui_controls/Button.h"
#include "UtlVector.h"
#include "mouseoverpanelbutton.h"

#include "weapons_moe_buy.h"

class CBuyMenu;

class CBuySubMenu : public vgui2::WizardSubPanel
{
private:
	DECLARE_CLASS_SIMPLE(CBuySubMenu, vgui2::WizardSubPanel);

public:
	CBuySubMenu(vgui2::Panel *parent, const char *name = "BuySubMenu");
	~CBuySubMenu(void);

public:
	virtual void SetVisible(bool state);
	virtual void DeleteSubPanels(void);

public:
	virtual void OnCommand(const char *command);

public:
	virtual void Close(void);
	virtual void GotoNextSubPanel(void);
	virtual void SetupNextSubPanel(const char *fileName);

protected:
	virtual void SetNextSubPanel(vgui2::WizardSubPanel *panel);
	virtual vgui2::WizardSubPanel *GetNextSubPanel(void);
	virtual vgui2::Panel *CreateControlByName(const char *controlName) override;
	virtual CBuySubMenu *CreateNewSubMenu(const char *name = "BuySubMenu");
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel);

protected:
	typedef struct
	{
		char filename[_MAX_PATH];
		CBuySubMenu *panel;
	}
	SubMenuEntry_t;

protected:
	vgui2::EditablePanel *m_pPanel;
	MouseOverPanelButton *m_pFirstButton;
	CUtlVector<SubMenuEntry_t> m_SubMenus;
	vgui2::WizardSubPanel *m_NextPanel;
};

#endif