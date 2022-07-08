
#include "keydefs.h"
#include "Utils.h"

#include "luash.hpp"

namespace ui
{
	int LuaUI_OpenEngfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "engfunc", &EngFuncs::engfuncs);
		return 0;
	}
}