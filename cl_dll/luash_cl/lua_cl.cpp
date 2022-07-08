
#include "cl_dll.h"

#include "lua_cl.h"

#include "luash.hpp"
#include "luash_cl_extern.h"

#include <string>
#include <cassert>
#include <unordered_map>

#include "ev_lua.h"
#ifdef XASH_IMGUI
#include "imgui_lua/imgui_lua.h"
#include "imgui_utils_lua.h"
#endif

namespace cl
{
	static lua_State *L;

	static void Cmd_LuaCL()
	{
		int n = gEngfuncs.Cmd_Argc();
		for(int i = 1; i<n;++i)
		{
			const char *str = gEngfuncs.Cmd_Argv(i);
			LuaCL_Exec(str);
		}
	}

	void LuaCL_Init()
	{
		L = luaL_newstate();
        luaL_openlibs(L);

		lua_register(L, "reload", LuaUI_GlobalReload);
		lua_register(L, "require", LuaCL_GlobalRequire);
		lua_register(L, "print", LuaCL_GlobalPrint);

		luash::OpenLibs(L);
#ifdef XASH_IMGUI
		luaopen_imgui(L);
		ImGuiUtils::LuaOpen_ImGuiUtils(L);
#endif
		luash::RegisterGlobal(L, "namespace", "cl");
#ifdef CLIENT_DLL
		luash::RegisterGlobal(L, "CLIENT_DLL", 1);
#endif
#ifdef CLIENT_WEAPONS
		luash::RegisterGlobal(L, "CLIENT_WEAPONS", 1);
#endif
		LuaCL_OpenEngfuncLib(L);
		LuaCL_OpenRenderLib(L);
		LuaCL_OpenMobileLib(L);
		LuaCL_OpenFileSystemLib(L);
		LuaCL_OpenEventScripts(L);

		LuaSV_ExportCppClass(L);
		LuaSV_InitEntityFactory(L);
		lua_register(L, "LINK_ENTITY_TO_CLASS", LuaSV_LinkEntityToClass);
		lua_register(L, "GetDamageTrack", LuaSV_GetDamageTrack);
		lua_register(L, "STRING", LuaSV_STRING);
		lua_register(L, "UTIL_WeaponTimeBase", LuaSV_UTIL_WeaponTimeBase);
		lua_register(L, "MAKE_STRING", LuaSV_MAKE_STRING);

		gEngfuncs.pfnAddCommand("luacl", Cmd_LuaCL);

		lua_getglobal(L, "require");
		lua_pushstring(L, "client/init");
		if (int errc = lua_pcall(L, 1, 0, 0))
		{
			const char* msg = lua_tostring(L, -1);
			gEngfuncs.Con_Printf("%s client init failed: %s", __FUNCTION__, msg);
			lua_pop(L, 1);
		}
	}

	void LuaCL_PrepEntity()
	{
#ifdef CLIENT_WEAPONS
		LuaSV_OpenEngfuncLib(L);
		LuaSV_OpenGlbLib(L);
#endif
	}

	void LuaCL_Shutdown()
	{
		lua_close(L);
		L = nullptr;
	}

	lua_State* LuaCL_Get()
	{
		return L;
	}
	lua_State* LuaSV_Get()
	{
		return LuaCL_Get();
	}

	void LuaCL_Exec(const char *str)
	{
		if(!L)
		{
			gEngfuncs.Con_Printf("%s", str);
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
			gEngfuncs.Con_Printf("%s Error: exec (%s) failed: %s", __FUNCTION__, str, msg);
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
	
	int LuaCL_GlobalRequire(lua_State* L)
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
		char* buffer = gEngfuncs.COM_LoadFile(path.c_str(), 5, &length);
		if(!buffer)
		{
			gEngfuncs.Con_Printf("%s Error: lua module (%s) not found\n", __FUNCTION__, ModuleName);
			return 0;
		}
		
		int errc = 0;

		errc = luaL_loadbufferx(L, buffer, length, ModuleName, "bt");
		// #3 = require function

		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			gEngfuncs.Con_Printf("%s Error: lua module (%s) loaded failed: %s\n", __FUNCTION__, ModuleName, msg);
			return 0;
		}

		lua_pushvalue(L, 1);
		// #4 = #1 = require path

		errc = lua_pcall(L, 1, 1, 0);
		// #3 = result / errormsg

		if(errc)
		{
			const char* msg = lua_tostring(L, -1);
			gEngfuncs.Con_Printf("%s Error: lua module (%s) loaded failed: %s\n", __FUNCTION__, ModuleName, msg);
			return 0;
		}

		if (!lua_isnil(L, -1))
		{
			lua_setfield(L, 2, ModuleName);
			// #2 = _G.LUA_LOADED_TABLE
		}

		lua_getfield(L, 2, ModuleName);
		// #3 = LUA_LOADED_TABLE[ModuleName]
		
		gEngfuncs.Con_DPrintf("%s Log: lua module (%s) loaded succ\n", __FUNCTION__, ModuleName);
		return 1;
	}

	int LuaCL_GlobalPrint(lua_State* L)
	{
		int n = lua_gettop(L);
		for (int i = 1; i <= n; i++) {
			const char *str = lua_tostring(L, i);
			if (!str)
				str = "";
			gEngfuncs.Con_Printf("%s", str);
		}
		gEngfuncs.Con_Printf("\n");
		return 0;
	}

	void LuaSV_PrintError(const char* msg)
	{
		gEngfuncs.Con_Printf("%s: %s", __FUNCTION__, msg);
	}

	void LuaCL_OnPrecache(resourcetype_t type, const char* name, int index)
	{
		if (type == t_eventscript)
			LuaCL_OnPrecacheEvent(name, index);
	}

	void LuaCL_OnGUI()
	{
		lua_State* L = LuaCL_Get();

        lua_getglobal(L, "debug"); // #1 = debug
        lua_getfield(L, -1, "traceback"); // #2 = debug.traceback

		lua_getglobal(L, "require");
		// #3 = require

		lua_pushstring(L, "imgui/client");
		// #4 = path

		lua_call(L, 1, 1);
		// #3 = function LuaCL_OnGUI

		if (!lua_isnil(L, -1))
		{

			int errc = lua_pcall(L, 0, 0, -2);
			// #0 on succ
			// #1 = errmsg on failed
			if (errc)
			{
				const char* msg = lua_tostring(L, -1);
				gEngfuncs.Con_Printf("%s Error: lua imgui failed: %s\n", __FUNCTION__, msg);
				lua_pop(L, 1);
				// #0
			}
            lua_pop(L, 2); // #0
		}
		else
		{
			lua_pop(L, 1);
			// #0
		}
	}
}
