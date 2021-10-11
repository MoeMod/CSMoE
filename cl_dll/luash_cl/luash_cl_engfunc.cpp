
#include "cl_dll.h"

#include "luash.hpp"


namespace cl
{
	int LuaCL_OpenEngfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "engine", &gEngfuncs);
		luash::RegisterGlobal(L, "engfunc", &gEngfuncs);
		return 0;
	}
}