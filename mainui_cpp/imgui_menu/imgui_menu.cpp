
#include <imgui.h>
#include <imgui_utils.h>

#include "imgui_menu_msgbox.h"
#include "imgui_menu_server.h"
#include "imgui_menu_update.h"

#include "luash_menu/luash_menu.h"
namespace ui{
void UI_OnGUI(struct ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);

	MsgBox_OnGui();
	Server_OnGui();
	Update_OnGui();
	LuaUI_OnGUI();
}
}