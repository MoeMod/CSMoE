#pragma once

typedef struct lua_State lua_State;

namespace ui
{
	void LuaUI_Init();
	void LuaUI_Shutdown();
	lua_State* LuaUI_Get();
	void LuaUI_Exec(const char* str);
	
	int LuaUI_GlobalReload(lua_State* L);
	int LuaUI_GlobalRequire(lua_State* L);
	int LuaUI_GlobalPrint(lua_State* L);

	void LuaUI_OnGUI();

	int LuaUI_OpenFileSystemLib(lua_State* L);
	int LuaUI_OpenEngfuncLib(lua_State* L);
	int LuaUI_OpenTextfuncLib(lua_State* L);
}