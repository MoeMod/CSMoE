
#include "keydefs.h"
#include "Utils.h"

#include "luash.hpp"

namespace ui
{
	int LuaUI_OpenFileSystemLib(lua_State* L)
	{
		luash::RegisterGlobal(L, "filesystem", &gFileSystemAPI);
		return 0;
	}
}