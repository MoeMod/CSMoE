/*
UIFuncs.cpp
Copyright (C) 2020 Moemod Haoyuan

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
#include "Utils.h"

#include "GameUI/GameUI.h"

#include <imgui.h>
#include <memory>

std::unique_ptr<ui::GameUI> g_pGameUI;

int UI_VidInit(void)
{
	return 0;
}
void UI_Init(void)
{
    g_pGameUI = std::make_unique<ui::GameUI>();
}
void UI_Shutdown(void)
{
	g_pGameUI = nullptr;
}
void UI_UpdateMenu(float flTime)
{

}
void UI_KeyEvent(int key, int down)
{

}
void UI_MouseMove(int x, int y)
{

}
void UI_SetActiveMenu(int fActive)
{
	// don't continue firing if we leave game
	EngFuncs::KEY_ClearStates();

	if( fActive )
	{
		EngFuncs::KEY_SetDest( KEY_MENU );
	}

	g_pGameUI->SetActivate(fActive);
}
void UI_AddServerToList(netadr_t adr, const char* info)
{

}
void UI_GetCursorPos(int* pos_x, int* pos_y)
{

}
void UI_SetCursorPos(int pos_x, int pos_y)
{

}
void UI_ShowCursor(int show)
{

}
void UI_CharEvent(int key)
{

}
int UI_MouseInRect(void)
{
	return 0;
}
int UI_IsVisible(void)
{
	return 0;
}
int UI_CreditsActive(void)
{
	return 0;
}
void UI_FinalCredits(void)
{

}
void UI_OnGUI(struct ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	g_pGameUI->OnGUI();
}
extern "C" EXPORT void AddTouchButtonToList(const char* name, const char* texture, const char* command, unsigned char* color, int flags)
{

}