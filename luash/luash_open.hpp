#pragma once

namespace luash
{

	inline int OpenLibs(lua_State* L)
	{
		lua_pushcfunction(L, SetupPtrMetaTable);
		lua_call(L, 0, 0);
		return 0;
	}
}
