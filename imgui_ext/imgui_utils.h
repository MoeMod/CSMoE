/*
imgui_utils.h
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

#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "minmax.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace ImGuiUtils {
	inline void CenterNextWindow(ImGuiCond cond = 0) {
		auto& io = ImGui::GetIO();
		const auto& ds = io.DisplaySize;
		ImGui::SetNextWindowPos(ImVec2(ds.x / 2, ds.y / 2), cond, ImVec2(0.5f, 0.5f));
	}
	inline float GetScaledValue(float in)
	{
		auto& io = ImGui::GetIO();
		float scale = std::max(1.0f, io.FontGlobalScale * 2);
		return in * scale;
	}
	inline ImVec2 GetScaledSize(ImVec2 in)
	{
		auto& io = ImGui::GetIO();
		float scale = std::max(1.0f, io.FontGlobalScale * 2);
		return { in.x * scale, in.y * scale };
	}

	inline bool CitrusLogo(const char *label, const ImVec2& size_arg, float radius = 0, float angle = 0)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;
		auto drawlist = window->DrawList;

		auto mins = window->DC.CursorPos;
		const ImGuiID id = window->GetID(label);

		const ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 size = ImGui::CalcItemSize(size_arg, style.WindowMinSize.x / 2.0f, style.WindowMinSize.y / 2.0f);

		ImRect bb(mins, ImVec2(mins.x + size.x, mins.y + size.y));
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, id))
			return false;
		
		ImVec2 center = ImVec2((mins.x + size.x / 2), (mins.y + size.y / 2));
		radius = (radius ? std::min({ size.x, size.y, radius }) : std::min(size.x, size.y)) / 2;
		float thickness = radius / 12;
		drawlist->AddCircleFilled(center, radius, ImColor(255, 124, 34, 255));

		{
			drawlist->PathLineTo(center);
			drawlist->PathLineTo(ImVec2(center.x + radius * cos((angle + 22.5f) / 180 * M_PI), center.y - radius * sin((angle + 22.5f) / 180 * M_PI)));
			drawlist->PathLineTo(ImVec2(center.x + radius * cos((angle + 45.0f) / 180 * M_PI), center.y - radius * sin((angle + 45.0f) / 180 * M_PI)));
			drawlist->PathLineTo(ImVec2(center.x + radius * cos((angle + 67.5f) / 180 * M_PI), center.y - radius * sin((angle + 67.5f) / 180 * M_PI)));
			drawlist->PathFillConvex(ImColor(101, 254, 0, 255));
		}
		for(int i = 0; i < 4; ++i)
		{
			drawlist->AddLine(
				ImVec2(center.x + radius * cos((angle + 22.5f + i * 45.0) / 180 * M_PI), center.y - radius * sin((angle + 22.5f + i * 45.0) / 180 * M_PI)),
				ImVec2(center.x + radius * cos((angle + 202.5f + i * 45.0) / 180 * M_PI), center.y - radius * sin((angle + 202.5f + i * 45.0) / 180 * M_PI)),
				ImColor(255, 255, 255, 255),
				thickness / 2
			);
		}
		drawlist->AddLine(
			ImVec2(center.x + radius * 0.25 * cos((angle + 45.0f) / 180 * M_PI), center.y - radius * 0.25 * sin((angle + 45.0f) / 180 * M_PI)),
			ImVec2(center.x + radius * 0.30 * cos((angle + 45.0f) / 180 * M_PI), center.y - radius * 0.30 * sin((angle + 45.0f) / 180 * M_PI)),
			ImColor(255, 255, 255, 180),
			thickness / 2
		);
		drawlist->AddCircle(center, radius, ImColor(255, 255, 255, 255), 0, thickness);

		return true;
	}
}
