#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "EngineInterface.h"

#include <tier0/dbg.h>

#include "interface.h"
#include "vgui_controls/controls.h"
#include "vgui_controls/MessageBox.h"

#include "KeyValues.h"
#include <IEngineVGui.h>
#include <IGameUIFuncs.h>
#include <IBaseUI.h>
#include <ServerBrowser/IServerBrowser.h>
#include <IVGuiModule.h>

#include <IEngineVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>


#include "BasePanel.h"
#include "ModInfo.h"
#include "GameConsole.h"
#include "LoadingDialog.h"
#include "GameLoading.h"

#include "GameUI_Interface.h"

IServerBrowser *serverbrowser = NULL;
static CBasePanel *staticPanel = NULL;

static CGameUI g_GameUI;
CGameUI *g_pGameUI = NULL;

static IEngineVGui* enginevguifuncs = nullptr;
IGameUIFuncs* gameuifuncs = nullptr;
IBaseUI* baseuifuncs = nullptr;

namespace vgui2 {
extern cl_enginefunc_t gEngfuncs;
}
using namespace vgui2;
cl_enginefunc_t* engine = &gEngfuncs;

vgui2::DHANDLE<CLoadingDialog> g_hLoadingDialog;
vgui2::DHANDLE<CGameLoading>g_hGameLoading;

CGameUI &GameUI(void)
{
	return g_GameUI;
}

vgui2::Panel *StaticPanel(void)
{
	return staticPanel;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameUI, IGameUI, GAMEUI_INTERFACE_VERSION, g_GameUI);

CGameUI::CGameUI(void)
{
	g_pGameUI = this;
	m_szPreviousStatusText[0] = 0;
	m_bLoadlingLevel = false;
}

CGameUI::~CGameUI(void)
{
	g_pGameUI = NULL;
}

void CGameUI::Initialize(CreateInterfaceFn *factories, int count)
{
	auto pEngFactory = factories[0];
	CreateInterfaceFn pClientFactory = nullptr;

	if (count > 5)
		pClientFactory = factories[5];

	m_FactoryList[0] = Sys_GetFactoryThis();
	m_FactoryList[1] = pEngFactory;
	m_FactoryList[2] = factories[1];
	m_FactoryList[3] = factories[2];
	m_FactoryList[4] = pClientFactory;

	m_iNumFactories = count;
	
	if (!vgui2::VGuiControls_Init("GameUI", factories, count))
		return;

    vgui2::surface()->SetLanguage("schinese");
	vgui2::localize()->AddFile(vgui2::filesystem(), "Resource/gameui_%language%.txt");
	vgui2::localize()->AddFile(vgui2::filesystem(), "Resource/valve_%language%.txt");
	vgui2::localize()->AddFile(vgui2::filesystem(), "Resource/vgui_%language%.txt");
	vgui2::localize()->AddFile(vgui2::filesystem(), "Resource/cstrike_%language%.txt");

	staticPanel = new CBasePanel();
	staticPanel->SetBounds(0, 0, 640, 480);
	staticPanel->SetPaintBorderEnabled(false);
	staticPanel->SetPaintBackgroundEnabled(true);
	staticPanel->SetPaintEnabled(false);
	staticPanel->SetVisible(true);
	staticPanel->SetMouseInputEnabled(false);
	staticPanel->SetKeyBoardInputEnabled(false);

	gameuifuncs = static_cast<IGameUIFuncs*>(pEngFactory(ENGINE_GAMEUIFUNCS_INTERFACE_VERSION, nullptr));
	enginevguifuncs = static_cast<IEngineVGui*>(pEngFactory(VENGINE_VGUI_VERSION, nullptr));
	
	staticPanel->SetParent(enginevguifuncs->GetPanel(PANEL_GAMEUIDLL));
	
	baseuifuncs = static_cast<IBaseUI*>(pEngFactory(BASEUI_INTERFACE_VERSION, nullptr));

	/*
	HINTERFACEMODULE hServerBrowser = Sys_LoadModule("platform\\servers\\serverbrowser.dll");
	CreateInterfaceFn pfnServerBrowserFactory = Sys_GetFactory(hServerBrowser);
	
	serverbrowser = (IServerBrowser *)pfnServerBrowserFactory(SERVERBROWSER_INTERFACE_VERSION, NULL);
	serverbrowserModule = (IVGuiModule *)pfnServerBrowserFactory("VGuiModuleServerBrowser001", NULL);
	
	if (serverbrowserModule)
	{
		serverbrowserModule->Initialize(factories, count);
		serverbrowserModule->SetParent(staticPanel->GetVPanel());

		serverbrowserModule->PostInitialize(factories, count);
	}*/

	/*
	serverbrowser = (IServerBrowser *)CreateInterface(SERVERBROWSER_INTERFACE_VERSION, NULL);
	//serverbrowser = &ServerBrowser();

	if (serverbrowser)
		serverbrowser->Initialize(factories, count);

	if (serverbrowser)
		serverbrowser->SetParent(staticPanel->GetVPanel());
		*/
	
	vgui2::surface()->SetAllowHTMLJavaScript(true);
}

