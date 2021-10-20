#pragma once

#include "meta/StructMemberCount.hpp"
#include "meta/StructBreak.hpp"
#include <tuple>

namespace luash
{
	template<class T, class = void> struct IsSupportedClass : std::false_type {};
	template<class T> struct IsSupportedClass<T, std::void_t<typename ClassTraits<T>::ThisClass, typename ClassTraits<T>::BaseClass, typename ClassTraits<T>::PrivateData>> : std::true_type {};

	template<class T> struct GetAmountOfPrivateDataFields : std::integral_constant<std::size_t,
		StructMemberCount<typename ClassTraits<T>::PrivateData>::value + GetAmountOfPrivateDataFields<typename ClassTraits<T>::BaseClass>::value
	> {};
	template<> struct GetAmountOfPrivateDataFields<void> : std::integral_constant<std::size_t, 0> {};

	template<std::size_t N, class TT> decltype(auto) GetStructMember(TT&& x)
	{
		using T = typename std::decay<TT>::type;
		using BaseClass = typename ClassTraits<T>::BaseClass;
		using PrivateData = typename ClassTraits<T>::PrivateData;
		if constexpr (N >= GetAmountOfPrivateDataFields<typename ClassTraits<T>::BaseClass>::value)
		{
			return StructApply(static_cast<typename std::conditional<std::is_const<TT>::value, const PrivateData&, PrivateData&>::type>(x), [](auto&&...args) -> decltype(auto) { return std::get<N - GetAmountOfPrivateDataFields<BaseClass>::value>(std::tie(args...)); });
		}
		else
		{
			return GetStructMember<N>(static_cast<typename std::conditional<std::is_const<TT>::value, const BaseClass&, BaseClass&>::type>(x));
		}
	}
}