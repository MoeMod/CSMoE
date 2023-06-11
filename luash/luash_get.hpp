#pragma once


namespace luash
{
	inline void GetBoolean(lua_State* L, int N, bool& x)
	{
		x = lua_toboolean(L, N);
	}
	template<class T> auto GetInteger(lua_State* L, int N, T &x) -> typename std::enable_if<std::is_integral<T>::value>::type
	{
		x = lua_tointeger(L, N);
	}
	template<class T> auto GetFloat(lua_State* L, int N, T& x) -> typename std::enable_if<std::is_floating_point<T>::value>::type
	{
		x = lua_tonumber(L, N);
	}
	template<class T, class Ratio> auto GetFloat(lua_State* L, int N, std::chrono::duration<T, Ratio> &x) -> typename std::enable_if<std::is_floating_point<T>::value>::type
	{
		T y;
		GetFloat(L, N, y);
		x = std::chrono::duration<T, std::ratio<1>>(y);
	}
	template<class Clock, class Duration> auto GetFloat(lua_State* L, int N, std::chrono::time_point<Clock, Duration>& x) -> typename std::enable_if<std::is_floating_point<typename Duration::rep>::value>::type
	{
		Duration z;
		GetFloat(L, N, z);
		x = std::chrono::time_point<Clock, Duration>() + z;
	}
	template<class T> auto GetEnum(lua_State* L, int N, T &x) -> typename std::enable_if<std::is_enum<T>::value>::type
	{
		typename std::underlying_type<T>::type y;
		GetInteger(L, N, y);
		x = static_cast<T>(y);
	}
	inline void GetString(lua_State* L, int N, const char* &sz)
	{
		if (void *data = lua_touserdata(L, N))
		{
			sz = reinterpret_cast<const char*>(data);
		}
		else
		{
			sz = lua_tostring(L, N);
		}
	}
	inline void GetString(lua_State* L, int N, std::string& str)
	{
		str = lua_tostring(L, N);
	}
	inline void GetString(lua_State* L, int N, std::string_view &sv)
	{
		sv = lua_tostring(L, N);
	}
	inline void GetNil(lua_State* L, int N, std::nullptr_t& np) {}
	template<class T, std::size_t Size> auto GetArray(lua_State* L, int N, std::array<T, Size>& arr)
	{
		lua_pushvalue(L, N);
		// #1 = arr
		for(std::size_t i = 0; i < Size; ++i)
		{
			lua_pushinteger(L, i + 1);
			// #2 = i + 1
			lua_gettable(L, -2);
			// #2 = arr[i+1]
			Get(L, -1, arr[i]);
			//
			lua_pop(L, 1);
			// #1 = arr
		}
		lua_pop(L, 1);
		// #0
	}
	template<class T, std::size_t Size> auto GetArray(lua_State* L, int N, T(&arr)[Size])
	{
		lua_pushvalue(L, N);
		// #1 = arr
		for (std::size_t i = 0; i < Size; ++i)
		{
			lua_pushinteger(L, i + 1);
			// #2 = i + 1
			lua_gettable(L, -2);
			// #2 = arr[i+1]
			Get(L, -1, arr[i]);
			//
			lua_pop(L, 1);
			// #1 = arr
		}
		lua_pop(L, 1);
		// #0
	}
	template<class T> auto GetArray(lua_State* L, int N, T& v) -> decltype(v.begin(), v.end(), v.resize(0), void())
	{
		lua_pushvalue(L, N); // #1 = arr
		const auto Size = lua_rawlen(L, -1);
		v.resize(Size);
		int i = 0;
		for (auto& x : v)
		{
			lua_pushinteger(L, ++i); // #2 = i + 1
			lua_gettable(L, -2); // #2 = arr[i+1]
			Get(L, -1, x);
			lua_pop(L, 1); // #1 = arr
		}
		lua_pop(L, 1); // #0
	}
	namespace detail
	{
		template<std::size_t...I, class...Args> void GetStructImpl(lua_State* L, std::index_sequence<I...>, Args &...args)
		{
			// #1 = struct
			(..., (
				lua_pushinteger(L, I + 1),
				// #2 = I+1
				lua_gettable(L, -2),
				// #2 = vec[I+1]
				Get(L, -1, args),
				//
				lua_pop(L, 1)
				// #1 = struct
				));
		}
	}
	template<class T> auto GetStruct(lua_State* L, int N, T& v) -> typename std::enable_if<std::is_aggregate<typename std::decay<T>::type>::value>::type
	{
		lua_pushvalue(L, N);
		// #1 = struct
		StructApply(v, [L](auto &...args) {
			detail::GetStructImpl(L, std::make_index_sequence<sizeof...(args)>(), args...);
			});
		lua_pop(L, 1);
		// #0
	}

