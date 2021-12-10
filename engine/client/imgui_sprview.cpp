/*
imgui_sprview.cpp
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

#define HSPRITE WINAPI_HSPRITE

#include "imgui.h"
#include "imgui_utils.h"
#include "imgui_sprview.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

#undef HSPRITE

class ImGui_SprView
{
	bool enabled = false;
	std::vector<std::string> recent_open;
	HSPRITE g_spr = 0;
	int cur_frame = 0;
	float last_update;
	float framerate = 15;
	bool playing = true;
	bool loop = true;
public:
	void OpenFile(std::string abs_path)
	{
		FS_AllowDirectPaths(true);

		g_spr = pfnSPR_Load(abs_path.c_str());

		FS_AllowDirectPaths(false);

		cur_frame = 0;
		last_update = 0;

		if (std::find(recent_open.begin(), recent_open.end(), abs_path) == recent_open.end())
			recent_open.emplace_back(std::move(abs_path));
		if (recent_open.size() > 5)
			recent_open.erase(recent_open.begin(), recent_open.end() - 5);
	}

	static void Con_SprView_f();

	void Init()
	{
		Cmd_AddCommand("sprview", Con_SprView_f, "open sprview");
	}

	void OnGUI(void)
	{
		if (!enabled)
			return;

		int spr = g_spr;
		int frameWidth = 0, frameHeight = 0, numFrames = 0;
		int texture = 0;
		if (spr)
		{
			model_t* pSprite = &clgame.sprites[spr];
			R_GetSpriteParms(&frameWidth, &frameHeight, &numFrames, cur_frame, pSprite);
			texture = R_GetSpriteTexture(pSprite, cur_frame);

			if (!texture)
			{
				fs_offset_t size;
				byte* buf = FS_LoadFile(pSprite->name, &size, false);

				qboolean loaded;
				Mod_LoadSpriteModel(pSprite, buf, &loaded, 0);
			}
		}

		if (ImGui::Begin("SprView", &enabled, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open File"))
					{
						igfd::ImGuiFileDialog::Instance()->OpenModal("SprView - Open", "SprView - Open", ".spr", ".");
					}

					if (ImGui::MenuItem("Close File"))
					{
						g_spr = 0;
						cur_frame = 0;
						last_update = 0;
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
					ImGui::MenuItem("Playing", "", &playing);
					ImGui::MenuItem("Loop", "", &loop);
					ImGui::DragFloat("Framerate", &framerate, 0.2f, 1.0f, std::numeric_limits<float>::max(), "%.0f fps");

					ImGui::SliderInt("Current Frame", &cur_frame, 0, numFrames - 1);

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			if (spr)
			{
				auto [x1, y1] = ImGui::GetWindowContentRegionMin();
				auto [x2, y2] = ImGui::GetWindowContentRegionMax();

				if (playing && ImGui::GetTime() > last_update + 1 / framerate)
				{
					if(loop)
						cur_frame = ++cur_frame % numFrames;
					else
						cur_frame = std::min(cur_frame + 1, numFrames - 1);

					last_update = ImGui::GetTime();
				}

				if(texture)
					ImGui::Image((ImTextureID)texture, ImGuiUtils::GetScaledSize(ImVec2(frameWidth, frameHeight)));
			}

			ImGui::End();
		}
		

		if (igfd::ImGuiFileDialog::Instance()->FileDialog("SprView - Open"))
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
			igfd::ImGuiFileDialog::Instance()->CloseDialog("SprView - Open");
		}
	}
};

std::unique_ptr<ImGui_SprView> g_SprView;

void ImGui_SprView_Init()
{
	g_SprView = std::make_unique<ImGui_SprView>();
	g_SprView->Init();
}

void ImGui_SprView_OnGUI(void)
{
	g_SprView->OnGUI();
}

void ImGui_SprView::Con_SprView_f()
{
	g_SprView->enabled = true;

	if (Cmd_Argc() >= 2)
	{
		g_SprView->OpenFile(Cmd_Argv(1));
	}
}