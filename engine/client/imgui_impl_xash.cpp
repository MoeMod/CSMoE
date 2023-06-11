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
#include "imgui_connectprogress.h"
#include "imgui_menu_server.h"
#include "imgui_menu_update.h"
#include "imgui_menu_msgbox.h"
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

#include "gl_shader.h"

// Data
double g_Time = 0.0;
bool g_MouseJustPressed[5] = { false, false, false, false, false };
bool g_KeysJustPressed[512];

float g_MouseLastPos[2];
float g_MouseWheel = 0.0f;
GLuint g_FontTexture = 0;

float g_ImGUI_DPI = 1.0f;
float g_ImGui_Font_DPI = 1.0f;

ImGuiContext* g_EngineContext = nullptr;

bool g_bShowDemoWindow = false;

int g_imguiRenderMode = 0;

extern "C" mempool_t *imgui_pool = nullptr;

#define XASH_SHADER

#ifdef XASH_SHADER
static GLuint       g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static GLint        g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;                                // Uniforms location
static GLuint       g_AttribLocationVtxPos = 0, g_AttribLocationVtxUV = 0, g_AttribLocationVtxColor = 0; // Vertex attributes location
static unsigned int g_VboHandle = 0, g_ElementsHandle = 0;
#endif

void* ImGui_ImplGL_OperatorNew(size_t sz, void* user_data)
{
	return Mem_Alloc(imgui_pool, sz);
}

void ImGui_ImplGL_OperatorDelete(void* ptr, void* user_data)
{
	return ptr ? Mem_Free(ptr) : void();
}

#ifdef XASH_SHADER
static void ImGui_ImplOpenGL3_SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height)
{
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    //glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float N = -1.0f;
    float F = +1.0f;
    const float ortho_projection[4][4] =
            {
                    { 2.0f/(R-L),   0.0f,         0.0f,         0.0f },
                    { 0.0f,         2.0f/(T-B),   0.0f,         0.0f },
                    { 0.0f,         0.0f,         2.0f/(F-N),   0.0f },
                    { (R+L)/(L-R),  (T+B)/(B-T),  (N+F)/(N-F),  1.0f },
            };
    pglUseProgramObjectARB(g_ShaderHandle);
    pglUniform1iARB(g_AttribLocationTex, 0);
    pglUniformMatrix4fvARB(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

    // Bind vertex/index buffers and setup attributes for ImDrawVert
    pglBindBufferARB(GL_ARRAY_BUFFER_ARB, g_VboHandle);
    pglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, g_ElementsHandle);

    pglEnableVertexAttribArrayARB(g_AttribLocationVtxPos);
    pglEnableVertexAttribArrayARB(g_AttribLocationVtxUV);
    pglEnableVertexAttribArrayARB(g_AttribLocationVtxColor);
    pglVertexAttribPointerARB(g_AttribLocationVtxPos,   2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    pglVertexAttribPointerARB(g_AttribLocationVtxUV,    2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    pglVertexAttribPointerARB(g_AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
}

static void ImGui_ImplOpenGL3_ResetRenderState(ImDrawData* draw_data, int fb_width, int fb_height)
{
    pglDisableVertexAttribArrayARB(g_AttribLocationVtxPos);
    pglDisableVertexAttribArrayARB(g_AttribLocationVtxUV);
    pglDisableVertexAttribArrayARB(g_AttribLocationVtxColor);
    // Restore modified GL state
    pglUseProgramObjectARB(0);
    pglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    pglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}
#endif
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
	pglShadeModel(GL_SMOOTH);
	g_imguiRenderMode = kRenderTransAlpha;
	GL_SetRenderMode(kRenderTransAlpha);
	// Render command lists

#ifdef XASH_SHADER
    // Setup desired GL state
    // Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
    // The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height);

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        pglBufferDataARB(GL_ARRAY_BUFFER_ARB, (GLsizeiptrARB)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW_ARB);
        pglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, (GLsizeiptrARB)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW_ARB);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                GL_Bind( XASH_TEXTURE0, (ptrdiff_t)pcmd->TextureId );
                pglScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                pglDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
            }
        }
    }
    ImGui_ImplOpenGL3_ResetRenderState(draw_data, fb_width, fb_height);
