#pragma once

#include <type_traits>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	class CBaseEntity;
	template<class ThisClass, class BaseClass> struct PrivateData;
}

namespace luash {
	namespace detail {
	#ifdef CLIENT_DLL
			using namespace cl;
	#else
			using namespace sv;
	#endif

			template<class ThisClass, class BaseClass> auto FindBaseClass(ThisClass*, PrivateData<ThisClass, BaseClass>*)->BaseClass;
			template<class ThisClass, class BaseClass> auto FindPrivateData(ThisClass*, PrivateData<ThisClass, BaseClass>*)->PrivateData<ThisClass, BaseClass>;
	}
}

template<class T> struct luash::ClassTraits<T, typename std::enable_if<std::is_base_of<
#ifdef CLIENT_DLL
	cl::CBaseEntity,
#else
	sv::CBaseEntity,
#endif
	T>::value>::type>
{
	using ThisClass = T;
	using BaseClass = decltype(detail::FindBaseClass((T*)nullptr, (T*)nullptr));
	using PrivateData = decltype(detail::FindPrivateData((T*)nullptr, (T*)nullptr));
};