#pragma once

#include <stdexcept>

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace luash
	{
		class LuaError : std::runtime_error
		{
		public:
			using std::runtime_error::runtime_error;

			LuaError(lua_State* LL, const std::string& msg) : std::runtime_error(msg), L(LL) {}

			LuaError(lua_State* LL, const char* msg) : std::runtime_error(msg), L(LL) {}

		public:
			lua_State* L;
		};
	}
}