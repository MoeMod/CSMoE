/*
imgui_impl_xash.cpp
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

#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "input.h"
#include "input_ime.h"
#include "minmax.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_xash.h"
#include "imgui_lcsm_warning.h"
#include "imgui_console.h"
#include "imgui_imewindow.h"
#include "imgui_sprview.h"

#include <keydefs.h>
#include <utility>
#include <algorithm>
#include <memory>

#ifdef XASH_SDL
#include <SDL.h>
#include <SDL_syswm.h>
#endif

#ifdef XASH_WINRT
#include "platform/winrt/winrt_interop.h"
#endif

// Data
double g_Time = 0.0;
bool g_MouseJustPressed[5] = { false, false, false, false, false };
float g_MouseLastPos[2];
float g_MouseWheel = 0.0f;
GLuint g_FontTexture = 0;

float g_ImGUI_DPI = 1.0f;

ImGuiContext* g_EngineContext = nullptr;

bool g_bShowDemoWindow = false;

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
void ImGui_ImplGL_RenderDrawLists(ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// We are using the OpenGL fixed pipeline to make the example code simpler to read!
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
	GLint last_texture; pglGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_polygon_mode[2]; pglGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
	GLint last_viewport[4]; pglGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; pglGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	//pglPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	GLboolean last_enable_blend = pglIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = pglIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = pglIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = pglIsEnabled(GL_SCISSOR_TEST);

	pglEnable(GL_BLEND);
	pglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	pglDisable(GL_CULL_FACE);
	pglDisable(GL_DEPTH_TEST);
	pglEnable(GL_SCISSOR_TEST);
	pglEnableClientState(GL_VERTEX_ARRAY);
	pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	pglEnableClientState(GL_COLOR_ARRAY);
	pglEnable(GL_TEXTURE_2D);
	pglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	pglShadeModel(GL_SMOOTH);

	// Setup viewport, orthographic projection matrix
//	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glLoadIdentity();

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
		pglVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
		pglTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
		pglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				//pglBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				GL_Bind( XASH_TEXTURE0, (ptrdiff_t)pcmd->TextureId );
				pglScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				pglDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
			}
			idx_buffer += pcmd->ElemCount;
		}
	}

	// Restore modified state
	pglDisableClientState(GL_COLOR_ARRAY);
	pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	pglDisableClientState(GL_VERTEX_ARRAY);
	pglBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
//	pglMatrixMode(GL_MODELVIEW);
//	pglPopMatrix();
//	pglMatrixMode(GL_PROJECTION);
//	pglPopMatrix();
	//pglPopAttrib();

	if (last_enable_blend) pglEnable(GL_BLEND); else pglDisable(GL_BLEND);
	if (last_enable_cull_face) pglEnable(GL_CULL_FACE); else pglDisable(GL_CULL_FACE);
	if (last_enable_depth_test) pglEnable(GL_DEPTH_TEST); else pglDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) pglEnable(GL_SCISSOR_TEST); else pglDisable(GL_SCISSOR_TEST);

	pglPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
//	pglViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	pglScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

//=============
// -button
//    0:left-button
//    1:right-button
//    2:middle-button
// -action
//    1:down 0:up
//=============

extern qboolean	in_mouseactive;
qboolean ImGui_ImplGL_MouseButtonCallback(int button, int action)
{
	if (!g_EngineContext)
		return false;
	if (cls.key_dest == key_game)
		return false;
	ImGuiIO& io = ImGui::GetIO();
	if (button >= 0 && button < 5)
	{
		if(action)
			g_MouseJustPressed[button] = true;
		if(!action)
			g_MouseJustPressed[button] = false;
	}
	return io.WantCaptureMouse;
}

//=============
// -input
//    key ( engine key )
//    action ( 1:down 0:up )
//=============
static void KeyCallback(int key, int action)
{
	ImGuiIO& io = ImGui::GetIO();
	if (action == 1)
		io.KeysDown[key] = true;
	if (action == 0)
		io.KeysDown[key] = false;

	io.KeyCtrl = io.KeysDown[K_CTRL];
	io.KeyShift = io.KeysDown[K_SHIFT];
	io.KeyAlt = io.KeysDown[K_ALT];
	//io.KeySuper
}

static void ScrollCallback(double xoffset, double yoffset)
{
	g_MouseWheel += (float)yoffset; // Use fractional mouse wheel.
}

qboolean ImGui_ImplGL_KeyEvent( int key, qboolean down )
{
	if (!g_EngineContext)
		return false;
	if (cls.key_dest == key_game)
		return false;
	ImGuiIO& io = ImGui::GetIO();
	// IMGUI input
	switch (key)
	{
		case K_MWHEELDOWN:
			ScrollCallback(0, -1.0);
			break;
		case K_MWHEELUP:
			ScrollCallback(0, 1.0);
			break;
		case K_MOUSE1:
		case K_MOUSE2:
		case K_MOUSE3:
		case K_MOUSE4:
		case K_MOUSE5:
			//ImGui_ImplGL_MouseButtonCallback(key - K_MOUSE1, 1);
			break;
		default:
			KeyCallback(key, down);
			break;
	}
	return io.WantCaptureKeyboard;
}

void ImGui_ImplGL_CharCallback(unsigned int c)
{
	if (!g_EngineContext)
		return void();
	ImGuiIO& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter(c);
}

qboolean ImGui_ImplGL_CharCallback(int c)
{
	if (!g_EngineContext)
		return false;
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter((unsigned)c);
	return io.WantTextInput;
}

qboolean ImGui_ImplGL_CharCallbackUTF(const char *c)
{
	if (!g_EngineContext)
		return false;
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharactersUTF8(c);
	return io.WantTextInput;
}

#define GL_UNPACK_ROW_LENGTH			0x0CF2

qboolean ImGui_ImplGL_CreateDeviceObjects(void)
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height, bytes_per_pixel;
	// Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

	// Upload texture to graphics system
	//GLint last_texture;
	//pglGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	//pglGenTextures(1, &g_FontTexture);
	//g_FontTexture = 888888;	// UNDONE: create texture by surface()->CreateNewTextureId()
	rgbdata_t	r_image;
	char	texName[32];

	Q_strncpy( texName, "*imgui_font", sizeof( texName ) );
	Q_memset( &r_image, 0, sizeof( r_image ));

	r_image.width = width;
	r_image.height = height;
	r_image.depth = bytes_per_pixel;
	r_image.type = PF_RGBA_32;
	r_image.size = r_image.width * r_image.height * r_image.depth;
	r_image.flags = IMAGE_HAS_COLOR|IMAGE_HAS_ALPHA;
	r_image.buffer = (byte *)pixels;

	//pglPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	auto i = GL_LoadTextureInternal( "*imgui_font", &r_image, (texFlags_t)TF_IMAGE, false );
	g_FontTexture = i;
	//GL_Bind(XASH_TEXTURE0, i);
	//pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//pglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(ptrdiff_t)g_FontTexture;

	// Restore state
	//pglBindTexture(GL_TEXTURE_2D, last_texture);

	// fix IME
#ifdef XASH_SDL
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(host.hWnd, &wmInfo);
#if defined _WIN32 && !defined XASH_WINRT
	io.ImeWindowHandle = wmInfo.info.win.window;
	io.ImeSetInputScreenPosFn; // use default
#endif
	io.ImeSetInputScreenPosFn = [](int x, int y) { IME_SetInputScreenPos(x, y); };
#endif

	return true;
}

void ImGui_ImplGL_InvalidateDeviceObjects(void)
{
	if (g_FontTexture)
	{
		//glDeleteTextures(1, &g_FontTexture);	// !!!
		ImGui::GetIO().Fonts->TexID = nullptr;
		g_FontTexture = 0;
	}
}

int pfnGetScreenInfo( SCREENINFO *pscrinfo );

void ImGui_ImplGL_ReloadFonts()
{
	ImGuiIO& io = ImGui::GetIO();
	search_t		*t;
	int		i;

	float size_pixels = 14 * 2; // 14 * 2

	if (size_pixels <= 0.0f)
		return;
	
	t = FS_Search( "resource/font/*.ttf", true, false );
	if (t)
	{
		for (i = 0; i < t->numfilenames; i++)
		{
			const char* filename = t->filenames[i];
			fs_offset_t length = 0;
			auto file = FS_LoadFile(filename, &length, false);
			std::unique_ptr<void, decltype(&ImGui::MemFree)> new_ptr(ImGui::MemAlloc(length + 1), ImGui::MemFree);
			memcpy(new_ptr.get(), file, length);
			Mem_Free(std::exchange(file, nullptr));
			auto font = io.Fonts->AddFontFromMemoryTTF(new_ptr.release(), length, size_pixels, NULL, io.Fonts->GetGlyphRangesChineseFull());
		}
		Mem_Free(t);
	}
	else
	{
#ifdef _WIN32
		char buffer[MAX_PATH];
		GetSystemDirectoryA(buffer, sizeof(buffer));
		strcat(buffer, "\\..\\Fonts\\simhei.ttf");
		auto font = io.Fonts->AddFontFromFileTTF(buffer, size_pixels, NULL, io.Fonts->GetGlyphRangesChineseFull());
#endif
#ifdef TARGET_OS_MAC
		auto font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/PingFang.ttc", size_pixels, NULL, io.Fonts->GetGlyphRangesChineseFull());
#endif
	}

}

#ifdef XASH_SDL

static SDL_Cursor* g_MouseCursors[ImGuiMouseCursor_COUNT] = {};

static void ImGui_ImplSDL2_Init()
{
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)
	
	// Load mouse cursors
	g_MouseCursors[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	g_MouseCursors[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	g_MouseCursors[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	g_MouseCursors[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	g_MouseCursors[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	g_MouseCursors[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	g_MouseCursors[ImGuiMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
}

void ImGui_ImplSDL2_Shutdown()
{
	// Destroy SDL mouse cursors
	for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
		SDL_FreeCursor(g_MouseCursors[cursor_n]);
	memset(g_MouseCursors, 0, sizeof(g_MouseCursors));
}

static void ImGui_ImplSDL2_UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		//SDL_ShowCursor(SDL_FALSE);
	}
	else
	{
		// Show OS mouse cursor
		SDL_SetCursor(g_MouseCursors[imgui_cursor] ? g_MouseCursors[imgui_cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
		//SDL_ShowCursor(SDL_TRUE);
	}
}
#endif

void Cmd_ImGui_f()
{
	if(Cmd_Argc() < 2)
	{
		Con_Print("Usage : imgui demo\n");
		return;
	}
	if(!strcmp(Cmd_Argv(1), "demo"))
	{
		g_bShowDemoWindow = true;
	}
}

qboolean ImGui_ImplGL_Init(void)
{
    g_EngineContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(g_EngineContext);
	ImGui::StyleColorsLight();

	ImGuiIO& io = ImGui::GetIO();
	// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
	io.KeyMap[ImGuiKey_Tab] = K_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = K_LEFTARROW;
	io.KeyMap[ImGuiKey_RightArrow] = K_RIGHTARROW;
	io.KeyMap[ImGuiKey_UpArrow] = K_UPARROW;
	io.KeyMap[ImGuiKey_DownArrow] = K_DOWNARROW;
	io.KeyMap[ImGuiKey_PageUp] = K_PGUP;
	io.KeyMap[ImGuiKey_PageDown] = K_PGDN;
	io.KeyMap[ImGuiKey_Home] = K_HOME;
	io.KeyMap[ImGuiKey_End] = K_END;
	io.KeyMap[ImGuiKey_Delete] = K_DEL;
	io.KeyMap[ImGuiKey_Backspace] = K_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = K_ENTER;
	io.KeyMap[ImGuiKey_Escape] = K_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'a';
	io.KeyMap[ImGuiKey_C] = 'c';
	io.KeyMap[ImGuiKey_V] = 'v';
	io.KeyMap[ImGuiKey_X] = 'x';
	io.KeyMap[ImGuiKey_Y] = 'y';
	io.KeyMap[ImGuiKey_Z] = 'z';

	io.SetClipboardTextFn = [](void* user_data, const char* text) { Sys_SetClipboardData((const byte *)text, strlen(text)); };
	io.GetClipboardTextFn = [](void* user_data) -> const char * { return Sys_GetClipboardData(); };
	io.ClipboardUserData = nullptr;

	//io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;
	io.IniFilename = nullptr;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	// Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	//io.RenderDrawListsFn = ImGui_ImplGL_RenderDrawLists;

#ifdef XASH_SDL
	ImGui_ImplSDL2_Init();
#endif

	//io.Fonts->AddFontFromFileTTF("msyh.ttf", 16, NULL, io.Fonts->GetGlyphRangesChinese());
	ImGui_ImplGL_ReloadFonts();

	ImGui_Console_Init();
	ImGui_ImeWindow_Init();
	ImGui_SprView_Init();

	Cmd_AddCommand("imgui", Cmd_ImGui_f, "imgui demo");

	return true;
}

void ImGui_ImplGL_Shutdown(void)
{
	if (!g_EngineContext)
		return void();

#ifdef XASH_SDL
	ImGui_ImplSDL2_Shutdown();
#endif

	ImGui_ImplGL_InvalidateDeviceObjects();
    ImGui::DestroyContext(std::exchange(g_EngineContext, nullptr));
	//ImGui::Shutdown();
}

void ImGui_ImplGL_SetColors(ImGuiStyle* style)
{
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
	colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.72f, 0.44f, 0.44f, 0.40f); //
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.72f, 0.44f, 0.44f, 0.67f); //
	colors[ImGuiCol_TitleBg]                = ImVec4(0.82f, 0.82f, 0.82f, 1.00f); //
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.92f, 0.66f, 0.66f, 1.00f); // 0.74f, 0.18f, 0.18f, 1.00f
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.72f, 0.44f, 0.44f, 1.00f); //
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.72f, 0.44f, 0.44f, 0.78f); //
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.74f, 0.18f, 0.18f, 1.00f); //
	colors[ImGuiCol_Button]                 = ImVec4(0.72f, 0.44f, 0.44f, 0.40f); //
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.72f, 0.44f, 0.44f, 1.00f); //
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.74f, 0.18f, 0.18f, 1.00f); //
	colors[ImGuiCol_Header]                 = ImVec4(0.72f, 0.44f, 0.44f, 0.31f); //
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.72f, 0.44f, 0.44f, 0.80f); //
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.72f, 0.44f, 0.44f, 1.00f); //
	colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.72f, 0.44f, 0.44f, 0.67f); //
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.72f, 0.44f, 0.44f, 0.95f); //
	colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.90f);
	colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.72f, 0.44f, 0.44f, 0.35f); //
	colors[ImGuiCol_DragDropTarget]         = ImVec4(0.72f, 0.44f, 0.44f, 0.95f); //
	colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void ImGui_ImplGL_NewFrame(void)
{
	if (!g_FontTexture)
	{
		ImGui_ImplGL_CreateDeviceObjects();
	}

	ImGuiIO& io = ImGui::GetIO();


	ImGuiStyle nst;
	ImGui_ImplGL_SetColors(&nst);
	// Setup display size (assume the size of the game window is never changed)
	io.DisplaySize = ImVec2((float)glState.width, (float)glState.height);
	//io.DisplayFramebufferScale = ImVec2((float)glState.width / (float)si.iWidth, (float)glState.height / (float)si.iHeight);
	float new_dpi = hud_scale->value;
	if (new_dpi == 0)
		new_dpi = 2;
	if(g_ImGUI_DPI != new_dpi)
	{
		g_ImGUI_DPI = new_dpi;
	}
	nst.WindowMenuButtonPosition = ImGuiDir_None;
	nst.WindowPadding = {16, 16};
	nst.WindowMinSize = {400, 300};
#ifdef _WIN32
	nst.WindowTitleAlign = { 0.0, 0.5 };
	nst.WindowRounding = 0.0f;
#else
	nst.WindowTitleAlign = { 0.5, 0.5 };
	nst.WindowRounding = 8.0f;
#endif
	nst.FramePadding = {16, 9};
	nst.ScaleAllSizes(g_ImGUI_DPI);
	io.FontGlobalScale = g_ImGUI_DPI / 2;
	ImGui::GetStyle() = nst;

	// Setup time step
	double current_time = Sys_DoubleTime();
	io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
	io.DeltaTime = io.DeltaTime > 0.0f ? io.DeltaTime : 0.0f;	// NOTE: ??? Must be greater than zero
	g_Time = current_time;

	if ( host.state != HOST_NOFOCUS )
	{
		// NOTE: the game window is active
		io.MousePos = ImVec2(g_MouseLastPos[0], g_MouseLastPos[1]);
	}
	else
	{
		// without focus
		io.MousePos = ImVec2(NAN, NAN);
	}

	// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
	std::copy(std::begin(g_MouseJustPressed), std::end(g_MouseJustPressed), io.MouseDown);
	//std::fill(std::begin(g_MouseJustPressed), std::end(g_MouseJustPressed), false);

	io.MouseWheel = g_MouseWheel;
	g_MouseWheel = 0.0f;

	if(io.WantTextInput)
		Key_EnableTextInput(true, false);

#ifdef XASH_SDL
	ImGui_ImplSDL2_UpdateMouseCursor();
#endif

	// Hide OS mouse cursor if ImGui is drawing it
	//if( !io.MouseDrawCursor ) IN_ActivateMouse( false );
	//else IN_DeactivateMouse();

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();
}

qboolean ImGui_ImplGL_MouseMove(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	g_MouseLastPos[0] = (float)x;
	g_MouseLastPos[1] = (float)y;
	return io.WantCaptureMouse;
}

void ImGui_ImplGL_Render(void)
{
	ImGuiIO& io = ImGui::GetIO();
    ImGui::Render();
	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	//pglViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//pglClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	//pglClear(GL_COLOR_BUFFER_BIT);
	//pglUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
	ImGui_ImplGL_RenderDrawLists(ImGui::GetDrawData());
}

void Engine_OnGUI(struct ImGuiContext *context)
{
	ImGui::SetCurrentContext(context);
	ImGui_LCSM_OnGUI();
	ImGui_Console_OnGUI();
	ImGui_ImeWindow_OnGUI();
	ImGui_SprView_OnGUI();
	if(g_bShowDemoWindow)
		ImGui::ShowDemoWindow(&g_bShowDemoWindow);
}

void ImGui_ImplGL_Client_OnGUI(void)
{
	//if (clgame.dllFuncs.pfnOnGUI && cls.key_dest == key_game)
	//	clgame.dllFuncs.pfnOnGUI(g_EngineContext);
}

void ImGui_ImplGL_Menu_OnGUI(void)
{
	//if (menu.dllFuncs.pfnOnGUI && cls.key_dest == key_menu)
	//	menu.dllFuncs.pfnOnGUI(g_EngineContext);
}

void ImGui_ImplGL_Engine_OnGUI(void)
{
	Engine_OnGUI(g_EngineContext);
}

#define TO_SCRN_Y(x) (scr_height->integer * (x))
#define TO_SCRN_X(x) (scr_width->integer * (x))

qboolean ImGui_ImplGL_TouchCallback(touchEventType type, int fingerID, float x, float y, float dx, float dy, float pressure)
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGL_MouseMove(TO_SCRN_X(x), TO_SCRN_Y(y));
	ImGui_ImplGL_MouseButtonCallback(0, type != event_up);
	return io.WantCaptureMouse;
}
