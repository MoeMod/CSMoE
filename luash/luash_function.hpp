#pragma once

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	namespace luash
	{
		namespace detail
		{
			template<class Ret, class...Args, std::size_t...I>
			constexpr lua_CFunction TransformFunctionDecl(std::index_sequence<I...>, lua_State* L, Ret(*pfn)(Args...))
			{
				return +[](lua_State* L) -> int
				{
					if (int argn = lua_gettop(L); argn < sizeof...(I))
					{
						luaL_error(L, "bad function call with unmatched args, excepted %d got %d", static_cast<int>(sizeof...(I)), argn);
						return 0;
					}
					Ret (*pfn)(Args...) = reinterpret_cast<Ret(*)(Args...)>(lua_touserdata(L, lua_upvalueindex(1)));
					
					std::tuple<typename std::remove_const<typename std::remove_reference<Args>::type>::type...> args;
					(..., Get(L, I + 1, std::get<I>(args)));
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
				};
			}
		}
		template<class Ret, class...Args>
		void PushFunction(lua_State * L, Ret (*pfn)(Args...))
		{
			lua_CFunction f = detail::TransformFunctionDecl(std::index_sequence_for<Args...>(), L, pfn);
			lua_pushlightuserdata(L, reinterpret_cast<void *>(f));
			lua_pushcclosure(L, f, 1);
		}
	}
}