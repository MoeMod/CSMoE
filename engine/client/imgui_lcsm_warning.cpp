/*
imgui_lcsm_warning.cpp
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

#include "imgui_lcsm_warning.h"
#include "imgui.h"
#include "imgui_utils.h"

#include <algorithm>

static bool lcsm_enabled = true;

void ImGui_LCSM_OnGUI(void)
{
	if (!lcsm_enabled)
		return;

	ImGuiUtils::CenterNextWindow(ImGuiCond_Always);
	ImGui::OpenPopup("CSMoE LCSM Warning");
	if (ImGui::BeginPopupModal("CSMoE LCSM Warning", &lcsm_enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Counter-Strike Mobile-oriented Edition (aka CSMoE or CSBTE-Mobile)");
		ImGui::Text("版权所有：BTE Team|CSMoE Team|百度csoldjb吧");
		ImGui::Text("本游戏采用GPLv3协议完全开源免费，请勿使用本游戏进行任何形式的盈利");
		ImGui::Text("声明：任何提供收费服务的商家均非官方，我们不对非官方版本提供任何技术担保。");
		ImGui::Text("\t如出现任何意外（包括但不限于死机、蓝屏、锁机、ban号、中毒、变砖、爆炸等）后果自负。");
		ImColor warning_color(1.0f, 0.2f, 0.2f, 1.0f);
		ImGui::TextColored(warning_color, "-----------------------------------特别声明---------------------------------");
		ImGui::TextColored(warning_color, "1.EC精英社所谓“CS破茧”“CS神器时代”均为盗版游戏，我们强烈谴责盗版游戏的行为");
		ImGui::TextColored(warning_color, "2.Wilson（真名：刘澄）恶意申请原CS:BTE游戏著作权并加密游戏后利用著作权在淘宝盈利");
		ImGui::TextColored(warning_color, "淘宝店铺：洋葱数码商城|淘宝名：丨低调奢华有内涵|B站ID：进口柚柚子|贴吧ID:liu1340567980");
		ImGui::TextColored(warning_color, "现在此揭露刘澄：一个完全的插件小白，靠网上东拼西凑代码，或直接搬运外网的更新加入游戏。");
		ImGui::TextColored(warning_color, "但因其持有原CSBTE版权我们无法维权。希望广大网友擦亮眼睛，不要被无耻小人骗走金钱与信任！");
		ImGui::TextColored(warning_color, "3.CSMoE所使用的素材均为热心网友整理而来，仅供学习、交流与欣赏，请于下载后24小时内删除！");
		ImGui::TextColored(warning_color, "----------------------------------------------------------------------------");
		/*
		ImGui::Text("请在下方输入：「我同意」代表您已经明确以上声明。");
		ImGui::Text("否则请按下“退出”按钮退出游戏");

		static char buf[256];
		static bool enabled = 0;
		ImGui::InputTextWithHint("    ", "我同意", buf, sizeof(buf), 0, [](ImGuiInputTextCallbackData *data){
			return 0;
		});

		if(ImGui::Selectable("进入游戏", false, !!strcmp(buf, "我同意") ? ImGuiSelectableFlags_Disabled : 0, ImGuiUtils::GetScaledSize({0, 36})))
		 */
		ImGui::NewLine();
		ImGui::SameLine(ImGui::GetWindowSize().x * 1 / 4 - 80);
		if (ImGui::Button("进入游戏", ImGuiUtils::GetScaledSize({ 160, 36 }))) {
			lcsm_enabled = false;
		}
		ImGui::SameLine(ImGui::GetWindowSize().x / 2 + 80);
		if (ImGui::Button("退出游戏", ImGuiUtils::GetScaledSize({ 160, 36 }))) {
			exit(0);
		}
		ImGui::EndPopup();
	}
}