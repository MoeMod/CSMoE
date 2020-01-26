/*
imgui_cl_main.cpp
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

#include "imgui.h"
#include "hud.h"
#include "cl_util.h"
#include "cl_dll.h"
#include "imgui_cl_buy.h"

namespace cl {

extern "C"
void DLLEXPORT HUD_OnGUI( struct ImGuiContext *context )
{

    static CImGuiBuyMenu buy;
    ImGui::SetCurrentContext(context);
	/*
	static bool show_demo_window = false;
	static float f = 0.0f;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	static bool show_main_window = false;
	static bool show_debug_window = false;

	ImGui::Begin("测试窗口");
	ImGui::Text("测试下中文显示");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	ImGui::ColorEdit3("clear color", (float*)&clear_color);
	if (ImGui::Button("Demo 窗口"))
		show_demo_window ^= 1;
	if (ImGui::Button("测试窗口"))
		show_debug_window ^= 1;
	ImGui::Text("平均 %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	if (show_debug_window)
	{
		ImGui::Text("Hello World!");

	}

	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&show_demo_window);
	}
	 */

    buy.OnGUI();
}

}