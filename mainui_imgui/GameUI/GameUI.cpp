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

namespace ui {

    struct GameUI::impl_t {

        explicit impl_t(GameUI *parent) :
            menu(parent)
        {}

        bool activated = false;
        GameMenu menu;
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


}
