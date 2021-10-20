#include "luash.hpp"

namespace luash
{
	
	std::unordered_map<std::string_view, const ITypeInterface* >& KnownTypeInterfaceMap()
	{
		static std::unordered_map<std::string_view, const ITypeInterface* > KnownTypeInterfaceMapInst;
		return KnownTypeInterfaceMapInst;
	}

	int OpenLibs(lua_State* L)
	{
		lua_pushcfunction(L, SetupPtrMetaTable);
		lua_call(L, 0, 0);
		lua_pushcfunction(L, SetupCppNewDelete);
		lua_call(L, 0, 0);
		return 0;
	}

	namespace {
		int CppNew(lua_State* L)
		{
			const char* name = lua_tostring(L, 1);
			auto& ti_map = KnownTypeInterfaceMap();
			if (auto iter = ti_map.find(name); iter != ti_map.end())
			{
				auto ti = iter->second;
				lua_remove(L, 1);
				if (auto size = lua_tointeger(L, 1); size > 0)
				{
					return ti->NewVector(L);
				}
				return ti->NewScalar(L);
			}
			luaL_error(L, "unrecognized type %s", name);
			return 0;
		}

		int CppDelete(lua_State* L)
		{
			void* value = lua_touserdata(L, 1);
			if (const ITypeInterface* ti = GetTypeInterface(L, value))
			{
				if (auto size = lua_tointeger(L, 2); size > 0)
				{
					return ti->DeleteVector(L);
				}
				return ti->DeleteScalar(L);
			}
			luaL_error(L, "unrecognized ref ptr %x", value);
			return 0;
		}
	}

	namespace detail {
		void TypeInterfaceStringIndexReplace(const ITypeInterface* ti, lua_State* L, std::string_view PathPrefix, std::string_view name)
		{
			// #1 = object
			// #2 = key (TO BE REPLACED)
			// ...(optional)
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				lua_getglobal(L, "require"); // #N+1 = require
				lua_pushlstring(L, PathPrefix.data(), PathPrefix.size()); // #N+2 = "struct/"
				lua_pushlstring(L, name.data(), name.size()); // #N+3 = "typename"
				lua_concat(L, 2); // #N+2 = "struct/typename"
				lua_call(L, 1, 1);	// #N+1 = index table
				if (!lua_isnil(L, -1))
				{
					lua_pushvalue(L, 2); // #N+1 = N
					lua_gettable(L, -2); // #N+2 = new N
					lua_replace(L, 2); // #N+1 = index table
					lua_pop(L, 1); // #N = N => new N
				}
			}
		}
	}

	template<class...Args> struct TypeList {};
	using BuiltinTypes = TypeList<bool, char, char16_t, char32_t, wchar_t, short, int, long, long long, float, double, long double>;

	template<class...Args>
	static void SetupBuiltinTypeInterfaceImpl(TypeList<Args...>)
	{
		(..., (CreateTypeInterface<Args>()));
	}

	int SetupBuiltinTypeInterface(lua_State* L)
	{
		SetupBuiltinTypeInterfaceImpl(BuiltinTypes());
		return 0;
	}

	int SetupPtrMetaTable(lua_State* L)
	{
		lua_pushlightuserdata(L, nullptr); // #1 = lightuseradata
		lua_newtable(L); // #2 = metatable
		lua_pushcfunction(L, RefValueMetaDispatch<&ITypeInterface::MetaIndex>); // #3 = func
		lua_setfield(L, -2, "__index"); // #2
		lua_pushcfunction(L, RefValueMetaDispatch<&ITypeInterface::MetaNewIndex>); // #3 = func
		lua_setfield(L, -2, "__newindex"); // #2
		lua_pushcfunction(L, RefValueMetaDispatch<&ITypeInterface::MetaCall>); // #3 = func
		lua_setfield(L, -2, "__call"); // #2
		lua_pushcfunction(L, RefValueMetaDispatch<&ITypeInterface::MetaToString>); // #3 = func
		lua_setfield(L, -2, "__tostring"); // #2
		lua_setmetatable(L, -2); // #1 = lightuserdata
		lua_pop(L, 1); // #0
		return 0;
	}

	int SetupCppNewDelete(lua_State* L)
	{
		lua_newtable(L); // #1 = table
		lua_pushcfunction(L, CppNew);
		lua_setfield(L, -2, "new");
		lua_pushcfunction(L, CppDelete);
		lua_setfield(L, -2, "delete");
		lua_setglobal(L, "cpp");
		return 0;
	}

	int RequireMeta(lua_State* L)
	{
		lua_getglobal(L, "require");
		// #1 = require

		lua_pushstring(L, "core/luash_meta");
		// #2 = path

		lua_call(L, 1, 1);
		// #1 = meta

		return 1;
	}

	int CallOnNamedStructCreate(lua_State* L, std::string_view struct_name)
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
}