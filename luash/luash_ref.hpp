#pragma once

#include <lua.hpp>
#include <nameof.hpp>
#include <typeindex>
#include <tuple>
#include <unordered_map>

namespace luash
{
	class ITypeInterface
	{
	public:
		// constexpr variable don't need virtual destructor
		//virtual ~ITypeInterface() = 0;

		virtual std::string_view GetTypeName() const = 0;
		virtual std::type_index GetTypeID() const = 0;
		virtual bool IsConst() const = 0;
		virtual size_t GetMemberCount() const = 0;
		virtual int Copy(lua_State* L) const = 0;
		virtual int Assign(lua_State* L) const = 0;
		virtual int NewCopy(lua_State* L) const = 0;
		virtual int NewScalar(lua_State* L) const = 0;
		virtual int NewVector(lua_State* L) const = 0;
		virtual int DeleteScalar(lua_State* L) const = 0;
		virtual int DeleteVector(lua_State* L) const = 0;
		virtual int MetaIndex(lua_State* L) const = 0;
		virtual int MetaNewIndex(lua_State* L) const = 0;
		virtual int MetaCall(lua_State* L) const = 0;
		virtual int MetaToString(lua_State* L) const = 0;
		/*
		virtual int MetaCall(lua_State* L) = 0;
		virtual int MetaAdd(lua_State* L) = 0;
		virtual int MetaSub(lua_State* L) = 0;
		virtual int MetaMul(lua_State* L) = 0;
		virtual int MetaDiv(lua_State* L) = 0;
		virtual int MetaMod(lua_State* L) = 0;
		virtual int MetaUnm(lua_State* L) = 0;
		virtual int MetaConcat(lua_State* L) = 0;
		virtual int MetaEq(lua_State* L) = 0;
		virtual int MetaLt(lua_State* L) = 0;
		virtual int MetaLe(lua_State* L) = 0;
		*/
	};

	template<class T, class Base>
	class TypeInterfaceDecoratorCommon : public Base
	{
	public:
		std::string_view GetTypeName() const override
		{
			return nameof::nameof_type<T>();
		}
		bool IsConst() const override
		{
			return std::is_const<T>::value;
		}
	};
	namespace detail
	{
		template<std::size_t...I, class...Args> bool MetaIndexImpl(lua_State* L, int N, std::index_sequence<I...>, Args &&...args)
		{
			return (... || ((I == N - 1) ? (Push(L, std::forward<Args>(args)), true) : false));
		}
		template<class T, class...Args> bool MetaNewIndexImpl2(lua_State* L, int N, T &arg)
		{
			Get(L, 3, arg);
			return true;
		}
		template<class T, class...Args> bool MetaNewIndexImpl2(lua_State* L, int N, const T& arg)
		{
			// cannot set const value
			return false;
		}
		template<std::size_t...I, class...Args> bool MetaNewIndexImpl(lua_State* L, int N, std::index_sequence<I...>, Args &...args)
		{
			return (... || ((I == N - 1) ? MetaNewIndexImpl2(L, N, args) : false));
		}
	}
	template<class T, class Base, bool = IsCompleteType<typename std::remove_cv<T>::type>::value>
	class TypeInterfaceDecoratorComplete : public Base
	{
	public:
		std::type_index GetTypeID() const override
		{
			return typeid(void);
		}
		int Copy(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not copy incomplete ref type %s", type.c_str());
		}
		int Assign(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not assign incomplete ref type %s", type.c_str());
		}
		int NewCopy(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not new incomplete ref type %s", type.c_str());
		}
		int NewScalar(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not new incomplete ref type %s", type.c_str());
		}
		int NewVector(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not new incomplete ref type %s", type.c_str());
		}
		int DeleteScalar(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not delete incomplete ref type %s", type.c_str());
		}
		int DeleteVector(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not delete incomplete ref type %s", type.c_str());
		}
	};
	template<class T, class Base>
	class TypeInterfaceDecoratorComplete<T, Base, true> : public Base
	{
	public:
		std::type_index GetTypeID() const override
		{
			return typeid(typename std::remove_cv<T>::type);
		}
		int Copy(lua_State* L) const override
		{
			auto value = lua_touserdata(L, 1);
			auto object = reinterpret_cast<T*>(value);
			Push(L, *object);
			return 1;
		}
		int NewCopy(lua_State* L) const override
		{
			auto value = lua_touserdata(L, 1);
			auto object = reinterpret_cast<T*>(value);
			auto new_object = new auto(*object);
            Push(L, new_object);
			return 1;
		}
		int Assign(lua_State* L) const override
		{
			if constexpr (!std::is_copy_assignable_v<T>)
			{
				std::string type(this->GetTypeName());
				return luaL_error(L, "can not assign const ref type %s", type.c_str());
			}
			else
			{
				auto value = lua_touserdata(L, 1);
				auto object = reinterpret_cast<T*>(value);
				Get(L, 2, *object);
				return 1;
			}
		}
		int NewScalar(lua_State* L) const override
		{
			auto new_object = new T();
			Push(L, new_object);
			return 1;
		}
		int NewVector(lua_State* L) const override
		{
			auto len = lua_tointeger(L, 1);
			if (len < 0)
			{
				return luaL_error(L, "invalid new[] len %d", (int)len);
			}
			auto new_object = new T[len]{};
            Push(L, new_object);
			return 1;
		}
		int DeleteScalar(lua_State* L) const override
		{
			auto value = lua_touserdata(L, 1);
			auto object = reinterpret_cast<T*>(value);
			delete object;
			return 0;
		}
		int DeleteVector(lua_State* L) const override
		{
			auto value = lua_touserdata(L, 1);
			auto object = reinterpret_cast<T*>(value);
			delete[] object;
			return 0;
		}
	};

