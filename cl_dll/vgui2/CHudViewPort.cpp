
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <vgui/IEngineVgui.h>

#include "CHudViewport.h"
#include "CGameUITestPanel.h"

#include "hud.h"

void CHudViewport::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundEnabled(false);

	//extern vgui2::HFont g_HudTextVgui_TextFont;
	//g_HudTextVgui_TextFont = pScheme->GetFont("Default");
}

void CHudViewport::Start()
{
	BaseClass::Start();
}

void CHudViewport::HideScoreBoard()
{
	BaseClass::HideScoreBoard();
	gHUD.m_Scoreboard.UserCmd_HideScores();
}

void CHudViewport::ActivateClientUI()
{
	BaseClass::ActivateClientUI();
	if (gHUD.m_iIntermission)
		gHUD.m_Scoreboard.UserCmd_ShowScores();
}

void CHudViewport::HideClientUI()
{
	BaseClass::HideClientUI();
}

void CHudViewport::CreateDefaultPanels()
{
	//AddNewPanel(CreatePanelByName(VIEWPORT_PANEL_MOTD));
	//AddNewPanel(CreatePanelByName(VIEWPORT_PANEL_SCORE));

	AddNewGameUIPanel(CreateGameUIPanelByName("GameUITestPanel"));
}

IViewportPanel* CHudViewport::CreatePanelByName(const char* pszName)
{
	IViewportPanel* pPanel = nullptr;
	/*
	if (Q_strcmp(VIEWPORT_PANEL_MOTD, pszName) == 0)
	{
		pPanel = new CClientMOTD(this);
	}
	else if (Q_strcmp(VIEWPORT_PANEL_SCORE, pszName) == 0)
	{
		pPanel = new CScorePanel(this);
	}
	*/
	return pPanel;
}

IGameUIPanel *CHudViewport::CreateGameUIPanelByName(const char *pszName)
{
	IGameUIPanel *pPanel = nullptr;
	
	if (Q_strcmp("GameUITestPanel", pszName) == 0)
	{
		//pPanel = new CGameUITestPanel(engineVgui()->GetPanel(PANEL_ROOT));
		pPanel = new CGameUITestPanel(this->GetVPanel());
	}
	
	return pPanel;
}
