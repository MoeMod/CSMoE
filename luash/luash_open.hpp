#pragma once

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace luash
	{

		int OpenLibs(lua_State* L)
		{
			lua_pushcfunction(L, SetupPtrMetaTable);
			lua_call(L, 0, 0);
			return 0;
		}
	}

}