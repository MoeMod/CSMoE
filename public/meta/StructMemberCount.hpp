#pragma once

#include "TypeUtil.hpp"
#include <type_traits>
#include <utility>

namespace detail
{
    // 构造一个可以隐式转换为任意类型的类型
    template<class T>
    struct any_converter {
        // 不能convert至自身
        template <class U, class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value>::type> constexpr operator U ();
    };

    template<class T, std::size_t I> struct any_converter_tagged : any_converter<T> {};

    // 判断T是否可以使用Args...进行聚合初始化：T{ std::declval<Args>()... }
    template<class T, class...Args> constexpr auto is_aggregate_constructible_impl(T&&, Args &&...args) -> decltype(T{ {args}... }, std::true_type());
    // 多加一个重载可以去掉讨厌的clang warning
    template<class T, class Arg> constexpr auto is_aggregate_constructible_impl(T&&, Arg&& args) -> decltype(T{ args }, std::true_type());
    // 这个函数千万别改成模板函数否则会死机的！
    //template<class T> constexpr auto is_aggregate_constructible_impl(T &&, ...) -> std::false_type;
    constexpr auto is_aggregate_constructible_impl(...)->std::false_type;

    template<class T, class...Args> struct is_aggregate_constructible : decltype(is_aggregate_constructible_impl(std::declval<T>(), std::declval<Args>()...)) {};

    template<class T, std::size_t...I>
    constexpr auto StructMemberCount_impl(std::index_sequence<I...>, PriorityTag<2>) -> typename std::enable_if<is_aggregate_constructible<T, any_converter_tagged<T, I>...>::value, std::size_t>::type { return sizeof...(I); }

    constexpr auto StructMemberCount_impl(std::index_sequence<>, PriorityTag<1>) -> std::size_t { return 0; }

    template<class T, class Seq>
    constexpr auto StructMemberCount_impl(Seq, PriorityTag<0>) -> std::size_t { return StructMemberCount_impl<T>(std::make_index_sequence<Seq::size() - 1>(), PriorityTag<2>()); };

    template<class T>
    constexpr auto StructMemberCount_impl() -> std::size_t { static_assert(sizeof(T) > 0, "Error : T should be complete type"); return StructMemberCount_impl<T>(std::make_index_sequence<sizeof(T)>(), PriorityTag<2>()); };
}

template<class T> struct StructMemberCount : std::integral_constant<std::size_t, detail::StructMemberCount_impl< typename std::remove_cv<typename std::remove_reference<T>::type>::type >()> {};