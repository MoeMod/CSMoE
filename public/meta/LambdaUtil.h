#pragma once

#include <type_traits>

struct AnyType {
    template<class T> operator T && () const;
};

namespace detail
{
	template<class T> struct TypeIdentity : std::enable_if<true, T> {};
	template<class T, std::size_t...I> auto IsArgNumSfinae(T *x, std::index_sequence<I...>) -> decltype((*x)((I, AnyType())...), std::true_type());
	auto IsArgNumSfinae(...) -> std::false_type;

	template<class T, std::size_t N> struct DeclGen {
	    friend constexpr auto loophole(DeclGen);
	};
	template<class T, std::size_t N, class Value> struct DefiGen {
	    friend constexpr auto loophole(DeclGen<T, N>) { return detail::TypeIdentity<Value>{}; }
	};
	template<class T, std::size_t N> struct Inspector {
	    template<class Value, class = decltype(DefiGen<T, N, Value>())> operator Value();
	};

#if _MSC_VER
	template<class T, std::size_t N> struct GetParameterTypeImpl3 : decltype(loophole(DeclGen<T, N>{})) {};
	template<class Ret, class T, std::size_t...I> struct GetParameterTypeImpl2 : detail::TypeIdentity < Ret(typename GetParameterTypeImpl3<T, I>::type...) > {};
#else
	template<class Ret, class T, std::size_t...I> struct GetParameterTypeImpl2 : detail::TypeIdentity < Ret(typename decltype(loophole(detail::DeclGen<T, I>{}))::type...) > {};
#endif
	template<class T, class Seq> struct GetParameterTypeImpl;
	template<class T, std::size_t...I> struct GetParameterTypeImpl<T, std::index_sequence<I...>> : GetParameterTypeImpl2<decltype(std::declval<T>()(detail::Inspector<T, I>()...)), T, I...> {};
}

template<class T, std::size_t N = 0> struct GetArgNum : std::conditional< decltype(detail::IsArgNumSfinae((T *)nullptr, std::make_index_sequence<N>()))::value, std::integral_constant<std::size_t, N>, GetArgNum<T, N + 1>>::type {};
template<class T> struct GetParameterType2 : detail::GetParameterTypeImpl <T, std::make_index_sequence<GetArgNum<T>::value>> {};

namespace detail
{
	template<class>
	struct DeduceOperatorInvokeType;
	template<class T, class Ret, class...Args>
	struct DeduceOperatorInvokeType<Ret(T::*)(Args...)>
	{
		using type = Ret(Args...);
	};
	template<class T, class Ret, class...Args>
	struct DeduceOperatorInvokeType<Ret(T::*)(Args...) const>
	{
		using type = Ret(Args...);
	};
}

template<class T> struct GetParameterType : detail::DeduceOperatorInvokeType<decltype(&T::operator())> {};

namespace detail
{
	template<class T> auto HasSingleOperatorInvokeSfinae(T *) -> decltype(&T::operator(), std::true_type());
	std::false_type HasSingleOperatorInvokeSfinae(...);
}

template<class T> struct HasSingleOperatorInvoke : decltype(detail::HasSingleOperatorInvokeSfinae((T *)nullptr)) {};
 
template<class T> struct IsStatelessLambda : std::integral_constant<bool, std::is_class<T>::value && std::is_empty<T>::value && HasSingleOperatorInvoke<T>::value> {};

// only C++20 lambdas have default constructor, hack for C++14 here
namespace detail
{
	template <class LambdaType>
	auto ConstructStatelessLambda(std::false_type) -> typename std::enable_if<std::is_empty<LambdaType>::value, LambdaType>::type
	{
	    char a;
	    return reinterpret_cast<LambdaType&>(a);
	}
	template <class LambdaType>
	auto ConstructStatelessLambda(std::true_type) -> typename std::enable_if<std::is_empty<LambdaType>::value, LambdaType>::type
	{
	    return LambdaType();
	}
}    // namespace detail

template <class LambdaType>
auto ConstructStatelessLambda() -> typename std::enable_if<std::is_empty<LambdaType>::value, LambdaType>::type
{
    return detail::ConstructStatelessLambda<LambdaType>(std::is_default_constructible<LambdaType>());
}

template <class LambdaType, class... Args>
auto InvokeStatelessLambda(Args&&... args)
{
    return ConstructStatelessLambda<LambdaType>()(std::forward<Args>(args)...);
}