#else
    pglEnableClientState(GL_VERTEX_ARRAY);
	pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	pglEnableClientState(GL_COLOR_ARRAY);
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
#if !defined XASH_NANOGL || defined XASH_WES && defined __EMSCRIPTEN__ // WebGL need to know array sizes
                if( pglDrawRangeElements )
                    pglDrawRangeElements( GL_TRIANGLES, 0, (GLsizei)pcmd->ElemCount, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer );
                else
#endif
				pglDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
			}
			idx_buffer += pcmd->ElemCount;
		}
	}

	// Restore modified state
	pglDisableClientState(GL_COLOR_ARRAY);
	pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	pglDisableClientState(GL_VERTEX_ARRAY);
#endif
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
	{
		io.KeysDown[key] = true;
		g_KeysJustPressed[key] = true;
	}
	if (action == 0)
	{
		if(!g_KeysJustPressed[key])
			io.KeysDown[key] = false;
	}

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
	ImGuiIO& io = ImGui::GetIO();
	bool bIsMouse = false;
	// IMGUI input
	switch (key)
	{
		case K_MWHEELDOWN:
			ScrollCallback(0, -1.0);
			bIsMouse = true;
			break;
		case K_MWHEELUP:
			ScrollCallback(0, 1.0);
			bIsMouse = true;
			break;
		case K_MOUSE1:
		case K_MOUSE2:
		case K_MOUSE3:
		case K_MOUSE4:
		case K_MOUSE5:
			bIsMouse = true;
			//ImGui_ImplGL_MouseButtonCallback(key - K_MOUSE1, 1);
			break;
		default:
			KeyCallback(key, down);
			break;
	}

	if(bIsMouse)
		return io.WantCaptureMouse;
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
#ifdef XASH_SHADER
    // compile shaders for es2
    const char *vertex_shader_list[2] = { xe::GetShaderVersionString(),
            "uniform mat4 ProjMtx;\n"
            "attribute vec2 Position;\n"
            "attribute vec2 UV;\n"
            "attribute vec4 Color;\n"
            "varying vec2 Frag_UV;\n"
            "varying vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n"
     };

    const GLcharARB* fragment_shader_list[2] = { xe::GetShaderVersionString(),
            "uniform sampler2D Texture;\n"
            "varying vec2 Frag_UV;\n"
            "varying vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
            "}\n"
        };
    g_VertHandle = pglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    pglShaderSourceARB(g_VertHandle, 2, vertex_shader_list, NULL);
    pglCompileShaderARB(g_VertHandle);
    xe::CheckShader(g_VertHandle, "vertex shader");

    g_FragHandle = pglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    pglShaderSourceARB(g_FragHandle, 2, fragment_shader_list, NULL);
    pglCompileShaderARB(g_FragHandle);
    xe::CheckShader(g_FragHandle, "fragment shader");

    g_ShaderHandle = pglCreateProgramObjectARB();
    pglAttachObjectARB(g_ShaderHandle, g_VertHandle);
    pglAttachObjectARB(g_ShaderHandle, g_FragHandle);
    pglLinkProgramARB(g_ShaderHandle);
    xe::CheckProgram(g_ShaderHandle, "shader program");

    g_AttribLocationTex = pglGetUniformLocationARB(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = pglGetUniformLocationARB(g_ShaderHandle, "ProjMtx");
    g_AttribLocationVtxPos = (GLuint)pglGetAttribLocationARB(g_ShaderHandle, "Position");
    g_AttribLocationVtxUV = (GLuint)pglGetAttribLocationARB(g_ShaderHandle, "UV");
    g_AttribLocationVtxColor = (GLuint)pglGetAttribLocationARB(g_ShaderHandle, "Color");

    // Create buffers
    pglGenBuffersARB(1, &g_VboHandle);
    pglGenBuffersARB(1, &g_ElementsHandle);
#endif

	ImGui_ImplGL_ReloadFonts();

	unsigned char* pixels;
	int width, height, bytes_per_pixel;
	// Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
#ifdef XASH_SHADER
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);
#else
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height, &bytes_per_pixel);
#endif

	// Upload texture to graphics system
	//GLint last_texture;
	//pglGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	//pglGenTextures(1, &g_FontTexture);
	//g_FontTexture = 888888;	// UNDONE: create texture by surface()->CreateNewTextureId()
	auto	r_image = Image_NewTemp();
	char	texName[32];

	Q_strncpy( texName, "*imgui_font", sizeof( texName ) );

	r_image->width = width;
	r_image->height = height;
	r_image->depth = bytes_per_pixel;
	r_image->type = PF_A_8;
	r_image->size = r_image->width * r_image->height * r_image->depth;
	r_image->flags = IMAGE_HAS_ALPHA | IMAGE_TEMP;
	r_image->buffer = (byte *)pixels;

