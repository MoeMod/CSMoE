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

#include "GameUI.h"
#include "GameMenu.h"
#include "Utils.h"
#include "enginecallback_menu.h"
#include "IGameDialog.h"
#include "OptionsDialog.h"

#include <vector>
#include <memory>
#include <functional>
#include <imgui.h>

namespace ui {

    struct DialogEntry
    {
        std::string name;
        std::shared_ptr<IGameDialog> dialog;
        bool enabled = true;
    };

    struct GameUI::impl_t {

        explicit impl_t(GameUI *parent) :
            menu(parent)
        {}

        bool activated = false;
        GameMenu menu;
        std::vector<DialogEntry> m_vecSubDialogs;
    };

    GameUI::GameUI() : pimpl(std::make_unique<impl_t>(this)) {

    }

    GameUI::~GameUI() {

    }

    void GameUI::VidInit() {

    }

    void GameUI::OnGUI() {
        if(pimpl->activated)
        {
            OnDrawBackground();
            pimpl->menu.OnGUI();

            for(auto &entry : pimpl->m_vecSubDialogs)
            {
                if(ImGui::Begin(entry.name.c_str(), &entry.enabled, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    entry.dialog->OnGUI();
                    ImGui::End();
                }
            }
            pimpl->m_vecSubDialogs.erase(std::stable_partition(pimpl->m_vecSubDialogs.begin(), pimpl->m_vecSubDialogs.end(), std::mem_fn(&DialogEntry::enabled)), pimpl->m_vecSubDialogs.end());
        }
    }

    void GameUI::OnDrawBackground()
    {
        EngFuncs::FillRGBA(0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 255);
    }

    void GameUI::SetActivate(bool x)
    {
        pimpl->activated = x;
    }

    void GameUI::PushDialog(std::string name, std::shared_ptr<IGameDialog> dialog)
    {
        auto iter = std::find_if(pimpl->m_vecSubDialogs.begin(), pimpl->m_vecSubDialogs.end(), std::bind(std::equal_to<std::string>(), std::bind(&DialogEntry::name, std::placeholders::_1), name));
        if(iter == pimpl->m_vecSubDialogs.end())
            pimpl->m_vecSubDialogs.emplace_back(DialogEntry{ name, std::move(dialog) });
    }

    void GameUI::OpenCreateGameDialog()
    {
        // TODO
    }

    void GameUI::OpenServerBrowser()
    {
        // TODO
    }

    void GameUI::OpenOptionsDialog()
    {
        PushDialog("Options", std::make_shared<OptionsDialog>());
    }


}
