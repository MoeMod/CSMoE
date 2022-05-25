/*
imgui_surface.cpp
Copyright (C) 2022 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

int VGUI2_Surface_GetCharWidth(int ch);
int VGUI2_Surface_GetCharHeight();
int VGUI2_Surface_DrawConsoleString(int x0, int y0, const char* string, byte r, byte g, byte b, byte a);
void VGUI2_Surface_DrawStringLen(const char* pText, int* length, int* height);
void VGUI2_Surface_DrawStringLenUnicode(const wchar_t *ch, int* length, int* height);
int VGUI2_Surface_DrawChar(int x, int y, int ch, byte r, byte g, byte b, byte a);