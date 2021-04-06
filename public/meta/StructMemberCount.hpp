#pragma once

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
    constexpr auto is_aggregate_constructible_impl(...)->std::false_type;

    template<class T, class...Args> struct is_aggregate_constructible : decltype(is_aggregate_constructible_impl(std::declval<T>(), std::declval<Args>()...)) {};

    template<class T, class Seq> struct is_aggregate_constructible_with_n_args;
    template<class T, std::size_t...I> struct is_aggregate_constructible_with_n_args<T, std::index_sequence<I...>> : is_aggregate_constructible<T, any_converter_tagged<T, I>...> {};

    // （原）线性查找法
    template<class T, class Seq = std::make_index_sequence<sizeof(T)>> struct struct_member_count_impl1;
    template<class T, std::size_t...I> struct struct_member_count_impl1<T, std::index_sequence<I...>> : std::integral_constant<std::size_t, (... + !!(is_aggregate_constructible_with_n_args<T, std::make_index_sequence<I + 1>>::value))> {};

    template<class B, class T, class U> struct lazy_conditional : lazy_conditional<typename B::type, T, U> {};
    template<class T, class U> struct lazy_conditional<std::true_type, T, U> : T {};
    template<class T, class U> struct lazy_conditional<std::false_type, T, U> : U {};

	// 二分查找法
    template<class T, class Seq = std::index_sequence<0>> struct struct_member_count_impl2;
    template<class T, std::size_t...I> struct struct_member_count_impl2<T, std::index_sequence<I...>> : lazy_conditional<
        std::conjunction<is_aggregate_constructible_with_n_args<T, std::make_index_sequence<I + 1>>...>,
        struct_member_count_impl2<T, std::index_sequence<I..., (I + sizeof...(I))...>>,
        std::integral_constant<std::size_t, (... + !!(is_aggregate_constructible_with_n_args<T, std::make_index_sequence<I + 1>>::value))>
    >::type {};
    
}

template<class T> struct StructMemberCount : detail::struct_member_count_impl2<T> {};