	bool GetPtrByLuaObject(lua_State* L, int N, void*&);
	template<class T>
	inline void GetPointer(lua_State* L, int N, T* &out)
	{
		if (void* ptr = nullptr; GetPtrByLuaObject(L, N, ptr))
		{
			out = reinterpret_cast<T*>(ptr);
			return;
		}
		void* ptr = lua_touserdata(L, N);
		if (const ITypeInterface* ti = GetTypeInterface(L, ptr))
		{
			out = reinterpret_cast<T*>(ptr);
			return;
		}
		out = nullptr;
		return;
	}
	template<class T> auto GetStatelessLambda(lua_State* L, int N, T& const_mem_ptr) -> typename std::enable_if<IsStatelessLambda<T>::value>::type
	{
		// ignore stateless lambda
	}
	// just unsupported
	template<class C, class U> void GetMemberPointer(lua_State* L, int N, U C::* &mem_ptr)
	{
		mem_ptr = nullptr;
	}
	template<class C, class U> void GetMemberPointer(lua_State* L, int N, const U C::* &const_mem_ptr)
	{
		const_mem_ptr = nullptr;
	}
	/*
	template<class C, class R, class...A> void GetMemberPointer(lua_State* L, int N, R(C::* &mem_fn_ptr)(A...))
	{
		mem_fn_ptr = nullptr;
	}
	template<class C, class R, class...A> void GetMemberPointer(lua_State* L, int N, R(C::* &const_mem_fn_ptr)(A...) const)
	{
		const_mem_fn_ptr = nullptr;
	}
	*/
	template<class T> auto GetCustom(lua_State* L, int N, T& x) -> decltype(x.LuaGet(L, N))
	{
		return x.LuaGet(L, N);
	}

	namespace detail
	{
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<10>) -> decltype(GetCustom(L, N, x))
		{
			return GetCustom(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<8>) -> decltype(GetBoolean(L, N, x))
		{
			return GetBoolean(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<7>) -> decltype(GetEnum(L, N, x))
		{
			return GetEnum(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<6>, int = 0) -> decltype(GetInteger(L, N, x))
		{
			return GetInteger(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<6>, float = 0) -> decltype(GetFloat(L, N, x))
		{
			return GetFloat(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<5>) -> decltype(GetString(L, N, x))
		{
			return GetString(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<4>) -> decltype(GetEntity(L, N, x))
		{
			return GetEntity(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<4>) -> decltype(GetVector(L, N, x))
		{
			return GetVector(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<3>) -> decltype(GetArray(L, N, x))
		{
			return GetArray(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<2>) -> decltype(GetStruct(L, N, x))
		{
			return GetStruct(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<1>) -> decltype(GetMemberPointer(L, N, x))
		{
			return GetMemberPointer(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<1>) -> decltype(GetStatelessLambda(L, N, x))
		{
			return GetStatelessLambda(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<0>) -> decltype(GetPointer(L, N, x))
		{
			return GetPointer(L, N, x);
		}
		template<class T> auto GetUnknownImpl(lua_State* L, int N, T& x, PriorityTag<0>, std::nullptr_t = nullptr) -> typename std::enable_if<std::is_null_pointer<T>::value>::type
		{
			return GetNil(L, N, x);
		}
		using MaxPriorityTag = PriorityTag<10>;
		template<class T>
		auto GetImpl(lua_State* L, int N, T& value) -> decltype(GetUnknownImpl(L, N, value, MaxPriorityTag()))
		{
			return GetUnknownImpl(L, N, value, MaxPriorityTag());
		}
	}
}