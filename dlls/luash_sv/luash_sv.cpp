
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

#if defined(__ANDROID__) || ( TARGET_OS_IOS || TARGET_OS_IPHONE )
#else
#define LUASH_HOT_RELOAD
#endif


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
		lua_register(L, "MOE_SETUP_BUY_INFO", LuaSV_MOE_SETUP_BUY_INFO);

        luash::RegisterGlobal(L, "UTIL_PrecacheOtherWeapon", UTIL_PrecacheOtherWeapon);
        luash::RegisterGlobal(L, "UTIL_PrecacheOther", UTIL_PrecacheOther);

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
        lua_pushcfunction(L, LuaSV_ExceptionHandler);
        int errc = luaL_loadstring(L, str);
		// #1 = str
        if(errc == LUA_OK)
		    errc = lua_pcall(L, 0, 0, -2);
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
        lua_pop(L, 1);
		assert(stack_cnt == lua_gettop(L));
	}

    std::string LuaSV_ModulePath(const std::string &ModuleName)
    {
        std::string path = "addons/luash/";
        path += ModuleName;
        if (path.substr(path.size() - 4) != ".lua")
            path += ".lua";
        return path;
    }

#ifdef LUASH_HOT_RELOAD
    std::unordered_map<std::string, fs_offset_t> g_LoadedLuaFileTime;
#endif

	int LuaSV_GlobalReload(lua_State* L)
	{
		int NumParams = lua_gettop(L);
		if (NumParams < 1)
		{
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
		lua_setfield(L, -2, ModuleName);

#ifdef LUASH_HOT_RELOAD
        g_LoadedLuaFileTime.erase(ModuleName);
#endif
        using namespace luash;
        eval(L, _require(_1), ModuleName);

        char buffer[256];
        snprintf(buffer, 256, "%s: reload %s \n", __FUNCTION__, ModuleName);
        SERVER_PRINT(buffer);
		return 0;
	}

    extern "C" void LuaSV_AutoReload()
    {
#ifdef LUASH_HOT_RELOAD
        auto L = LuaSV_Get();
        if(!L) return; // not yet initialized
        std::vector<std::string> changed_modules;
        for(const auto &[ModuleName, time] : g_LoadedLuaFileTime)
        {
            auto path = LuaSV_ModulePath(ModuleName);
            auto new_time = gFileSystemAPI.FS_FileTime(path.c_str(), true);
            if(time != new_time)
            {
                changed_modules.push_back(ModuleName);
            }
        }
        for(const auto &ModuleName : changed_modules)
        {
            using namespace luash;
            xpeval(L, LuaSV_ExceptionHandler, (
                    _G[_S("reload")](_1)
            ), ModuleName);
        }
#endif
    }

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

        auto path = LuaSV_ModulePath(ModuleName);

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
#ifdef LUASH_HOT_RELOAD
        auto time = gFileSystemAPI.FS_FileTime(path.c_str(), true);
        g_LoadedLuaFileTime.emplace(ModuleName, time);
#endif

		lua_getfield(L, 2, ModuleName);
		// #3 = LUA_LOADED_TABLE[ModuleName]
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

    int LuaSV_ExceptionHandler(lua_State* L)
    {
        // #1 = err
        lua_getglobal(L, "debug");           // #2 = debug
        lua_getfield(L, -1, "traceback");    // #3 = debug.traceback
        lua_remove(L, -2);                   // remove 'debug'    // #2 = debug.traceback
        lua_pushvalue(L, 1);                 // #3 = err
        lua_call(L, 1, 1);                   // #2 = debug.traceback(err)

        const char* errmsg = lua_tostring(L, -1);
        SERVER_PRINT(errmsg);    // print it

        return 1;    // return debug.traceback(err)
    }
}
