
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <IEngineVgui.h>

#include "CHudViewport.h"
#include "CGameUITestPanel.h"
#include "CClientMOTD.h"

#include "hud.h"
#include "parsemsg.h"

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

	static CHudViewport * const s_pHudViewPort = this;

	gEngfuncs.pfnHookUserMsg("VGUIMenu", [](const char *pszName, int iSize, void *pbuf) { return s_pHudViewPort->MsgFunc_MOTD(pszName, iSize, pbuf); });

	gEngfuncs.pfnAddCommand("motd_open", []() { s_pHudViewPort->m_pMOTD->Activate(gHUD.m_szServerName, "wow"); });
}

int CHudViewport::MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (m_bGotAllMOTD)
		m_szMOTD.clear();

	BufferReader buf(pszName, pbuf, iSize);

	m_bGotAllMOTD = buf.ReadByte();

	m_szMOTD += buf.ReadString();

	//CClientMOTD *panel = dynamic_cast<CClientMOTD *>(g_pViewport->FindPanelByName("ClientMOTD"));
	CClientMOTD *panel = m_pMOTD;
	if (panel)
	{
		panel->Activate(gHUD.m_szServerName, m_szMOTD.c_str());
	}
	else
		gEngfuncs.Con_Printf("MsgFunc_MOTD() : Error! CClientMOTD is nullptr\n");

	return 1;
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
	AddNewPanel(CreatePanelByName("ClientMOTD"));
	//AddNewPanel(CreatePanelByName(VIEWPORT_PANEL_SCORE));

	AddNewGameUIPanel(CreateGameUIPanelByName("GameUITestPanel"));
}

IViewportPanel* CHudViewport::CreatePanelByName(const char* pszName)
{
	IViewportPanel* pPanel = nullptr;
	
	if (Q_strcmp("ClientMOTD", pszName) == 0)
	{
		if(!m_pMOTD)
			m_pMOTD = new CClientMOTD(this);
		pPanel = m_pMOTD;
	}
	/*else if (Q_strcmp(VIEWPORT_PANEL_SCORE, pszName) == 0)
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
		//pPanel = new CGameUITestPanel(engineVgui()->GetPanel(PANEL_GAMEUIDLL));
		pPanel = new CGameUITestPanel(this->GetVPanel());
	}
	
	return pPanel;
}
