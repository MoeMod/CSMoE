#pragma once
#include "custom.h"

typedef struct lua_State lua_State;

namespace cl
{
	void LuaCL_Init();
	void LuaCL_Shutdown();
	lua_State* LuaCL_Get();
	void LuaCL_Exec(const char* str);
	
	int LuaCL_GlobalRequire(lua_State* L);
	int LuaCL_GlobalPrint(lua_State* L);

	void LuaCL_OnPrecache(resourcetype_t type, const char* name, int index);
}