#include "events.h"
#include "ev_lua.h"

#include "lua_cl.h"
#include "luash.hpp"

namespace cl
{
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

		const char* path = gEngfuncs.pEventAPI->EV_EventForIndex(index);
		if(!path)
		{
			// assert false
			return;
		}

		lua_State* L = LuaCL_Get();

		lua_getglobal(L, "require");
		// #1 = require
		
		lua_pushstring(L, path);
		// #2 = path

		lua_call(L, 1, 1);
		// #1 = function EV_FireXXX

		luash::Push(L, *args);
		lua_call(L, 1, 0);
		// #0
	}
	
	void Lua_HookEvents(void)
	{
		
		gEngfuncs.pfnHookEvent("events/lua.sc", EV_Lua);
	}
}