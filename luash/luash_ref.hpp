#pragma once

#include <lua.hpp>
#include <nameof.hpp>
#include <typeindex>
#include <tuple>
#include <unordered_map>
#include "luash_class.hpp"

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
		// hackme : ref type as pointer when pushed by __index to support a.b.c=2
		template<class T> auto MetaIndexImpl2(lua_State* L, T&& arg)
		{
			if constexpr(std::disjunction<typename std::negation<IsCompleteType<typename std::remove_cv<T>::type>>::type, std::is_aggregate<typename std::remove_cv<T>::type>, IsSupportedClass<typename std::remove_cv<T>::type>>::value)
				Push(L, &arg);
			else
				Push(L, arg);
			return true;
		}
		template<std::size_t...I, class...Args> bool MetaIndexImpl(lua_State* L, int N, std::index_sequence<I...>, Args &&...args)
		{
			return (... || ((I == N - 1) ? MetaIndexImpl2(L, std::forward<Args>(args)) : false));
		}
		template<class T> bool MetaNewIndexImpl2(lua_State* L, int N, T &arg)
		{
			Get(L, 3, arg);
			return true;
		}
		template<class T> bool MetaNewIndexImpl2(lua_State* L, int N, const T& arg)
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
	
	template<class T, class Base, bool = std::conjunction<IsCompleteType<typename std::remove_cv<T>::type>, std::negation<IsSupportedClass<typename std::remove_cv<T>::type>>>::value>
	class TypeInterfaceDecoratorCopy : public Base
	{
	public:
		int Copy(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not copy unsupported ref type %s", type.c_str());
		}
		int Assign(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not assign unsupported ref type %s", type.c_str());
		}
		int NewCopy(lua_State* L) const override
		{
			std::string type(this->GetTypeName());
			return luaL_error(L, "can not new unsupported ref type %s", type.c_str());
		}
	};	

	template<class T, class Base>
	class TypeInterfaceDecoratorCopy<T, Base, true> : public Base
	{
	public:
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

	template<class T, class Base, 
		bool = std::conjunction<IsCompleteType<typename std::remove_cv<T>::type>, std::is_aggregate<typename std::remove_cv<T>::type>>::value,
		bool = std::conjunction<IsCompleteType<typename std::remove_cv<T>::type>, IsSupportedClass<typename std::remove_cv<T>::type>>::value
	>
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
				if (!strcmp(lua_tostring(L, 2), "self"))
					return this->Copy(L);
				return 0;
			}
			return 0;
		}
		int MetaNewIndex(lua_State* L) const override
		{
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				if (!strcmp(lua_tostring(L, 2), "self"))
					return this->Assign(L);
				return 0;
			}
			return 0;
		}
	};

	namespace detail {
		void TypeInterfaceStringIndexReplace(const ITypeInterface* ti, lua_State* L, std::string_view PathPrefix, std::string_view name);
	}

	template<class T, class Base>
	class TypeInterfaceDecoratorIndex<T, Base, true, false> : public Base
	{
	public:
		size_t GetMemberCount() const override
		{
			return StructMemberCount<T>();
		}
		int MetaIndex(lua_State* L) const override
		{
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				if (!strcmp(lua_tostring(L, 2), "self"))
					return this->Copy(L);
				detail::TypeInterfaceStringIndexReplace(this, L, "struct/", nameof::nameof_short_type<typename std::decay<T>::type>());
			}
			void* value = lua_touserdata(L, 1);
			int N = lua_tointeger(L, 2);
			T* object = reinterpret_cast<T*>(value);
			bool success = StructApply(*object, [L, N](auto &&...args) {
				return detail::MetaIndexImpl(L, N, std::make_index_sequence<sizeof...(args)>(), std::forward<decltype(args)>(args)...);
				});
			return success ? 1 : 0;
		}
		int MetaNewIndex(lua_State* L) const override
		{

			if (lua_type(L, 2) == LUA_TSTRING)
			{
				if (!strcmp(lua_tostring(L, 2), "self"))
					return this->Assign(L);
				detail::TypeInterfaceStringIndexReplace(this, L, "struct/", nameof::nameof_short_type<typename std::decay<T>::type>());
			}
			void* value = lua_touserdata(L, 1);
			int N = lua_tointeger(L, 2);
			T* object = reinterpret_cast<T*>(value);
			bool success = StructApply(*object, [L, N](auto &&...args) {
				return detail::MetaNewIndexImpl(L, N, std::make_index_sequence<sizeof...(args)>(), std::forward<decltype(args)>(args)...);
				});
			if (!success)
			{
				std::string type(this->GetTypeName());
				return luaL_error(L, "can not assign field %s[%d]", type.c_str(), N);
			}
			return success;
		}
	};


	template<class T, class Base>
	class TypeInterfaceDecoratorIndex<T, Base, false, true> : public Base
	{
	public:
		static constexpr auto MemberCount = GetAmountOfPrivateDataFields<typename std::remove_const<T>::type>::value;
		size_t GetMemberCount() const override
		{
			return MemberCount;
		}
		template<std::size_t...I>
		int MetaIndexImpl(lua_State* L, T &objref, std::size_t N, std::index_sequence<I...>) const
		{
			static constexpr void (*meta_switch[sizeof...(I)])(lua_State * L, T & objref) = {
				+[](lua_State* L, T& objref) { detail::MetaIndexImpl2(L, GetStructMember<I>(objref)); }...
			};
			if (N < 1 || N > sizeof...(I))
				return 0;
			meta_switch[N - 1](L, objref);
			return 1;
		}
		template<std::size_t...I>
		int MetaNewIndexImpl(lua_State* L, T& objref, std::size_t N, std::index_sequence<I...>) const
		{
			static constexpr void (*meta_switch[sizeof...(I)])(lua_State * L, T & objref) = {
				+[](lua_State* L, T& objref) { Get(L, -1, GetStructMember<I>(objref)); }...
			};
			if (N < 1 || N > sizeof...(I))
				return 0;
			meta_switch[N - 1](L, objref);
			return 1;
		}
		int MetaIndex(lua_State* L) const override
		{
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				if (!strcmp(lua_tostring(L, 2), "self"))
					return this->Copy(L);
			}
			detail::TypeInterfaceStringIndexReplace(this, L, "class/", nameof::nameof_short_type<typename std::decay<T>::type>());
			void* value = lua_touserdata(L, 1);
			int N = lua_tointeger(L, 2);
			if (T* object = reinterpret_cast<T*>(value))
				return MetaIndexImpl(L, *object, N, std::make_index_sequence<MemberCount>());
			return 0;
		}
		int MetaNewIndex(lua_State* L) const override
		{
			if (lua_type(L, 2) == LUA_TSTRING)
			{
				if (!strcmp(lua_tostring(L, 2), "self"))
					return this->Assign(L);
			}
			detail::TypeInterfaceStringIndexReplace(this, L, "class/", nameof::nameof_short_type<typename std::decay<T>::type>());
			void* value = lua_touserdata(L, 1);
			int N = lua_tointeger(L, 2);
			T* object = reinterpret_cast<T*>(value);
			if (T* object = reinterpret_cast<T*>(value))
				return MetaNewIndexImpl(L, *object, N, std::make_index_sequence<MemberCount>());
			return 0;
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

	template<class T> inline bool PushIfLvRef(lua_State* L, T& obj, std::true_type)
	{
		Push(L, obj);
		return true;
	}
	template<class T> constexpr bool PushIfLvRef(lua_State* L, T&& obj, std::false_type)
	{
		return false;
	}

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
			//static_assert((... && !(std::is_lvalue_reference<Args>::value && !std::is_const<typename std::remove_reference<Args>::type>::value)));

			std::tuple<typename std::remove_const<typename std::remove_reference<Args>::type>::type...> args;
			(..., Get(L, I + 2, std::get<I>(args)));
			if constexpr (std::is_void_v<Ret>)
			{
				(*pfn)(std::get<I>(args)...);
			}
			else
			{
				Ret ret = (*pfn)(std::get<I>(args)...);
				Push(L, std::move(ret));
			}

			return (!std::is_void_v<Ret> + ... + PushIfLvRef(L, std::get<I>(args), std::bool_constant<(std::is_lvalue_reference<Args>::value && !std::is_const<typename std::remove_reference<Args>::type>::value)>()));
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
		TypeInterfaceDecoratorCopy<T,
		TypeInterfaceDecoratorCall<T,
		TypeInterfaceDecoratorToString<T,
		ITypeInterface>>>>>>
	{
	public:
		TypeInterface();
	};

	template<class T>
	inline TypeInterface<T> TypeInterfaceSingleton;

	std::unordered_map<std::string_view, const ITypeInterface* > &KnownTypeInterfaceMap();

	template<class T>
	inline TypeInterface<T>::TypeInterface()
	{
		KnownTypeInterfaceMap().emplace(this->GetTypeName(), this);
	}

	template<class T>
	const ITypeInterface* CreateTypeInterface()
	{
		const ITypeInterface* ti = &TypeInterfaceSingleton<T>;
		return ti;
	}

	const ITypeInterface* GetTypeInterfaceTop(lua_State* L);

	const ITypeInterface* GetTypeInterface(lua_State* L, void* ptr);

	// function(value, i)
	template<int(ITypeInterface::* pmemfn)(lua_State*) const> int RefValueMetaDispatch(lua_State* L)
	{
		void* value = lua_touserdata(L, 1);
		if (const ITypeInterface* ti = GetTypeInterface(L, value))
			return (ti->*pmemfn)(L);
		luaL_error(L, "unrecognized ref ptr %x", value);
		return 0;
	}

	int SetupTypeInterfaceTable(lua_State* L);
	int SetupBuiltinTypeInterface(lua_State* L);
	int SetupPtrMetaTable(lua_State* L);
	int SetupCppNewDelete(lua_State* L);
}
