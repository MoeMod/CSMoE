#pragma once

#include <type_traits>
#include <utility>

namespace detail
{
    // 构造一个可以隐式转换为任意类型的类型
    template<class T>
    struct any_converter {
        // 不能convert至自身
        template <class U, class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value>::type> operator U &&() const noexcept;
    };

    template<class T, std::size_t I> struct any_converter_tagged : any_converter<T> {};

    // 判断T是否可以使用Args...进行聚合初始化：T{ std::declval<Args>()... }
    template<class T, std::size_t...I> constexpr auto is_aggregate_constructible_with_n_args_impl(T&&, std::index_sequence<I...>) -> decltype(T{ any_converter_tagged<T, I>()... }, std::true_type());
    // 这个函数千万别改成模板函数否则会死机的！
    constexpr auto is_aggregate_constructible_with_n_args_impl(...)->std::false_type;

    template<class T, class Seq> struct is_aggregate_constructible_with_n_args : decltype(is_aggregate_constructible_with_n_args_impl(std::declval<T>(), Seq())) {};
    // 这里添加一个元函数是用来支持沙雕GCC的，不知道为什么GCC里面变长模板参数不能作为嵌套的实参展开……
    template<class T, std::size_t N> struct is_aggregate_constructible_with_n_args_ex : is_aggregate_constructible_with_n_args<T, std::make_index_sequence<N>> {};

    // （原）线性查找法
    template<class T, class Seq = std::make_index_sequence<sizeof(T)>> struct struct_member_count_impl1;
    template<class T, std::size_t...I> struct struct_member_count_impl1<T, std::index_sequence<I...>> : std::integral_constant<std::size_t, (... + !!(is_aggregate_constructible_with_n_args_ex<T, I + 1>::value))> {};

    template<class B, class T, class U> struct lazy_conditional : lazy_conditional<typename B::type, T, U> {};
    template<class T, class U> struct lazy_conditional<std::true_type, T, U> : T {};
    template<class T, class U> struct lazy_conditional<std::false_type, T, U> : U {};

	// 二分查找法
    template<class T, class Seq = std::index_sequence<0>> struct struct_member_count_impl2;
    template<class T, std::size_t...I> struct struct_member_count_impl2<T, std::index_sequence<I...>> : lazy_conditional<
        std::conjunction<is_aggregate_constructible_with_n_args_ex<T, I + 1>...>,
        struct_member_count_impl2<T, std::index_sequence<I..., (I + sizeof...(I))...>>,
        std::integral_constant<std::size_t, (... + !!(is_aggregate_constructible_with_n_args_ex<T, I + 1>::value))>
    >::type {};
    
}

template<class T> struct StructMemberCount : detail::struct_member_count_impl2<T> {};