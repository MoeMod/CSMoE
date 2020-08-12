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
#include "OptionsDialog.h"

namespace ui {

    void OptionsDialog::OnGUI()
    {
        if(ImGui::BeginTabBar("OptionsDialog.TabBar"))
        {

            if(ImGui::BeginTabItem("Multiplayer"))
            {
                ImGui::TextUnformatted("Multiplayer Page");
                // TODO
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Keyboard"))
            {
                ImGui::TextUnformatted("Keyboard Page");
                // TODO
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Mouse"))
            {
                ImGui::TextUnformatted("Mouse Page");
                // TODO
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Audio"))
            {
                ImGui::TextUnformatted("Audio Page");
                // TODO
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Video"))
            {
                ImGui::TextUnformatted("Video Page");
                // TODO
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Voice"))
            {
                ImGui::TextUnformatted("Voice Page");
                // TODO
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Lock"))
            {
                ImGui::TextUnformatted("Lock Page");
                // TODO
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

    }

}

