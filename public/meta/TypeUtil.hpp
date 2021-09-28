
#pragma once

#include <cstdint>

template<std::size_t N> struct PriorityTag : PriorityTag<N - 1> {};
template<> struct PriorityTag<0> {};

template<class T> struct TypeIdentity
{
    using type = T;
};


template<class T, class = void> struct IsCompleteType : std::false_type {};
template<class T> struct IsCompleteType<T, decltype(sizeof(T) > 0, void())> : std::true_type{};