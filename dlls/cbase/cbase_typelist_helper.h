/*
cbase_typelist.h - CSMoE Gameplay server
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include <type_traits>

template<class...Ts> struct TypeList {};

template<class, class> struct TypeList_Append;
template<template<class...> class L, class...Ts, class T> struct TypeList_Append<L<Ts...>, T> { using type = L<Ts..., T>; };
template<class> struct TypeList_Size;
template<template<class...> class L, class...Ts> struct TypeList_Size < L<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

// specialize on every new line to append new type
template<std::size_t Line> struct CurrentList : CurrentList<Line - 1> {};
template<> struct CurrentList<0> { using type = TypeList<>; };

#define TL_ADD(_Ty) DECLEAR_ENTITY_CLASS(_Ty) template<> struct CurrentList<__LINE__> { using type = typename TypeList_Append<typename CurrentList<__LINE__ - 1>::type, _Ty>::type; };
#define TL_END(_Ls) using _Ls = typename CurrentList<__LINE__ - 1>::type;