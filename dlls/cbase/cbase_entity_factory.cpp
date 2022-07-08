/*
cbase_entity_register.cpp - CSMoE Gameplay server
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "cbase_entity_factory.h"
#include "cbase_typelist.h"

#include <unordered_map>
#include <string>

namespace sv {

using MapType = std::unordered_map<std::string, EntityMetaData>;

namespace detail
{
	template<class...MdTypes>
	MapType WeaponEntityFindList_CreateImpl(const MdTypes&...mds)
	{
		return { { mds.ClassName, mds }... };
	}

	template<class...Types>
	MapType WeaponEntityFindList_Create(TypeList<Types...>)
	{
		return WeaponEntityFindList_CreateImpl(GetEntityMetaDataFor(type_identity<Types>())...);
	}
}
	
static const MapType &MapSingleton()
{
	static const MapType x = detail::WeaponEntityFindList_Create(AllEntityTypeList());
	return x;
}

int MoE_EntityFactory(edict_t *pent, const char *szName )
{
	if(!szName || !szName[0])
		return -1;

	entvars_t* pev = &pent->v;

	if (LuaGetClassPtr(szName, pev))
		return 1;

	auto &sfmap = MapSingleton();
	auto iter = sfmap.find(std::string(szName));
	if(iter != sfmap.end())
	{
		(*iter->second.GetClassPtr)(pev);
		return 0; // OK
	}


	ALERT(at_warning, "MoE_EntityFactory() : Unrecognized entity %s \n", szName);
	return -1; // NOT FOUND
}

}
