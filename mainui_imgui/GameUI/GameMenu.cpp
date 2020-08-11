/*
GameUI.cpp
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
#include <imgui_utils.h>
#include <Utils.h>
#include "GameMenu.h"

#include "enginecallback_menu.h"

namespace ui {


    void GameMenu::OnGUI()
    {
        auto& io = ImGui::GetIO();
        const auto& ds = io.DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(50, ds.y - 50), ImGuiCond_Always, ImVec2(0.0f, 1.0f));

        if(ImGui::Begin("GameMenu", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::TextUnformatted("Counter-Strike Mobile-oriented Edition");
            ImGui::NewLine();

            auto ItemSize = ImGuiUtils::GetScaledSize({360, 36});
            if (ImGui::Button("Console", ItemSize))
            {
                UI_SetActiveMenu( FALSE );
                EngFuncs::KEY_SetDest( KEY_CONSOLE );
            }
            if (ImGui::Button("New Game", ItemSize))
            {
                // TODO
            }
            if (ImGui::Button("Find Servers", ItemSize))
            {
                // TODO
            }
            if (ImGui::Button("Options", ItemSize))
            {
                // TODO
            }
            if (ImGui::Button("Quit", ItemSize))
            {
                ImGui::OpenPopup("Quit Confirm");
            }

            if(ImGui::BeginPopupModal("Quit Confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                float textWidth = ImGui::CalcItemWidth();
                ImGui::Text("Do you wish to stop playing now? ");
                ImGui::NewLine();

                if(ImGui::Button("Quit game", {textWidth / 2, 0}))
                    EngFuncs::ClientCmd(true, "quit\n");

                ImGui::SameLine();
                if(ImGui::Button("Cancel", {textWidth / 2, 0}))
                    ImGui::CloseCurrentPopup();

                ImGui::EndPopup();
            }

            ImGui::End();
        }
    }


}