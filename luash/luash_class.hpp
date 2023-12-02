#pragma once

#include "meta/StructMemberCount.hpp"
#include "meta/StructBreak.hpp"
#include <tuple>

namespace luash
{
	template<class T, class = void> struct IsSupportedClass : std::false_type {};
	template<class T> struct IsSupportedClass<T, std::void_t<typename ClassTraits<T>::ThisClass, typename ClassTraits<T>::BaseClass>> : std::true_type {};
}