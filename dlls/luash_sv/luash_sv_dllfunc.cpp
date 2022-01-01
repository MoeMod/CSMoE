
#include "extdll.h"

#include "luash.hpp"
#include "luash_sv/luash_sv_extern.h"

namespace sv
{
	extern DLL_FUNCTIONS gFunctionTable; // cbase.cpp
	int LuaSV_OpenDllfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "dllfunc", &gFunctionTable);
		return 0;
	}
}