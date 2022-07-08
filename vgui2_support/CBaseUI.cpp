#include <vector>
#include <string.h>

#include "vgui_api.h"
#include "cdll_int.h"
#include "menu_int.h"

#include "entity_state.h"
#include "usercmd.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "cdll_exp.h"

#include "CBaseUI.h"
#include <FileSystem.h>
#include "vgui/IVGui.h"
#include "vgui/IPanel.h"
#include "vgui/ILocalize.h"
#include "vgui/IScheme.h"
#include "vgui/ISystem.h"
#include "IClientVGUI.h"
#include "vgui/IInputInternal.h"
#include "vgui_controls/controls.h"
#include "vgui_controls/Panel.h"
#include "BaseUISurface.h"
#include "keydefs.h"
#include "keydest.h"

#include "GameUI/IGameUI.h"
#include "GameUI/IGameConsole.h"
#include "GameUI/ICareerUI.h"

namespace ui
{
	extern ui_enginefuncs_t engfuncs;
#ifndef XASH_DISABLE_FWGS_EXTENSIONS
	extern ui_textfuncs_t	textfuncs;
#endif
}

class CStaticPanel;
class CEnginePanel;

namespace vgui2 {
extern cl_enginefunc_t gEngfuncs;
IClientVGUI *staticClient;
IGameUI* staticGameUIFuncs;
IGameConsole* staticGameConsole;
ICareerUI* staticCareerUI;
vgui2::IInputInternal *g_pInputInternal;

CStaticPanel *staticPanel;
CEnginePanel *staticClientDLLPanel;
CEnginePanel *staticGameUIPanel;
}
BaseUISurface *staticSurface;
using namespace vgui2;

class CEnginePanel : public vgui2::Panel {
public:
	CEnginePanel() : Panel() {
		m_bCanFocus = true;
	}

	CEnginePanel(Panel* parent, const char* panelName) : Panel(parent, panelName)
	{
		m_bCanFocus = true;
	}

	vgui2::VPANEL IsWithinTraverse(int x, int y, bool traversePopups) {
		if (!m_bCanFocus) {
			return 0;
		}

		vgui2::VPANEL result = vgui2::Panel::IsWithinTraverse(x, y, traversePopups);

		if (result == GetVPanel()) {
			return 0;
		}

		return result;
	}

private:
	bool m_bCanFocus;
};

class CStaticPanel : public vgui2::Panel {
public:
	vgui2::VPANEL GetCurrentKeyFocus() {
		return staticClientDLLPanel->GetCurrentKeyFocus();
	}

	vgui2::VPANEL IsWithinTraverse(int x, int y, bool traversePopups) {
		vgui2::VPANEL result = vgui2::Panel::IsWithinTraverse(x, y, traversePopups);

		if (result == GetVPanel()) {
			return 0;
		}

		return result;
	}
};

