/*
imgui_hlmv.h
Copyright (C) 2024 Moemod Hymei

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
#include "entity_types.h"

#include "imgui.h"
#include "imgui_utils.h"
#include "imgui_hlmv.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

class ImGui_HLMV
{
	bool enabled = false;
	std::vector<std::string> recent_open;
	GLuint rbo_depth = 0, rbo_stencil = 0, fbo = 0;
	GLuint texture = 0;
	ref_params_t refdef;
	cl_entity_t  hlmvent;
public:
	int frameWidth = 800, frameHeight = 600;

	void OpenFile(std::string abs_path)
	{
		FS_AllowDirectPaths(true);

		refdef.viewport[0] = 0;//m_scPos.x;
		refdef.viewport[1] = 0;//m_scPos.y;
		refdef.viewport[2] = frameWidth;//m_scSize.w;
		refdef.viewport[3] = frameHeight;//m_scSize.h;

		refdef.fov_x = 90.0f; // this is a final fov value
		refdef.fov_y = V_CalcFov( &cl.refdef.fov_x, refdef.viewport[2], refdef.viewport[3] );

		hlmvent = {};
		hlmvent.index = 0;
		hlmvent.curstate.body = 0;
		hlmvent.curstate.number = 1;	// IMPORTANT: always set playerindex to 1
		hlmvent.curstate.animtime = cl.time;	// start animation
		hlmvent.curstate.sequence = 1;
		hlmvent.curstate.scale = 1.0f;
		hlmvent.curstate.frame = 0.0f;
		hlmvent.curstate.framerate = 1.0f;
		hlmvent.curstate.effects |= EF_FULLBRIGHT;
		hlmvent.curstate.controller[0] = 127;
		hlmvent.curstate.controller[1] = 127;
		hlmvent.curstate.controller[2] = 127;
		hlmvent.curstate.controller[3] = 127;
		hlmvent.latched.prevcontroller[0] = 127;
		hlmvent.latched.prevcontroller[1] = 127;
		hlmvent.latched.prevcontroller[2] = 127;
		hlmvent.latched.prevcontroller[3] = 127;
		hlmvent.origin[0] = hlmvent.curstate.origin[0] = 45.0f / tan( DEG2RAD( refdef.fov_y / 2.0f ));
		hlmvent.origin[2] = hlmvent.curstate.origin[2] = 2.0f;
		hlmvent.angles[1] = hlmvent.curstate.angles[1] = 180.0f;

		hlmvent.player = false; // yes, draw me as playermodel
		hlmvent.model = Mod_ForName( abs_path.c_str(), false );
		hlmvent.curstate.eflags = EFLAG_DEPTH_CHANGED;

		FS_AllowDirectPaths(false);

		if (std::find(recent_open.begin(), recent_open.end(), abs_path) == recent_open.end())
			recent_open.emplace_back(std::move(abs_path));
		if (recent_open.size() > 5)
			recent_open.erase(recent_open.begin(), recent_open.end() - 5);
	}

	void CloseFile()
	{
		//Mod_UnloadStudioModel(hlmvent.model);
		hlmvent = {};
	}

	static void Con_HLMV_f();
	static void ImGUI_HLMV_DrawModel();

	void Init()
	{
		Cmd_AddCommand("hlmv", Con_HLMV_f, "open hlmv");
	}

	void DrawModel()
	{
		//GL_SetRenderMode(kRenderNormal);
		if( Mod_Extradata( hlmvent.model ) )
		{
			R_ClearScene();
			CL_AddVisibleEntity(&hlmvent, ET_NORMAL);
			R_RenderFrame( &refdef, false );
		}
	}

	void CreateFBO()
	{
		texture = GL_AllocateTextureSlot( "*hlmv" );
		GL_Bind(XASH_TEXTURE0, texture);
		pglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		pglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		pglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		pglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		pglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		GL_Bind(XASH_TEXTURE0, 0);

		pglGenRenderbuffers(1, &rbo_depth);
		pglBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
		pglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, frameWidth, frameHeight);
		pglBindRenderbuffer(GL_RENDERBUFFER, 0);

		pglGenRenderbuffers(1, &rbo_stencil);
		pglBindRenderbuffer(GL_RENDERBUFFER, rbo_stencil);
		pglRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, frameWidth, frameHeight);
		pglBindRenderbuffer(GL_RENDERBUFFER, 0);

		pglGenFramebuffers(1, &fbo);
		pglBindFramebuffer(GL_FRAMEBUFFER, fbo);
		pglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		pglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
		pglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_stencil);

		if(pglCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			Sys_Error("FBO create failed: %d", (int)pglCheckFramebufferStatus(GL_FRAMEBUFFER));
		}
		pglBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DrawFBO()
	{
		pglBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if(pglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		{
			pglViewport(0, 0, frameWidth, frameHeight);
			pglClearColor(0.5, 0.75, 0.75, 0.0);
			pglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			DrawModel();
		}
		pglBindFramebuffer(GL_FRAMEBUFFER, 0);
		pglViewport( 0, 0, glState.width, glState.height );
	}

	void OnGUI(void)
	{
		if (!enabled)
			return;



		if (ImGui::Begin("Half-Life Model Viewer", &enabled, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open File"))
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("HLMV - Open", "HLMV - Open", ".mdl", ".");
					}

					if (ImGui::MenuItem("Close File"))
					{
						CloseFile();
					}

					if (ImGui::BeginMenu("Open Recent"))
					{
						for (auto& file : recent_open)
						{
							if (ImGui::MenuItem(file.c_str()))
							{
								OpenFile(file);
							}
						}
						ImGui::EndMenu();
					}

					if (ImGui::MenuItem("Quit"))
					{
						enabled = false;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("View"))
				{
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
#if 1
			if(!fbo)
				CreateFBO();
			DrawFBO();
			ImGui::Image((ImTextureID)texture, ImGuiUtils::GetScaledSize(ImVec2(frameWidth, frameHeight)));
#else
			bool bDraw = false;
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			auto mins = window->DC.CursorPos;
			auto size = ImGuiUtils::GetScaledSize(ImVec2(frameWidth, frameHeight));
			ImRect bb(mins, ImVec2(mins.x + size.x, mins.y + size.y));
			ImGui::ItemSize(bb);
			if (ImGui::ItemAdd(bb, 0))
			{
				auto drawlist = ImGui::GetBackgroundDrawList();
				drawlist->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd){
					ImGUI_HLMV_DrawModel();
				}, this);
				drawlist->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			}
#endif
			ImGui::End();
		}

		if (igfd::ImGuiFileDialog::Instance()->FileDialog("HLMV - Open"))
		{
			// action if OK
			if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
			{
				std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilepathName();
				std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
				// action
				OpenFile(filePathName);
			}
			// close
			igfd::ImGuiFileDialog::Instance()->CloseDialog("HLMV - Open");
		}
	}
};

std::unique_ptr<ImGui_HLMV> g_HLMV;

void ImGui_HLMV_Init()
{
	g_HLMV = std::make_unique<ImGui_HLMV>();
	g_HLMV->Init();
}

void ImGui_HLMV_OnGUI(void)
{
	g_HLMV->OnGUI();
}

void ImGui_HLMV::Con_HLMV_f()
{
	g_HLMV->enabled = true;

	if (Cmd_Argc() >= 2)
	{
		g_HLMV->OpenFile(Cmd_Argv(1));
	}
}

void ImGui_HLMV::ImGUI_HLMV_DrawModel()
{
	g_HLMV->DrawModel();
}