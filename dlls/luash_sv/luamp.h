#pragma once
#include "custom.h"

typedef struct lua_State lua_State;

namespace sv
{
	void LuaMP_Init();
	void LuaMP_Shutdown();
	lua_State* LuaCL_Get();
	void LuaMP_Exec(const char* str);

	int LuaMP_GlobalRequire(lua_State* L);
	int LuaMP_GlobalPrint(lua_State* L);
}