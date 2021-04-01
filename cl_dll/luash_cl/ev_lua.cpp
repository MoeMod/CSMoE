#include "events.h"
#include "ev_lua.h"

#include "lua_cl.h"
#include "luash.hpp"

namespace cl
{
	std::string_view LuaCLStripLuaPath(std::string_view sv)
	{
		if (sv.size() < 10)
			return {};
		if (strnicmp(sv.data(), "addons/luash/", 13) != 0)
			return {};
		if (stricmp(sv.data() + sv.size() - 4, ".lua") != 0)
			return {};
		return sv.substr(13, sv.size() - 13 - 4);
	}
	void EV_Lua(event_args_t* args)
	{
		if (!gEngfuncs.pEventAPI->EV_GetCurrentEventIndex)
		{
			// API unsupported
			return;
		}
		
		auto index = gEngfuncs.pEventAPI->EV_GetCurrentEventIndex();
		if(!index)
		{
			// assert false
			return;
		}

		const char* name = gEngfuncs.pEventAPI->EV_EventForIndex(index);
		std::string path(LuaCLStripLuaPath(name));
		if(path.empty())
		{
			// assert false
			return;
		}

		lua_State* L = LuaCL_Get();

		lua_getglobal(L, "require");
		// #1 = require

		luash::PushString(L, path);
		// #2 = path

		lua_call(L, 1, 1);
		// #1 = function EV_FireXXX

		if (lua_isnil(L, -1))
		{
			gEngfuncs.Con_Printf("%s Error: lua event (%s) load failed\n", __FUNCTION__, path.c_str());
			lua_pop(L, 1);
			return;
		}
		
		luash::Push(L, *args);
		
		int errc = lua_pcall(L, 1, 0, 0);
		if(errc)
		{
			// #1 = errmsg
			const char* msg = lua_tostring(L, -1);
			gEngfuncs.Con_Printf("%s Error: lua event (%s) error: %s\n", __FUNCTION__, path.c_str(), msg);
			lua_pop(L, 1);
		}

		// #0
	}
	
	void LuaCL_HookEvents(void)
	{
		
		
	}

	void LuaCL_OnPrecacheEvent(const char *name, int index)
	{
		auto path = LuaCLStripLuaPath(name);
		if (path.empty())
			return;

		lua_State* L = LuaCL_Get();

		lua_getglobal(L, "require");
		// #1 = require

		luash::PushString(L, path);
		// #2 = path

		lua_call(L, 1, 1);
		// #1 = function EV_FireXXX

		if(!lua_isnil(L, -1))
			gEngfuncs.pfnHookEvent(name, EV_Lua);

		lua_pop(L, 1);
	}
}