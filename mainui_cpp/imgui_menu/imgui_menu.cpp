
#include <imgui.h>
#include <imgui_utils.h>

#include "imgui_menu_options.h"
#include "imgui_menu_msgbox.h"

#include "luash_menu/luash_menu.h"

void UI_OnGUI(struct ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);

	ui::OptionsDialog_OnGui();
	ui::MsgBox_OnGui();

	ui::LuaUI_OnGUI();
}