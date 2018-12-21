#pragma once

#include <type_traits>

// Note: GetClassPtr should know the complete type of CBaseEntity
// and know that there are some custom operator new/delete.
// place #include "cbase_memory.h" below class CBaseEntity { ... }
// or it will crash in GCC.
// Thanks to Microsoft, who does't support Two-phase name lookup,
// for leaving this bug so long...

#ifndef CLIENT_DLL
// pEntityVars should point to entvars_t, but can have different static type to deduce T
template <class T> auto GetClassPtr(T *a) noexcept -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value, T *>::type
{
	// this is the real type...
	entvars_t *pev = reinterpret_cast<entvars_t *>(a);

	// call from mp to create entity ?
	if (pev == nullptr)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = GET_PRIVATE<T>(ENT(pev));

	// call from engine to placement new CBase ?
	if (a == nullptr)
	{
		a = new(pev) T();
		// should auto assign a->pev
		// a->pev = pev;
		assert(a->pev == pev);
	}

	// call from mp to static_cast<CDerived *>(pCBase) ?
	return a;
}

#endif