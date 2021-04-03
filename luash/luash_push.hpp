#pragma once

#include "util/u_convert.hpp"

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace luash
	{
		struct PushLambda {
			template<class T> auto operator()(T&& in) { return Push(L, std::forward<T>(in)); }
			lua_State* const L;
		};
		
		template<class T> auto PushInteger(lua_State* L, T x) -> typename std::enable_if<std::is_integral<T>::value>::type
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
		template<class T, class Duration> auto PushFloat(lua_State* L, std::chrono::time_point<Duration> x) -> decltype(PushFloat(L, x.time_since_epoch()))
		{
			PushFloat(L, x.time_since_epoch());
		}
		template<class T> auto PushEnum(lua_State* L, T x) -> typename std::enable_if<std::is_enum<T>::value>::type
		{
			return Push(L, static_cast<typename std::underlying_type<T>::type>(x));
		}
		inline void PushString(lua_State* L, const char* str)
		{
			lua_pushstring(L, str);
		}
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
			lua_pushnil(L);
		}
		template<class T> auto PushEntity(lua_State* L, T ent) -> typename std::enable_if<std::is_invocable<moe::convert::ConvertFunc<T, edict_t *>, T>::value>::type
		{
			int id = ent_cast<int>(ent);
			return PushInteger(L, id);
		}
		namespace detail {
			template<class T, std::size_t N, std::size_t...I>
			inline void PushVectorImpl(lua_State* L, const moe::VectorBase<T, N>& x, std::index_sequence<I...>)
			{
				(lua_newtable(L), ..., (lua_pushinteger(L, I + 1), Push(L, x[I]), lua_settable(L, -3)));
			}
		}
		template<class T, std::size_t N> void PushVector(lua_State* L, const moe::VectorBase<T, N>& x)
		{
			detail::PushVectorImpl(L, x, std::make_index_sequence<N>());
		}
		using std::begin;
		using std::end;
		template<class T> auto PushArray(lua_State* L, T&& v) -> decltype(begin(std::forward<T>(v)), end(std::forward<T>(v)), void())
		{
			int i = 0;
			for(auto &&x : std::forward<T>(v))
			{
				lua_newtable(L);
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

		namespace detail
		{
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
			template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<3>) -> decltype(PushEntity(L, std::forward<T>(x)))
			{
				return PushEntity(L, std::forward<T>(x));
			}
			template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<3>) -> decltype(PushVector(L, std::forward<T>(x)))
			{
				return PushVector(L, std::forward<T>(x));
			}
			template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<2>) -> decltype(PushArray(L, std::forward<T>(x)))
			{
				return PushArray(L, std::forward<T>(x));
			}
			template<class T> auto PushUnknownImpl(lua_State* L, T &&x, PriorityTag<1>) -> decltype(PushStruct(L, std::forward<T>(x)))
			{
				return PushStruct(L, std::forward<T>(x));
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
}