void CBaseUI::Initialize(CreateInterfaceFn* factories, int count) {
	if (staticClient) {
		return;
	}

#ifndef XASH_STATIC_GAMELIB
	m_hFileSystemModule = Sys_LoadModule("FileSystem_stdio.dll");
	m_hVGuiModule = Sys_LoadModule("vgui2.dll");
	m_hChromeModule = Sys_LoadModule("chromehtml.dll");
	m_FactoryList[0] = factories[0];
	m_FactoryList[1] = Sys_GetFactory(m_hVGuiModule);
	m_FactoryList[2] = Sys_GetFactory(m_hFileSystemModule);
	m_FactoryList[3] = Sys_GetFactory(m_hChromeModule);
	m_iNumFactories = 4;
#else
    m_FactoryList[0] = factories[0];
    m_FactoryList[1] = Sys_GetFactoryThis();
    m_FactoryList[2] = Sys_GetFactoryThis();
    m_FactoryList[3] = Sys_GetFactoryThis();
    m_iNumFactories = 4;
#endif
	
	vgui2::VGuiControls_Init("BaseUI", m_FactoryList, m_iNumFactories);

#ifdef XASH_STATIC_GAMELIB
    auto gameUIFactory = Sys_GetFactoryThis();
    m_FactoryList[m_iNumFactories] = gameUIFactory;
#else
	if(!(m_hStaticGameUIModule = Sys_LoadModule("GameUI.dll")))
	{
		char szGameUIDLLPath[_MAX_PATH];
		g_pFullFileSystem->GetLocalPath("cl_dlls/GameUI.dll", szGameUIDLLPath, sizeof(szGameUIDLLPath));
		m_hStaticGameUIModule = Sys_LoadModule(szGameUIDLLPath);
	}
	auto gameUIFactory = Sys_GetFactory(m_hStaticGameUIModule);
	m_FactoryList[m_iNumFactories] = gameUIFactory;
#endif

	if (gameUIFactory)
	{
		staticGameUIFuncs = static_cast<IGameUI*>(gameUIFactory(GAMEUI_INTERFACE_VERSION, nullptr));
		staticGameConsole = static_cast<IGameConsole*>(gameUIFactory(GAMECONSOLE_INTERFACE_VERSION, nullptr));
		staticCareerUI = static_cast<ICareerUI*>(gameUIFactory(CAREERUI_INTERFACE_VERSION, nullptr));

		++m_iNumFactories;
	}
	
	g_pInputInternal = (vgui2::IInputInternal *)m_FactoryList[1](VGUI_INPUTINTERNAL_INTERFACE_VERSION, NULL);

	//char szClientDLLPath[_MAX_PATH];
	//g_pFullFileSystem->GetLocalPath("cl_dlls/client.dll", szClientDLLPath, sizeof(szClientDLLPath));
	//m_hClientModule = Sys_LoadModule(szClientDLLPath);
	m_FactoryList[4] = Sys_GetFactoryThis(); // Sys_GetFactory(m_hClientModule);
	m_iNumFactories = 5;
	staticClient = (IClientVGUI *)m_FactoryList[4](CLIENTVGUI_INTERFACE_VERSION, NULL);
	//gClDllFuncs.pfnInitialize(&gEngfuncs, CLDLL_INTERFACE_VERSION);
}

