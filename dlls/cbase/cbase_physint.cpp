/*
cbase_physint.cpp - CSMoE Gameplay server
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

#include "cbase_physint.h"
#include "cbase_entity_factory.h"

namespace sv {



static constexpr physics_interface_t gPhysicsInterface =
{
	SV_PHYSICS_INTERFACE_VERSION,
	MoE_EntityFactory,
	NULL,
	NULL,		// SV_LoadEntities
	NULL,			// SV_UpdatePlayerBaseVelocity
	NULL,			// SV_AllowSaveGame
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

[[hlsdk::dll_global]] server_physics_api_t g_physfuncs = {};

}

using namespace sv;

extern "C" int EXPORT Server_GetPhysicsInterface( int iVersion, server_physics_api_t *pfuncsFromEngine, physics_interface_t *pFunctionTable )
{
	if ( !pFunctionTable || !pfuncsFromEngine || iVersion != SV_PHYSICS_INTERFACE_VERSION )
	{
		return FALSE;
	}

	size_t iExportSize = sizeof( server_physics_api_t );
	size_t iImportSize = sizeof( physics_interface_t );

	// copy new physics interface
	memcpy( &g_physfuncs, pfuncsFromEngine, sizeof( server_physics_api_t ) );

	// fill engine callbacks
	memcpy( pFunctionTable, &gPhysicsInterface, sizeof( physics_interface_t ) );

	return TRUE;
}
