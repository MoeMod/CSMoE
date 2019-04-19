#include <cassert>
#include <clocale>


#include <vgui/VGUI2.h>
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>

#include <vgui/ISurface.h>
#include <vgui/IEngineVgui.h>
#include "CHudViewport.h"

#include <vgui_controls/Controls.h>

#include "IGameUIFuncs.h"
#include "IBaseUI.h"

#include "KeyValuesCompat.h"

#include "CClientVGUI.h"

namespace
{
CClientVGUI g_ClientVGUI;

IGameUIFuncs* g_GameUIFuncs = nullptr;

IBaseUI* g_pBaseUI = nullptr;

IEngineVGui *g_EngineVgui = nullptr;
}

CClientVGUI* clientVGUI()
{
	return &g_ClientVGUI;
}

IGameUIFuncs* gameUIFuncs()
{
	return g_GameUIFuncs;
}

IBaseUI* baseUI()
{
	return g_pBaseUI;
}

IEngineVGui* engineVgui()
{
	return g_EngineVgui;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CClientVGUI, IClientVGUI, CLIENTVGUI_INTERFACE_VERSION, g_ClientVGUI );

CClientVGUI::CClientVGUI()
{
}

void CClientVGUI::Initialize( CreateInterfaceFn* pFactories, int iNumFactories )
{
	/*
	*	Factories in the given array:
	*	engine
	*	vgui2
	*	filesystem
	*	chrome HTML
	*	GameUI
	*	client (this library)
	*/

	//4 factories to use.
	assert( static_cast<size_t>( iNumFactories ) >= NUM_FACTORIES - 1 );

	m_FactoryList[ 0 ] = Sys_GetFactoryThis();

	for( size_t uiIndex = 0; uiIndex < NUM_FACTORIES - 1; ++uiIndex )
	{
		m_FactoryList[ uiIndex + 1 ] = pFactories[ uiIndex ];
	}

	if( !vgui2::VGuiControls_Init( "CLIENT", m_FactoryList, NUM_FACTORIES ) )
	{
		//Msg( "Failed to initialize VGUI2\n" );
		return;
	}

	if( !KV_InitKeyValuesSystem( m_FactoryList, NUM_FACTORIES ) )
	{
		//Msg( "Failed to initialize IKeyValues\n" );
		return;
	}

	g_GameUIFuncs = ( IGameUIFuncs* ) pFactories[ 0 ](ENGINE_GAMEUIFUNCS_INTERFACE_VERSION, nullptr );
	g_pBaseUI = ( IBaseUI* ) pFactories[ 0 ](BASEUI_INTERFACE_VERSION, nullptr );
	g_EngineVgui = (IEngineVGui* ) pFactories[ 0 ](VENGINE_VGUI_VERSION, nullptr );

	//Constructor sets itself as the viewport.

	new CHudViewport();

	g_pViewport->Initialize( pFactories, iNumFactories );
}

void CClientVGUI::Start()
{
	g_pViewport->Start();

#if 1
	vgui2::Frame* pFrame = new vgui2::Frame(g_pViewport, "TestFrame");
	pFrame->SetProportional(false);
	pFrame->SetTitleBarVisible(true);
	pFrame->SetMinimizeButtonVisible(false);
	pFrame->SetMaximizeButtonVisible(false);
	pFrame->SetCloseButtonVisible(true);
	pFrame->SetSizeable(true);
	pFrame->SetMoveable(true);
	pFrame->SetVisible(true);
	//pFrame->SetScheme(vgui2::scheme()->LoadSchemeFromFile("ui/resource/SourceScheme.res", "SourceScheme"));
	pFrame->SetSize(500, 500);
	pFrame->SetTitle("Test Frame", true);
	pFrame->Activate();
#endif

}

void CClientVGUI::SetParent( vgui2::VPANEL parent )
{
	m_vRootPanel = parent;

	g_pViewport->SetParent( parent );
}

bool CClientVGUI::UseVGUI1()
{
	return g_pViewport->UseVGUI1();
}

void CClientVGUI::HideScoreBoard()
{
	g_pViewport->HideScoreBoard();
}

void CClientVGUI::HideAllVGUIMenu()
{
	g_pViewport->HideAllVGUIMenu();
}

void CClientVGUI::ActivateClientUI()
{
	g_pViewport->ActivateClientUI();
}

void CClientVGUI::HideClientUI()
{
	g_pViewport->HideClientUI();
}

void CClientVGUI::Shutdown()
{
	g_pViewport->Shutdown();
}