void CBaseUI::Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion) {
	
	staticPanel = new CStaticPanel();
	staticPanel->SetCursor(vgui2::dc_none);

	{
		const auto color = staticPanel->GetBgColor();

		//Set alpha to maximum.
		staticPanel->SetBgColor(Color(color.r(), color.g(), color.b(), 0xFF));
	}

	//staticPanel->SetBounds(0, 0, 40, 30);

	staticPanel->SetPaintBorderEnabled(false);
	staticPanel->SetPaintBackgroundEnabled(false);
	staticPanel->SetPaintEnabled(false);

	staticPanel->SetCursor(vgui2::dc_none);
	staticPanel->SetVisible(true);
	staticPanel->SetZPos(0);

	staticSurface = (BaseUISurface *)m_FactoryList[0](VGUI_SURFACE_INTERFACE_VERSION, NULL);
	IHTMLChromeController *chromeController = nullptr;
	if(m_FactoryList[3])
		chromeController = (IHTMLChromeController *)m_FactoryList[3](HTML_CHROME_CONTROLLER_INTERFACE_VERSION, NULL);
	staticSurface->Init(staticPanel->GetVPanel(), chromeController);
	staticSurface->SetLanguage("schinese");
	staticSurface->IgnoreMouseVisibility(true);

	vgui2::scheme()->LoadSchemeFromFile("resource/TrackerScheme.res", "BaseUI");
	vgui2::localize()->AddFile(vgui2::filesystem(), "resource/tracker_%language%.txt");
	vgui2::localize()->AddFile(vgui2::filesystem(), "resource/valve_%language%.txt");

	const char *szGameDir = gEngfuncs.pfnGetGameDirectory();

	if (strcmp(szGameDir, "valve")) {
		char szModLocalizeFile[_MAX_PATH];
		snprintf(szModLocalizeFile, _MAX_PATH, "resource/%s_%%language%%.txt", szGameDir);
	}

	vgui2::ivgui()->Start();
	vgui2::ivgui()->SetSleep(false);

	staticClientDLLPanel = new CEnginePanel(staticPanel, "BaseClientPanel");

	{
		Color color = staticClientDLLPanel->GetBgColor();

		//Set alpha to maximum.
		staticClientDLLPanel->SetBgColor(Color(color.r(), color.g(), color.b(), 0xFF));
	}

	staticClientDLLPanel->SetPaintBorderEnabled(false);
	staticClientDLLPanel->SetPaintBackgroundEnabled(false);
	staticClientDLLPanel->SetPaintEnabled(false);
	
	staticClientDLLPanel->SetVisible(true);
	staticClientDLLPanel->SetCursor(vgui2::dc_none);

	//Draw above static panel.
	staticClientDLLPanel->SetZPos(25);

	staticGameUIPanel = new CEnginePanel(staticPanel, "BaseGameUIPanel");

	{
		Color color = staticGameUIPanel->GetBgColor();

		//Set alpha to maximum.
		staticGameUIPanel->SetBgColor(Color(color.r(), color.g(), color.b(), 0xFF));
	}

	staticGameUIPanel->SetPaintBorderEnabled(false);
	staticGameUIPanel->SetPaintBackgroundEnabled(false);
	staticGameUIPanel->SetPaintEnabled(false);

	staticGameUIPanel->SetVisible(true);
	staticGameUIPanel->SetCursor(vgui2::dc_none);

	//Draw above static and client panels.
	staticGameUIPanel->SetZPos(50);
	
	if (staticGameUIFuncs)
	{
		staticGameUIFuncs->Initialize(m_FactoryList, m_iNumFactories);
	}

	if (staticClient) {
		staticClient->Initialize(m_FactoryList, m_iNumFactories);
		staticSurface->SetVGUI2MouseControl(true);
	}

	if (staticGameUIFuncs)
	{
		void* system = nullptr;
		staticGameUIFuncs->Start(&gEngfuncs, CLDLL_INTERFACE_VERSION, system);
	}

	staticClientDLLPanel->SetScheme("ClientScheme");

	if (staticClient) {
		staticClient->Start();
		staticClient->SetParent(staticClientDLLPanel->GetVPanel());
	}

	int wide, tall;
	staticSurface->GetScreenSize(wide, tall);
	
	staticPanel->SetBounds(0, 0, wide, tall);
	staticGameUIPanel->SetBounds(0, 0, wide, tall);
	staticClientDLLPanel->SetBounds(0, 0, wide, tall);
	
	if (staticGameConsole)
	{
		staticGameConsole->Initialize();
		staticGameConsole->SetParent(staticGameUIPanel->GetVPanel());
	}

	staticSurface->IgnoreMouseVisibility(false);

	if (staticGameUIFuncs)
	{
		staticGameUIFuncs->ActivateGameUI();
	}
}

void CBaseUI::Shutdown() {
	//vgui2::ivgui()->RunFrame();
	vgui2::ivgui()->Shutdown();

	if (staticGameUIFuncs) {
		staticGameUIFuncs->Shutdown();
	}
	
	if (staticClient) {
		staticClient->Shutdown();
	}

#ifndef XASH_STATIC_GAMELIB
	Sys_UnloadModule(m_hStaticGameUIModule);
	m_hStaticGameUIModule = nullptr;
#endif

	staticGameUIFuncs = nullptr;
	staticGameConsole = nullptr;
	staticCareerUI = nullptr;

	vgui2::system()->SaveUserConfigFile();
#ifndef XASH_STATIC_GAMELIB
	Sys_UnloadModule(m_hClientModule);
	m_hClientModule = NULL;
	staticSurface->Shutdown();
	Sys_UnloadModule(m_hChromeModule);
	m_hChromeModule = NULL;
	Sys_UnloadModule(m_hVGuiModule);
	m_hVGuiModule = NULL;
	Sys_UnloadModule(m_hFileSystemModule);
	m_hFileSystemModule = NULL;
#endif
}

int CBaseUI::Key_Event(int down, int keynum, const char* pszCurrentBinding) {
	if (keynum == '`' || keynum == '~')
	{
		// toggle console in keys.cpp -> Key_Event Con_ToggleConsole_f
	}
	else
	{
		if (keynum == K_ESCAPE && down)
		{
			const char* pszLevelName = gEngfuncs.pfnGetLevelName();
			if (pszLevelName && *pszLevelName)
			{
				if (staticGameUIFuncs->IsGameUIActive())
				{
					HideGameUI();
					return false;
				}
				ActivateGameUI();
				HideConsole();
				return true;
			}
		}
		else if (m_bHidingGameUI && keynum == K_MOUSE1 && down)
		{
			m_bHidingGameUI = false;
			return false;
		}
		else
		{
			return vgui2::surface()->NeedKBInput();
		}
	}

	return false;
}

