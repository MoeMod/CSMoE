#pragma once

#include "meta/LambdaUtil.h"

namespace luash
{
    template<class T> inline bool PushIfLvRef(lua_State* L, T& obj, std::true_type)
    {
        Push(L, obj);
        return true;
    }
    template<class T> constexpr bool PushIfLvRef(lua_State* L, T&& obj, std::false_type)
    {
        return false;
    }

    template<class T, class = typename GetParameterType<T>::type>
    struct LuaStatelessLambdaCaller;
    template<class T, class Ret, class...Args>
    struct LuaStatelessLambdaCaller<T, Ret(Args...)>
    {
    public:
        template<std::size_t...I>
        static int MetaCallImpl(lua_State* L, std::index_sequence<I...>)
        {
            std::tuple<typename std::decay<typename std::remove_reference<Args>::type>::type...> args;
            (..., Get(L, I + 1, std::get<I>(args)));
            if constexpr (std::is_void_v<Ret>)
                ConstructStatelessLambda<T>()(std::forward<Args>(std::get<I>(args))...);
            else
                Push(L, (Ret)(ConstructStatelessLambda<T>()(std::forward<Args>(std::get<I>(args))...)));

            return (!std::is_void_v<Ret> + ... + PushIfLvRef(L, std::forward<Args>(std::get<I>(args)), std::bool_constant<(std::is_lvalue_reference<Args>::value && !std::is_const<typename std::remove_reference<Args>::type>::value)>()));
        }

        static int MetaCall(lua_State* L)
        {
            return MetaCallImpl(L, std::index_sequence_for<Args...>());
        }
    };
}