#include "extdll_menu.h"
#include "imgui.h"
#include "imgui_utils.h"
#include "Utils.h"

int UI_VidInit(void)
{
	return 0;
}
void UI_Init(void)
{

}
void UI_Shutdown(void)
{

}
void UI_UpdateMenu(float flTime)
{

}
void UI_KeyEvent(int key, int down)
{

}
void UI_MouseMove(int x, int y)
{

}
void UI_SetActiveMenu(int fActive)
{
	EngFuncs::KEY_ClearStates();
	if (fActive)
	{
		EngFuncs::KEY_SetDest(KEY_MENU);
	}
}
void UI_AddServerToList(netadr_t adr, const char* info)
{

}
void UI_GetCursorPos(int* pos_x, int* pos_y)
{

}
void UI_SetCursorPos(int pos_x, int pos_y)
{

}
void UI_ShowCursor(int show)
{
	
}
void UI_CharEvent(int key)
{

}
int UI_MouseInRect(void)
{
	return 0;
}
int UI_IsVisible(void)
{
	return 0;
}
int UI_CreditsActive(void)
{
	return 0;
}
void UI_FinalCredits(void)
{

}
extern "C" qboolean CL_IsInMenu(void);
void UI_OnGUI(struct ImGuiContext* context)
{
	if(CL_IsInMenu())
	{
		bool open = true;
		bool x = false;
		if (CL_IsActive())
		{
			ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
			EngFuncs::FillRGBA(0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 50);
			x = ImGui::Begin("柑橘CitruS", &open, ImGuiWindowFlags_NoResize);
		}
		else
		{
			ImGuiUtils::CenterNextWindow(ImGuiCond_Appearing);
			EngFuncs::FillRGBA(0, 0, ScreenWidth, ScreenHeight, 255, 255, 255, 255);
			x = ImGui::Begin("柑橘CitruS", NULL, ImGuiWindowFlags_NoResize);
		}
		if (x)
		{
			if (CL_IsActive() && ImGui::Button("返回游戏", ImVec2(-1, 0)))
			{
				open = false;
			}
			if (CL_IsActive() && ImGui::Button("断开连接", ImVec2(-1, 0)))
			{
				EngFuncs::ClientCmd(false, "disconnect\n");
			}
			if (ImGui::Button("打开控制台", ImVec2(-1, 0)))
			{
				EngFuncs::KEY_SetDest(KEY_CONSOLE);
			}
			if (ImGui::Button("退出游戏", ImVec2(-1, 0)))
			{
				EngFuncs::ClientCmd(false, "quit\n");
			}
			ImGui::End();
		}
		if(!open)
		{
			EngFuncs::KEY_SetDest(KEY_GAME);
		}
	}
}
extern "C" EXPORT void AddTouchButtonToList(const char* name, const char* texture, const char* command, unsigned char* color, int flags)
{

}