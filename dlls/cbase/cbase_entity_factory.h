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

#ifndef CLIENT_DLL
typedef struct edict_s edict_t;
typedef struct entvars_s entvars_t;
#endif

template<class T> struct type_identity { using type = T; };

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
}
#endif

#ifdef CLIENT_DLL
#include "cs_wpn/bte_weapons_register.h"
#define DECLEAR_ENTITY_CLASS(DLLClassName) \
	class DLLClassName; \
	template int REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(); \
	extern EntityMetaData GetEntityMetaDataFor(type_identity<DLLClassName>); 
#define LINK_ENTITY_TO_CLASS(mapClassName, DLLClassName) \
	extern template int REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(); \
	EntityMetaData GetEntityMetaDataFor(type_identity<DLLClassName>) { return { REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(), #mapClassName, WeaponEntityPlaceHolderFactory<DLLClassName>() }; }
#else
#define DECLEAR_ENTITY_CLASS(DLLClassName) \
	class DLLClassName; \
	template int REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(); \
	extern EntityMetaData GetEntityMetaDataFor(type_identity<DLLClassName>); 
#define LINK_ENTITY_TO_CLASS(mapClassName, DLLClassName) \
	extern "C" EXPORT void mapClassName(entvars_t *pev) { (void)GetClassPtr<DLLClassName>(pev); } \
	extern template int REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(); \
	EntityMetaData GetEntityMetaDataFor(type_identity<DLLClassName>) { return { REMEMBER_TO_ADD_IN_cbase_typelist_h_<DLLClassName>(), #mapClassName, &mapClassName }; }
#endif
