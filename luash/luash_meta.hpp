#pragma once

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace luash
	{
		inline int RequireMeta(lua_State* L)
		{
			lua_getglobal(L, "require");
			// #1 = require

			lua_pushstring(L, "core/luash_meta");
			// #2 = path

			lua_call(L, 1, 1);
			// #1 = meta

			return 1;
		}

		inline int CallOnNamedStructCreate(lua_State* L, std::string_view struct_name)
		{
			RequireMeta(L);
			// #2 = core/meta

			lua_getfield(L, -1, "OnStructCreate");
			// #3 = meta.SetStructMetaTable

			lua_pushvalue(L, -2);
			// #4 = core/meta

			lua_pushvalue(L, -4);
			// #5 = #1

			lua_pushlstring(L, struct_name.data(), struct_name.size());
			// #6 = struct_name

			if (int errc = lua_pcall(L, 3, 0, 0))
			{
				const char* errmsg = lua_tostring(L, -1);
				// #2 = errmsg
				lua_pop(L, 1);
			}
			// #2 = core/meta
			lua_pop(L, 1);
			return 0;
		}
		
		template<class T>
		int CallOnStructCreate(lua_State* L)
		{
			// #1 = target lua struct
			auto struct_name = nameof::nameof_short_type<T>();
			return CallOnNamedStructCreate(L, struct_name);
		}
	}
}