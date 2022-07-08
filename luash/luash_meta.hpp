#pragma once

namespace luash
{
	int RequireMeta(lua_State* L);

	int CallOnNamedStructCreate(lua_State* L, std::string_view struct_name);

	template<class T>
	int CallOnStructCreate(lua_State* L)
	{
		// #1 = target lua struct
		auto struct_name = nameof::nameof_short_type<T>();
		return CallOnNamedStructCreate(L, struct_name);
	}
}
