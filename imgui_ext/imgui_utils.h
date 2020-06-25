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
#include "minmax.h"

namespace ImGuiUtils {
	inline void CenterNextWindow(ImGuiCond cond = 0) {
		auto& io = ImGui::GetIO();
		const auto& ds = io.DisplaySize;
		ImGui::SetNextWindowPos(ImVec2(ds.x / 2, ds.y / 2), cond, ImVec2(0.5f, 0.5f));
	}
	inline ImVec2 GetScaledSize(ImVec2 in)
	{
		auto& io = ImGui::GetIO();
		float scale = std::max(1.0f, io.FontGlobalScale * 2);
		return { in.x * scale, in.y * scale };
	}
}