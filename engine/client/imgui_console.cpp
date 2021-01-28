/*
imgui_console.cpp
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

#include "imgui_console.h"
#include "imgui.h"
#include "imgui_utils.h"

extern "C" {
#include "client.h"
}

#include "minmax.h"
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <system_error>

#include "imterm/terminal.hpp"
#include "imterm/terminal_helpers.hpp"

extern "C" { extern convar_t* cvar_vars; }
typedef struct cmd_s
{
	char* name; // must be first, to match cvar_t
	struct cmd_s* next;
	xcommand_t	 function;
	char* desc;
	int          flags;
} cmd_t;

struct TerminalHelper {

	using value_type = std::string; // < Mandatory, this type will be passed to commands via argument_type
	using term_t = ImTerm::terminal<TerminalHelper>;
	using command_type = ImTerm::command_t<ImTerm::terminal<TerminalHelper>>;
	using argument_type = ImTerm::argument_t<ImTerm::terminal<TerminalHelper>>;
	using command_type_cref = std::reference_wrapper<const command_type>;

	std::vector<command_type_cref> find_commands_by_prefix(std::string_view prefix) {
		if (prefix.empty())
			return {};

		static command_type current;
		current = { prefix, "", CommandCallback, CompleteNullCallback };

		std::vector<command_type_cref> ret;
		for (auto first = m_Commands.lower_bound({ prefix }); first != m_Commands.end(); ++first)
		{
			if (first->name.find(prefix) != 0)
				break;
			ret.emplace_back(std::cref(*first));
		}
		ret.emplace_back(std::cref(current));
		return ret;
	}

	std::vector<command_type_cref> find_commands_by_prefix(const char* beg, const char* end) {
		return find_commands_by_prefix(std::string_view(beg, std::distance(beg, end)));
	}

	static void CommandCallback(ImTerm::argument_t<term_t>& arg)
	{
		std::ostringstream oss;
		std::copy(arg.command_line.begin(), arg.command_line.end(), std::ostream_iterator<value_type>(oss, " "));
		Cbuf_AddText(oss.str().c_str());
	}

	static std::vector<std::string> CompleteCvarCallback(ImTerm::argument_t<term_t>& arg)
	{
		convar_t* var = Cvar_FindVar(arg.command_line.front().c_str());
		std::vector<std::string> ret;
		if (var)
		{
			if (var->string)
				ret.emplace_back(var->string);
			if (var->reset_string)
				ret.emplace_back(var->reset_string);
		}

		return ret;
	}

	static std::vector<std::string> CompleteCmdCallback(ImTerm::argument_t<term_t>& arg)
	{
		return {};
	}

	static std::vector<std::string> CompleteNullCallback(ImTerm::argument_t<term_t>& arg)
	{
		return {};
	}

	void update_commands_from_xash()
	{
		for (convar_t* var = cvar_vars; var; var = var->next)
		{
			const char* name = var->name;
			const char* desc = ""; // TODO : var->description is not initialized
			m_Commands.insert({ name, desc, CommandCallback, CompleteCvarCallback });
		}

		for (cmd_t* cmd = Cmd_GetFirstFunctionHandle(); cmd; cmd = cmd->next)
		{
			const char* name = cmd->name;
			const char* desc = cmd->desc ? cmd->desc : "";
			m_Commands.insert({ name, desc, CommandCallback, CompleteCmdCallback });
		}
	}

	std::vector<command_type_cref> list_commands() {
		return { m_Commands.begin(), m_Commands.end() };
	}

	std::set<command_type> m_Commands;

	std::optional<ImTerm::message> format(std::string str, [[maybe_unused]] ImTerm::message::type msg_type) {
		ImTerm::message msg;
		msg.value = std::move(str);
		msg.color_beg = msg.color_end = 0u;
		return { std::move(msg) };
	}
};

static std::string s_term_arg;
static ImTerm::terminal<TerminalHelper> s_term(s_term_arg, "CSMoE 控制台");

void ImGui_Console_UpdateCommandList()
{
	s_term.get_terminal_helper()->update_commands_from_xash();
}

void ImGui_Console_Init()
{
	ImGui_Console_UpdateCommandList();

	s_term.theme().log_level_colors[ImTerm::message::severity::trace] = { 255,   0,   0, 255 }; // ^1
	s_term.theme().log_level_colors[ImTerm::message::severity::debug] = { 0, 255,   0, 255 }; // ^2
	s_term.theme().log_level_colors[ImTerm::message::severity::info] = { 255, 255,   0, 255 }; // ^3
	s_term.theme().log_level_colors[ImTerm::message::severity::warn] = { 0,   0, 255, 255 }; // ^4
	s_term.theme().log_level_colors[ImTerm::message::severity::err] = { 0, 255, 255, 255 }; // ^5
	s_term.theme().log_level_colors[ImTerm::message::severity::critical] = { 255,   0, 255, 255 }; // ^6
	// ^7 for normal
}

void ImGui_Console_PrintSegment(std::string_view sv, const ImTerm::message::severity::severity_t *color = nullptr)
{
	if (color)
	{
		ImTerm::message msg;
		msg.severity = *color;
		msg.value = sv;
		msg.color_beg = 0;
		msg.color_end = sv.size();
		msg.is_term_message = false;

		s_term.add_message(std::move(msg));
	}
	else
	{
		s_term.add_text(std::string(sv));
	}
}

void ImGui_Console_Print(const char* txt)
{
	std::string_view sv = txt;
	static ImTerm::message::severity::severity_t color_var;
	ImTerm::message::severity::severity_t *last_color = nullptr;
	for (auto seg = sv.find_first_of("^\n"); seg != sv.npos; seg = sv.find_first_of("^\n"))
	{
		if (sv[seg] == '^' && seg != sv.size() - 1)
		{
			if (sv[seg + 1] >= '1' && sv[seg + 1] <= '7')
			{
				ImGui_Console_PrintSegment(sv.substr(0, seg), last_color);

				if (sv[seg + 1] == '7')
					last_color = nullptr;
				else
					last_color = &(color_var = static_cast<ImTerm::message::severity::severity_t>(sv[seg + 1] - '1'));

				sv = sv.substr(seg + 2);
				continue;
			}
			else
			{
				ImGui_Console_PrintSegment(sv.substr(0, seg), last_color);
				sv = sv.substr(seg + 1);
				continue;
			}
		}
		else if (sv[seg] == '\n' && seg != sv.size() - 1)
		{
			ImGui_Console_PrintSegment(sv.substr(0, seg), last_color);
			ImGui_Console_PrintSegment({}, last_color); // \n

			sv = sv.substr(seg + 1);
			continue;
		}
		break;
	}
	if (!sv.empty())
	{
		ImGui_Console_PrintSegment(sv, last_color);
	}
	ImGui_Console_PrintSegment({}, last_color); // \n
}

void ImGui_Console_Clear()
{
	s_term.clear();
}

static bool enabled = false;

void ImGui_ToggleConsole(qboolean x)
{
	enabled = x;
}

std::vector<std::function<void()>> pfnCallbackOnGUI;

int ImGui_Console_AddGenericString(int x, int y, const char* string, rgba_t setColor)
{
	if (!ImGui::GetDrawListSharedData()->Font)
		return 0;
	pfnCallbackOnGUI.push_back([x, y, setColor, stdstr = std::string(string)]()
		{
			auto col = ImColor(setColor[0], setColor[1], setColor[2], setColor[3]);
			auto shadow_col = ImColor(int(setColor[0] * 0.55), int(setColor[1] * 0.34), int(setColor[2] * 0.11), setColor[3]);
			auto shadow_col2 = ImColor(0, 0, 0, setColor[3]);
			ImDrawList* drawlist = ImGui::GetForegroundDrawList();
			drawlist->AddText(ImVec2(x - 1, y - 1), shadow_col, stdstr.c_str());
			drawlist->AddText(ImVec2(x - 1, y + 1), shadow_col, stdstr.c_str());
			drawlist->AddText(ImVec2(x + 1, y - 1), shadow_col, stdstr.c_str());
			drawlist->AddText(ImVec2(x + 1, y + 1), shadow_col, stdstr.c_str());
			drawlist->AddText(ImVec2(x + 2, y + 2), shadow_col2, stdstr.c_str());
			drawlist->AddText(ImVec2(x, y), col, stdstr.c_str());
		});
	return static_cast<int>(ImGui::CalcTextSize(string).x);
}

void ImGui_Console_DrawStringLen(const char* pText, int* length, int* height)
{
	ImVec2 size = {};
	if (ImGui::GetDrawListSharedData()->Font)
	{
		size = ImGui::CalcTextSize(pText);
	}
	if (length) *length = size.x;
	if (height) *height = size.y;
}

void ImGui_Console_OnGUI(void)
{

	bool set_focus = false;
	//if(!std::exchange(enabled, cls.key_dest == key_console))
	//	s_term.set_should_take_focus(set_focus = true);

	if (enabled)
	{
		if (set_focus)
		{
			ImGuiUtils::CenterNextWindow(ImGuiCond_Always);
			//ImGui::SetNextWindowSize(ImGuiUtils::GetScaledSize(ImVec2(640, 480)), ImGuiCond_Always);
			auto size = ImGuiUtils::GetScaledSize(ImVec2(640, 480));
			s_term.set_width(size.x);
			s_term.set_height(size.y);
		}

		if (!s_term.show({  }, &enabled) || !enabled)
		{
			if (cls.state == ca_active && !cl.background)
				Key_SetKeyDest(key_game);
			else UI_SetActiveMenu(true);
		}

		if (set_focus)
			s_term.set_should_take_focus(false);
	}
	
	for (const auto& f : pfnCallbackOnGUI)
		f();
	pfnCallbackOnGUI.clear();
}