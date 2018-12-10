#pragma once

#include <type_traits>

class CBaseEntity;

#ifndef CLIENT_DLL
// pEntityVars should point to entvars_t, but can have different static type to deduce T
template <class T> auto GetClassPtr(T * const pEntityVars) -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value, T *>::type
{
	// this is the real type...
	entvars_t *pev = reinterpret_cast<entvars_t *>(pEntityVars);

	// call from mp to create entity ?
	if (pev == nullptr)
		return new T;

	// get the private data
	T *pResult = GET_PRIVATE<T>(ENT(pev));

	// call from engine to placement new CBase ?
	if (pResult == nullptr)
	{
		return new(pev) T;
	}

	// call from mp to static_cast<CDerived *>(pCBase) ?
	return pResult;
}

#endif