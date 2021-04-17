
#pragma once

#include <cstdint>

template<std::size_t N> struct PriorityTag : PriorityTag<N - 1> {};
template<> struct PriorityTag<0> {};

template<class T> struct TypeIdentity
{
    using type = T;
};