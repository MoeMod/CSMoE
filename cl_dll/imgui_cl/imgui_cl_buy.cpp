/*
imgui_cl_buy.cpp
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "hud.h"
#include "imgui.h"
#include "imgui_cl_buy.h"

struct CImGuiBuyMenu::impl_t {
    bool draw = true;
};

CImGuiBuyMenu::CImGuiBuyMenu() : pimpl(std::make_unique<impl_t>()) {

}

CImGuiBuyMenu::~CImGuiBuyMenu() = default;

namespace ImGuiUtils {
    void CenterNextWindow(ImGuiCond cond = 0) {
        auto &io = ImGui::GetIO();
        const auto &ds = io.DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(ds.x / 2, ds.y / 2), cond, ImVec2(0.5f, 0.5f));
    }
    ImVec2 GetScaledSize(ImVec2 in)
    {
        auto &io = ImGui::GetIO();
        float scale = std::max(1.0f, io.FontGlobalScale * 2);
        return {in.x * scale, in.y * scale};
    }
}

namespace ImGuiWpn{
    auto WeaponSection(const char *name)
    {
        return ImGui::Selectable(name, false, 0, ImGuiUtils::GetScaledSize({0, 36}));
    }
}

void CImGuiBuyMenu::OnGUI() {
    if(!pimpl->draw)
        return;

    ImGuiUtils::CenterNextWindow();
    ImGui::SetNextWindowSize(ImGuiUtils::GetScaledSize(ImVec2(640, 480)));

    //ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.0f, 0.5f});

    ImGui::Begin("购买武器");
    ImGui::Text("测试版本");
    if (ImGui::CollapsingHeader("手枪"))
    {
        ImGuiWpn::WeaponSection("1. USP45");
        ImGuiWpn::WeaponSection("2. Glock");
        ImGuiWpn::WeaponSection("3. P228");
        ImGuiWpn::WeaponSection("4. 沙漠之鹰");
        ImGuiWpn::WeaponSection("5. FiveSeven");
        ImGuiWpn::WeaponSection("6. 双持Elites");
    }
    if (ImGui::CollapsingHeader("散弹枪"))
    {
        ImGuiWpn::WeaponSection("1. M3");
        ImGuiWpn::WeaponSection("2. XM1014");
    }
    if (ImGui::CollapsingHeader("冲锋枪"))
    {
        ImGuiWpn::WeaponSection("1. TMP");
        ImGuiWpn::WeaponSection("2. MP5");
        ImGuiWpn::WeaponSection("3. UMP45");
        ImGuiWpn::WeaponSection("4. P90");
    }
    if (ImGui::CollapsingHeader("自动步枪"))
    {
        ImGuiWpn::WeaponSection("1. FAMAS");
        ImGuiWpn::WeaponSection("2. M4A1");
        ImGuiWpn::WeaponSection("3. AUG");
        ImGuiWpn::WeaponSection("4. SG550");
        ImGuiWpn::WeaponSection("5. AWP");
    }
    if (ImGui::CollapsingHeader("机枪"))
    {
        ImGuiWpn::WeaponSection("1. M249");
    }
    ImGuiWpn::WeaponSection("主武器弹药");
    ImGuiWpn::WeaponSection("副武器弹药");
    if (ImGui::CollapsingHeader("装备"))
    {
        ImGuiWpn::WeaponSection("1. 防弹衣");
        ImGuiWpn::WeaponSection("2. 防弹衣+防弹头盔");
        ImGuiWpn::WeaponSection("3. 闪光弹");
        ImGuiWpn::WeaponSection("4. 高爆手雷");
        ImGuiWpn::WeaponSection("5. 烟雾弹");
        ImGuiWpn::WeaponSection("6. 拆弹器");
    }
    if (ImGui::CollapsingHeader("近身武器"))
    {
        ImGuiWpn::WeaponSection("1. 海豹短刀");
    }
    if (ImGui::Selectable("关闭"))
        pimpl->draw = false;

    //ImGui::PopStyleVar(ImGuiStyleVar_SelectableTextAlign);

    ImGui::End();
}
