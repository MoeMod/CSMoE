
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

#if defined(__ANDROID__) || ( TARGET_OS_IOS || TARGET_OS_IPHONE )
#else
#define LUASH_HOT_RELOAD
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
        lua_register(L, "MOE_SETUP_BUY_INFO", LuaSV_MOE_SETUP_BUY_INFO);

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
        lua_pushcfunction(L, LuaCL_ExceptionHandler);
        int errc = luaL_loadstring(L, str);
		// #1 = str
        if(errc == LUA_OK)
            errc = lua_pcall(L, 0, 0, -2);
		// #0 on succ
		// #1 = errmsg on failed
		if(errc)
		{
			const char* msg = lua_tostring(L, -1);
			gEngfuncs.Con_Printf("%s Error: exec (%s) failed: %s", __FUNCTION__, str, msg);
			lua_pop(L, 1);
		}
        lua_pop(L, 1);
		assert(stack_cnt == lua_gettop(L));
	}

    std::string LuaCL_ModulePath(const std::string &ModuleName)
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

	int LuaUI_GlobalReload(lua_State* L)
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

        gEngfuncs.Con_Printf("%s: reload %s \n", __FUNCTION__, ModuleName);
		return 0;
	}

    extern "C" void LuaCL_AutoReload()
    {
#ifdef LUASH_HOT_RELOAD
        auto L = LuaCL_Get();
        if(!L) return; // not yet initialized
        std::vector<std::string> changed_modules;
        for(const auto &[ModuleName, time] : g_LoadedLuaFileTime)
        {
            auto path = LuaCL_ModulePath(ModuleName);
            auto new_time = gFileSystemAPI.FS_FileTime(path.c_str(), true);
            if(time != new_time)
            {
                changed_modules.push_back(ModuleName);
            }
        }
        for(const auto &ModuleName : changed_modules)
        {
            using namespace luash;
            xpeval(L, LuaCL_ExceptionHandler, (
                    _G[_S("reload")](_1)
            ), ModuleName);
        }
#endif
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

        auto path = LuaCL_ModulePath(ModuleName);

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

        gEngfuncs.COM_FreeFile(buffer);

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
#ifdef LUASH_HOT_RELOAD
        auto time = gFileSystemAPI.FS_FileTime(path.c_str(), true);
        g_LoadedLuaFileTime.emplace(ModuleName, time);
#endif

		lua_getfield(L, 2, ModuleName);
		// #3 = LUA_LOADED_TABLE[ModuleName]
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

    int LuaSV_ExceptionHandler(lua_State* L)
    {
        return LuaCL_ExceptionHandler(L);
    }

    int LuaCL_ExceptionHandler(lua_State* L)
    {
        // #1 = err
        lua_getglobal(L, "debug");           // #2 = debug
        lua_getfield(L, -1, "traceback");    // #3 = debug.traceback
        lua_remove(L, -2);                   // remove 'debug'    // #2 = debug.traceback
        lua_pushvalue(L, 1);                 // #3 = err
        lua_call(L, 1, 1);                   // #2 = debug.traceback(err)

        const char* errmsg = lua_tostring(L, -1);
        gEngfuncs.Con_Printf(errmsg);    // print it

        return 1;    // return debug.traceback(err)
    }

	void LuaCL_OnPrecache(resourcetype_t type, const char* name, int index)
	{
		if (type == t_eventscript)
			LuaCL_OnPrecacheEvent(name, index);
	}

	void LuaCL_OnGUI()
	{
#if 0
        using namespace luash;
        xpeval(LuaCL_Get(), LuaCL_ExceptionHandler, (
                _local(v1) = _require(_S("imgui/client")),
                _if(v1)._then(
                        v1()
                )._end
        ));
#endif
	}
}
