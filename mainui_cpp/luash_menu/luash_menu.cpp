
#include "keydefs.h"
#include "Utils.h"

#include "luash_menu.h"
#include "luash.hpp"

#include <string>
#include <cassert>
#include <unordered_map>

#ifdef XASH_IMGUI
#include "imgui_lua/imgui_lua.h"
#include "imgui_utils_lua.h"
#endif

namespace ui
{
	static lua_State *L;

	static void Cmd_LuaUI()
	{
		ImVec2 x;
		int n = EngFuncs::CmdArgc();
		for(int i = 1; i<n;++i)
		{
			const char *str = EngFuncs::CmdArgv(i);
			LuaUI_Exec(str);
		}
	}

	void LuaUI_Init()
	{
		L = luaL_newstate();
        luaL_openlibs(L);

		lua_register(L, "require", LuaUI_GlobalRequire);
		lua_register(L, "reload", LuaUI_GlobalReload);
		lua_register(L, "print", LuaUI_GlobalPrint);

		luash::OpenLibs(L);
#ifdef XASH_IMGUI
		luaopen_imgui(L);
		ImGuiUtils::LuaOpen_ImGuiUtils(L);
#endif
		luash::RegisterGlobal(L, "namespace", "ui");
		LuaUI_OpenTextfuncLib(L);
		LuaUI_OpenEngfuncLib(L);
		LuaUI_OpenFileSystemLib(L);

		EngFuncs::Cmd_AddCommand("luaui", Cmd_LuaUI);
	}

	void LuaUI_Shutdown()
	{
		lua_close(L);
		L = nullptr;
	}

	lua_State* LuaUI_Get()
	{
		return L;
	}

	void LuaUI_Exec(const char *str)
	{
		if(!L)
		{
			Con_Printf("%s", str);
			return;
		}

		int stack_cnt = lua_gettop(L);
		luaL_loadstring(L, str);
		// #1 = str
		int errc = lua_pcall(L, 0, 0, 0);
		// #0 on succ
		// #1 = errmsg on failed
		if(errc)
		{
			const char* msg = lua_tostring(L, -1);
			Con_Printf("%s Error: exec (%s) failed: %s", __FUNCTION__, str, msg);
			lua_pop(L, 1);
		}
		assert(stack_cnt == lua_gettop(L));
	}

	int LuaUI_GlobalReload(lua_State* L)
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
	
	int LuaUI_GlobalRequire(lua_State* L)
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
				lua_pushcfunction(L, LuaUI_GlobalReload);
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
		char* buffer = EngFuncs::COM_LoadFile(path.c_str(), &length);
		if(!buffer)
		{
			Con_Printf("%s Error: lua module (%s) not found\n", __FUNCTION__, ModuleName);
			return 0;
		}
		
		int errc = 0;

		errc = luaL_loadbufferx(L, reinterpret_cast<char*>(buffer), length, ModuleName, "bt");
		// #3 = require function

		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			Con_Printf("%s Error: lua module (%s) loaded failed: %s\n", __FUNCTION__, ModuleName, msg);
			return 0;
		}

		lua_pushvalue(L, 1);
		// #4 = #1 = require path

		errc = lua_pcall(L, 1, 1, 0);
		// #3 = result / errormsg

		if(errc)
		{
			const char* msg = lua_tostring(L, -1);
			Con_Printf("%s Error: lua module (%s) loaded failed: %s\n", __FUNCTION__, ModuleName, msg);
			return 0;
		}

		if (!lua_isnil(L, -1))
		{
			lua_setfield(L, 2, ModuleName);
			// #2 = _G.LUA_LOADED_TABLE
		}

		lua_getfield(L, 2, ModuleName);
		// #3 = LUA_LOADED_TABLE[ModuleName]

		Con_Printf("%s Log: lua module (%s) loaded succ\n", __FUNCTION__, ModuleName);
		return 1;
	}

	int LuaUI_GlobalPrint(lua_State* L)
	{
		int n = lua_gettop(L);
		for (int i = 1; i <= n; i++) {
			const char *str = lua_tostring(L, i);
			if (!str)
				str = "";
			Con_Printf("%s", str);
		}
		Con_Printf("\n");
		return 0;
	}

	void LuaUI_OnGUI()
	{
		lua_State* L = LuaUI_Get();

		lua_getglobal(L, "require");
		// #1 = require

		lua_pushstring(L, "imgui/mainui");
		// #2 = path

		lua_call(L, 1, 1);
		// #1 = function LuaCL_OnGUI

		if (!lua_isnil(L, -1))
		{
			int errc = lua_pcall(L, 0, 0, 0);
			// #0 on succ
			// #1 = errmsg on failed
			if (errc)
			{
				const char* msg = lua_tostring(L, -1);
				Con_Printf("%s Error: lua imgui failed: %s", __FUNCTION__, msg);
				lua_pop(L, 1);
				// #0
			}
		}
		else
		{
			lua_pop(L, 1);
			// #0
		}
	}
}
