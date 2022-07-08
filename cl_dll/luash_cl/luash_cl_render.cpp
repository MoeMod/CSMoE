
#include "cl_dll.h"

#include "luash.hpp"
#include "luash_cl_extern.h"


namespace cl
{
	int LuaCL_OpenRenderLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "render", &gRenderAPI);
		return 0;
	}
}