void CGameUI::Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion, void *system)
{
	//memcpy(&gEngfuncs, engineFuncs, sizeof(gEngfuncs));
	engine = &gEngfuncs;

	ModInfo().LoadCurrentGameInfo();
	
	/*
	if (serverbrowser)
	{
		serverbrowser->ActiveGameName(ModInfo().GetGameDescription(), engine->pfnGetGameDirectory());
		serverbrowser->Reactivate();
	}
	*/

	// start mp3 playing
	engine->pfnClientCmd("mp3 loop media/gamestartup.mp3\n");
	//engine->pfnClientCmd("fmod loop media/gamestartup.mp3\n");
}

void CGameUI::Shutdown(void)
{
	/*if (serverbrowserModule)
	{
	//	serverbrowser->Deactivate();
	//	serverbrowser->Shutdown();
		serverbrowserModule->Shutdown();
	}*/
	/*
	if (serverbrowser)
	{
		serverbrowser->Deactivate();
		serverbrowser->Shutdown();
	}
	*/
	ModInfo().FreeModInfo();
}

int CGameUI::ActivateGameUI(void)
{
	if (!m_bLoadlingLevel && g_hLoadingDialog.Get() && IsInLevel())
	{
		g_hLoadingDialog->Close();
		g_hLoadingDialog = NULL;
	}

	if (!m_bLoadlingLevel)
	{
		if (IsGameUIActive())
			return 1;
	}

	staticPanel->OnGameUIActivated();
	staticPanel->SetVisible(true);

	engine->pfnClientCmd("setpause");

	return 1;
}

int CGameUI::ActivateDemoUI(void)
{
	return 1;
}

int CGameUI::HasExclusiveInput(void)
{
	return IsGameUIActive();
}

void CGameUI::RunFrame(void)
{
	int wide, tall;
	vgui2::surface()->GetScreenSize(wide, tall);
	staticPanel->SetSize(wide, tall);

	if (staticPanel->IsVisible())
        staticPanel->RunFrame();
}

void CGameUI::ConnectToServer(const char *game, int IP, int port)
{
	/*
	if (serverbrowser)
		serverbrowser->ConnectToGame(IP, port);
		*/

	engine->pfnClientCmd("mp3 stop\n");
	//engine->pfnClientCmd("fmod stop\n");

	baseuifuncs->HideGameUI();
	/*
	KeyValues *pKV = new KeyValues("ConnectedToGame");
	pKV->SetInt("ip", IP);
	pKV->SetInt("port", port);
	pKV->SetString("gamedir", game);

	vgui2::ivgui()->PostMessageA(serverbrowserModule->GetPanel(), pKV, staticPanel->GetVPanel());*/
}

void CGameUI::DisconnectFromServer(void)
{
	/*
	if (serverbrowser)
		serverbrowser->DisconnectFromGame();
		*/

	baseuifuncs->ActivateGameUI();
}

void CGameUI::HideGameUI(void)
{
	if (!IsGameUIActive())
		return;

	if (!IsInLevel())
		return;

	staticPanel->SetVisible(false);

	engine->pfnClientCmd("unpause");
	engine->pfnClientCmd("hideconsole");

	if (GameConsole().IsConsoleVisible())
		GameConsole().Hide();

	if (!m_bLoadlingLevel && g_hLoadingDialog.Get())
	{
		g_hLoadingDialog->Close();
		g_hLoadingDialog = NULL;
	}
}

bool CGameUI::IsGameUIActive(void)
{
	return staticPanel->IsVisible();
}

void CGameUI::LoadingStarted(const char *resourceType, const char *resourceName)
{
	m_bLoadlingLevel = true;

	/*if (!g_hGameLoading.Get())
	{
		VPANEL GameUIPanel = enginevguifuncs->GetPanel(PANEL_GAMEUIDLL);
		g_hGameLoading = new CGameLoading(NULL, "GameLoading");
		g_hGameLoading->SetParent(GameUIPanel);
	}
	g_hGameLoading->Activate();*/

	engine->pfnClientCmd("unpause");
	engine->pfnClientCmd("hideconsole");
	GameConsole().Hide();

	staticPanel->OnLevelLoadingStarted(resourceName);
}

