#include "hud.h"
#include "CBaseViewport.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "cstrikebuymenu.h"
#include "cstrikebuysubmenu.h"
#include "shared_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "vgui_controls/RichText.h"
#include "buymouseoverpanelbutton.h"

#include <string>

using namespace vgui2;

CCSBaseBuyMenu::CCSBaseBuyMenu(IViewport *pViewPort) : CBuyMenu(pViewPort)
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);
}

void CCSBaseBuyMenu::SetupControlSettings()
{
	LoadControlSettings("Resource/UI/Buymenu.res", "GAME");
	LoadControlSettings("Resource/UI/CSO_Buymenu.res", "GAME");

	if (m_pMainMenu)
	{
		m_pMainMenu->LoadControlSettings("Resource/UI/cso_buysubmenu_ver2.res", "GAME");
		m_pMainMenu->SetVisible(false);
	}
	
}

void CCSBaseBuyMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *defaultButton = FindChildByName("QuitButton");

		if (defaultButton)
			defaultButton->RequestFocus();

		SetMouseInputEnabled(true);
		if(m_pMainMenu)
			m_pMainMenu->SetMouseInputEnabled(true);
	}
}

void CCSBaseBuyMenu::Init(void)
{
}

void CCSBaseBuyMenu::VidInit(void)
{
	SetVisible(false);
}

void CCSBaseBuyMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (cl::gHUD.m_iIntermission || cl::gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);
}

void CCSBaseBuyMenu::Paint(void)
{
	BaseClass::Paint();
}

void CCSBaseBuyMenu::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSBaseBuyMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CCSBaseBuyMenu::GotoMenu(int iMenu)
{
	if (!m_pMainMenu)
		return;

	const char *command = NULL;

	switch (iMenu)
	{
	case MENU_BUY_PISTOL:
	{
		command = "VGUI_BuyMenu_Show 0";

		break;
	}

	case MENU_BUY_SHOTGUN:
	{
		command = "VGUI_BuyMenu_Show 1";

		break;
	}

	case MENU_BUY_RIFLE:
	{
		command = "VGUI_BuyMenu_Show 2";

		break;
	}

	case MENU_BUY_SUBMACHINEGUN:
	{
		command = "VGUI_BuyMenu_Show 3";

		break;
	}

	case MENU_BUY_MACHINEGUN:
	{
		command = "VGUI_BuyMenu_Show 4";
		break;
	}

	case MENU_BUY_ITEM:
	{
		command = "VGUI_BuyMenu_Show 5";

		break;
	}
	case MENU_BUY:
	{
		command = "VGUI_BuyMenu_Show";

		break;
	}
	}

	ActivateNextSubPanel(m_pMainMenu);
	if (command)
	{
		m_pMainMenu->OnCommand(command);
		m_pMainMenu->GotoNextSubPanel();
	}
}

void CCSBaseBuyMenu::ActivateMenu(int iMenu)
{
	GotoMenu(iMenu);

	g_pViewport->ShowPanel(this, true);
}

void CCSBaseBuyMenu::SetTeam(int iTeam)
{
	if (!m_pMainMenu)
		return;
	m_iTeam = iTeam;
	if(iTeam == TEAM_TERRORIST)
		m_pMainMenu->OnCommand("VGUI_BuyMenu_SetTeam 1");
	else if (iTeam == TEAM_CT)
		m_pMainMenu->OnCommand("VGUI_BuyMenu_SetTeam 2");
	else
		m_pMainMenu->OnCommand("VGUI_BuyMenu_SetTeam 0");
}

void CCSBaseBuyMenu::UpdateGameMode()
{
	ResetHistory();
	ResetCurrentSubPanel();
	if (m_pMainMenu)
		m_pMainMenu->DeletePanel();

	if (cl::gHUD.IsZombieMod() && (cl::gHUD.m_iModRunning != MOD_ZB4))
		m_pMainMenu = new CCSBuySubMenu_ZombieMod(this);
	else if ((cl::gHUD.m_iModRunning == MOD_ZB4) || (cl::gHUD.m_iModRunning == MOD_TDM) || (cl::gHUD.m_iModRunning == MOD_DM))
		m_pMainMenu = new CCSBuySubMenu_ZombieMod(this); // new CCSBuySubMenu_DeathMatch(this);
	else
		m_pMainMenu = new CCSBuySubMenu_DefaultMode(this);


	SetupControlSettings();
}
