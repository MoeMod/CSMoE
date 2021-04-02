#pragma once

typedef struct lua_State lua_State;

namespace cl
{
	void LuaCL_HookEvents(void);
	void LuaCL_OnPrecacheEvent(const char* name, int index);
	int LuaCL_OpenEventScripts(lua_State* L);
	
}