#include <vgui_controls/RichText.h>
#include "CGameUITestPanel.h"
#include "vgui/IEngineVgui.h"
#include "CBaseViewport.h"
#include "cl_dll.h"
#include "cl_util.h"

static void __CmdFunc_OpenGameUITestPanel()
{
	CGameUITestPanel *panel = dynamic_cast<CGameUITestPanel *>(g_pViewport->FindGameUIPanelByName("GameUITestPanel"));
	if (!panel)
	{
		gEngfuncs.Con_Printf("__CmdFunc_OpenGameUITestPanel: panel is NULL\n");
		return;
	}

	panel->Activate();
}

CGameUITestPanel::CGameUITestPanel(vgui2::VPANEL parent) : BaseClass(nullptr, "GameUITestPanel")
{
	BaseClass::SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(true);
	SetSizeable(false);
	SetMoveable(true);
	SetVisible(false);
	SetTitle(L"Quote of the day", false);

	m_pRichText = new vgui2::RichText(this, "QuoteBox");
	m_pRichText->SetVerticalScrollbar(false);

	SetScheme("ClientScheme");
	Reset();
	HOOK_COMMAND("gameui_open_test_panel", OpenGameUITestPanel);
}

CGameUITestPanel::~CGameUITestPanel() {}

void CGameUITestPanel::Reset()
{
	constexpr int WIDE = 400, TALL = 200, GAP = 6;
	SetSize(WIDE, TALL);
	MoveToCenterOfScreen();
	m_pRichText->SetSize(WIDE - GAP - GAP, TALL - GAP - GAP - GetCaptionHeight());
	m_pRichText->SetPos(GAP, GetCaptionHeight() + GAP);

	const wchar_t *quotes[] = {
		L"\"DO YOU **** *****?\" - Gunnery Sergeant Hartman, your senior drill instructor",
		L"\"Freeman, you fool!\" - that scientist from the teleport chamber",
		L"\"Did you submit your status report to the administrator, today?\" - Did you submit your status report to the administrator, today?"
	};

	int idx = rand() % ARRAYSIZE(quotes);
	m_pRichText->SetText(quotes[idx]);
}

void CGameUITestPanel::OnCommand(const char* command)
{
	if (!strcmp(command, "Close"))
	{
		m_bIsOpen = false;
		BaseClass::OnCommand(command);
	}
	else BaseClass::OnCommand(command);
}

void CGameUITestPanel::Activate()
{
	if (!m_bIsOpen)
	{
		m_bIsOpen = true;
		Reset();
	}
	BaseClass::Activate();
}

const char *CGameUITestPanel::GetName()
{
	return "GameUITestPanel";
}

void CGameUITestPanel::ShowPanel(bool state)
{
	if (BaseClass::IsVisible() == state)
		return;

	if (state)
	{
		BaseClass::Activate();
	}
	else
	{
		BaseClass::SetVisible(false);
	}
}

void CGameUITestPanel::OnGameUIActivated()
{
	if (m_bIsOpen)
		ShowPanel(true);
}

void CGameUITestPanel::OnGameUIDeactivated()
{
	if (m_bIsOpen)
		ShowPanel(false);
}

vgui2::VPANEL CGameUITestPanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CGameUITestPanel::IsVisible()
{
	return BaseClass::IsVisible();
}
