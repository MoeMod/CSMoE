
#include "cl_dll.h"

#include "luash.hpp"
#include "luash_cl_extern.h"


namespace cl
{
	int LuaCL_OpenMobileLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "mobile", &gMobileAPI);
		return 0;
	}
}