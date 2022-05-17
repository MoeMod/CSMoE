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

#include "common.h"
#include "client.h"
#include "gl_local.h"

#include "imgui_surface.h"
#include "imgui.h"
#include "imgui_utils.h"
#include "imgui_console.h"

#include <string>

#include "client.h"

extern int g_imguiRenderMode;

ImColor RGBAtoImColor(rgba_t setColor)
{
	return ImColor(setColor[0], setColor[1], setColor[2], setColor[3]);
}

void ImGui_SetTextAdjustSize(bool x)
{
    clgame.ds.adjust_size = x;
}

int ImGui_Surface_DrawConsoleString(int x0, int y0, const char* string, byte r, byte g, byte b, byte a)
{
	if (!ImGui::GetDrawListSharedData()->Font)
		return 0;

	ImGui_Surface_SetRenderMode(kRenderTransTexture);
	TextAdjustSize( &x0, &y0, NULL, NULL);

	static auto print_segment = [](int x, int y, std::string_view sv, ImColor col)
	{
		ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
		auto shadow_col = ImColor(col.Value.x * 0.55, col.Value.y * 0.34, col.Value.z * 0.11, col.Value.w);
		auto shadow_col2 = ImColor(0.f, 0.f, 0.f, col.Value.w);
		drawlist->AddText(ImVec2(x - 1, y - 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x - 1, y + 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x + 1, y - 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x + 1, y + 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x + 2, y + 2), shadow_col2, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x, y), col, sv.data(), sv.data() + sv.length());
	};

	static auto text_size = [](std::string_view sv) { return ImGui::CalcTextSize(sv.data(), sv.data() + sv.length()); };

	extern rgba_t g_color_table[8];

	std::string_view sv = string;
	ImColor col(r, g, b, a);
	ImColor last_color = col;
	int x = x0, y = y0;
	auto find_str = "^\n\x01\x02\x03\x04\x05\x06\x07";
	for (auto seg = sv.find_first_of(find_str); seg != sv.npos; seg = sv.find_first_of(find_str))
	{
		if (sv[seg] == '^' && seg != sv.size() - 1 && sv[seg + 1] >= '1' && sv[seg + 1] <= '7')
		{
			print_segment(x, y, sv.substr(0, seg), last_color);
			auto size = text_size(sv.substr(0, seg));
			x += size.x;

			if (sv[seg + 1] == '7')
				last_color = col;
			else
				last_color = RGBAtoImColor(g_color_table[sv[seg + 1] - '0']);

			sv = sv.substr(seg + 2);
			continue;
		}
		else if (sv[seg] >= '\x01' && sv[seg] <= '\x07' && seg != sv.size() - 1)
		{
			print_segment(x, y, sv.substr(0, seg), last_color);
			auto size = text_size(sv.substr(0, seg));
			x += size.x;
			// ignored
			/*
			if (sv[seg] == '\x07')
				last_color = col;
			else
				last_color = RGBAtoImColor(g_color_table[sv[seg] - '\x00']);
			*/
			sv = sv.substr(seg + 1);
			continue;
		}
		else if (sv[seg] == '\n' && seg != sv.size() - 1)
		{
			print_segment(x, y, sv.substr(0, seg), last_color);
			auto size = text_size(sv.substr(0, seg));
			y += size.y;
			x = x0;
			sv = sv.substr(seg + 1);
			continue;
		}
		break;
	}
	if (!sv.empty())
	{
		print_segment(x, y, sv, last_color);
		auto size = text_size(sv);
		x += size.x;
	}

	//float xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	int result = x - x0;
	TextAdjustSizeReverse(&result, NULL, NULL, NULL);
	return result;
}

void ImGui_Surface_DrawStringLen(const char* pText, int* length, int* height)
{
	ImVec2 size = {};
	std::string filter_str;
	for(const char* p = pText; *p; ++p)
	{
		if (p[0] == '^' && p[1] >= '1' && p[1] <= '7')
			p += 2;
		if (p[0] >= '\x01' && p[0] <= '\x07')
			p += 1;
		filter_str.push_back(*p);
	}
	if (ImGui::GetDrawListSharedData()->Font)
	{
		size = ImGui::CalcTextSize(filter_str.c_str(), filter_str.c_str() + filter_str.length());
	}
	if (length) *length = size.x;
	if (height) *height = size.y;
	TextAdjustSizeReverse(NULL, NULL, length, height);
}

