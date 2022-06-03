/*
QuitConfirm.cpp
Copyright (C) 2021 Moemod Hymei
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

#include <string>
#include <string_view>
#include <functional>

#include "common.h"
#include "crtlib.h"
#include "eiface.h"
#include "gl_local.h"

#include <vector>
#include <algorithm>

namespace ui {
    static std::string s_msg;
    static std::function<void()> s_command;
    static bool s_next_open = false;
    static float s_last_update;

    void MsgBox_Open(std::string msg, std::function<void()> command, float flCloseDelay)
    {
        s_msg = std::move(msg);
        s_command = std::move(command);
        s_next_open = true;
        s_last_update = ImGui::GetTime() + std::move(flCloseDelay);
    }

    void MsgBox_OnGui()
    {
        if (std::exchange(s_next_open, false))
            ImGui::OpenPopup("柑橘 CSMoE 提示");
        if (ImGui::BeginPopupModal("柑橘 CSMoE 提示", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("%s\n\n", s_msg.c_str());
            ImGui::Separator();
            if (s_last_update <= ImGui::GetTime())
            {
                if (s_command)
                {
                    if (ImGui::Button("确认", ImGuiUtils::GetScaledSize(ImVec2(120, 0)))) { s_command(); }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("取消", ImGuiUtils::GetScaledSize(ImVec2(120, 0)))) { ImGui::CloseCurrentPopup(); }

                    if (ImGui::IsKeyPressed(ImGuiKey_Enter)) { s_command(); }
                    else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { ImGui::CloseCurrentPopup(); }
                }
                else
                {
                    ImGui::SetItemDefaultFocus();
                    if (ImGui::Button("确认", ImGuiUtils::GetScaledSize(ImVec2(120, 0)))) { ImGui::CloseCurrentPopup(); }
                    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter)) { ImGui::CloseCurrentPopup(); }
                }
            }
            ImGui::EndPopup();
        }
    }

}