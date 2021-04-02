#pragma once


#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace luash
	{
		inline void PopVector(lua_State *L, Vector &out)
		{
			// #1 = vec
			lua_pushnumber(L, 1);
			// #2 = 1
			lua_gettable(L, -2);
			// #2 = vec[1]
			if (lua_isnil(L, -1))
				out[0] = 0;
			else
				out[0] = lua_tonumber(L, -1);
			lua_pop(L, 1);
			// #1 = vec

			lua_pushnumber(L, 2);
			// #2 = 2
			lua_gettable(L, -2);
			// #2 = vec[2]
			if (lua_isnil(L, -1))
				out[1] = 0;
			else
				out[1] = lua_tonumber(L, -1);
			lua_pop(L, 1);
			// #1 = vec

			lua_pushnumber(L, 3);
			// #2 = 3
			lua_gettable(L, -2);
			// #2 = vec[3]
			if (lua_isnil(L, -1))
				out[2] = 0;
			else
				out[2] = lua_tonumber(L, -1);
			lua_pop(L, 1);
			// #1 = vec

			lua_pop(L, 1);
		}
	}
}