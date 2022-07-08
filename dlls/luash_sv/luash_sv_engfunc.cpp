
#include "extdll.h"
#include "enginecallback.h"

#include "luash.hpp"
#include "luash_sv/luash_sv_extern.h"

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