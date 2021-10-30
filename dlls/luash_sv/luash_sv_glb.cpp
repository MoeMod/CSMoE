
#include "extdll.h"

#include "luash.hpp"

namespace sv
{
	extern globalvars_t* gpGlobals; // h_export.h
	int LuaSV_OpenGlbLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "gpGlobals", gpGlobals);
		luash::RegisterGlobal(L, "global", gpGlobals);
		return 0;
	}
}