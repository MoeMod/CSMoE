#include "hud.h"
#include "CBaseViewport.h"
#include "buymenu.h"
#include "buysubmenu.h"
#include "mouseoverpanelbutton.h"

using namespace vgui2;

CBuyMenu::CBuyMenu(IViewport *pViewPort) : WizardPanel(NULL, PANEL_BUY), m_pViewPort(pViewPort)
{
	SetScheme("ClientScheme");
	SetTitle("#Cstrike_Buy_Menu", true);

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	SetTitleBarVisible(false);
	SetAutoDelete(false);

	m_pMainMenu = new CBuySubMenu(this, "BuySubMenu");

	LoadControlSettings("Resource/UI/BuyMenu.res", "GAME");

	m_pMainMenu->LoadControlSettings("Resource/UI/MainBuyMenu.res", "GAME");
	m_pMainMenu->SetVisible(false);

	ShowButtons(false);
}

CBuyMenu::~CBuyMenu(void)
{
	if (m_pMainMenu)
		m_pMainMenu->DeleteSubPanels();
}

void CBuyMenu::Init(void)
{
}

void CBuyMenu::VidInit(void)
{
	SetVisible(false);
}

void CBuyMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Update();
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

void CBuyMenu::Update(void)
{
	NULL;
}

void CBuyMenu::OnClose(void)
{
	BaseClass::OnClose();

	ResetHistory();
}