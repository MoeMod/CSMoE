
#include <string_view>
#include "imgui_cl_menu.h"
#include "imgui.h"
#include "imgui_utils.h"
#include "hud.h"

#define MAX_MENU_STRING	512

extern char g_szMenuString[MAX_MENU_STRING];
extern char g_szPrelocalisedMenuString[MAX_MENU_STRING];

namespace cl {
	
	void ImGuiCL_Menu_OnGUI()
	{
		bool display = gHUD.m_Menu.m_fMenuDisplayed;
		if(display)
		{
			std::string_view menu_sv = g_szMenuString;
			
			auto nlc = std::count(menu_sv.begin(), menu_sv.end(), '\n');
			int y = ScreenHeight - 40; // make sure it is above the say text
			int x = 20;

			ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always, ImVec2(0, 0.5));
			ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255, 255).Value);
			ImGui::PushStyleColor(ImGuiCol_Border, ImColor(255, 255, 255, 0).Value);
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImColor(255, 255, 255, 0).Value);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0, 0, 0, 100).Value);
			ImGui::PushStyleColor(ImGuiCol_Button, ImColor(0.72f, 0.72f, 0.33f, 0.40f).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(0.72f, 0.72f, 0.33f, 1.00f).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(0.74f, 0.74f, 0.18f, 1.00f).Value);
			
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize;
			
			//flags |= ImGuiWindowFlags_NoMouseInputs;
			if(ImGui::Begin("HUD Menu", NULL, flags))
			{
				ImColor text_col(255, 255, 255, 255);
				auto sv = menu_sv;
				int desired_option = 1;
				bool new_line = true;
				for (auto seg = sv.find_first_of("\n\\."); seg != sv.npos; seg = sv.find_first_of("\n\\."))
				{
					if (sv[seg] == '\\' && seg != sv.size() - 1)
					{
						if(seg > 0)
						{
							ImGui::TextUnformatted(sv.data(), sv.data() + seg);
							ImGui::SameLine();
							new_line = false;
						}
						switch(sv[seg + 1])
						{
						case 'r':
							ImGui::PopStyleColor(1);
							ImGui::PushStyleColor(ImGuiCol_Text, ImColor(210, 24, 0, 255).Value);
							break;
						case 'd':
							ImGui::PopStyleColor(1);
							ImGui::PushStyleColor(ImGuiCol_Text, ImColor(100, 100, 100, 255).Value);
							break;
						case 'y':
							ImGui::PopStyleColor(1);
							ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 210, 64, 255).Value);
							break;
						case 'w':
							ImGui::PopStyleColor(1);
							ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255, 255).Value);
							break;
						}
						sv = sv.substr(seg + 2);
					}
					else if (sv[seg] == '.')
					{
						if(new_line && seg > 0 && isdigit(sv[seg - 1]))
						{
							const char str[] = { sv[seg - 1], '\0' };
							ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255, 255).Value);
							if(ImGui::Button(str))
							{
								int iSlot = sv[seg - 1] - '0';
								gHUD.m_Menu.SelectMenuItem(iSlot ? iSlot : 10);
							}
							ImGui::PopStyleColor(1);
							ImGui::SameLine();
							new_line = false;
							sv = sv.substr(seg + 1);
						}
						else
						{
							ImGui::TextUnformatted(sv.data(), sv.data() + seg + 1);
							ImGui::SameLine();
							new_line = false;
							sv = sv.substr(seg + 1);
						}
					}
					else if(sv[seg] == '\n')
					{
						ImGui::TextUnformatted(sv.data(), sv.data() + seg);
						new_line = true;
						sv = sv.substr(seg + 1);
					}
					else if (sv[seg] == '\r')
					{
						// ignored
						new_line = true;
						sv = sv.substr(seg + 1);
					}
				}
				if (!sv.empty())
				{
					new_line = true;
					ImGui::TextUnformatted(sv.data(), sv.data() + sv.length());
				}
			}
			ImGui::PopStyleVar(1);
			ImGui::PopStyleColor(7);

			ImGui::End();
		}
	}

}