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

#ifndef PROJECT_CBASE_ENTITY_REGISTER_H
#define PROJECT_CBASE_ENTITY_REGISTER_H

typedef struct edict_s edict_t;
typedef struct entvars_s entvars_t;

namespace sv {

void MoE_EntityRegister(void (*pfn)(entvars_t *), const char *name);
int MoE_EntityFactory(edict_t *pent, const char *szName );

template<class T>
class MoE_Entity_AutoRegister
{
public:
	MoE_Entity_AutoRegister(void (*pfn)(entvars_t *), const char *name)
	{
		MoE_EntityRegister(pfn, name);
	}
};

}


#endif //PROJECT_CBASE_ENTITY_REGISTER_H
