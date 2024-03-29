/*
cbase_entity_register.h - CSMoE Gameplay server
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include "meta/TypeIdentity.h"

#ifndef CLIENT_DLL
typedef struct edict_s edict_t;
typedef struct entvars_s entvars_t;
#endif

#ifdef CLIENT_DLL
namespace cl {
class CBasePlayerWeapon;

struct EntityMetaData
{
	int DeclearLine;
	const char* ClassName;
	CBasePlayerWeapon* PlaceHolderEntity;
};

template<class T> int REMEMBER_TO_ADD_IN_cbase_typelist_h_() { return 1; }
bool LuaGetClassPtr(const char* pevClassName, entvars_t* pev);
}
#else
namespace sv {
int MoE_EntityFactory(edict_t* pent, const char* szName);

struct EntityMetaData
{
	int DeclearLine;
	const char* ClassName;
	void(* GetClassPtr)(entvars_t*);
};

template<class T> int REMEMBER_TO_ADD_IN_cbase_typelist_h_() { return 2; }
bool LuaGetClassPtr(const char* pevClassName, entvars_t* pev);
}
#endif

#ifndef XASH_UNITY_BUILD
#define DECLEAR_ENTITY_CLASS_REMINDER(DLLClassName) template int REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>();
#define LINK_ENTITY_TO_REMINDER(DLLClassName) extern template int REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>();
#else
#define DECLEAR_ENTITY_CLASS_REMINDER(DLLClassName)
#define LINK_ENTITY_TO_REMINDER(DLLClassName)
#endif

#ifdef CLIENT_DLL
#include "cs_wpn/bte_weapons_register.h"
#define DECLEAR_ENTITY_CLASS(DLLClassName) \
	class DLLClassName; \
	DECLEAR_ENTITY_CLASS_REMINDER(DLLClassName) \
	extern EntityMetaData GetEntityMetaDataFor(TypeIdentity<DLLClassName>);
#define LINK_ENTITY_TO_CLASS(mapClassName, DLLClassName) \
	LINK_ENTITY_TO_REMINDER(DLLClassName) \
	EntityMetaData GetEntityMetaDataFor(TypeIdentity<DLLClassName>) { return { REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(), #mapClassName, WeaponEntityPlaceHolderFactory<DLLClassName>() }; }
#else
#define DECLEAR_ENTITY_CLASS(DLLClassName) \
	class DLLClassName; \
	DECLEAR_ENTITY_CLASS_REMINDER(DLLClassName); \
	extern EntityMetaData GetEntityMetaDataFor(TypeIdentity<DLLClassName>);
#define LINK_ENTITY_TO_CLASS(mapClassName, DLLClassName) \
	extern "C" EXPORT void mapClassName(entvars_t *pev) { LuaGetClassPtr(#mapClassName, pev) || (GetClassPtr<DLLClassName>(pev), true); } \
	LINK_ENTITY_TO_REMINDER(DLLClassName) \
	EntityMetaData GetEntityMetaDataFor(TypeIdentity<DLLClassName>) { return { REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(), #mapClassName, &mapClassName }; }
#endif
