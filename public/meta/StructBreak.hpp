#pragma once

#include "StructMemberCount.hpp"
#include <boost/preprocessor.hpp>

namespace detail {

#define APPLYER_DEF(Z,N,DATA) \
    template<class T, class ApplyFunc> decltype(auto) StructApply_impl(T &&my_struct, ApplyFunc f, std::integral_constant<std::size_t, BOOST_PP_INC(N)>) { \
        auto &&[BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(N), x)] = std::forward<T>(my_struct); \
        return f(BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(N), x)); \
    } \

    BOOST_PP_REPEAT(233, APPLYER_DEF, ~)
#undef APPLYER_DEF
}

// StructApply : 把结构体解包为变长参数调用可调用对象f
template<class T, class ApplyFunc>
decltype(auto) StructApply(T&& my_struct, ApplyFunc f)
{
    return detail::StructApply_impl(std::forward<T>(my_struct), f, StructMemberCount<typename std::decay<T>::type>());
}

// StructTransformMeta : 把结构体各成员的类型作为变长参数调用元函数MetaFunc
template<class T, template<class...> class MetaFunc> struct StructTransformMeta
{
    struct FakeApplyer
    {
        template<class...Args> auto operator()(Args ... args)->MetaFunc<decltype(args)...>;
    };
    using type = decltype(StructApply(std::declval<T>(), FakeApplyer()));
};