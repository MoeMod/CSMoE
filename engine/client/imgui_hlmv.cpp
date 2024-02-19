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
#include "studio.h"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

class ImGui_HLMV
{
	bool enabled = false;
	std::vector<std::string> recent_open;
	GLuint texture = 0;
	ref_params_t refdef;
	cl_entity_t  hlmvent;
	vec3_t vieworg;
	bool is_paused = false;

	enum class TabItems
	{
		ModelDisplay,
		BodyParts,
		Textures,
		Sequences,
		WeaponOrigin,
		Max,
	};
	TabItems SelectedTab;
public:
	int frameWidth = 800, frameHeight = 600;

	void OpenFile(std::string abs_path)
	{
		FS_AllowDirectPaths(true);

		hlmvent = {};
		hlmvent.index = 0;
		hlmvent.curstate.body = 0;
		hlmvent.curstate.number = 1;	// IMPORTANT: always set playerindex to 1
		hlmvent.curstate.animtime = Sys_DoubleTime();	// start animation
		hlmvent.curstate.sequence = 0;
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

		hlmvent.player = false; // yes, draw me as playermodel
		hlmvent.model = Mod_ForName( abs_path.c_str(), false );
		hlmvent.curstate.eflags = EFLAG_DEPTH_CHANGED;

		FS_AllowDirectPaths(false);

		if (std::find(recent_open.begin(), recent_open.end(), abs_path) == recent_open.end())
			recent_open.emplace_back(std::move(abs_path));
		if (recent_open.size() > 5)
			recent_open.erase(recent_open.begin(), recent_open.end() - 5);
	}

