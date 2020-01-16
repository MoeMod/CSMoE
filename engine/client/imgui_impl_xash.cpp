/*
imgui_impl_xash.cpp
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

extern "C" {
#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "input.h"
}

#include "imgui.h"
#include "imgui_impl_xash.h"

#include <keydefs.h>
#include <utility>
#include <algorithm>

// Data
double g_Time = 0.0;
bool g_MouseJustPressed[5] = { false, false, false, false, false };
float g_MouseLastPos[2];
float g_MouseWheel = 0.0f;
GLuint g_FontTexture = 0;

float g_ImGUI_DPI = 1.0f;

ImGuiContext* g_EngineContext = nullptr;

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
	pglPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
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
				pglBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
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
	pglPopAttrib();
	pglPolygonMode(GL_FRONT, last_polygon_mode[0]); pglPolygonMode(GL_BACK, last_polygon_mode[1]);
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
bool ImGui_ImplGL_MouseButtonCallback(int button, int action)
{
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

bool ImGui_ImplGL_KeyEvent( int key, qboolean down )
{
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
	ImGuiIO& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter(c);
}

bool ImGui_ImplGL_CharCallbackUTF(const char *c)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharactersUTF8(c);
	return io.WantTextInput;
}

bool ImGui_ImplGL_CreateDeviceObjects(void)
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	// Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Upload texture to graphics system
	GLint last_texture;
	pglGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	//glGenTextures(1, &g_FontTexture);
	g_FontTexture = 888888;	// UNDONE: create texture by surface()->CreateNewTextureId()
	pglBindTexture(GL_TEXTURE_2D, g_FontTexture);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	pglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

	// Restore state
	pglBindTexture(GL_TEXTURE_2D, last_texture);

	return true;
}

void ImGui_ImplGL_InvalidateDeviceObjects(void)
{
	if (g_FontTexture)
	{
		//glDeleteTextures(1, &g_FontTexture);	// !!!
		ImGui::GetIO().Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

extern "C" int pfnGetScreenInfo( SCREENINFO *pscrinfo );

void ImGui_ImplGL_ReloadFonts()
{
	ImGuiIO& io = ImGui::GetIO();
	search_t		*t;
	int		i;

	t = FS_Search( "resource/font/*.ttf", true, false );
	if( !t ) return;

	for( i = 0; i < t->numfilenames; i++ )
	{
		const char *filename = t->filenames[i];
		fs_offset_t length = 0;
		auto file = FS_LoadFile(filename, &length, false);
		std::unique_ptr<void, decltype(&ImGui::MemFree)> new_ptr(ImGui::MemAlloc(length + 1), ImGui::MemFree);
		memcpy(new_ptr.get(), file, length);
		Mem_Free( std::exchange(file, nullptr) );
		auto font = io.Fonts->AddFontFromMemoryTTF(new_ptr.release(), length, 14 * 2, NULL, io.Fonts->GetGlyphRangesChineseFull());
	}

	Mem_Free( t );
}

bool ImGui_ImplGL_Init(void)
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

	//io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;
	io.IniFilename = nullptr;

	// Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	//io.RenderDrawListsFn = ImGui_ImplGL_RenderDrawLists;
#ifdef _WIN32
	io.ImeWindowHandle = NULL;
#endif

	//io.Fonts->AddFontFromFileTTF("msyh.ttf", 16, NULL, io.Fonts->GetGlyphRangesChinese());
	ImGui_ImplGL_ReloadFonts();

	return true;
}

void ImGui_ImplGL_Shutdown(void)
{
	ImGui_ImplGL_InvalidateDeviceObjects();
    ImGui::DestroyContext(std::exchange(g_EngineContext, nullptr));
	//ImGui::Shutdown();
}

extern "C" extern qboolean	in_mouseactive;
void ImGui_ImplGL_NewFrame(void)
{
	if (!g_FontTexture)
	{
		ImGui_ImplGL_CreateDeviceObjects();
	}

	ImGuiIO& io = ImGui::GetIO();


	ImGuiStyle nst;
	ImGui::StyleColorsLight(&nst);
	// Setup display size (assume the size of the game window is never changed)
	io.DisplaySize = ImVec2((float)glState.width, (float)glState.height);
	//io.DisplayFramebufferScale = ImVec2((float)glState.width / (float)si.iWidth, (float)glState.height / (float)si.iHeight);
	float new_dpi = 2;
	if(g_ImGUI_DPI != new_dpi)
	{
		g_ImGUI_DPI = new_dpi;
	}
	nst.WindowMenuButtonPosition = ImGuiDir_None;
	nst.WindowTitleAlign = {0.5, 0.5};
	nst.WindowPadding = {16, 16};
	nst.WindowMinSize = {400, 300};
	nst.WindowRounding = 8.0f;
	nst.FramePadding = {16, 9};
	nst.ScaleAllSizes(g_ImGUI_DPI);
	io.FontGlobalScale = g_ImGUI_DPI / 2;
	ImGui::GetStyle() = nst;

	// Setup time step
	double current_time = cl.time;
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

	// Hide OS mouse cursor if ImGui is drawing it
	//if( !io.MouseDrawCursor ) IN_ActivateMouse( false );
	//else IN_DeactivateMouse();

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();
}

bool ImGui_ImplGL_MouseMove(int x, int y)
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
	pglViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//pglClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	//pglClear(GL_COLOR_BUFFER_BIT);
	//pglUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
	ImGui_ImplGL_RenderDrawLists(ImGui::GetDrawData());
}

void Engine_OnGUI(struct ImGuiContext *context)
{
	ImGui::SetCurrentContext(context);

}

void ImGui_ImplGL_OnGUI(void)
{
	Engine_OnGUI(g_EngineContext);

	if(clgame.dllFuncs.pfnOnGUI)
		clgame.dllFuncs.pfnOnGUI(g_EngineContext);

}

void ImGui_ImplGL_TouchCallback(touchEventType type, int fingerID, float x, float y, float dx, float dy, float pressure)
{
	// TODO
}
