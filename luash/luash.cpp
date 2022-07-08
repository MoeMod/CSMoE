#include "luash.hpp"
#include "util/u_vector_luash.hpp"

namespace moe {
    template void VectorBase<float, 2>::LuaPush(lua_State* L) const;
    template void VectorBase<float, 2>::LuaGet(lua_State* L, int idx);
    template void VectorBase<float, 3>::LuaPush(lua_State* L) const;
    template void VectorBase<float, 3>::LuaGet(lua_State* L, int idx);
#ifdef U_VECTOR_SIMD
    void VectorBase<float, 3, 16>::LuaPush(lua_State* L) const
    {
        return LuaPush_impl(L, *this, std::make_index_sequence<N>());
    }

    void VectorBase<float, 3, 16>::LuaGet(lua_State* L, int idx)
    {
        return LuaGet_impl(L, idx, *this, std::make_index_sequence<N>());
    }
#else
    template void VectorBase<float, 3, 16>::LuaPush(lua_State* L) const;
    template void VectorBase<float, 3, 16>::LuaGet(lua_State* L, int idx);
#endif
}

namespace luash
{
	
	std::unordered_map<std::string_view, const ITypeInterface* >& KnownTypeInterfaceMap()
	{
		static std::unordered_map<std::string_view, const ITypeInterface* > KnownTypeInterfaceMapInst;
		return KnownTypeInterfaceMapInst;
	}

	int RequireCoreLuaFunc(lua_State* L)
	{
		lua_getglobal(L, "require"); // #1 = require

		lua_pushvalue(L, -1); // #2 = require
		lua_pushstring(L, "core/luash_struct"); // #3 = path
		lua_call(L, 1, 0); // #1 = require

		lua_pushvalue(L, -1); // #2 = require
		lua_pushstring(L, "core/luash_class"); // #3 = path
		lua_call(L, 1, 0); // #1 = require

		lua_pop(L, 1);
		return 0;
	}

	int OpenLibs(lua_State* L)
	{
		lua_pushcfunction(L, SetupTypeInterfaceTable);
		lua_call(L, 0, 0);
		lua_pushcfunction(L, SetupPtrMetaTable);
		lua_call(L, 0, 0);
		lua_pushcfunction(L, SetupCppNewDelete);
		lua_call(L, 0, 0);
		lua_pushcfunction(L, RequireCoreLuaFunc);
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

	int SetupTypeInterfaceTable(lua_State* L)
	{
		lua_newtable(L); // #1 = {}
		lua_setglobal(L, "_LUAOBJECT"); // #0
		lua_newtable(L); // #1 = {}
		lua_setglobal(L, "_LUACLASS"); // #0
		return 0;
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

	int CallOnNamedStructCreate(lua_State* L, std::string_view struct_name)
	{
		// #1 = struct
		lua_pushlstring(L, struct_name.data(), struct_name.size()); // #2 = struct_name

		lua_getglobal(L, "OnStructCreate");
		// #3 = OnStructCreate
		lua_insert(L, -3); // #1=OnStructCreate #2=struct #3=struct_name

		if (int errc = lua_pcall(L, 2, 1, 0))
		{
			const char* errmsg = lua_tostring(L, -1);
			// TODO: print errmsg
			// #1 = errmsg
			lua_pop(L, 1);
			lua_pushnil(L);
			return 1;
		}
		// #1 = result
		return 1;
	}

	int PushLuaObjectByPtr(lua_State* L, void* Ptr)
	{
		assert(Ptr != nullptr);
		lua_getglobal(L, "_LUAOBJECT"); // #1 = _R._LUAOBJECT
		assert(!lua_isnil(L, -1));
		lua_pushlightuserdata(L, Ptr); // #2 = ptr
		lua_gettable(L, -2); // #2 = LuaObject
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2); // #0
			return 0;
		}
		else
		{
			lua_remove(L, -2); // #1 = LuaObject
			return 1;
		}
	}

	int LinkPtrToLuaObject(lua_State* L, void *Ptr)
	{
		assert(lua_istable(L, -1)); // #1 = LuaObject
		// add this pointer
		lua_pushlightuserdata(L, Ptr); // #2 = ptr
		lua_setfield(L, -1, "this"); // #1 = LuaObject

		// #1 = LuaObject
		lua_getglobal(L, "_LUAOBJECT"); // #2 = _R._LUACLASS
		assert(!lua_isnil(L, -1));
		lua_pushlightuserdata(L, Ptr); // #3 = ptr
		lua_pushvalue(L, -3); //  #4 = LuaObject
		lua_settable(L, -3);  // #2 = _R._LUACLASS
		lua_pop(L, 1); // #1 = LuaObject
		return 0;
	}

	int RemoveLuaObject(lua_State* L, void* Ptr)
	{
		lua_getglobal(L, "_LUAOBJECT"); // #1 = _R._LUACLASS
		assert(!lua_isnil(L, -1));
		lua_pushlightuserdata(L, Ptr); // #2 = ptr
		lua_pushnil(L); // #3 = nil
		lua_settable(L, -3); // #1 = _R._LUACLASS
		lua_pop(L, 1); // #0
		return 0;
	}

	bool GetPtrByLuaObject(lua_State* L, int N, void*&out)
	{
		if (lua_isnil(L, N))
			return false;
		lua_getfield(L, N, "this");
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			return false;
		}
		out = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return true;
	}

	void SetupRefTypeInterface(lua_State* L, void* ptr, const ITypeInterface* ti) 
	{
		lua_pushlightuserdata(L, (void*)ptr); // #1 = ptr
		lua_pushlightuserdata(L, (void*)ti); // #2 = interface
		lua_settable(L, LUA_REGISTRYINDEX); // #0
	}
	void ReleaseRefTypeInterface(lua_State* L, void* ptr)
	{
		lua_pushlightuserdata(L, (void*)ptr); // #1 = ptr
		lua_pushnil(L); // #2 = nil
		lua_settable(L, LUA_REGISTRYINDEX); // #0
	}
	const ITypeInterface* GetTypeInterfaceTop(lua_State* L)
	{
		// #1 = ptr
		lua_gettable(L, LUA_REGISTRYINDEX); // #1 = interface
		const ITypeInterface* ti = reinterpret_cast<const ITypeInterface*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return ti;
	}
	const ITypeInterface* GetTypeInterface(lua_State* L, void* ptr)
	{
		lua_pushlightuserdata(L, ptr);
		return GetTypeInterfaceTop(L);
	}
}