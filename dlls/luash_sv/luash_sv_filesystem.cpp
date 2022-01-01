
#include "extdll.h"
#include <stdio.h>

#include "fs_int.h"

#include "luash.hpp"
#include "luash_sv/luash_sv_extern.h"

namespace sv
{
	extern fs_api_t gFileSystemAPI;
	int LuaSV_OpenFileSystemLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "filesystem", &gFileSystemAPI);
		return 0;
	}
}