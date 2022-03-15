#pragma once

#include "u_vector.hpp"
#include "lua.hpp"

namespace moe {

template<class VecType, std::size_t...I>
inline void LuaPush_impl(lua_State* L, VecType vec, std::index_sequence<I...>)
{
    return (lua_newtable(L), ..., ( // #1 = res
            lua_pushnumber(L, vec.template get<I>()), // #2 = res[1]
                    lua_rawseti(L, -2, I + 1) // #1
    ));
}
template<class VecType, std::size_t...I>
inline void LuaGet_impl(lua_State* L, int idx, VecType &vec, std::index_sequence<I...>)
{
    lua_pushvalue(L, idx);
    vec = {
            (lua_rawgeti(L, -1 - (int)I, I + 1), static_cast<typename VecType::value_type>(lua_tonumber(L, -1)))...
    };
    lua_pop(L, 1 + (int)sizeof...(I));
}

template<class T, std::size_t N, std::size_t Align>
void VectorBase<T, N, Align>::LuaPush(lua_State* L) const
{
    return LuaPush_impl(L, *this, std::make_index_sequence<N>());
}

template<class T, std::size_t N, std::size_t Align>
void VectorBase<T, N, Align>::LuaGet(lua_State* L, int idx)
{
    return LuaGet_impl(L, idx, *this, std::make_index_sequence<N>());
}

}