#pragma once

#include <type_traits>

#ifdef CLIENT_DLL
#define ns cl
#else
#define ns sv
#endif

#define CLASS_BEGIN(ClassName, BaseClassName) \
namespace ns { \
class ClassName; \
struct ClassTraits_##ClassName { \
	using ThisClass = ClassName; \
	using BaseClass = BaseClassName; \
}; \
} \
template<> struct luash::ClassTraits<ns::ClassName, void> : ns::ClassTraits_##ClassName {}; \

template<class T> struct luash::ClassTraits<T, typename std::enable_if<std::is_base_of<
		ns::CBaseEntity,
		T>::value>::type>
{
	using ThisClass = T;
	using BaseClass = ns::CBaseEntity;
};

#include "cbase_iter.h"