    void UpdateRefdef(int x, int y, int w, int h)
    {
        refdef.viewport[0] = x;
        refdef.viewport[1] = y;
        refdef.viewport[2] = w;
        refdef.viewport[3] = h;

	    refdef.time = host.newtime;
	    cl.refdef.frametime = host.frametime;

        refdef.fov_x = 90.0f; // this is a final fov value
        refdef.fov_y = V_CalcFov( &cl.refdef.fov_x, w, h );
	    if(SelectedTab == TabItems::WeaponOrigin)
	    {
		    refdef.vieworg = { -vieworg.x - 0.03f, vieworg.y - 0.02f, vieworg.z + 1.0172f };
		    refdef.viewangles = { 0.0f, 0.0f, 0.0f };
	    }
	    else
	    {
		    refdef.vieworg = { 45.0f, 0.0f, 2.0f };
		    refdef.viewangles = { 0.0f, 180.0f, 0.0f };
	    }
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
			float save_framerate = hlmvent.curstate.framerate;
			if(is_paused)
			{
				hlmvent.curstate.framerate = 0;
			}

			R_ClearScene();
			CL_AddVisibleEntity(&hlmvent, ET_NORMAL);
			R_RenderFrame( &refdef, false );

			CheckLoopPlay();

			if(is_paused)
			{
				hlmvent.curstate.framerate = save_framerate;
			}
		}
	}

	void SetFrame(float n)
	{
		hlmvent.curstate.animtime = Sys_DoubleTime();
		hlmvent.curstate.frame = n;
	}

	float GetFrame()
	{
		if(is_paused)
		{
			return hlmvent.curstate.frame;
		}

		auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );
		auto pseqdesc = (const mstudioseqdesc_t *)((const byte *)studiohdr + studiohdr->seqindex) + hlmvent.curstate.sequence;

		extern float R_StudioEstimateFrame( cl_entity_t *e, mstudioseqdesc_t *pseqdesc );
		float frame = R_StudioEstimateFrame(&hlmvent, (mstudioseqdesc_t *)pseqdesc);
		return frame;
	}

	void CheckLoopPlay()
	{
		auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );
		auto pseqdesc = (const mstudioseqdesc_t *)((const byte *)studiohdr + studiohdr->seqindex) + hlmvent.curstate.sequence;

		double dfdt = (RI.refdef.time - hlmvent.curstate.animtime) * hlmvent.curstate.framerate * pseqdesc->fps;
		double f = (hlmvent.curstate.frame * (pseqdesc->numframes - 1)) / 256.0;

		if(f + dfdt >= pseqdesc->numframes)
		{
			SetFrame(0);
		}
	}

	void OnGUI_Stub()
	{

	}

	void OnGUI_WeaponOrigin()
	{
		auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );
		auto pseqdesc = (const mstudioseqdesc_t *)((const byte *)studiohdr + studiohdr->seqindex) + hlmvent.curstate.sequence;
		ImGui::BeginGroup();
		{
			ImGui::BeginGroup();
			{
				bool sequence_changed = ImGui::Combo("Sequence", &hlmvent.curstate.sequence, [](void* data, int idx, const char** out_text) -> bool {
					auto studiohdr = (const studiohdr_t *)data;
					auto pseqdesc = (const mstudioseqdesc_t *)((const byte *)studiohdr + studiohdr->seqindex) + idx;
					*out_text = pseqdesc->label;
					return true;
				}, (void *)studiohdr, studiohdr->numseq);
				if(sequence_changed)
				{
					SetFrame(0);
				}

				float fps = hlmvent.curstate.framerate * pseqdesc->fps;
				if(ImGui::DragFloat("Speed", &fps, 1.0, 0.0, 120.0, "%.2f fps", ImGuiSliderFlags_Logarithmic))
				{
					hlmvent.curstate.framerate = fps / pseqdesc->fps;
				}

				int frame = GetFrame();
				if(ImGui::DragInt("Frame", &frame, 1, 0, pseqdesc->numframes - 1))
				{
					SetFrame(frame);
				}

				if(ImGui::Checkbox("Pause", &is_paused))
				{
					SetFrame(frame);
				}
			}
			ImGui::EndGroup();
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::Text(
				"Sequence#: %d\n"
				"Frames: %d\n"
				"FPS: %d\n"
				"Blends: %d\n"
				"# of events: %d\n",
				hlmvent.curstate.sequence,
				pseqdesc->numframes,
				(int)pseqdesc->fps,
				pseqdesc->numblends,
				pseqdesc->numevents
		);
	}

	// Center

	void OnGUI_DrawTexture()
	{

	}

	void OnGUI_DrawModel()
	{
		// draw model
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		auto mins = window->DC.CursorPos;
		auto size = ImGui::CalcItemSize(ImVec2(-1, -1), 0, 0);
		ImRect bb(mins, ImVec2(mins.x + size.x, mins.y + size.y));
		ImGui::ItemSize(bb);
		if(ImGui::ItemAdd(bb, 0))
		{
			UpdateRefdef(mins.x, mins.y, size.x, size.y);
			auto drawlist = window->DrawList;
			drawlist->AddRectFilled(bb.Min, bb.Max, ImColor(0.5f, 0.75f, 0.75f, 1.0f));
			drawlist->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd){
				ImGUI_HLMV_DrawModel();
			}, this);
			drawlist->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
		}
	}

	// All

	void OnGUI(void)
	{
		if (!enabled)
			return;

        ImGui::SetNextWindowSize(ImGuiUtils::GetScaledSize(ImVec2(frameWidth, frameHeight)), ImGuiCond_Appearing);
		if (ImGui::Begin("Half-Life Model Viewer", &enabled, ImGuiWindowFlags_MenuBar))
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

			if(ImGui::BeginTabBar("HLMV TAB"))
			{
				const char *TabNames[] = {
						"Model Display",
						"Body Parts",
						"Textures",
						"Sequences",
						"Weapon Origin"
				};
				for(int i = 0; i < (int)TabItems::Max; ++i)
				{
					if(ImGui::BeginTabItem(TabNames[i]))
					{
						SelectedTab = (TabItems)i;

						void (ImGui_HLMV::*TabHandlers[])() = {
								&ImGui_HLMV::OnGUI_Stub,
								&ImGui_HLMV::OnGUI_Stub,
								&ImGui_HLMV::OnGUI_Stub,
								&ImGui_HLMV::OnGUI_Stub,
								&ImGui_HLMV::OnGUI_WeaponOrigin,
						};
						(this->*TabHandlers[(int)SelectedTab])();

						ImGui::EndTabItem();
					}
				}
			}
			ImGui::EndTabBar();

			if(SelectedTab == TabItems::Textures)
			{
				OnGUI_DrawTexture();
			}
			else
			{
				OnGUI_DrawModel();
			}

		}
		ImGui::End();

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