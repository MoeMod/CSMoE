
#include "extdll.h"

#include "luash.hpp"

namespace sv
{
	extern DLL_FUNCTIONS gFunctionTable; // cbase.cpp
	int LuaSV_OpenDllfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "dllfunc", &gFunctionTable);
		return 0;
	}
}