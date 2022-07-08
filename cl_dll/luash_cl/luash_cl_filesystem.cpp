
#include "cl_dll.h"

#include "luash.hpp"
#include "luash_cl_extern.h"


namespace cl
{
	int LuaCL_OpenFileSystemLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "filesystem", &gFileSystemAPI);
		return 0;
	}
}