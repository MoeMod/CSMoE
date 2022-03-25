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
#include "QRCode.h"

#include "common.h"
#include "crtlib.h"
#include "eiface.h"
#include "gl_local.h"

#include <vector>
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
		ImGui::Text(xorstr("柑橘 CSMoE v22 Counter-Strike Mobile-oriented Edition (原名 CSBTE-Mobile)").crypt_get());
		ImGui::Text(xorstr("版权所有：BTE Team|CSMoE Team|百度csoldjb吧|死神CS社区|Faith Zone|幻夜CS社区|柑橘CitruS").crypt_get());
		ImColor warning_color(1.0f, 0.2f, 0.2f, 1.0f);
		ImGui::TextColored(warning_color, xorstr("1.EC精英社所谓“CS破茧”“CS神器时代”“CS创新纪元”均为盗版游戏，我们强烈谴责盗版游戏的行为").crypt_get());
		ImGui::TextColored(warning_color, xorstr("2.进口柚柚子（刘澄）恶意申请原CS:BTE游戏著作权并加密游戏后利用著作权在淘宝盈利并举报B站视频").crypt_get());
		ImGui::TextColored(warning_color, xorstr("3.CS01服务器 利用CSMoE支持中文、小地图作为客户端，用网上偷来的插件收费开服，望各位玩家周知。").crypt_get());
		ImGui::TextColored(warning_color, xorstr("详情见B站Monster-egg 2021-6-19动态，CSMoE未在应用市场发布，如橘子CSMoE、饕餮、恶煞、王炸均为").crypt_get());
		ImGui::TextColored(warning_color, xorstr("第三方改版，我们不提供这些版本的支持，CSMoE将坚决抵制换膜、开服、倒卖的行为。").crypt_get());

		ImGui::BeginGroup();
		ImGui::NewLine();
		ImGui::SameLine(0, ImGuiUtils::GetScaledValue(80));
		ImGui::BeginGroup();
		ImColor tipssss_color(0.3f, 0.6f, 0.2f, 1.0f);
		ImGui::TextColored(tipssss_color, xorstr("-----------------------------------------------").crypt_get());
		ImGui::TextColored(tipssss_color, xorstr("柑橘 CSMoE 现已开设免费公益的官方联机服供大家体验").crypt_get());
		ImGui::TextColored(tipssss_color, xorstr("您可以加群706711420和爱好者们交流（右边扫码进群）").crypt_get());
		ImGui::TextColored(tipssss_color, xorstr("或者直接点击下方按钮进服击剑~~~").crypt_get());
		ImGui::TextColored(tipssss_color, xorstr("-----------------------------------------------").crypt_get());
		ImGui::EndGroup();

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
		ImGui::SameLine(ImGuiUtils::GetScaledValue(40));
		char buf[32] = {};
		strncpy(buf, Cvar_VariableString("name"), 32);
		if (ImGui::InputText(xorstr("<- 请修改名字后再进服联机").crypt_get(), buf, 32, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiInputTextCallbackData* data)
		{
			return (int)CheckIsNameValid(data->Buf);
		}))
		{
			Cvar_Set("name", buf);
		}

		ImGui::NewLine();
		ImGui::SameLine(ImGuiUtils::GetScaledValue(80));
		if (ImGui::Button(xorstr("进入柑橘联机服").crypt_get(), ImGuiUtils::GetScaledSize({ 320, 36 }))) {
			lcsm_enabled = false;
			Cbuf_AddText(xorstr("citrus_servers\n").crypt_get());
		}

		ImGui::EndGroup();
		ImGui::SameLine(0, ImGuiUtils::GetScaledValue(50));

		static ImTextureID qrTexture = 0;
		if(!qrTexture)
		{
			qrcode::QRCode s_qrcode;
			std::vector<uint8_t> s_qrCodeBytes;
			s_qrCodeBytes.resize(qrcode::getBufferSize(9));
			qrcode::initText(&s_qrcode, s_qrCodeBytes.data(), 9, qrcode::ECC_LOW, xorstr("https://qm.qq.com/cgi-bin/qm/qr?k=m-wI8acAT7XurfGiy_2JZy9DBupc1tXe&authKey=ZgiTHa6sHJjpkKqMLE8XQH31pHL1gFGF4mOi3xH5zW6D169YuTGbKJqcRI0B7cVX").crypt_get());

			auto	r_image = Image_NewTemp();
			char	texName[32];

			Q_strncpy( texName, xorstr("*citrus_qrcode").crypt_get(), sizeof( texName ) );

			static std::vector<ImU32> pixels(s_qrcode.size * s_qrcode.size, ImColor(255, 255, 255, 255));

			for (std::uint8_t y = 0; y < s_qrcode.size; y++) {
				for (std::uint8_t x = 0; x < s_qrcode.size; x++) {
					if(qrcode::getModule(&s_qrcode, x, y))
						pixels[x * s_qrcode.size + y] = ImColor(0, 0, 0, 255);
				}
			}

			r_image->width = s_qrcode.size;
			r_image->height = s_qrcode.size;
			r_image->depth = 4;
			r_image->type = PF_RGBA_32;
			r_image->size = r_image->width * r_image->height * r_image->depth;
			r_image->flags = IMAGE_HAS_COLOR|IMAGE_HAS_ALPHA|IMAGE_TEMP;
			r_image->buffer = (byte *)pixels.data();

			qrTexture = (void *)(ptrdiff_t)GL_LoadTextureInternal( xorstr("*citrus_qrcode").crypt_get(), r_image, (texFlags_t)(TF_IMAGE | TF_NEAREST), false );
		}

		ImGui::Image(qrTexture, ImGuiUtils::GetScaledSize({ 160, 160 }));
		ImGui::EndPopup();
	}
}