void CBaseUI::CallEngineSurfaceProc(void* hwnd, unsigned int msg, unsigned int wparam, long lparam) {
}

void CBaseUI::Paint(int x, int y, int right, int bottom) {
	if (!staticSurface || !staticSurface->GetEmbeddedPanel()) {
		return;
	}

    staticGameUIFuncs->RunFrame();
	vgui2::ivgui()->RunFrame();
	staticSurface->SetScreenBounds(x, y, right - x, bottom - y);
	staticPanel->SetBounds(0, 0, right, bottom);
	staticGameUIPanel->SetBounds(0, 0, right, bottom);
	staticClientDLLPanel->SetBounds(0, 0, right, bottom);
	//staticPanel->PerformApplySchemeSettings();
	//staticPanel->InvalidateLayout(false, true);
	static_cast<vgui2::IClientPanel*>( staticPanel )->Think();
	vgui2::surface()->PaintTraverse(staticSurface->GetEmbeddedPanel());
}

void CBaseUI::HideGameUI() {
	ui::engfuncs.pfnSetKeyDest(key_game);
	
	staticGameUIFuncs->HideGameUI();
	staticGameConsole->Hide();

	const char* pszLevelName = gEngfuncs.pfnGetLevelName();

	if (pszLevelName && *pszLevelName)
	{
		staticGameUIPanel->SetVisible(false);
		staticGameUIPanel->SetPaintBackgroundEnabled(false);

		staticClientDLLPanel->SetVisible(true);
		staticClientDLLPanel->SetMouseInputEnabled(true);

		if (staticClient)
			staticClient->ActivateClientUI();
	}

	if (vgui2::input()->IsMouseDown(vgui2::MOUSE_LEFT))
		m_bHidingGameUI = true;
}

void CBaseUI::ActivateGameUI() {
	ui::engfuncs.pfnSetKeyDest(key_menu);
	
	staticGameUIFuncs->ActivateGameUI();
	staticGameUIPanel->SetVisible(true);

	staticClientDLLPanel->SetVisible(false);
	staticClientDLLPanel->SetMouseInputEnabled(false);

	if (staticClient)
	{
		staticClient->HideClientUI();
		staticClient->HideScoreBoard();
	}
}

void CBaseUI::HideConsole() {
	staticGameConsole->Hide();
}

void CBaseUI::ShowConsole() {
	staticGameConsole->Activate();
	ui::engfuncs.pfnSetKeyDest(key_console);
}

bool CBaseUI::IsGameUIVisible() {
	return staticGameUIPanel->IsVisible();
}

vgui2::VPANEL CEngineVGui::GetPanel(VGUIPANEL type)
{
	switch (type)
	{
	default:
	case PANEL_ROOT:		return staticPanel->GetVPanel();
	case PANEL_CLIENTDLL:	return staticClientDLLPanel->GetVPanel();
	case PANEL_GAMEUIDLL:	return staticGameUIPanel->GetVPanel();
	}
	return NULL;
}

bool CEngineVGui::SteamRefreshLogin(const char* password, bool isSecure)
{
	return true;
}

bool CEngineVGui::SteamProcessCall(bool* finished, TSteamProgress* progress, TSteamError* steamError)
{
	return true;
}

void CEngineVGui::SetEngineVisible(bool state)
{
	m_bVisible = state;
#if 0
	if (!g_bIsDedicatedServer)
	{
		if (state)
			ClientDLL_ActivateMouse();
		else
			ClientDLL_DeactivateMouse();
	}
#endif
}

EXPOSE_SINGLE_INTERFACE(CBaseUI, IBaseUI, BASEUI_INTERFACE_VERSION);
EXPOSE_SINGLE_INTERFACE(CEngineVGui, IEngineVGui, VENGINE_VGUI_VERSION);