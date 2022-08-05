#include "hud.h"
#include "cstriketeammenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <cl_entity.h>
#include <cdll_dll.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

#include "CBaseViewport.h"
#include "shared_util.h"
#include "igbutton.h"

using namespace vgui2;

using cl::gEngfuncs;
using cl::g_PlayerExtraInfo;
using cl::gHUD;
using cl::g_iTeamNumber;

CCSTeamMenu::CCSTeamMenu(IViewport* pViewPort) : CTeamMenu(pViewPort)
{

}

void CCSTeamMenu::SetupControlSettings()
{
	LoadControlSettings("Resource/UI/CSO_TeamMenu.res", "GAME");
	InvalidateLayout();
}

CCSTeamMenu::~CCSTeamMenu(void)
{
}

void CCSTeamMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);
}

void CCSTeamMenu::Update(void)
{
	BaseClass::Update();

	if (g_pViewport->GetAllowSpectators())
	{
		if (g_iTeamNumber == TEAM_UNASSIGNED || (g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].dead))
			SetVisibleButton("specbutton", true);
		else
			SetVisibleButton("specbutton", false);
	}
	else
		SetVisibleButton("specbutton", false);

	m_bVIPMap = false;

	char mapName[32];
	Q_FileBase(gEngfuncs.pfnGetLevelName(), mapName, sizeof(mapName));

	if (!Q_strncmp(mapName, "maps/as_", 8))
		m_bVIPMap = true;

	if (!m_bVIPMap || (g_iTeamNumber != TEAM_CT))
		SetVisibleButton("vipbutton", false);
	else
		SetVisibleButton("vipbutton", true);

	if (g_iTeamNumber == TEAM_UNASSIGNED)
		SetVisibleButton("CancelButton", false);
	else
		SetVisibleButton("CancelButton", true);
}

void CCSTeamMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *pAutoButton = FindChildByName("autobutton");

		if (pAutoButton)
			pAutoButton->RequestFocus();
	}
}

void CCSTeamMenu::OnCommand(const char *command)
{
	if (Q_stricmp(command, "vguicancel"))
	{
		cl::gEngfuncs.pfnClientCmd(const_cast<char *>(command));
	}

	Close();

	m_pViewPort->ShowBackGround(false);

	BaseClass::OnCommand(command);
}

void CCSTeamMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetVisible(state);
}

void CCSTeamMenu::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSTeamMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CCSTeamMenu::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

vgui2::Panel* CCSTeamMenu::CreateControlByName(const char* controlName)
{
	if (!Q_stricmp("IGButton", controlName))
	{
		return new IGButton(this, controlName);
	}
	else
		return BaseClass::CreateControlByName(controlName);
}

void CCSTeamMenu::UpdateGameMode()
{
	SetupControlSettings();
}