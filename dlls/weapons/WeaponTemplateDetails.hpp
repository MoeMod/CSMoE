/*
WeaponTemplateDetails.hpp
Copyright (C) 2019 Moemod Haoyuan

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

namespace detail
{
    template<std::size_t N> struct priority_tag : priority_tag<N-1> {};
    template<> struct priority_tag<0> {};

}

namespace df
{
    using detail::priority_tag;
// must be used in namespace detail
#define GENERATE_TEMPLATE_GETTER(FieldName) namespace FieldName {\
    namespace impl { \
        template<class T> constexpr auto Get_impl(T &&data, priority_tag<2>) -> decltype(data.FieldName) { return data.FieldName; } \
        template<class T> constexpr auto Get_impl(T &&data, priority_tag<3>) -> decltype(data.Get##FieldName ()) { return data.Get##FieldName (); } \
        template<class T> constexpr auto Get_impl(T &&data, priority_tag<4>) -> decltype(data.FieldName()) { return data.FieldName (); } \
        template<class T> constexpr auto Get(T &&data) -> decltype(Get_impl(data, priority_tag<5>())) { return Get_impl (data, priority_tag<5>()); } \
        template<class T> constexpr auto Has_impl(T &&data, priority_tag<0>) -> std::false_type { return {}; } \
        template<class T> constexpr auto Has_impl(T &&data, priority_tag<1>) -> decltype(Get(data), std::true_type()) { return {}; } \
        template<class T> constexpr auto Has_impl(T &&data, priority_tag<2>) -> decltype(data.Has##FieldName()) { return {}; } \
        template<class T> constexpr auto Has_impl(T &&data, priority_tag<3>) -> decltype(data.has_##FieldName()) { return {}; } \
        template<class T> constexpr auto Has(T &&data) -> decltype(Has_impl(data, priority_tag<2>())) { return Has_impl(data, priority_tag<4>()); } \
        template<class T, class V> constexpr auto Try_impl(T &&data, V &&defaultval, std::true_type) -> decltype(Get(data)) { return Get(data); } \
        template<class T, class V> constexpr auto Try_impl(T &&data, V &&defaultval, std::false_type) -> typename std::decay<V>::type  { return std::forward<V>(defaultval); } \
        template<class T, class V> constexpr auto Try(T &&data, V &&defaultValue) -> decltype(Try_impl(data, std::forward<V>(defaultValue), Has(data))) { return Try_impl(data, std::forward<V>(defaultValue), Has(data)); } \
    } \
    using impl::Get; \
    using impl::Has; \
    using impl::Try; \
    }

#define GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(FieldName, DefaultValue) namespace FieldName { \
    namespace impl { \
        template<class T> constexpr auto Get_impl(T &&data, priority_tag<1>) -> decltype(DefaultValue) { return DefaultValue; }; \
    } \
    } \
	GENERATE_TEMPLATE_GETTER(FieldName)

}