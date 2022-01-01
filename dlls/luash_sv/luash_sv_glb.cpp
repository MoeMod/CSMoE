
#include "extdll.h"

#include "luash.hpp"
#include "luash_sv/luash_sv_extern.h"

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif

	extern globalvars_t* gpGlobals; // h_export.h cs_baseentity.cpp
	int LuaSV_OpenGlbLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "gpGlobals", gpGlobals);
		luash::RegisterGlobal(L, "global", gpGlobals);
		return 0;
	}
}