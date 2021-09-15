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
extern "C" {
#include "common.h"
#include "crtlib.h"
#include "eiface.h"
}

#include <algorithm>

#define JM_XORSTR_DISABLE_AVX_INTRINSICS
#include <xorstr.hpp>

static bool lcsm_enabled = true;

static bool CheckIsNameValid(const char* name)
{
	if (!name || !*name)
		return false;

	// exclude some default names, that may be set from engine or just come with config files
	static struct
	{
		const char* name;
		bool substring;
	} prohibitedNames[] =
	{
	{ "default", false, },
	{ "unnamed", false, },
	{ "Player", false, },
	{ "<Warrior> Player", false, },
	{ "Shinji", false, },
	{ "CSDuragiCOM", true },
	{ "Nero Claudius", true }, // *purrrt* you found a secret area!
	{ "CSWGE", true },
	};

	for (size_t i = 0; i < ARRAYSIZE(prohibitedNames); i++)
	{
		if (prohibitedNames[i].substring)
		{
			if (strstr(name, prohibitedNames[i].name))
			{
				return false;
			}
		}
		else
		{
			if (!strcmp(name, prohibitedNames[i].name))
			{
				return false;
			}
		}
	}

	return true;
}

void ImGui_LCSM_OnGUI(void)
{
	if (!lcsm_enabled)
		return;

	ImGuiUtils::CenterNextWindow(ImGuiCond_Always);
	ImGui::OpenPopup(xorstr("柑橘 CSMoE LCSM 公告").crypt_get());
	if (ImGui::BeginPopupModal(xorstr("柑橘 CSMoE LCSM 公告").crypt_get(), &lcsm_enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text(xorstr("Counter-Strike Mobile-oriented Edition (aka CSMoE or CSBTE-Mobile)").crypt_get());
		ImGui::Text(xorstr("版权所有：BTE Team|CSMoE Team|百度csoldjb吧").crypt_get());
		ImGui::Text(xorstr("本游戏采用GPLv3协议完全开源免费，请勿使用本游戏进行任何形式的盈利").crypt_get());
		ImGui::Text(xorstr("声明：任何提供收费服务的商家均非官方，我们不对非官方版本提供任何技术担保。").crypt_get());
		ImGui::Text(xorstr("\t如出现任何意外（包括但不限于死机、蓝屏、锁机、ban号、中毒、变砖、爆炸等）后果自负。").crypt_get());
		ImColor warning_color(1.0f, 0.2f, 0.2f, 1.0f);
		ImGui::TextColored(warning_color, xorstr("-----------------------------------特别声明---------------------------------").crypt_get());
		ImGui::TextColored(warning_color, xorstr("1.EC精英社所谓“CS破茧”“CS神器时代”均为盗版游戏，我们强烈谴责盗版游戏的行为").crypt_get());
		ImGui::TextColored(warning_color, xorstr("2.Wilson（真名：刘澄）恶意申请原CS:BTE游戏著作权并加密游戏后利用著作权在淘宝盈利").crypt_get());
		ImGui::TextColored(warning_color, xorstr("淘宝店铺：洋葱数码商城|淘宝名：丨低调奢华有内涵|B站ID：进口柚柚子|贴吧ID:liu1340567980").crypt_get());
		ImGui::TextColored(warning_color, xorstr("现在此揭露刘澄：一个完全的插件小白，靠网上东拼西凑代码，或直接搬运外网的更新加入游戏。").crypt_get());
		ImGui::TextColored(warning_color, xorstr("但因其持有原CSBTE版权我们无法维权。希望广大网友擦亮眼睛，不要被无耻小人骗走金钱与信任！").crypt_get());
		ImGui::TextColored(warning_color, xorstr("3.CSMoE所使用的素材均为热心网友整理而来，仅供学习、交流与欣赏，请于下载后24小时内删除！").crypt_get());
		ImGui::TextColored(warning_color, xorstr("----------------------------------------------------------------------------").crypt_get());
		ImColor tipssss_color(0.3f, 0.6f, 0.2f, 1.0f);
		ImGui::TextColored(tipssss_color, xorstr("----------------------------------------------------------------------------").crypt_get());
		ImGui::TextColored(tipssss_color, xorstr("柑橘CSMoE现已开设联机服供大家体验，您可以加群706711420体验，或点击下方按钮进服~~~").crypt_get());
		ImGui::TextColored(tipssss_color, xorstr("----------------------------------------------------------------------------").crypt_get());
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

		ImGui::TextUnformatted(xorstr("请修改您的名称后再进服联机：").crypt_get());
		char buf[32] = {};
		strncpy(buf, Cvar_VariableString("name"), 32);
		ImGui::SameLine();
		if (ImGui::InputText("name", buf, 32, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiInputTextCallbackData* data)
			{
				return (int)CheckIsNameValid(data->Buf);
			}))
		{
			Cvar_Set("name", buf);
		}

		ImGui::NewLine();
		ImGui::SameLine(ImGui::GetWindowSize().x * 1 / 4 - 80);
		if (ImGui::Button(xorstr("进入柑橘联机服").crypt_get(), ImGuiUtils::GetScaledSize({ 160, 36 }))) {
			lcsm_enabled = false;
			Cbuf_AddText(xorstr("connect z4cs.com:5555\n").crypt_get());
		}
		ImGui::SameLine(ImGui::GetWindowSize().x / 2 + 80);
		if (ImGui::Button(xorstr("退出游戏").crypt_get(), ImGuiUtils::GetScaledSize({ 160, 36 }))) {
			exit(0);
		}
		ImGui::EndPopup();
	}
}