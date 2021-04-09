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
			void PushFunctionDecl(std::index_sequence<I...>, Ret(*pfn)(Args...))
			{
				lua_CFunction f = [](lua_State* L) -> int
				{
					if (lua_gettop(L) < sizeof...(I))
						return 0;
					const int arg_offset = 1;
					Ret (*pfn)(Args...) = lua_topointer(L, 1);
					
					std::tuple<typename std::remove_const<typename std::remove_reference<Args>::type>::type...> args;
					(..., Get(L, I + 1 + arg_offset, std::get<I>(args)));
					Ret ret = (*pfn)(std::get<I>(args));
					Push(L, std::move(ret));
					return 1;
				};
				lua_pushcfunction(L, f);
			}
		}
		template<class Ret, class...Args>
		void PushFunctionDecl(Ret (*pfn)(Args...))
		{
			detail::PushFunctionDecl(std::index_sequence_for<Args...>(), pfn);
		}
	}
}