#ifdef XASH_SHADER
    r_image->type = PF_RGBA_32;
    r_image->flags |= IMAGE_HAS_COLOR;
#endif

	//pglPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#ifdef XASH_SHADER
	auto i = GL_LoadTextureInternal( "*imgui_font", r_image, (texFlags_t)(TF_FONT), false );
#else
	auto i = GL_LoadTextureInternal( "*imgui_font", r_image, (texFlags_t)(TF_FONT | TF_FONT_ALPHA), false );
#endif
	g_FontTexture = i;
	//GL_Bind(XASH_TEXTURE0, i);
	//pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//pglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(ptrdiff_t)g_FontTexture;

	// free memory
	io.Fonts->ClearTexData();

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

void ImGui_ImplGL_ClearFontCache()
{
    // clear font
    ImGuiIO& io = ImGui::GetIO();
    for(auto *font : io.Fonts->Fonts)
    {
        if(!font->ConfigData->FontDataOwnedByAtlas)
            FS_MapFree((const byte *)font->ConfigData->FontData, font->ConfigData->FontDataSize);
    }
    io.Fonts->ClearFonts();
}

void ImGui_ImplGL_ReloadFonts()
{
	ImGuiIO& io = ImGui::GetIO();
	search_t		*t;
	int		i;

	if(glConfig.max_2d_texture_size <= 2048)
		g_ImGui_Font_DPI = 0.5f;
	else if(glConfig.max_2d_texture_size <= 4096)
		g_ImGui_Font_DPI = 1.0f;
	else
		g_ImGui_Font_DPI = 2.0f;

	float size_pixels = 14 * g_ImGui_Font_DPI; // 14 * 2

	if (size_pixels <= 0.0f)
		return;

    ImGui_ImplGL_ClearFontCache();
	
	t = FS_Search( "resource/font/*.ttf", true, false );
	if (t)
	{
		for (i = 0; i < t->numfilenames; i++)
		{
			const char* filename = t->filenames[i];
			fs_offset_t length = 0;
			auto file = FS_MapFile(filename, &length, false);
            ImFontConfig cfg = {};
            cfg.FontDataOwnedByAtlas = false;
			auto font = io.Fonts->AddFontFromMemoryTTF((void *)file, length, size_pixels, &cfg, io.Fonts->GetGlyphRangesChineseFull());
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
	imgui_pool = Mem_AllocPool("ImGui");
	ImGui::SetAllocatorFunctions(ImGui_ImplGL_OperatorNew, ImGui_ImplGL_OperatorDelete);
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
	io.RenderDrawListsFn = ImGui_ImplGL_RenderDrawLists;

#ifdef XASH_SDL
	ImGui_ImplSDL2_Init();
#endif

	ImGui_Console_Init();
	ImGui_ImeWindow_Init();
	ImGui_SprView_Init();
	ImGui_ConnectProgress_Init();
	ui::ImGui_Server_Init();
    ui::Update_Initiate();

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

#if 1
	colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
	colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.48f, 0.61f, 0.88f, 0.40f); //
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.48f, 0.61f, 0.88f, 0.67f); //
	colors[ImGuiCol_TitleBg]                = ImVec4(0.92f, 0.93f, 1.00f, 1.00f); // !
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.83f, 0.87f, 0.99f, 1.00f); // !
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.48f, 0.61f, 0.88f, 1.00f); //
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.48f, 0.61f, 0.88f, 0.78f); //
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.35f, 0.47f, 0.71f, 1.00f); //
	colors[ImGuiCol_Button]                 = ImVec4(0.48f, 0.61f, 0.88f, 0.40f); //
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.48f, 0.61f, 0.88f, 1.00f); //
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.35f, 0.47f, 0.71f, 1.00f); //
	colors[ImGuiCol_Header]                 = ImVec4(0.48f, 0.61f, 0.88f, 0.31f); //
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.48f, 0.61f, 0.88f, 0.80f); //
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.48f, 0.61f, 0.88f, 1.00f); //
	colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.48f, 0.61f, 0.88f, 0.67f); //
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.48f, 0.61f, 0.88f, 0.95f); //
	colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.90f);
	colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.35f, 0.43f, 1.00f, 1.00f); // !!
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.35f, 0.47f, 0.71f, 1.00f); // !!
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.25f, 0.45f, 0.88f, 1.00f); // !!
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.48f, 0.61f, 0.88f, 0.35f); //
	colors[ImGuiCol_DragDropTarget]         = ImVec4(0.48f, 0.61f, 0.88f, 0.95f); //
	colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