	template<class T, class Base>
	class TypeInterfaceDecoratorToString : public Base
	{
		int MetaToString(lua_State* L) const override
		{
			auto value = lua_touserdata(L, 1);
			auto object = reinterpret_cast<T*>(value);
			std::string type(this->GetTypeName());
			lua_pushfstring(L, "(%s *)0x%x", type.c_str(), object);
			return 1;
		}
	};
	template<class Base>
	class TypeInterfaceDecoratorToString<const char, Base> : public Base
	{
		int MetaToString(lua_State* L) const override
		{
			auto value = lua_touserdata(L, 1);
			const char* object = static_cast<const char *>(value);
			lua_pushstring(L, object);
			return 1;
		}
	};
	template<class Base>
	class TypeInterfaceDecoratorToString<char, Base> : public TypeInterfaceDecoratorToString<const char, Base> {};

	template<class T, class Base, bool = std::conjunction<IsCompleteType<typename std::remove_cv<T>::type>, std::is_aggregate<typename std::remove_cv<T>::type>>::value>
	class TypeInterfaceDecoratorIndex : public Base
	{
	public:
		size_t GetMemberCount() const override
		{
			return 0;
		}
		int MetaIndex(lua_State* L) const override
		{
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				if (!strcmp(lua_tostring(L, 2), "Copy"))
					return this->Copy(L);
				if (!strcmp(lua_tostring(L, 2), "Assign"))
					return this->Assign(L);
				if (!strcmp(lua_tostring(L, 2), "NewCopy"))
					return this->NewCopy(L);
				return 0;
			}
			return 0;
		}
		int MetaNewIndex(lua_State* L) const override
		{
			return 0;
		}
	};

	template<class T, class Base>
	class TypeInterfaceDecoratorIndex<T, Base, true> : public Base
	{
	public:
		size_t GetMemberCount() const override
		{
			return StructMemberCount<T>();
		}
		int MetaIndex(lua_State* L) const override
		{
			void* value = lua_touserdata(L, 1);
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				if (!strcmp(lua_tostring(L, 2), "Copy"))
					return this->Copy(L);
				if (!strcmp(lua_tostring(L, 2), "Assign"))
					return this->Assign(L);
				if (!strcmp(lua_tostring(L, 2), "NewCopy"))
					return this->NewCopy(L);

				lua_getglobal(L, "require"); // #3 = require
				auto name = nameof::nameof_short_type<typename std::decay<T>::type>();
				lua_pushstring(L, "struct/"); // #4 = "struct/"
				lua_pushlstring(L, name.data(), name.size()); // #5 = "typename"
				lua_concat(L, 2); // #4 = "struct/typename"
				lua_call(L, 1, 1);	// #3 = index table
				if (lua_isnil(L, -1))
				{
					lua_pop(L, 1);
					return 0;
				}
				lua_pushvalue(L, 2); // #4 = N
				lua_gettable(L, -2); // #4 = new N
				lua_replace(L, 2); // #3 = index table
				lua_pop(L, 1); // #2 = N => new N
			}
			int N = lua_tointeger(L, 2);
			T* object = reinterpret_cast<T*>(value);
			bool success = StructApply(*object, [L, N](auto &&...args) {
				return detail::MetaIndexImpl(L, N, std::make_index_sequence<sizeof...(args)>(), std::forward<decltype(args)>(args)...);
				});
			return success ? 1 : 0;
		}
		int MetaNewIndex(lua_State* L) const override
		{
			void* value = lua_touserdata(L, 1);
			int N = lua_tointeger(L, 2);
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				lua_getglobal(L, "require"); // #4 = require
				auto name = nameof::nameof_short_type<typename std::decay<T>::type>();
				lua_pushstring(L, "struct/"); // #5 = "struct/"
				lua_pushlstring(L, name.data(), name.size()); // #6 = "typename"
				lua_concat(L, 2); // #5 = "struct/typename"
				lua_call(L, 1, 1);	// #4 = index table
				if (lua_isnil(L, -1))
				{
					lua_pop(L, 1);
					return 0;
				}
				lua_pushvalue(L, 2); // #5 = N
				lua_gettable(L, -2); // #5 = new N
				lua_replace(L, 2); // #4 = index table
				lua_pop(L, 1); // #3 = new value
			}
			T* object = reinterpret_cast<T*>(value);
			bool success = StructApply(*object, [L, N](auto &&...args) {
				return detail::MetaNewIndexImpl(L, N, std::make_index_sequence<sizeof...(args)>(), std::forward<decltype(args)>(args)...);
				});
			return success ? 1 : 0;
		}
	};

	namespace detail {
		template<class T, class = void> struct IsFunctionType : std::false_type {};
		template<class T> struct IsFunctionType<T, std::void_t<decltype(std::function(std::declval<T>()))>> : std::true_type {};
	}

	template<class T, class Base, bool = detail::IsFunctionType<T>::value>
	class TypeInterfaceDecoratorCall : public Base
	{
		int MetaCall(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			luaL_error(L, "attempt to call a non function ref type %s", type.c_str());
			return 0;
		}
	};
	template<class T, class Base>
	class TypeInterfaceDecoratorCall<T, Base, true> : public Base
	{
		template<std::size_t...I, class Ret, class...Args>
		int MetaCallImpl2(lua_State* L, std::index_sequence<I...>, TypeIdentity<std::function<Ret(Args...)>>) const
		{
			if (int argn = lua_gettop(L); argn - 1 < (int)sizeof...(I))
			{
				luaL_error(L, "bad function call with unmatched args, excepted %d got %d", static_cast<int>(sizeof...(I)), argn - 1);
				return 0;
			}

			void* value = lua_touserdata(L, 1);
			T* pfn = reinterpret_cast<T*>(value);

			// dont support lvalue reference
			static_assert((... && !(std::is_lvalue_reference<Args>::value && !std::is_const<typename std::remove_reference<Args>::type>::value)));

			std::tuple<typename std::remove_const<typename std::remove_reference<Args>::type>::type...> args;
			(..., Get(L, I + 2, std::get<I>(args)));
			if constexpr (std::is_void_v<Ret>)
			{
				(*pfn)(std::get<I>(std::move(args))...);
				return 0;
			}
			else
			{
				Ret ret = (*pfn)(std::get<I>(std::move(args))...);
				Push(L, std::move(ret));
				return 1;
			}
		}

		template<std::size_t...I, class Ret, class...Args>
		int MetaCallImpl(lua_State* L, TypeIdentity<std::function<Ret(Args...)>> tag) const
		{
			return MetaCallImpl2(L, std::index_sequence_for<Args...>(), tag);
		}

		int MetaCall(lua_State* L) const override
		{
			return MetaCallImpl(L, TypeIdentity<decltype(std::function(std::declval<T>()))>());
		}
	};

	template<class T> class TypeInterface : public
		TypeInterfaceDecoratorCommon<T,
		TypeInterfaceDecoratorIndex<T,
		TypeInterfaceDecoratorComplete<T,
		TypeInterfaceDecoratorCall<T,
		TypeInterfaceDecoratorToString<T,
		ITypeInterface>>>>> 
	{
	public:
		TypeInterface();
	};

	template<class T>
	inline TypeInterface<T> TypeInterfaceSingleton;

	inline std::unordered_map<std::string_view, const ITypeInterface* > KnownTypeInterfaceMap;

	template<class T>
	inline TypeInterface<T>::TypeInterface()
	{
		KnownTypeInterfaceMap.emplace(this->GetTypeName(), this);
	}

	template<class T>
	const ITypeInterface* CreateTypeInterface()
	{
		const ITypeInterface* ti = &TypeInterfaceSingleton<T>;
		return ti;
	}

	inline const ITypeInterface* GetTypeInterfaceTop(lua_State* L)
	{
		// #1 = ptr
		lua_gettable(L, LUA_REGISTRYINDEX); // #1 = interface
		const ITypeInterface* ti = reinterpret_cast<const ITypeInterface *>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return ti;
	}

	inline const ITypeInterface* GetTypeInterface(lua_State* L, void* ptr)
	{
		lua_pushlightuserdata(L, ptr);
		return GetTypeInterfaceTop(L);
	}

	// function(value, i)
	template<int(ITypeInterface::* pmemfn)(lua_State*) const>
	inline int RefValueMetaDispatch(lua_State* L)
	{
		void* value = lua_touserdata(L, 1);
		if (const ITypeInterface* ti = GetTypeInterface(L, value))
			return (ti->*pmemfn)(L);
		luaL_error(L, "unrecognized ref ptr %x", value);
		return 0;
	}

	template<class T> const ITypeInterface* SetupRefTypeInterface(lua_State* L, T* ptr)
	{
		const ITypeInterface* ti = CreateTypeInterface<T>();
		lua_pushlightuserdata(L, (void *)ptr); // #1 = ptr
		lua_pushlightuserdata(L, (void *)ti); // #2 = interface
		lua_settable(L, LUA_REGISTRYINDEX); // #0
		return ti;
	}

	template<class T> void ReleaseRefTypeInterface(lua_State* L, T* ptr)
	{
		lua_pushlightuserdata(L, (void *)ptr); // #1 = ptr
		lua_pushnil(L); // #2 = nil
		lua_settable(L, LUA_REGISTRYINDEX); // #0
	}

	inline int SetupPtrMetaTable(lua_State* L)
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

	inline int CppNew(lua_State* L)
	{
		const char *name = lua_tostring(L, 1);
		if (auto iter = KnownTypeInterfaceMap.find(name); iter != KnownTypeInterfaceMap.end())
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

	inline int CppDelete(lua_State* L)
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

	template<class...Args> struct TypeList {};
	using BuiltinTypes = TypeList<bool, char, char16_t, char32_t, wchar_t, short, int, long, long long, float, double, long double>;

	template<class...Args> 
	inline void SetupBuiltinTypeInterface(TypeList<Args...>)
	{
		(..., (CreateTypeInterface<Args>()));
	}

	inline int SetupCppNewDelete(lua_State* L)
	{
		lua_newtable(L); // #1 = table
		lua_pushcfunction(L, CppNew);
		lua_setfield(L, -2, "new");
		lua_pushcfunction(L, CppDelete);
		lua_setfield(L, -2, "delete");
		lua_setglobal(L, "cpp");
		SetupBuiltinTypeInterface(BuiltinTypes());
		return 0;
	}
}
