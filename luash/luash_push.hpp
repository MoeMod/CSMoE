#pragma once

namespace luash
{
	struct PushLambda {
		template<class T> auto operator()(T&& in) { return Push(L, std::forward<T>(in)); }
		lua_State* const L;
	};
	template<class T> auto PushBoolean(lua_State* L, T x) -> typename std::enable_if<std::is_same<T, bool>::value>::type
	{
		lua_pushboolean(L, x);
	}
	template<class T> auto PushInteger(lua_State* L, T x) -> typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type
	{
		lua_pushinteger(L, x);
	}
	template<class T> auto PushFloat(lua_State* L, T x) -> typename std::enable_if<std::is_floating_point<T>::value>::type
	{
		lua_pushnumber(L, x);
	}
	template<class T, class Ratio> auto PushFloat(lua_State* L, std::chrono::duration<T, Ratio> x) -> typename std::enable_if<std::is_floating_point<T>::value>::type
	{
		lua_pushnumber(L, std::chrono::duration_cast<std::chrono::duration<T, std::ratio<1>>>(x).count());
	}
	template<class Clock, class Duration> auto PushFloat(lua_State* L, const std::chrono::time_point<Clock, Duration> &x) -> typename std::enable_if<std::is_floating_point<typename Duration::rep>::value>::type
	{
		PushFloat(L, x.time_since_epoch());
	}
	template<class T> auto PushEnum(lua_State* L, T x) -> typename std::enable_if<std::is_enum<T>::value>::type
	{
		return Push(L, static_cast<typename std::underlying_type<T>::type>(x));
	}
	inline void PushString(lua_State* L, char* str);
	inline void PushString(lua_State* L, const char* str);
	inline void PushString(lua_State* L, const std::string& str)
	{
		lua_pushlstring(L, str.c_str(), str.size());
	}
	inline void PushString(lua_State* L, std::string_view str)
	{
		lua_pushlstring(L, str.data(), str.size());
	}
	inline void PushNil(lua_State* L, std::nullptr_t = nullptr)
	{
		lua_pushboolean(L, false);
	}
	using std::begin;
	using std::end;
	template<class T> auto PushArray(lua_State* L, T&& v) -> decltype(begin(std::forward<T>(v)), end(std::forward<T>(v)), void())
	{
		lua_newtable(L);
		int i = 0;
		for(auto &&x : std::forward<T>(v))
		{
			lua_pushinteger(L, ++i);
			Push(L, std::forward<decltype(x)>(x));
			lua_settable(L, -3);
		}
	}
	namespace detail
	{
		template<std::size_t...I, class...Args> void PushStructImpl(lua_State* L, std::index_sequence<I...>, Args &&...args)
		{
			(lua_newtable(L), ..., (lua_pushinteger(L, I + 1), Push(L, std::forward<Args>(args)), lua_settable(L, -3)));
		}
	}
	template<class T> auto PushStruct(lua_State* L, T&& v) -> typename std::enable_if<std::is_aggregate<typename std::decay<T>::type>::value>::type
	{
		StructApply(v, [L](auto &&...args)  {
			detail::PushStructImpl(L, std::make_index_sequence<sizeof...(args)>(), std::forward<decltype(args)>(args)...);
		});
		CallOnStructCreate<typename std::decay<T>::type>(L);
	}
	int PushLuaObjectByPtr(lua_State* L, void* Ptr);
	template<class T> void PushPointer(lua_State* L, T* ptr)
	{
		if (!ptr)
			return lua_pushboolean(L, false);
		if (!PushLuaObjectByPtr(L, (void*)ptr))
		{
			lua_pushlightuserdata(L, (void*)ptr); // #1 = ptr
			SetupRefTypeInterface(L, (void*)ptr, CreateTypeInterface<T>());
		}
	}
	// just unsupported
	template<class T, class U> void PushMemberPointer(lua_State* L, U T::* mem_ptr)
	{
		return lua_pushboolean(L, false);
	}
	template<class T, class U> void PushMemberPointer(lua_State* L, const U T::* const_mem_ptr)
	{
		return lua_pushboolean(L, false);
	}
	/*
	template<class T, class R, class...A> void PushMemberPointer(lua_State* L, R (T::* mem_fn_ptr)(A...))
	{
		lua_pushlightuserdata(L, nullptr);
	}
	template<class T, class R, class...A> void PushMemberPointer(lua_State* L, R(T::* const_mem_fn_ptr)(A...) const)
	{
		lua_pushlightuserdata(L, nullptr);
	}
	*/
	inline void PushString(lua_State* L, char* ptr)
	{
		PushPointer(L, ptr);
	}
	inline void PushString(lua_State* L, const char* ptr)
	{
		lua_pushstring(L, ptr);
	}
	template<class T> auto PushCustom(lua_State* L, const T& x) -> decltype(x.LuaPush(L))
	{
		return x.LuaPush(L);
	}

	namespace detail
	{
		template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<10>) -> decltype(PushCustom(L, x))
		{
			return PushCustom(L, x);
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T x, PriorityTag<7>) -> decltype(PushBoolean(L, x))
		{
			return PushBoolean(L, x);
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T x, PriorityTag<6>) -> decltype(PushEnum(L, x))
		{
			return PushEnum(L, x);
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T x, PriorityTag<5>, int = 0) -> decltype(PushInteger(L, x))
		{
			return PushInteger(L, x);
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T x, PriorityTag<5>, float = 0) -> decltype(PushFloat(L, x))
		{
			return PushFloat(L, x);
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<4>) -> decltype(PushString(L, std::forward<T>(x)))
		{
			return PushString(L, std::forward<T>(x));
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<3>) -> decltype(PushArray(L, std::forward<T>(x)))
		{
			return PushArray(L, std::forward<T>(x));
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<2>) -> decltype(PushStruct(L, std::forward<T>(x)))
		{
			return PushStruct(L, std::forward<T>(x));
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T x, PriorityTag<1>) -> decltype(PushMemberPointer(L, x))
		{
			return PushMemberPointer(L, x);
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T x, PriorityTag<0>) -> decltype(PushPointer(L, x))
		{
			return PushPointer(L, x);
		}
		template<class T> auto PushUnknownImpl(lua_State* L, T x, PriorityTag<0>, std::nullptr_t = nullptr) -> typename std::enable_if<std::is_null_pointer<T>::value>::type
		{
			return PushNil(L, x);
		}
		using MaxPriorityTag = PriorityTag<10>;
		template<class T>
		auto PushImpl(lua_State* L, T&& value) -> decltype(PushUnknownImpl(L, std::forward<T>(value), MaxPriorityTag()))
		{
			return PushUnknownImpl(L, std::forward<T>(value), MaxPriorityTag());
		}
	}
}