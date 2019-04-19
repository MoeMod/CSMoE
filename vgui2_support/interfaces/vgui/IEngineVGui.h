//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined( IENGINEVGUI_H )
#define IENGINEVGUI_H

#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include <vgui/VGUI2.h>

// Forward declarations.
namespace vgui2
{
	class Panel;
};

enum VGuiPanel_t
{
	PANEL_ROOT = 0,
	PANEL_GAMEUIDLL,
	PANEL_CLIENTDLL,
	PANEL_TOOLS,
	PANEL_INGAMESCREENS,
	PANEL_GAMEDLL,
	PANEL_CLIENTDLL_TOOLS
};

// In-game panels are cropped to the current engine viewport size
enum PaintMode_t
{
	PAINT_UIPANELS = (1 << 0),
	PAINT_INGAMEPANELS = (1 << 1),
	PAINT_CURSOR = (1 << 2), // software cursor, if appropriate
};

class IEngineVGui
{
public:
	virtual					~IEngineVGui(void) { }

	virtual vgui2::VPANEL	GetPanel(VGuiPanel_t type) = 0;

	virtual bool			IsGameUIVisible() = 0;
};

#define VENGINE_VGUI_VERSION	"VEngineVGui001"

extern IEngineVGui *engineVgui();

#endif // IENGINEVGUI_H
