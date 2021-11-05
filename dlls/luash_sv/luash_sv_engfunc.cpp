
#include "extdll.h"
#include "enginecallback.h"

#include "luash.hpp"

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif

	int LuaSV_OpenEngfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "engfunc", &g_engfuncs);
		return 0;
	}
}