void ImGui_Surface_DrawStringLenUnicode(const wchar_t* pText, int* length, int* height)
{
    ImFont* font = ImGui::GetDrawListSharedData()->Font;
    if (!font)
        return;
    ImGuiIO& io = ImGui::GetIO();

    int w = 0;
    for (auto text = pText; *text != 0; ++text) {
        int ch = *text;
        w += font->GetCharAdvance(ch) * io.FontGlobalScale; // / io.FontGlobalScale;
    }

    int h = font->FontSize * io.FontGlobalScale; // / io.FontGlobalScale;
    if (length) *length = w;
    if (height) *height = h;

	TextAdjustSizeReverse(NULL, NULL, length, height);
}

int ImGui_Surface_DrawChar(int x, int y, wchar_t ch, byte r, byte g, byte b, byte a)
{
	ImFont* font = ImGui::GetDrawListSharedData()->Font;
	if (!font)
		return 0;
	ImGui_Surface_SetRenderMode(kRenderTransTexture);
	TextAdjustSize(&x, &y, NULL, NULL);

	ImColor col(r, g, b, a);
	auto pos = ImVec2(x, y);

	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	drawlist->PushTextureID(font->ContainerAtlas->TexID);
	ImGuiIO& io = ImGui::GetIO();
	font->RenderChar(drawlist, font->FontSize * io.FontGlobalScale, pos, col, ch);
	drawlist->PopTextureID();

	int w = font->GetCharAdvance(ch) * io.FontGlobalScale; // / io.FontGlobalScale;
	TextAdjustSizeReverse(NULL, NULL, &w, NULL);

	return w;
}

int ImGui_Surface_GetCharWidth(wchar_t ch)
{
    ImFont* font = ImGui::GetDrawListSharedData()->Font;
    if (!font)
        return 0;
    ImGuiIO& io = ImGui::GetIO();
    int w = font->GetCharAdvance(ch) * io.FontGlobalScale; // / io.FontGlobalScale;
    TextAdjustSizeReverse(NULL, NULL, &w, NULL);

    return w;
}

int ImGui_Surface_GetCharHeight()
{
    ImFont* font = ImGui::GetDrawListSharedData()->Font;
    if (!font)
        return 0;
    ImGuiIO& io = ImGui::GetIO();
    int h = font->FontSize * io.FontGlobalScale; // / io.FontGlobalScale;
    TextAdjustSizeReverse(NULL, NULL, NULL, &h);

    return h;
}

template<int mode> void ImGui_Surface_SetRenderModeDrawCall(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
	GL_SetRenderMode(mode);
}

void ImGui_Surface_SetRenderMode( int mode )
{
	if(mode == g_imguiRenderMode)
		return;
	g_imguiRenderMode = mode;
	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	
	switch( mode )
	{
		case kRenderNormal:
		default:
			drawlist->AddCallback(&ImGui_Surface_SetRenderModeDrawCall<kRenderNormal>, nullptr);
			break;
		case kRenderTransColor:
			drawlist->AddCallback(&ImGui_Surface_SetRenderModeDrawCall<kRenderTransColor>, nullptr);
			break;
		case kRenderTransTexture:
			drawlist->AddCallback(&ImGui_Surface_SetRenderModeDrawCall<kRenderTransTexture>, nullptr);
			break;
		case kRenderTransAlpha:
			drawlist->AddCallback(&ImGui_Surface_SetRenderModeDrawCall<kRenderTransAlpha>, nullptr);
			break;
		case kRenderGlow:
			drawlist->AddCallback(&ImGui_Surface_SetRenderModeDrawCall<kRenderGlow>, nullptr);
			break;
		case kRenderTransAdd:
			drawlist->AddCallback(&ImGui_Surface_SetRenderModeDrawCall<kRenderTransAdd>, nullptr);
			break;
	}
}

void ImGui_Surface_SetCustomBlend( int blendsrc, int blenddst )
{
	// TODO
	ImGui_Surface_SetRenderMode(kRenderTransAdd);
}

void ImGui_Surface_DrawRectangle(float x, float y, float w, float h, byte r, byte g, byte b, byte a)
{
	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	drawlist->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImColor(r, g, b, a));
}

void ImGui_Surface_DrawImage(int texnum, float x1, float y1, float x2, float y2, float s1, float t1, float s2, float t2, byte r, byte g, byte b, byte a)
{
	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	drawlist->AddImage((ImTextureID)texnum, ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(s1, t1), ImVec2(s2, t2), ImColor(r, g, b, a));
}

void ImGui_Surface_EnableScissor(float x, float y, float w, float h)
{
	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	drawlist->PushClipRect(ImVec2(x, y), ImVec2(x + w, y + h));
}

void ImGui_Surface_EnableFullScreenScissor()
{
    ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
    drawlist->PushClipRectFullScreen();
}

void ImGui_Surface_DisableScissor()
{
	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	drawlist->PopClipRect();
}

void ImGui_Surface_Flush()
{
    ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
    drawlist->AddDrawCmd();
}