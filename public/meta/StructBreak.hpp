#pragma once

#include "StructMemberCount.hpp"
#include "EnumParams.h"
#include "EnumForEach.h"

#include <functional>

namespace detail {

#define APPLYER_DEF(N) \
    template<class T, class ApplyFunc> auto StructApply_impl(T &&my_struct, ApplyFunc f, std::integral_constant<std::size_t, N>) { \
        auto &&[ENUM_PARAMS(x, N)] = std::forward<T>(my_struct); \
        return std::invoke(f, ENUM_PARAMS(x, N)); \
    }; \

    ENUM_FOR_EACH(APPLYER_DEF, 128)
#undef APPLYER_DEF
}

// StructApply : 把结构体解包为变长参数调用可调用对象f
template<class T, class ApplyFunc>
auto StructApply(T&& my_struct, ApplyFunc f)
{
    return detail::StructApply_impl(std::forward<T>(my_struct), f, StructMemberCount<typename std::decay<T>::type>());
};

// StructTransformMeta : 把结构体各成员的类型作为变长参数调用元函数MetaFunc
template<class T, template<class...> class MetaFunc> struct StructTransformMeta
{
    struct FakeApplyer
    {
        template<class...Args> auto operator()(Args ... args)->MetaFunc<decltype(args)...>;
    };
    using type = decltype(StructApply(std::declval<T>(), FakeApplyer()));
};