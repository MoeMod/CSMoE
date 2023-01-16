
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <IEngineVgui.h>

#include "CHudViewport.h"
#include "CGameUITestPanel.h"
#include "CClientMOTD.h"

#include "hud.h"
#include "parsemsg.h"
#include "csmoe/BuyMenu/cstrikebuymenu.h"
#include "csmoe/cstriketeammenu.h"
#include "csmoe/cstrikeclassmenu.h"
#include "csmoe/zb2/Zb2ZombieSelector.h"
#include "csmoe/zsh/zshelterteamhousingdlg.h"

using cl::gHUD;
using cl::gEngfuncs;

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
#if 0
	CClientMOTD *panel = m_pMOTD;
	if (panel)
	{
		panel->Activate(gHUD.m_szServerName, m_szMOTD.c_str());
	}
	else
		gEngfuncs.Con_Printf("MsgFunc_MOTD() : Error! CClientMOTD is nullptr\n");
#endif
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
    AddNewPanel(CreatePanelByName(PANEL_TEAM));
    AddNewPanel(CreatePanelByName(PANEL_CLASS));
    AddNewPanel(CreatePanelByName(PANEL_BUY));
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
    else if (Q_strcmp(PANEL_TEAM, pszName) == 0)
    {
        if (!m_pTeamMenu)
        {
            m_pTeamMenu = new CCSTeamMenu(this);
            m_pTeamMenu->UpdateGameMode();
        }
        pPanel = m_pTeamMenu;
    }
    else if (Q_strcmp(PANEL_CLASS, pszName) == 0)
    {
        if (!m_pClassMenu)
        {
            m_pClassMenu = new CCSClassMenu(this);
            m_pClassMenu->UpdateGameMode();
        }
        pPanel = m_pClassMenu;
    }
	else if (Q_strcmp(PANEL_BUY, pszName) == 0)
	{
		if(!m_pBuyMenu)
        {
            m_pBuyMenu = new CCSBaseBuyMenu(this);
            m_pBuyMenu->UpdateGameMode();
        }
		pPanel = m_pBuyMenu;
	}
    else if (Q_strcmp(CZSHELTERTEAMHOUSINGDLG_NAME, pszName) == 0)
    {
        pPanel = new ZShelterTeamHousingDialog();
    }
    else if (Q_strcmp(PANEL_ZOMBIEKEEPER, pszName) == 0)
    {
        if (!m_pZombieKeeperMenu)
        {
            m_pZombieKeeperMenu = new CZb2ZombieKeeper();
            m_pZombieKeeperMenu->SetupPage(0);
        }
        pPanel = m_pZombieKeeperMenu;

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
		pPanel = new CGameUITestPanel(engineVgui()->GetPanel(PANEL_GAMEUIDLL));
	}
	
	return pPanel;
}

bool CHudViewport::ShowVGUIMenu(int iMenu)
{
    IViewportPanel *panel = NULL;

    switch (iMenu)
    {
        case MENU_TEAM:
        {
            if (m_pClassMenu->CheckShowType())
            {
                panel = m_pClassMenu;
                m_pClassMenu->m_iCurrentPage = 0;
                m_pClassMenu->SetupPage(0);
            }
            else
            {
                panel = m_pTeamMenu;
            }
            break;
        }
        case MENU_CLASS_T:
        {
            panel = m_pClassMenu;

            if (m_pClassMenu->CheckShowType())
            {
                panel = m_pClassMenu;
                m_pClassMenu->m_iCurrentPage = 0;
                m_pClassMenu->SetupPage(0);
            }
            else
            {
                m_pClassMenu->m_iCurrentTeamPage = TERRORIST;
                m_pClassMenu->SetTeam(TERRORIST);
            }
            break;
        }
        case MENU_CLASS_CT:
        {
            panel = m_pClassMenu;
            if (m_pClassMenu->CheckShowType())
            {
                panel = m_pClassMenu;
                m_pClassMenu->m_iCurrentPage = 0;
                m_pClassMenu->SetupPage(0);
            }
            else
            {
                m_pClassMenu->m_iCurrentTeamPage = CT;
                m_pClassMenu->SetTeam(CT);
            }
            break;
        }
        case MENU_BUY:
        case MENU_BUY_PISTOL:
        case MENU_BUY_SHOTGUN:
        case MENU_BUY_RIFLE:
        case MENU_BUY_SUBMACHINEGUN:
        case MENU_BUY_MACHINEGUN:
        case MENU_BUY_ITEM:
        {
            if (cl::g_iTeamNumber == TEAM_CT)
                m_pBuyMenu->SetTeam(TEAM_CT);
            else if (cl::g_iTeamNumber == TEAM_TERRORIST)
                m_pBuyMenu->SetTeam(TEAM_TERRORIST);
            else
                m_pBuyMenu->SetTeam(TEAM_UNASSIGNED);

            m_pBuyMenu->ActivateMenu(iMenu);
            return true;
        }
        case MENU_MOBILE_ZOMBIEKEEPER:
        {
            panel = m_pZombieKeeperMenu;
            m_pZombieKeeperMenu->SetupPage(0);
            //cl::gHUD.m_flZombieSelectTime = cl::gHUD.m_flTime + 20.0;

            break;
        }
    }

    if (panel)
    {
        ShowPanel(panel, true);
        return true;
    }

    return false;
}

bool CHudViewport::HideVGUIMenu(int iMenu)
{
    IViewportPanel *panel = NULL;

    switch (iMenu)
    {
    case MENU_TEAM:
    {
        if (m_pClassMenu->CheckShowType())
        {
            panel = m_pClassMenu;
            m_pClassMenu->SetupPage(0);
        }
        else
        {
            panel = m_pTeamMenu;
        }
        break;
    }
        case MENU_CLASS_T:
        case MENU_CLASS_CT:
        {
            panel = m_pClassMenu;
            break;
        }

        case MENU_BUY:
        case MENU_BUY_PISTOL:
        case MENU_BUY_SHOTGUN:
        case MENU_BUY_RIFLE:
        case MENU_BUY_SUBMACHINEGUN:
        case MENU_BUY_MACHINEGUN:
        case MENU_BUY_ITEM:
        {
            panel = m_pBuyMenu;
        }
    }

    if (panel)
    {
        ShowPanel(panel, false);
        return true;
    }

    return false;
}

void CHudViewport::UpdateGameMode()
{
    if(m_pBuyMenu)
        m_pBuyMenu->UpdateGameMode();
    if (m_pTeamMenu)
        m_pTeamMenu->UpdateGameMode();
    if (m_pClassMenu)
        m_pClassMenu->UpdateGameMode();
}

bool CHudViewport::ShowVGUIMenuByName(const char* szName)
{
    auto pPanel = FindPanelByName(szName);
    if (pPanel == nullptr)
        pPanel = CreatePanelByName(szName);

    if (pPanel == nullptr)
        return false;

    ShowPanel(pPanel, true);
    return true;
}