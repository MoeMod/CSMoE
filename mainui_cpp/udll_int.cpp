/*
dll_int.cpp - dll entry point
Copyright (C) 2010 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Utils.h"

namespace ui
{
ui_enginefuncs_t EngFuncs::engfuncs;
#ifndef XASH_DISABLE_FWGS_EXTENSIONS
ui_textfuncs_t	EngFuncs::textfuncs;
#endif

ui_globalvars_t	*gpGlobals;
fs_api_t gFileSystemAPI = { };

CMenu gMenu;

static UI_FUNCTIONS gFunctionTable = 
{
	UI_VidInit,
	UI_Init,
	UI_Shutdown,
	UI_UpdateMenu,
	UI_KeyEvent,
	UI_MouseMove,
	UI_SetActiveMenu,
	UI_AddServerToList,
	UI_GetCursorPos,
	UI_SetCursorPos,
	UI_ShowCursor,
	UI_CharEvent,
	UI_MouseInRect,
	UI_IsVisible,
	UI_CreditsActive,
	UI_FinalCredits,
#ifdef XASH_IMGUI
	UI_OnGUI
#else
	nullptr
#endif
};
}

//=======================================================================
//			GetApi
//=======================================================================
extern "C" EXPORT int GetMenuAPI(UI_FUNCTIONS *pFunctionTable, ui_enginefuncs_t* pEngfuncsFromEngine, ui_globalvars_t *pGlobals)
{
	if( !pFunctionTable || !pEngfuncsFromEngine )
	{
		return FALSE;
	}

	// copy HUD_FUNCTIONS table to engine, copy engfuncs table from engine
	memcpy( pFunctionTable, &ui::gFunctionTable, sizeof( UI_FUNCTIONS ));
	memcpy( &ui::EngFuncs::engfuncs, pEngfuncsFromEngine, sizeof( ui_enginefuncs_t ));
#ifndef XASH_DISABLE_FWGS_EXTENSIONS
	memset( &ui::EngFuncs::textfuncs, 0, sizeof( ui_textfuncs_t ));
#endif
	ui::gpGlobals = pGlobals;

	return TRUE;
}

#ifndef XASH_DISABLE_FWGS_EXTENSIONS
extern "C" EXPORT int GiveTextAPI( ui_textfuncs_t* pTextfuncsFromEngine )
{
	if( !pTextfuncsFromEngine )
	{
		return FALSE;
	}

	// copy HUD_FUNCTIONS table to engine, copy engfuncs table from engine
	memcpy( &ui::EngFuncs::textfuncs, pTextfuncsFromEngine, sizeof( ui_textfuncs_t ));

	return TRUE;
}
extern "C" EXPORT void AddTouchButtonToList( const char *name, const char *texture, const char *command, unsigned char *color, int flags ); // TouchButtons.cpp
#endif

#ifdef XASH_STATIC_GAMELIB
typedef struct dllexport_s
{
const char *name;
void *func;
} dllexport_t;

static dllexport_t switch_mainui_exports[] = {
	{ "GetMenuAPI", (void*)GetMenuAPI },
	{ "GiveTextAPI", (void*)GiveTextAPI },
	{ "AddTouchButtonToList", (void*)AddTouchButtonToList },
	{ NULL, NULL }
};

extern "C" int dll_register( const char *name, dllexport_t *exports );

extern "C" int switch_installdll_mainui( void )
{
	return dll_register( "menu", switch_mainui_exports );
}
#endif
