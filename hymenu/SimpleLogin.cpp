/*
SimpleLogin.cpp
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
#include "SimpleLogin.h"

#include "extdll_menu.h"
#include "Utils.h"

namespace ui {

	static bool visible = false;
	static char input_buffer[32] = "";
	static char chat_cmd[32];
	static char desc[128];
	static char button_text[32] = "确认";

	extern "C" int UI_HandleMessageMode_f(void)
	{
		if (!strcmp(EngFuncs::CmdArgv(1), "login"))
		{
			visible = true;
			
			strcpy(desc, "请输入您的账号和密码登录游戏。\n如果您不知道密码，请换个名字进入游戏重新注册或者联系管理员找回密码。");
			strcpy(chat_cmd, EngFuncs::CmdArgv(1));
			strcpy(button_text, "登录");
			EngFuncs::KEY_SetDest(KEY_MENU);
			return 1;
		}
		if (!strcmp(EngFuncs::CmdArgv(1), "register"))
		{
			visible = true;

			strcpy(desc, "欢迎来到柑橘CitruS游戏服务器\n本服务器需要玩家注册账号，请在下方输入您的账号密码进行注册。");
			strcpy(chat_cmd, EngFuncs::CmdArgv(1));
			strcpy(button_text, "注册");
			EngFuncs::KEY_SetDest(KEY_MENU);
			return 1;
		}
		if (!strcmp(EngFuncs::CmdArgv(1), "change"))
		{
			visible = true;

			strcpy(desc, "请输入你要修改的密码两次：");
			strcpy(chat_cmd, EngFuncs::CmdArgv(1));
			strcpy(button_text, "修改");
			EngFuncs::KEY_SetDest(KEY_MENU);
			return 1;
		}
		return 0;
	}

	void SimpleLogin_Init()
	{
		
	}
	
	void SimpleLogin_OnGui()
	{
		if (!visible)
		{
			input_buffer[0] = '\0';
			return;
		}
		const char* title = "柑橘CitruS 账号系统";
		ImGui::OpenPopup(title);
		ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
		if (ImGui::BeginPopupModal(title, &visible))
		{
			ImGuiUtils::CitrusLogo("cirtus logo", ImGuiUtils::GetScaledSize(ImVec2(-1, 128)), ImGuiUtils::GetScaledValue(128 * 0.75));

			ImGui::TextUnformatted(desc);
			ImGui::Separator();
			ImGui::TextUnformatted("账号：");
			ImGui::TextDisabled("%s", EngFuncs::GetCvarString("name"));
			ImGui::Spacing();
			ImGui::TextUnformatted("密码：");
			ImGui::InputText(chat_cmd, input_buffer, sizeof input_buffer, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);
			ImGui::Spacing();
			ImGui::Separator();
			if(ImGui::Button(button_text, ImGuiUtils::GetScaledSize(ImVec2(120, 0))))
			{
				visible = false;
				char buffer[128];
				snprintf(buffer, sizeof(buffer), "%s %s\n", chat_cmd, input_buffer);
				EngFuncs::ClientCmd(false, buffer);
				input_buffer[0] = '\0';
				EngFuncs::KEY_SetDest(KEY_GAME);
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine(0.0f, -1.0f);
			if(ImGui::Button("断开连接", ImGuiUtils::GetScaledSize(ImVec2(120, 0))))
			{
				visible = false;
				EngFuncs::ClientCmd(false, "disconnect\n");
				input_buffer[0] = '\0';
			}

			ImGui::EndPopup();
		}
	}
}
