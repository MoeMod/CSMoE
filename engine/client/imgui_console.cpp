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

ImColor RGBAtoImColor(rgba_t setColor)
{
	return ImColor(setColor[0], setColor[1], setColor[2], setColor[3]);
}

int ImGui_Console_AddGenericString(int x0, int y0, const char* string, rgba_t setColor)
{
	if (!ImGui::GetDrawListSharedData()->Font)
		return 0;

	TextAdjustSize( &x0, &y0, NULL, NULL);

	static auto print_segment = [](int x, int y, std::string_view sv, ImColor col)
	{
		ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
		auto shadow_col = ImColor(col.Value.x * 0.55, col.Value.y * 0.34, col.Value.z * 0.11, col.Value.w);
		auto shadow_col2 = ImColor(0.f, 0.f, 0.f, col.Value.w);
		drawlist->AddText(ImVec2(x - 1, y - 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x - 1, y + 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x + 1, y - 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x + 1, y + 1), shadow_col, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x + 2, y + 2), shadow_col2, sv.data(), sv.data() + sv.length());
		drawlist->AddText(ImVec2(x, y), col, sv.data(), sv.data() + sv.length());
	};

	static auto text_size = [](std::string_view sv) { return ImGui::CalcTextSize(sv.data(), sv.data() + sv.length()); };

	extern rgba_t g_color_table[8];

	std::string_view sv = string;
	ImColor col = RGBAtoImColor(setColor);
	ImColor last_color = col;
	int x = x0, y = y0;
	auto find_str = "^\n\x01\x02\x03\x04\x05\x06\x07";
	for (auto seg = sv.find_first_of(find_str); seg != sv.npos; seg = sv.find_first_of(find_str))
	{
		if (sv[seg] == '^' && seg != sv.size() - 1 && sv[seg + 1] >= '1' && sv[seg + 1] <= '7')
		{
			print_segment(x, y, sv.substr(0, seg), last_color);
			auto size = text_size(sv.substr(0, seg));
			x += size.x;

			if (sv[seg + 1] == '7')
				last_color = col;
			else
				last_color = RGBAtoImColor(g_color_table[sv[seg + 1] - '0']);

			sv = sv.substr(seg + 2);
			continue;
		}
		else if (sv[seg] >= '\x01' && sv[seg] <= '\x07' && seg != sv.size() - 1)
		{
			print_segment(x, y, sv.substr(0, seg), last_color);
			auto size = text_size(sv.substr(0, seg));
			x += size.x;
			// ignored
			/*
			if (sv[seg] == '\x07')
				last_color = col;
			else
				last_color = RGBAtoImColor(g_color_table[sv[seg] - '\x00']);
			*/
			sv = sv.substr(seg + 1);
			continue;
		}
		else if (sv[seg] == '\n' && seg != sv.size() - 1)
		{
			print_segment(x, y, sv.substr(0, seg), last_color);
			auto size = text_size(sv.substr(0, seg));
			y += size.y;
			x = x0;
			sv = sv.substr(seg + 1);
			continue;
		}
		break;
	}
	if (!sv.empty())
	{
		print_segment(x, y, sv, last_color);
		auto size = text_size(sv);
		x += size.x;
	}

	//float xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	return x - x0;
}

void ImGui_Console_DrawStringLen(const char* pText, int* length, int* height)
{
	ImVec2 size = {};
	std::string filter_str;
	for(const char* p = pText; *p; ++p)
	{
		if (p[0] == '^' && p[1] >= '1' && p[1] <= '7')
			p += 2;
		if (p[0] >= '\x01' && p[0] <= '\x07')
			p += 1;
		filter_str.push_back(*p);
	}
	if (ImGui::GetDrawListSharedData()->Font)
	{
		size = ImGui::CalcTextSize(filter_str.c_str(), filter_str.c_str() + filter_str.length());
	}
	if (length) *length = size.x;
	if (height) *height = size.y;
	TextAdjustSizeReverse(NULL, NULL, length, height);
}

int ImGui_Console_DrawChar(int x, int y, int ch, rgba_t setColor)
{
	ImFont* font = ImGui::GetDrawListSharedData()->Font;
	if (!font)
		return 0;
	TextAdjustSize(&x, &y, NULL, NULL);

	ImColor col = RGBAtoImColor(setColor);
	auto pos = ImVec2(x, y);
	
	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	drawlist->PushTextureID(font->ContainerAtlas->TexID);
	ImGuiIO& io = ImGui::GetIO();
	font->RenderChar(drawlist, font->FontSize * io.FontGlobalScale, pos, col, ch);
		
	int w = font->GetCharAdvance(ch) * io.FontGlobalScale; // / io.FontGlobalScale;
	TextAdjustSizeReverse(NULL, NULL, &w, NULL);

	return w;
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

		if (!s_term.show({  }, &enabled))
		{
			Key_SetKeyDest(key_menu);
		}

		if (set_focus)
			s_term.set_should_take_focus(false);
	}

	if (cls.key_dest == key_game)
		enabled = false;
}