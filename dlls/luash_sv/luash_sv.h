#pragma once

#include <map>
#include <string>

typedef struct lua_State lua_State;

namespace sv
{
	class CBaseEntity;

	void LuaSV_Init();
	void LuaSV_Shutdown();
	lua_State* LuaSV_Get();
	void LuaSV_Exec(const char* str);

	int LuaSV_GlobalReload(lua_State* L);
	int LuaSV_GlobalRequire(lua_State* L);
	int LuaSV_GlobalPrint(lua_State* L);

	int LuaSV_OpenFileSystemLib(lua_State* L);
	int LuaSV_OpenDllfuncLib(lua_State* L);

	template<class T> void LuaSetupRefTypeInterface(lua_State* L, CBaseEntity* ptr);
	template<class T> CBaseEntity* LuaCreateClassPtr(lua_State* L, entvars_t* pev);

	extern std::map<std::string, void (*)(lua_State* L, CBaseEntity* ptr)> g_pfnLuaSetupRefTypeInterfaceMap;
	extern std::map<std::string, CBaseEntity* (*)(lua_State* L, entvars_t* pev)> g_pfnLuaCreateClassPtrMap;
}

#include "luash_sv/luash_sv_shared.h"