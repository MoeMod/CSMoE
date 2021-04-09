#pragma once

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace luash
	{
		template<class T> auto Push(lua_State* L, T&&);
		template<class T> auto Get(lua_State* L, int N, T& out);
	}
}