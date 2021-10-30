
#include "extdll.h"
#include <stdio.h>

#include "fs_int.h"

#include "luash.hpp"

namespace sv
{
	extern fs_api_t gFileSystemAPI;
	int LuaSV_OpenFileSystemLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "filesystem", &gFileSystemAPI);
		return 0;
	}
}