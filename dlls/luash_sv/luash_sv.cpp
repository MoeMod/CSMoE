
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "enginecallback.h"

#include <stdio.h>
#include "fs_int.h"

#include "luash_sv.h"
#include "luash.hpp"
#include "luash_sv/luash_sv_extern.h"

#include <string>
#include <cassert>
#include <unordered_map>

namespace sv
{
	extern fs_api_t gFileSystemAPI;
	static lua_State* L;

	static void Cmd_LuaSV()
	{
		int n = CMD_ARGC();
		for (int i = 1; i < n; ++i)
		{
			const char* str = CMD_ARGV(i);
			LuaSV_Exec(str);
		}
	}

	void LuaSV_Init()
	{
		L = luaL_newstate();
        luaL_openlibs(L);

		lua_register(L, "require", LuaSV_GlobalRequire);
		lua_register(L, "reload", LuaSV_GlobalReload);
		lua_register(L, "print", LuaSV_GlobalPrint);

		luash::OpenLibs(L);

		luash::RegisterGlobal(L, "namespace", "sv");
#ifdef SERVER_DLL
		luash::RegisterGlobal(L, "SERVER_DLL", 1);
#endif
#ifdef CLIENT_WEAPONS
		luash::RegisterGlobal(L, "CLIENT_WEAPONS", 1);
#endif
		LuaSV_OpenEngfuncLib(L);
		LuaSV_OpenFileSystemLib(L);
		LuaSV_OpenDllfuncLib(L);
		LuaSV_OpenGlbLib(L);

		LuaSV_ExportCppClass(L);
		LuaSV_InitEntityFactory(L);
		lua_register(L, "LINK_ENTITY_TO_CLASS", LuaSV_LinkEntityToClass);
		lua_register(L, "GetDamageTrack", LuaSV_GetDamageTrack);
		lua_register(L, "STRING", LuaSV_STRING);
		lua_register(L, "UTIL_WeaponTimeBase", LuaSV_UTIL_WeaponTimeBase);
		lua_register(L, "MAKE_STRING", LuaSV_MAKE_STRING);

		ADD_SERVER_COMMAND("luasv", Cmd_LuaSV);

		lua_getglobal(L, "require");
		lua_pushstring(L, "server/init");
		if (int errc = lua_pcall(L, 1, 0, 0))
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s server init failed: %s", __FUNCTION__, msg);
			SERVER_PRINT(buffer);
			lua_pop(L, 1);
		}
	}

	void LuaSV_Shutdown()
	{
		lua_close(L);
		L = nullptr;
	}

	lua_State* LuaSV_Get()
	{
		return L;
	}

	void LuaSV_Exec(const char* str)
	{
		if (!L)
		{
			SERVER_PRINT(str);
			return;
		}

		int stack_cnt = lua_gettop(L);
		luaL_loadstring(L, str);
		// #1 = str
		int errc = lua_pcall(L, 0, 0, 0);
		// #0 on succ
		// #1 = errmsg on failed
		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s Error: exec (%s) failed: %s", __FUNCTION__, str, msg);
			SERVER_PRINT(buffer);
			lua_pop(L, 1);
		}
		assert(stack_cnt == lua_gettop(L));
	}

	int LuaSV_GlobalReload(lua_State* L)
	{
		int NumParams = lua_gettop(L);
		if (NumParams < 1)
		{
			// reload all
			lua_newtable(L);
			lua_setfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
			return 0;
		}

		const char* ModuleName = lua_tostring(L, 1);
		if (!ModuleName)
		{
			// Invalid module name!
			return 0;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
		lua_pushnil(L);
		lua_setfield(L, 2, ModuleName);
		return 0;
	}

#ifdef LUASH_HOT_RELOAD
	std::unordered_map<std::string, fs_offset_t> g_LoadedLuaFileTime;
#endif

	int LuaSV_GlobalRequire(lua_State* L)
	{
		int NumParams = lua_gettop(L);
		if (NumParams < 1)
		{
			// Invalid parameters!
			return 0;
		}

		const char* ModuleName = lua_tostring(L, 1);
		if (!ModuleName)
		{
			// Invalid module name!
			return 0;
		}

		lua_settop(L, 1);
		// #1 = require path

		std::string path = "addons/luash/";
		path += ModuleName;
		if (path.substr(path.size() - 4) != ".lua")
			path += ".lua";
#ifdef LUASH_HOT_RELOAD
		auto time = gFileSystemAPI.FS_FileTime(path.c_str(), true);
		auto iter = g_LoadedLuaFileTime.find(path);
		if (iter != g_LoadedLuaFileTime.end())
		{
			if (time > g_LoadedLuaFileTime[path])
			{
				iter->second = time;
				lua_pushcfunction(L, LuaSV_GlobalReload);
				lua_pushvalue(L, 1);
				lua_call(L, 1, 0);
			}
		}
		else
		{
			g_LoadedLuaFileTime.emplace(path, time);
		}
#endif

        luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE); // LUA_LOADED_TABLE
		// #2 = _G.LUA_LOADED_TABLE

		lua_getfield(L, 2, ModuleName);
		// #3 = LUA_LOADED_TABLE[ModuleName]
		if (lua_toboolean(L, -1))
		{
			return 1;
		}

		lua_pop(L, 1);
		// #2 = _G.LUA_LOADED_TABLE
		int length = 0;
		byte* buffer = LOAD_FILE_FOR_ME(path.c_str(), &length);
		if (!buffer)
		{
			char buffer[256];
			snprintf(buffer, 256, "%s Error: lua module (%s) not found\n", __FUNCTION__, ModuleName);
			SERVER_PRINT(buffer);
			return 0;
		}

		int errc = 0;

		errc = luaL_loadbufferx(L, reinterpret_cast<char*>(buffer), length, ModuleName, "t");
		// #3 = require function

		FREE_FILE(buffer);

		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s Error: lua module (%s) loaded failed: %s\n", __FUNCTION__, ModuleName, msg);
			SERVER_PRINT(buffer);
			return 0;
		}

		lua_pushvalue(L, 1);
		// #4 = #1 = require path

		errc = lua_pcall(L, 1, 1, 0);
		// #3 = result / errormsg

		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s Error: lua module (%s) loaded failed: %s\n", __FUNCTION__, ModuleName, msg);
			SERVER_PRINT(buffer);
			return 0;
		}

		if (!lua_isnil(L, -1))
		{
			lua_setfield(L, 2, ModuleName);
			// #2 = _G.LUA_LOADED_TABLE
		}

		lua_getfield(L, 2, ModuleName);
		// #3 = LUA_LOADED_TABLE[ModuleName]

		{
			char buffer[256];
			snprintf(buffer, 256, "%s Log: lua module (%s) loaded succ\n", __FUNCTION__, ModuleName);
			SERVER_PRINT(buffer);
		}
		return 1;
	}

	int LuaSV_GlobalPrint(lua_State* L)
	{
		int n = lua_gettop(L);
		for (int i = 1; i <= n; i++) {
			const char* str = lua_tostring(L, i);
			if (!str)
				str = "";
			SERVER_PRINT(str);
		}
		SERVER_PRINT("\n");
		return 0;
	}

	void LuaSV_PrintError(const char* msg)
	{
		SERVER_PRINT(msg);
	}
}
