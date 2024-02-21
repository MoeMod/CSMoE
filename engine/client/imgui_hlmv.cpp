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
#include "gl_texlru.h"
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
	float viewfov = 90;
	float viewaspect = 0; // TODO
	int current_event = 0;
	int current_texture = 0;
	int current_mesh = 0;
	int current_body_part = 0;
	int current_controller = 0;
	float texture_scale = 1;
	bool is_paused = false;
	bool is_play_sound = false; // TODO
	bool is_show_uv = false;
	bool is_overlap_uv = false;
	//bool is_anti_alias = false;
	bool is_show_all_meshes = true; // TODO : no UI
	bool is_draw_crosshair = false;
	bool is_draw_guidelines = false;

	enum class TabItems
	{
		ModelDisplay,
		BodyParts,
		Textures,
		Sequences,
		WeaponOrigin,
		Max,
	};
	TabItems SelectedTab = TabItems::ModelDisplay;
public:
	int frameWidth = 800, frameHeight = 480;

	void ResetDefault()
	{
		current_event = 0;
		current_texture = 0;
		current_mesh = 0;
		current_body_part = 0;
		current_controller = 0;

		texture_scale = 1;

		viewfov = 90;
		viewaspect = 0;
	}

	void OpenFile(std::string abs_path)
	{
		ResetDefault();

		FS_AllowDirectPaths(true);

		hlmvent = {};
		hlmvent.index = 1;
		hlmvent.curstate.body = 0;
		hlmvent.curstate.number = 1;	// IMPORTANT: always set playerindex to 1
		hlmvent.curstate.animtime = Sys_DoubleTime();	// start animation
		hlmvent.curstate.sequence = 0;
		hlmvent.curstate.scale = 1.0f;
		hlmvent.curstate.frame = 0.0f;
		hlmvent.curstate.framerate = 1.0f;
		hlmvent.curstate.effects |= EF_FULLBRIGHT;
		hlmvent.curstate.controller[0] = 0;
		hlmvent.curstate.controller[1] = 0;
		hlmvent.curstate.controller[2] = 0;
		hlmvent.curstate.controller[3] = 0;

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

        refdef.fov_x = viewfov; // this is a final fov value
        refdef.fov_y = V_CalcFov( &cl.refdef.fov_x, w, h );
	    if(SelectedTab == TabItems::WeaponOrigin)
	    {
		    refdef.vieworg = { -vieworg.y - 0.03f, vieworg.x - 0.02f, vieworg.z + 1.0172f };
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
		ResetDefault();
		//Mod_UnloadStudioModel(hlmvent.model);
		hlmvent = {};
	}

	static void Con_HLMV_f();
	static void ImGUI_HLMV_DrawModel();

	void Init()
	{
		Cmd_AddCommand("hlmv", Con_HLMV_f, "open hlmv");
		ResetDefault();
	}

	bool IsModelOpen() const
	{
		return Mod_Extradata( hlmvent.model );
	}

	void DrawModel()
	{
		//GL_SetRenderMode(kRenderNormal);
		if( IsModelOpen() )
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

	void OnGUI_AddSeqDesc()
	{
		auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );
		auto pseqdesc = (const mstudioseqdesc_t *)((const byte *)studiohdr + studiohdr->seqindex) + hlmvent.curstate.sequence;
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

	void OnGUI_AddSeqSel()
	{
		auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );
		auto pseqdesc = (const mstudioseqdesc_t *)((const byte *)studiohdr + studiohdr->seqindex) + hlmvent.curstate.sequence;

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
		if(ImGui::DragInt("Frame", &frame, 1, 0, pseqdesc->numframes - 1), "%d", ImGuiSliderFlags_ClampOnInput)
		{
			SetFrame(frame);
		}

		if(ImGui::Checkbox("Pause", &is_paused))
		{
			SetFrame(frame);
		}
	}

	void OnGUI_Stub()
	{

	}

	void UpdateBody()
	{

	}

	void OnGUI_BodyParts()
	{
		auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );

		ImGui::Combo("Body-part", &current_body_part, [](void* data, int idx, const char** out_text) -> bool {
			auto studiohdr = (const studiohdr_t *)data;
			auto pbodyparts = (mstudiobodyparts_t *) ((byte *) studiohdr + studiohdr->bodypartindex);
			*out_text = pbodyparts[idx].name;
			return true;
		}, (void *)studiohdr, studiohdr->numbodyparts);

		auto pbodypart = (mstudiobodyparts_t *) ((byte *) studiohdr + studiohdr->bodypartindex) + current_body_part;
		int current_sub_model = pbodypart->base > 0 ? hlmvent.curstate.body / pbodypart->base % pbodypart->nummodels : 0;
		std::string label = pbodypart->base > 0 && pbodypart->nummodels > 0 ? "Submodel " + std::to_string(current_sub_model + 1) : "";
		if (ImGui::BeginCombo("Sub-model", label.c_str()))
		{
			for (int i = 0; i < pbodypart->nummodels; i++)
			{
				const bool is_selected = (current_sub_model == i);
				std::string label = "Submodel " + std::to_string(i + 1);
				if (ImGui::Selectable(label.c_str(), is_selected))
					current_sub_model = i;
				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if(pbodypart->base > 0 && pbodypart->nummodels > 0)
		{
			hlmvent.curstate.body =
					hlmvent.curstate.body - hlmvent.curstate.body % (pbodypart->base * pbodypart->nummodels) +
					hlmvent.curstate.body % pbodypart->base +
					current_sub_model * pbodypart->base;
		}

		// update submodel

		short &current_skin = hlmvent.curstate.skin;
		label = "Skin " + std::to_string(current_skin + 1);
		if (ImGui::BeginCombo("Skin", label.c_str()))
		{
			for (int i = 0; i < studiohdr->numskinfamilies; i++)
			{
				const bool is_selected = (current_skin == i);
				std::string label = "Skin " + std::to_string(i + 1);
				if (ImGui::Selectable(label.c_str(), is_selected))
					current_skin = i;
				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		label = studiohdr->numbonecontrollers ? "Controller " + std::to_string(current_controller + 1) : "";
		if (ImGui::BeginCombo("Controller", label.c_str()))
		{
			for (int i = 0; i < studiohdr->numbonecontrollers; i++)
			{
				const bool is_selected = (current_controller == i);
				std::string label = "Controller " + std::to_string(i + 1);
				auto pbonecontroller = (mstudiobonecontroller_t *) ((byte *) studiohdr + studiohdr->bonecontrollerindex) + i;
				if(pbonecontroller->index == 4)
				{
					label = "Mouth";
				}
				if (ImGui::Selectable(label.c_str(), is_selected))
					current_controller = i;
				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if(current_controller < studiohdr->numbonecontrollers)
		{
			auto pbonecontroller = (mstudiobonecontroller_t *) ((byte *) studiohdr + studiohdr->bonecontrollerindex) + current_controller;
			byte start = pbonecontroller->start, end = pbonecontroller->end;
			ImGui::DragScalar("Value", ImGuiDataType_U8, &hlmvent.curstate.controller[pbonecontroller->index], 1, &start, &end, "%u");
		}
		ImGui::Separator();
		ImGui::Text("Textures: %d\n"
		            "Skin Families: %d\n"
		            "Bodyparts: %d\n"
		            "Attachments: %d\n"
		            "Transitions: %d\n",
		            studiohdr->numtextures,
		            studiohdr->numskinfamilies,
		            studiohdr->numbodyparts,
		            studiohdr->numattachments,
		            studiohdr->numtransitions);
		ImGui::Separator();
		ImGui::Text("Bones: %d\n"
		            "Bone Controllers: %d\n"
		            "Hit Boxes: %d\n"
		            "Sequences: %d\n"
		            "Sequence Groups: %d\n",
		            studiohdr->numbones,
		            studiohdr->numbonecontrollers,
		            studiohdr->numhitboxes,
		            studiohdr->numseq,
		            studiohdr->numseqgroups);
	}

	void OnGUI_Textures()
	{
		auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );

		bool is_chrome = false, is_additive = false, is_transparent = false, is_fullbright = false;
		if(current_texture < studiohdr->numtextures)
		{
			auto ptexture = (const mstudiotexture_t *)(((const byte *)studiohdr) + studiohdr->textureindex) + current_texture;
			auto &attr = xe::TexLru_GetAttr(ptexture);
			//ImGui::Text("Texture [ size: %d x %d ]", ptexture->width, ptexture->height);
			ImGui::Text("Texture [ size: %d x %d ]", attr.width, attr.height);

			is_chrome = attr.flags & STUDIO_NF_CHROME;
			is_additive = attr.flags & STUDIO_NF_ADDITIVE;
			is_transparent = attr.flags & STUDIO_NF_TRANSPARENT;
			is_fullbright = attr.flags & STUDIO_NF_FULLBRIGHT;
		}
		else
		{
			ImGui::Text("Texture");
		}

		ImGui::Combo("", &current_texture, [](void* data, int idx, const char** out_text) -> bool {
			auto studiohdr = (const studiohdr_t *)data;
			auto ptexture = (const mstudiotexture_t *)(((const byte *)studiohdr) + studiohdr->textureindex) + idx;
			*out_text = ptexture->name;
			return true;
		}, (void *)studiohdr, studiohdr->numtextures);


		ImGui::DragFloat("Scale", &texture_scale, 0.04, 0.0, 5.0, "%f", ImGuiSliderFlags_ClampOnInput);
		ImGui::Separator();
		ImGui::Checkbox("Chrome", &is_chrome);
		ImGui::Checkbox("Additive", &is_additive);
		ImGui::Checkbox("Transparent", &is_transparent);
		ImGui::Checkbox("FullBright", &is_fullbright);

		ImGui::Separator();
		is_show_uv |= is_overlap_uv;
		ImGui::Checkbox("Show UV Map", &is_show_uv);
		ImGui::Checkbox("Overlap UV Map", &is_overlap_uv);
		//ImGui::Checkbox("Anti-Alias Lines", &is_anti_alias);

		std::string label = "Mesh " + std::to_string(current_mesh + 1);
		if (ImGui::BeginCombo("Mesh", label.c_str()))
		{
			int k = 0;
			for (int i = 0; i < studiohdr->numbodyparts; i++)
			{
				mstudiobodyparts_t *pbodyparts = (mstudiobodyparts_t *) ((byte *) studiohdr + studiohdr->bodypartindex) + i;
				for (int j = 0; j < pbodyparts->nummodels; j++)
				{
					mstudiomodel_t *pmodels = (mstudiomodel_t *) ((byte *) studiohdr + pbodyparts->modelindex) + j;
					for (int l = 0; l < pmodels->nummesh; l++)
					{
						mstudiomesh_t *pmesh = (mstudiomesh_t *) ((byte *) studiohdr + pmodels->meshindex) + l;
						short *pskinref = (short *)((byte *)studiohdr + studiohdr->skinindex);
						if (pskinref[pmesh->skinref] == current_texture)
						{
							const bool is_selected = (current_mesh == i);
							std::string label = "Mesh " + std::to_string(++k);
							if (ImGui::Selectable(label.c_str(), is_selected))
								current_mesh = i;
							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
					}
				}
			}
			ImGui::EndCombo();
		}
	}

	void OnGUI_Sequences()
	{
		OnGUI_AddSeqDesc();
		ImGui::Separator();
		OnGUI_AddSeqSel();
		ImGui::Separator();

		{
			auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );
			auto pseqdesc = (const mstudioseqdesc_t *)((const byte *)studiohdr + studiohdr->seqindex) + hlmvent.curstate.sequence;

			std::string label = pseqdesc->numevents ? "Event #" + std::to_string(current_event) : "";
			if (ImGui::BeginCombo("Event", label.c_str()))
			{
				for (int i = 0; i < pseqdesc->numevents; i++)
				{
					const bool is_selected = (current_event == i);
					std::string label = "Event #" + std::to_string(i);
					if (ImGui::Selectable(label.c_str(), is_selected))
						current_event = i;
					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Checkbox("Play Sound", &is_play_sound);

			if(current_event < pseqdesc->numevents)
			{
				mstudioevent_t *pevent = (mstudioevent_t *) ((byte *)studiohdr + pseqdesc->eventindex) + current_event;
				ImGui::Text(
						"Frame: %d\n"
						"Event: %d\n"
						"Type: %d\n"
						"Options: %s",
						pevent->frame,
						pevent->event,
						pevent->type,
						pevent->options
				);
			}
		}
	}

	void OnGUI_WeaponOrigin()
	{
		OnGUI_AddSeqDesc();
		ImGui::Separator();
		OnGUI_AddSeqSel();
		ImGui::Separator();

		ImGui::DragFloat3("Origin", vieworg.data());
		ImGui::DragFloat("FOV", &viewfov);
		ImGui::DragFloat("Aspect", &viewaspect);

		ImGui::Separator();

		// TODO : support right hand
		//bool right_hand = Cvar_VariableInteger("cl_righthand");
		//if(ImGui::Checkbox("Right-hand", &right_hand))
		//{
		//	Cvar_Set("cl_righthand", right_hand ? "1" : "0");
		//}

		ImGui::Checkbox("Show Crosshair", &is_draw_crosshair);
		ImGui::Checkbox("Show Guidelines", &is_draw_guidelines);
	}

	// Center

	void OnGUI_DrawTexture()
	{
		// draw model
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		auto mins = window->DC.CursorPos;
		auto size = ImGui::CalcItemSize(ImVec2(-1, -1), 0, 0);
		ImRect bb(mins, ImVec2(mins.x + size.x, mins.y + size.y));
		ImGui::ItemSize(bb);
		if(ImGui::ItemAdd(bb, 0))
		{
			auto drawlist = window->DrawList;
			drawlist->AddRectFilled(bb.Min, bb.Max, ImColor(0.5f, 0.75f, 0.75f, 1.0f));

			auto studiohdr = (const studiohdr_t *)Mod_Extradata( hlmvent.model );
			if(current_texture < studiohdr->numtextures)
			{
				auto ptexture = (const mstudiotexture_t *)(((const byte *)studiohdr) + studiohdr->textureindex) + current_texture;
				auto &attr = xe::TexLru_GetAttr(ptexture);
				if(xe::TexLru_Upload(attr.index))
					xe::TexLru_UpdateAttr(ptexture);

				ImVec2 Center = bb.GetCenter();
				float w = (ImGuiUtils::GetScaledValue(attr.width) * texture_scale);
				float h = (ImGuiUtils::GetScaledValue(attr.height) * texture_scale);
				float x0 = Center.x - w / 2;
				float x1 = Center.x + w / 2;
				float y0 = Center.y - h / 2;
				float y1 = Center.y + h / 2;

				ImGui::PushClipRect(bb.Min, bb.Max, true);
				drawlist->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
				if(is_show_uv && is_overlap_uv || !is_show_uv)
				{
					drawlist->AddImage((ImTextureID) attr.index, ImVec2(x0, y0), ImVec2(x1, y1), ImVec2(0, 0),ImVec2(1, 1), IM_COL32_WHITE);
				}
				if(is_show_uv)
				{
					int index = -1;
					short *pskinref = (short *)((byte *)studiohdr + studiohdr->skinindex);

					for (int k = 0; k < studiohdr->numbodyparts; k++)
					{
						mstudiobodyparts_t *pbodyparts = (mstudiobodyparts_t *) ((byte *) studiohdr + studiohdr->bodypartindex) + k;

						for (int j = 0; j < pbodyparts->nummodels; j++)
						{
							mstudiomodel_t *pmodels = (mstudiomodel_t *) ((byte *) studiohdr + pbodyparts->modelindex) + j;

							for (int l = 0; l < pmodels->nummesh; l++)
							{
								mstudiomesh_t *pmesh = (mstudiomesh_t *) ((byte *) studiohdr + pmodels->meshindex) + l;

								++index;
								if (pskinref[pmesh->skinref] != current_texture || (current_mesh != index && !is_show_all_meshes))
									continue;

								short *ptricmds = (short *)((byte *)studiohdr + pmesh->triindex);

								float s = 1.0/(float)attr.width;
								float t = 1.0/(float)attr.height;

								int i;
								while ((i = *(ptricmds++)))
								{
									int tri_type;
									if (i < 0)
									{
										tri_type = GL_TRIANGLE_FAN;
										i = -i;
									}
									else
									{
										tri_type = GL_TRIANGLE_STRIP;
									}

									int j = 0;
									float a0 = 0;
									float b0 = 0;
									for ( ; i > 0; i--, ptricmds += 4, j++)
									{
										if(j == 0)
										{
											// N = 0
											a0 = ptricmds[2] * s * w + x0;
											b0 = ptricmds[3] * t * h + y0;
										}
										else if(j == 1)
										{
											// N = 1
											float a1 = ptricmds[2] * s * w + x0;
											float b1 = ptricmds[3] * t * h + y0;
											drawlist->AddLine(ImVec2(a0, b0), ImVec2(a1, b1), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1.0f));
										}
										else if(tri_type == GL_TRIANGLE_STRIP)
										{
											// N - 2
											float a0 = ptricmds[2 - 8] * s * w + x0;
											float b0 = ptricmds[3 - 8] * t * h + y0;
											// N - 1
											float a1 = ptricmds[2 - 4] * s * w + x0;
											float b1 = ptricmds[3 - 4] * t * h + y0;
											// N
											float a2 = ptricmds[2] * s * w + x0;
											float b2 = ptricmds[3] * t * h + y0;
											drawlist->AddLine(ImVec2(a0, b0), ImVec2(a2, b2), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1.0f));
											drawlist->AddLine(ImVec2(a1, b1), ImVec2(a2, b2), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1.0f));
										}
										else if(tri_type == GL_TRIANGLE_FAN)
										{
											// N - 1
											float a1 = ptricmds[2 - 4] * s * w + x0;
											float b1 = ptricmds[3 - 4] * t * h + y0;
											// N
											float a2 = ptricmds[2] * s * w + x0;
											float b2 = ptricmds[3] * t * h + y0;
											drawlist->AddLine(ImVec2(a0, b0), ImVec2(a2, b2), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1.0f));
											drawlist->AddLine(ImVec2(a1, b1), ImVec2(a2, b2), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1.0f));
										}
									}
								}
							}
						}
					}
				}
				ImGui::PopClipRect();
			}
		}
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

			ImVec2 Center = bb.GetCenter();
			if(is_draw_guidelines)
			{
				drawlist->AddLine(ImVec2(Center.x, Center.y + ImGuiUtils::GetScaledValue(14)), ImVec2(Center.x, bb.Max.y), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1));
				drawlist->AddLine(ImVec2(bb.Min.x + size.x * 0.085, bb.Min.y), ImVec2(bb.Min.x + size.x * 0.085, bb.Max.y), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1));
				drawlist->AddLine(ImVec2(bb.Min.x + size.x * 0.915, bb.Min.y), ImVec2(bb.Min.x + size.x * 0.915, bb.Max.y), IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1));
			}
			if(is_draw_crosshair)
			{
				float Size = 27;
				float x0 = Center.x - ImGuiUtils::GetScaledValue(Size / 2);
				float y0 = Center.y - ImGuiUtils::GetScaledValue(Size / 2);
				drawlist->AddLine(
						ImVec2(x0 + ImGuiUtils::GetScaledValue(Size / 2), y0 + ImGuiUtils::GetScaledValue(0)),
						ImVec2(x0 + ImGuiUtils::GetScaledValue(Size / 2), y0 + ImGuiUtils::GetScaledValue(Size * 0.37)),
						IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1));
				drawlist->AddLine(
						ImVec2(x0 + ImGuiUtils::GetScaledValue(Size / 2), y0 + ImGuiUtils::GetScaledValue(Size * 0.63)),
						ImVec2(x0 + ImGuiUtils::GetScaledValue(Size / 2), y0 + ImGuiUtils::GetScaledValue(Size)),
						IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1));
				drawlist->AddLine(
						ImVec2(x0 + ImGuiUtils::GetScaledValue(0), y0 + ImGuiUtils::GetScaledValue(Size / 2)),
						ImVec2(x0 + ImGuiUtils::GetScaledValue(Size * 0.37), y0 + ImGuiUtils::GetScaledValue(Size / 2)),
						IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1));
				drawlist->AddLine(
						ImVec2(x0 + ImGuiUtils::GetScaledValue(Size * 0.63), y0 + ImGuiUtils::GetScaledValue(Size / 2)),
						ImVec2(x0 + ImGuiUtils::GetScaledValue(Size), y0 + ImGuiUtils::GetScaledValue(Size / 2)),
						IM_COL32_WHITE, ImGuiUtils::GetScaledValue(1));
			}
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

			if(ImGui::BeginChild("HLMV TAB", ImVec2(ImGuiUtils::GetScaledValue(240), -1)))
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
					ImGui::SetNextItemOpen((int)SelectedTab == i && IsModelOpen(), ImGuiCond_Always);
					if(ImGui::CollapsingHeader(TabNames[i]) && IsModelOpen())
					{
						SelectedTab = (TabItems)i;

						void (ImGui_HLMV::*TabHandlers[])() = {
								&ImGui_HLMV::OnGUI_Stub,
								&ImGui_HLMV::OnGUI_BodyParts,
								&ImGui_HLMV::OnGUI_Textures,
								&ImGui_HLMV::OnGUI_Sequences,
								&ImGui_HLMV::OnGUI_WeaponOrigin,
						};
						(this->*TabHandlers[(int)SelectedTab])();
					}
				}
			}
			ImGui::EndChild();
			ImGui::SameLine();

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