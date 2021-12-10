/*
imgui_impl_xash.h
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef IMGUI_IMPL_GL_H
#define IMGUI_IMPL_GL_H

#include "touch.h"

qboolean ImGui_ImplGL_Init(void);
void ImGui_ImplGL_Shutdown(void);
void ImGui_ImplGL_NewFrame(void);
void ImGui_ImplGL_Render(void);
void ImGui_ImplGL_Client_OnGUI(void);
void ImGui_ImplGL_Menu_OnGUI(void);
void ImGui_ImplGL_Engine_OnGUI(void);
void ImGui_ImplGL_ReloadFonts(void);

// Use if you want to reset your rendering device without losing ImGui state.
void ImGui_ImplGL_InvalidateDeviceObjects(void);
qboolean ImGui_ImplGL_CreateDeviceObjects(void);

// Provided here if you want to chain callbacks yourself. You may also handle inputs yourself and use those as a reference.
qboolean ImGui_ImplGL_MouseButtonCallback(int button, int action);
qboolean ImGui_ImplGL_KeyEvent( int key, int down );
qboolean ImGui_ImplGL_CharCallback(int c);
qboolean ImGui_ImplGL_CharCallbackUTF(const char *c);
qboolean ImGui_ImplGL_MouseMove(int x, int y);

qboolean ImGui_ImplGL_TouchCallback(touchEventType type, int fingerID, float x, float y, float dx, float dy, float pressure);

#endif
