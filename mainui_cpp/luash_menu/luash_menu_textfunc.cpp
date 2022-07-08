
#include "keydefs.h"
#include "Utils.h"

#include "luash.hpp"

namespace ui
{
	int LuaUI_OpenTextfuncLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "textfunc", &EngFuncs::textfuncs);
		return 0;
	}
}