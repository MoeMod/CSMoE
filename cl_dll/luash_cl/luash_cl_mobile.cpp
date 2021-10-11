
#include "cl_dll.h"

#include "lua_cl.h"

#include "luash.hpp"

#include <string>
#include <cassert>


namespace cl
{
	int LuaCL_OpenMobileLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "mobile", &gMobileAPI);
		return 0;
	}
}