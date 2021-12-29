/*
imgui_surface.h
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

int ImGui_Surface_DrawConsoleString(int x0, int y0, const char* string, byte r, byte g, byte b, byte a);
void ImGui_Surface_DrawStringLen(const char* pText, int* length, int* height);
int ImGui_Surface_DrawChar(int x, int y, int ch, byte r, byte g, byte b, byte a);
void ImGui_Surface_SetRenderMode( int mode );
void ImGui_Surface_SetCustomBlend( int blendsrc, int blenddst );
void ImGui_Surface_DrawRectangle(float x, float y, float w, float h, byte r, byte g, byte b, byte a);
void ImGui_Surface_DrawImage(int texnum, float x1, float y1, float x2, float y2, float s1, float t1, float s2, float t2, byte r, byte g, byte b, byte a);
void ImGui_Surface_EnableScissor(float x, float y, float w, float h);
void ImGui_Surface_DisableScissor();