void CGameUI::LoadingFinished(const char *resourceType, const char *resourceName)
{
	m_bLoadlingLevel = false;

	if (g_hGameLoading.Get())
	{
		g_hGameLoading->SetVisible(false);
		g_hGameLoading->SetAutoDelete(true);
	}

	staticPanel->OnLevelLoadingFinished();
	baseuifuncs->HideGameUI();
}

void CGameUI::StartProgressBar(const char *progressType, int progressSteps)
{
	if (g_hGameLoading.Get())
	{
		if (g_hGameLoading->IsVisible())
		{
			g_hGameLoading->SetProgressRange(0, progressSteps);
			g_hGameLoading->SetProgressPoint(0.0f);

			if (!stricmp(progressType, "Server"))
			{
				g_hGameLoading->SetProgressVisible(false);
			}
			else if (!stricmp(progressType, "Connecting"))
			{
				g_hGameLoading->SetProgressVisible(true);
			}

			return;
		}
	}

	if (!g_hLoadingDialog.Get())
		g_hLoadingDialog = new CLoadingDialog(staticPanel);

	m_szPreviousStatusText[0] = 0;
	g_hLoadingDialog->SetProgressRange(0, progressSteps);
	g_hLoadingDialog->SetProgressPoint(0.0f);
	g_hLoadingDialog->Open();
}

int CGameUI::ContinueProgressBar(int progressPoint, float progressFraction)
{
	if (g_hGameLoading.Get())
	{
		if (g_hGameLoading->IsVisible())
		{
			return g_hGameLoading->SetProgressPoint(progressPoint);
		}
	}

	if (!g_hLoadingDialog.Get())
	{
		g_hLoadingDialog = new CLoadingDialog(staticPanel);
		g_hLoadingDialog->SetProgressRange(0, 24);
		g_hLoadingDialog->SetProgressPoint(0.0f);
		g_hLoadingDialog->Open();
	}

	g_hLoadingDialog->Activate();
	return g_hLoadingDialog->SetProgressPoint(progressPoint);
}

void CGameUI::StopProgressBar(bool bError, const char *failureReason, const char *extendedReason)
{
	if (g_hGameLoading.Get())
	{
		if (g_hGameLoading->IsVisible())
		{
			return;
		}
	}

	if (!g_hLoadingDialog.Get() && bError)
		g_hLoadingDialog = new CLoadingDialog(staticPanel);

	if (!g_hLoadingDialog.Get())
		return;

	if (bError)
	{
		g_hLoadingDialog->DisplayGenericError(failureReason, extendedReason);
	}
	else
	{
		g_hLoadingDialog->Close();
		g_hLoadingDialog = NULL;
	}
}

int CGameUI::SetProgressBarStatusText(const char *statusText)
{
	if (g_hGameLoading.Get())
	{
		if (g_hGameLoading->IsVisible())
		{
			g_hGameLoading->SetStatusText(statusText);
			return false;
		}
	}

	if (!g_hLoadingDialog.Get())
		return false;

	if (!statusText)
		return false;

	if (!stricmp(statusText, m_szPreviousStatusText))
		return false;

	g_hLoadingDialog->SetStatusText(statusText);
	Q_strncpy(m_szPreviousStatusText, statusText, sizeof(m_szPreviousStatusText));
	return true;
}

void CGameUI::SetSecondaryProgressBar(float progress)
{
	if (g_hGameLoading.Get())
	{
		if (g_hGameLoading->IsVisible())
			return;
	}

	if (!g_hLoadingDialog.Get())
		return;

	g_hLoadingDialog->SetSecondaryProgress(progress);
}

void CGameUI::SetSecondaryProgressBarText(const char *statusText)
{
	if (g_hLoadingDialog.Get())
	{
		if (g_hLoadingDialog->IsVisible())
			return;
	}

	if (!g_hLoadingDialog.Get())
		return;

	g_hLoadingDialog->SetSecondaryProgressText(statusText);
}

void CGameUI::ValidateCDKey(bool force, bool inConnect)
{
	
}

bool CGameUI::IsServerBrowserValid(void)
{
	return serverbrowser != NULL;
}

void CGameUI::ActivateServerBrowser(void)
{
	/*if (serverbrowser)
		serverbrowser->Activate();*/

	engine->pfnClientCmd("citrus_servers");
}

bool CGameUI::IsInLevel(void)
{
	const char *levelName = engine->pfnGetLevelName();

	if (strlen(levelName) > 0)
		return true;

	return false;
}

bool CGameUI::IsInMultiplayer(void)
{
	return (IsInLevel() && engine->GetMaxClients() > 1);
}