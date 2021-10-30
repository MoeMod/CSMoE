
#include "extdll.h"
#include "enginecallback.h"

#include "luash.hpp"

namespace sv
{
	int LuaSV_OpenEngfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "engfunc", &g_engfuncs);
		return 0;
	}
}