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
#include "imgui_utils.h"
#include "luash.hpp"

namespace ImGuiUtils {
	struct ImVec2Alt {
		float _1, _2;
		operator ImVec2() { return { _1, _2 }; }
	};
	inline ImVec2Alt ToAlt(const ImVec2& in) { return { in.x, in.y }; }
	inline int LuaOpen_ImGuiUtils(lua_State *L)
	{
		lua_newtable(L);
		luash::Push(L, CenterNextWindow);
		lua_setfield(L, -2, "CenterNextWindow");
		luash::Push(L, GetScaledValue);
		lua_setfield(L, -2, "GetScaledValue");
		luash::Push(L, +[](ImVec2Alt in) -> ImVec2Alt { return ToAlt(GetScaledSize(in)); });
		lua_setfield(L, -2, "GetScaledSize");
		luash::Push(L, +[](ImDrawList* drawlist, ImVec2Alt center, float radius, float angle) { return AddCitrusLogo(drawlist, center, radius, angle); });
		lua_setfield(L, -2, "AddCitrusLogo");
		luash::Push(L, +[](const char* label, ImVec2Alt size_arg, float radius, float angle) { return CitrusLogo(label, size_arg, radius, angle); });
		lua_setfield(L, -2, "CitrusLogo");
		lua_setglobal(L, "imgui_utils");
		return 0;
	}
}
