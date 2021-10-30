#pragma once

#include <type_traits>

template<class T> struct TypeIdentity : std::enable_if<true, T> {};
template<class T> struct type_identity : TypeIdentity<T> {};
