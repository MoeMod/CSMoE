#pragma once

#include <type_traits>

// Note: GetClassPtr should know the complete type of CBaseEntity
// and know that there are some custom operator new/delete.
// place #include "cbase_memory.h" below class CBaseEntity { ... }
// or it will crash in GCC.
// Thanks to Microsoft, who does't support Two-phase name lookup,
// for leaving this bug so long...

#ifndef CLIENT_DLL
namespace sv {
//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template<class T>
NODISCARD auto GetClassPtr(entvars_t *pev) noexcept -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value, T *>::type
{
	// call from mp to create entity ?
	if (pev == nullptr)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	T *a = GET_PRIVATE<T>(ENT(pev));

	// call from engine to placement new CBase ?
	if (a == nullptr) {
		a = new(pev) T();
		// should auto assign a->pev
		// a->pev = pev;
		assert(a->pev == pev);
	}

	// call from mp to static_cast<CDerived *>(pCBase) ?
	return a;
}

// pEntityVars should point to entvars_t, but can have different static type to deduce T
// deprecated
template<class T>
NODISCARD DEPRECATED auto GetClassPtr(T *a) noexcept -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value, T *>::type
{
	// this is the real type...
	entvars_t *pev = reinterpret_cast<entvars_t *>(a);
	return GetClassPtr<T>(pev);
}

template<class T>
NODISCARD T *CreateClassPtr()
{
	return GetClassPtr<T>(nullptr);
}

// CBaseEntity * <=> CBasePlayer *
template<class Result, class T>
constexpr auto static_ent_cast(T *p)
-> typename std::enable_if<
		(std::is_base_of<typename std::remove_cv<typename std::remove_pointer<Result>::type>::type, typename std::remove_cv<T>::type>::value ||
		std::is_base_of<typename std::remove_cv<T>::type, typename std::remove_cv<typename std::remove_pointer<Result>::type>::type>::value) &&
		!std::is_same<typename std::remove_cv<T>::type, typename std::remove_cv<typename std::remove_pointer<Result>::type>::type>::value,
		Result
>::type
{
	return static_cast<Result>(p);
}

// CBaseEntity * => entvars_t *
template<class Result, class T>
constexpr auto static_ent_cast(T *p)
-> typename std::enable_if<
		std::is_base_of<CBaseEntity, typename std::remove_cv<T>::type>::value &&
		std::is_same<Result, entvars_t *>::value &&
		!std::is_same<typename std::remove_cv<T>::type, typename std::remove_cv<typename std::remove_pointer<Result>::type>::type>::value,
		Result
>::type
{
	return p->pev;
}

// CBaseEntity * => edict_t *
template<class Result, class T>
constexpr auto static_ent_cast(T *p)
-> typename std::enable_if<
		std::is_base_of<CBaseEntity, typename std::remove_cv<T>::type>::value &&
		std::is_same<Result, edict_t *>::value &&
		!std::is_same<typename std::remove_cv<T>::type, typename std::remove_cv<typename std::remove_pointer<Result>::type>::type>::value,
		Result
>::type
{
	return p->edict();
}

// edict_t * / entvars_t * => CBaseEntity *
template<class Result, class T>
auto static_ent_cast(T *p) -> decltype(CBaseEntity::Instance(p), Result())
{
	return static_ent_cast<Result>(CBaseEntity::Instance(p));
}

// nullptr => *
template<class Result>
Result static_ent_cast(std::nullptr_t)
{
	return nullptr;
}


// * => *
template<class Result>
Result static_ent_cast(Result p)
{
	return p;
}

// CBaseEntity * <=> CBasePlayer *
template<class Result, class T>
constexpr auto dynamic_ent_cast(T *p)
-> typename std::enable_if<
		!std::is_base_of<typename std::remove_cv<typename std::remove_pointer<Result>::type>::type, typename std::remove_cv<T>::type>::value &&
		std::is_base_of<typename std::remove_cv<T>::type, typename std::remove_cv<typename std::remove_pointer<Result>::type>::type>::value,
		Result
>::type
{
	return dynamic_cast<Result>(p);
}

// edict_t * / entvars_t * => CBaseEntity *
template<class Result, class T>
auto dynamic_ent_cast(T *p) -> decltype(CBaseEntity::Instance(p), Result())
{
	return dynamic_ent_cast<Result>(CBaseEntity::Instance(p));
}

} // namespace sv
#endif
