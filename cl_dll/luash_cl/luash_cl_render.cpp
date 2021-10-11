
#include "cl_dll.h"

#include "luash.hpp"

#include <string>
#include <cassert>


namespace cl
{
	int LuaCL_OpenRenderLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "render", &gRenderAPI);
		return 0;
	}
}