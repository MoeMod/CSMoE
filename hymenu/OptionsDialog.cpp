/*
OptionsDialog.cpp
Copyright (C) 2020 Moemod Hymei
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <imgui.h>
#include "imgui_utils.h"
#include "OptionsDialog.h"

#include <string_view>

#include "Utils.h"

namespace ui {

    static bool visible = false;

    void OptionsDialog_Init()
    {
	    
    }

    void OptionsDialog_SetVisible(bool x)
    {
        visible = x;
    }

    void OptionsDialog_OnGui()
    {
        if (!visible)
            return;
        ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
    	if(ImGui::Begin("游戏设置", &visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
    	{
            if (ImGui::BeginTabBar("OptionsDialog.TabBar"))
            {
                if (ImGui::BeginTabItem("多人运动"))
                {
                    ImGui::TextUnformatted("玩家名称");
                    char buf[32] = {};
                    strncpy(buf, EngFuncs::GetCvarString("name"), 32);
                    if(ImGui::InputText("name", buf, 32, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiInputTextCallbackData* data)
                    {
                        return (int)UI::Names::CheckIsNameValid(data->Buf);
                    }))
                    {
                        EngFuncs::CvarSetString("name", buf);
                    }
                    ImGui::TextUnformatted("准星外观");
                	
                    int cl_crosshair_type = EngFuncs::GetCvarFloat("cl_crosshair_type");
                    if(ImGui::Combo("cl_crosshair_type", &cl_crosshair_type, "十字型\0点射型\0圆形\0组合型\0单点\0"))
                    {
                        EngFuncs::CvarSetValue("cl_crosshair_type", cl_crosshair_type);
                    }
                	
                    std::string_view cl_crosshair_size = EngFuncs::GetCvarString("cl_crosshair_size");
                    const char *cl_crosshair_size_values[4] = {
                        "auto",
                        "small",
                        "medium",
                        "large"
                    };
                    const char* cl_crosshair_size_names[] = { "自动调整", "小", "中等", "大" };
                    int cl_crosshair_size_idx = std::clamp(static_cast<int>(std::find(cl_crosshair_size_values, cl_crosshair_size_values + 4, cl_crosshair_size) - cl_crosshair_size_values), 0, 3);

                    if (ImGui::SliderInt("cl_crosshair_size", &cl_crosshair_size_idx, 0, 3, cl_crosshair_size_names[cl_crosshair_size_idx]))
                    //if (ImGui::Combo("cl_crosshair_size", &cl_crosshair_size_idx, "自动调整\0小\0中等\0大\0"))
                    {
                        EngFuncs::CvarSetString("cl_crosshair_size", cl_crosshair_size_values[cl_crosshair_size_idx]);
                    }

                    int rgb[3] = {255,255,255};
                    sscanf(EngFuncs::GetCvarString("cl_crosshair_color"), "%d %d %d", rgb, rgb + 1, rgb + 2);

                    static ImColor color(rgb[0], rgb[1], rgb[2], 255);
                    if(ImGui::ColorEdit3("cl_crosshair_color", (float*)&color))
                    {
                        rgb[0] = color.Value.x * 255.0f;
                        rgb[1] = color.Value.y * 255.0f;
                        rgb[2] = color.Value.z * 255.0f;
                        char buffer[32];
                        sprintf(buffer, "%d %d %d", rgb[0], rgb[1], rgb[2]);
                        EngFuncs::CvarSetString("cl_crosshair_color", buffer);
                    }

                	bool cl_crosshair_translucent = EngFuncs::GetCvarFloat("cl_crosshair_translucent");
                    if(ImGui::Checkbox("准星透明", &cl_crosshair_translucent))
                    {
                        EngFuncs::CvarSetValue("cl_crosshair_translucent", cl_crosshair_translucent);
                    }
                    ImGui::SameLine(ImGui::CalcItemWidth());
                    ImGui::TextUnformatted("cl_crosshair_translucent");
                	
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("键盘"))
                {
                    ImGui::TextUnformatted("键盘 Page");
                    // TODO
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("鼠标"))
                {
                    ImGui::TextUnformatted("鼠标 Page");
                    float sensitivity_value = EngFuncs::GetCvarFloat("sensitivity");
                    if (ImGui::SliderFloat("鼠标灵敏度", &sensitivity_value, 0.0f, 20.0f))
                    {
                        EngFuncs::CvarSetValue("sensitivity", sensitivity_value);
                    }
                    // TODO
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("音频"))
                {
                    ImGui::TextUnformatted("音频 Page");
                    float volume_value = EngFuncs::GetCvarFloat("volume");
                    if (ImGui::SliderFloat("声音效果音量", &volume_value, 0.0f, 1.0f))
                    {
                        EngFuncs::CvarSetValue("volume", volume_value);
                    }

                    float MP3Volume_value = EngFuncs::GetCvarFloat("MP3Volume");
                    if (ImGui::SliderFloat("MP3音量", &MP3Volume_value, 0.0f, 1.0f))
                    {
                        EngFuncs::CvarSetValue("MP3Volume", MP3Volume_value);
                    }

                    // TODO
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("视频"))
                {
                    ImGui::TextUnformatted("视频 Page");   
                    float gamma_value = EngFuncs::GetCvarFloat("gamma");               
                    if (ImGui::SliderFloat("亮度", &gamma_value, 1.8f, 3.0f))
                    {
                        EngFuncs::CvarSetValue("gamma", gamma_value);
                    }

                    float brightness_value = EngFuncs::GetCvarFloat("brightness");
                    if (ImGui::SliderFloat("灰度（需要重启游戏）", &brightness_value, 0.0f, 2.0f))
                    {
                        EngFuncs::CvarSetValue("brightness", brightness_value);
                    }
                    // TODO
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("声音"))
                {
                    ImGui::TextUnformatted("声音 Page");
                    // TODO
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("锁定"))
                {
                    ImGui::TextUnformatted("锁定 Page");
                    // TODO
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
            ImGui::End();
    	}
        

    }

}