#else
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
#endif
}

void ImGui_ImplGL_NewFrame(void)
{
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
	io.FontGlobalScale = g_ImGUI_DPI / g_ImGui_Font_DPI;
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

	for(int i = 0; i < 512; ++i)
	{
		if(g_KeysJustPressed[i])
			io.KeysDown[i] = false;
	}
	std::fill(std::begin(g_KeysJustPressed), std::end(g_KeysJustPressed), false);
}

qboolean ImGui_ImplGL_MouseMove(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	g_MouseLastPos[0] = (float)x;
	g_MouseLastPos[1] = (float)y;
	return io.WantCaptureMouse;
}

qboolean ImGui_ImplGL_IsCursorVisible()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse;
}

void ImGui_ImplGL_Render(void)
{
	ImGuiIO& io = ImGui::GetIO();
	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	//pglViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//pglClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	//pglClear(GL_COLOR_BUFFER_BIT);
	//ppglUseProgramObjectARB(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
	//ImGui_ImplGL_RenderDrawLists(ImGui::GetDrawData());
    ImGui::Render();
}

void Engine_OnGUI(struct ImGuiContext *context)
{
	ImGui::SetCurrentContext(context);
	ImGui_LCSM_OnGUI();
	ImGui_Console_OnGUI();
	ImGui_ConnectProgress_OnGUI();
	ImGui_ImeWindow_OnGUI();
	ImGui_SprView_OnGUI();
	ui::ImGui_Server_OnGui();
    ui::Update_OnGui();
    ui::MsgBox_OnGui();
	if(g_bShowDemoWindow)
		ImGui::ShowDemoWindow(&g_bShowDemoWindow);
}

void ImGui_ImplGL_Client_OnGUI(void)
{
	if (clgame.dllFuncs.pfnOnGUI && cls.key_dest == key_game)
		clgame.dllFuncs.pfnOnGUI(g_EngineContext);
}

void ImGui_ImplGL_Menu_OnGUI(void)
{
	if (menu.dllFuncs.pfnOnGUI && cls.key_dest == key_menu)
		menu.dllFuncs.pfnOnGUI(g_EngineContext);
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
