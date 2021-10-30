
#include "cl_dll.h"

#include "luash.hpp"


namespace cl
{
	int LuaCL_OpenEngfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "engine", &gEngfuncs);
		return 0;
	}
}