
#include "cl_dll.h"

#include "lua_cl.h"

#include "luash.hpp"
#include "luash_vectorlib.hpp"

#include <string>
#include <cassert>

#include "ev_lua.h"

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

		// we should ban some dangerous libs
		luaopen_base(L);
		luaopen_math(L);
		luaopen_string(L);
		luaopen_table(L);
		//luaopen_io(L);
		//luaopen_os(L);
		//luaopen_package(L);
		luaopen_debug(L);
		luaopen_bit(L);
		luaopen_jit(L);
		//luaopen_ffi(L);
		
		lua_register(L, "require", LuaCL_GlobalRequire);
		lua_register(L, "print", LuaCL_GlobalPrint);

		LuaCL_OpenEventScripts(L);

		gEngfuncs.pfnAddCommand("luacl", Cmd_LuaCL);
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

		lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED"); // LUA_LOADED_TABLE
		// #2 = _G.LUA_LOADED_TABLE

		lua_getfield(L, 2, ModuleName);
		// #3 = LUA_LOADED_TABLE[ModuleName]
		if (lua_toboolean(L, -1))
		{
			return 1;
		}

		lua_pop(L, 1);
		// #2 = _G.LUA_LOADED_TABLE

		std::string path = "addons/luash/";
		path += ModuleName;
		if (path.substr(path.size() - 4) != ".lua")
			path += ".lua";
		int length = 0;
		byte* buffer = gEngfuncs.COM_LoadFile(path.c_str(), 5, &length);
		if(!buffer)
		{
			gEngfuncs.Con_Printf("%s Error: lua module (%s) not found\n", __FUNCTION__, ModuleName);
			return 0;
		}
		
		int errc = 0;

		errc = luaL_loadbufferx(L, reinterpret_cast<char*>(buffer), length, ModuleName, "bt");
		// #3 = require function

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

	void LuaCL_OnPrecache(resourcetype_t type, const char* name, int index)
	{
		if (type == t_eventscript)
			LuaCL_OnPrecacheEvent(name, index);
	}
}
