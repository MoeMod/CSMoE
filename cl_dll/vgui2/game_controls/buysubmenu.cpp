#include "hud.h"
#include "CBaseViewport.h"
#include "buysubmenu.h"

#include "tier1/KeyValues.h"
#include "vgui_controls/WizardPanel.h"
#include "FileSystem.h"
#include "cdll_dll.h"

#include <string>

using namespace vgui2;

CBuySubMenu::CBuySubMenu(vgui2::Panel *parent, const char *name) : WizardSubPanel(parent, name)
{
	m_NextPanel = NULL;
	m_pFirstButton = NULL;

	SetProportional(true);

	m_pPanel = new EditablePanel(this, "ItemInfo");
	m_pPanel->SetProportional(true);
}

CBuySubMenu::~CBuySubMenu(void)
{
}

Panel *CBuySubMenu::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp("MouseOverPanelButton", controlName))
	{
		MouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pPanel);

		if (!m_pFirstButton)
			m_pFirstButton = newButton;

		return newButton;
	}
	else
		return BaseClass::CreateControlByName(controlName);
}

void CBuySubMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	for (int i = 0; i < GetChildCount(); i++)
	{
		MouseOverPanelButton *buyButton = dynamic_cast<MouseOverPanelButton *>(GetChild(i));

		if (buyButton)
		{
			if (buyButton == m_pFirstButton && state == true)
				buyButton->ShowPage();
			else
				buyButton->HidePage();

			buyButton->InvalidateLayout();
		}
	}
}

void CBuySubMenu::Close(void)
{
	if (GetWizardPanel())
		GetWizardPanel()->Close();

    g_pViewport->ShowBackGround(false);
}

CBuySubMenu *CBuySubMenu::CreateNewSubMenu(const char *name)
{
	return new CBuySubMenu(this, name);
}

MouseOverPanelButton *CBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, NULL, panel);
}

void CBuySubMenu::OnCommand(const char *command)
{
	if (Q_strstr(command, ".res"))
	{
		SetupNextSubPanel(command);
		GotoNextSubPanel();
	}
	else
	{
		Close();

		if (Q_stricmp(command, "vguicancel") != 0)
			cl::gEngfuncs.pfnClientCmd((char *)command);

		BaseClass::OnCommand(command);
	}
}

void CBuySubMenu::DeleteSubPanels(void)
{
	if (m_NextPanel)
	{
		m_NextPanel->SetVisible(false);
		m_NextPanel = NULL;
	}

	m_pFirstButton = NULL;
}

void CBuySubMenu::GotoNextSubPanel(void)
{
	if (GetWizardPanel())
		GetWizardPanel()->OnNextButton();
}

void CBuySubMenu::SetupNextSubPanel(const char *fileName)
{
	int i;

	for (i = 0; i < m_SubMenus.Count(); i++)
	{
		if (!Q_stricmp(m_SubMenus[i].filename, fileName))
		{
			m_NextPanel = m_SubMenus[i].panel;
			Assert(m_NextPanel);
			m_NextPanel->InvalidateLayout();
			return;
		}
	}

	if (i == m_SubMenus.Count())
	{
		SubMenuEntry_t newEntry;
		newEntry.filename[0] = 0;
		newEntry.panel = NULL;

		CBuySubMenu *newMenu = CreateNewSubMenu();
		newMenu->LoadControlSettings(fileName, "GAME");
		m_NextPanel = newMenu;
		Q_strncpy(newEntry.filename, fileName, sizeof(newEntry.filename));
		newEntry.panel = newMenu;
		m_SubMenus.AddToTail(newEntry);
	}
}

void CBuySubMenu::SetNextSubPanel(vgui2::WizardSubPanel *panel)
{
	m_NextPanel = panel;
}

vgui2::WizardSubPanel *CBuySubMenu::GetNextSubPanel(void)
{
	return m_NextPanel;
}