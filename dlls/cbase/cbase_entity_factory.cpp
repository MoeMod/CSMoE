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

#include "common.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "cbase_entity_factory.h"

#include <unordered_map>
#include <string>

namespace sv {

using MapType = std::unordered_map<std::string, void (*)(entvars_s *)>;

static MapType &MapSingleton()
{
	static MapType x;
	return x;
}

void MoE_EntityRegister(void (*pfn)(entvars_s *), const char *name)
{
	MapSingleton().emplace(name, pfn);
}

int MoE_EntityFactory(edict_t *pent, const char *szName )
{
	if(!szName || !szName[0])
		return -1;

	auto &sfmap = MapSingleton();
	auto iter = sfmap.find(std::string(szName));
	if(iter == sfmap.end())
	{
		ALERT(at_warning, "MoE_EntityFactory() : Unrecognized entity %s \n", szName);
		return -1; // NOT FOUND
	}

	entvars_t *pev = &pent->v;

	/*
	try
	{
		iter->second(pev);
	}
	catch(const std::exception &e)
	{
		ALERT(at_warning, "MoE_EntityFactory() : Fail to create %s with exception saying: %s\n", szName, e.what());
		return -1; // WTF ? eat that exception and return...
	}
	 */
    iter->second(pev);

	return 0; // OK
}

}