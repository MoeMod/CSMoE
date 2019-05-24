//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
// eventscripts.h
#if !defined ( EVENTSCRIPTSH )
#define EVENTSCRIPTSH

#include "pmtrace.h"
#include "pm_shared.h"
#include "event_api.h"
#include "r_efx.h"

// defaults for clientinfo messages
#define IS_FIRSTPERSON_SPEC ( g_iUser1 == OBS_IN_EYE || (g_iUser1 && (gHUD.m_Spectator.m_pip->value == INSET_IN_EYE)) )

// Some of these are HL/TFC specific?
void EV_GetGunPosition( struct event_args_s *args, float *pos, float *origin );
void EV_GetDefaultShellInfo( struct event_args_s *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale, bool bReverseDirection = false );
void CreateCorpse(Vector *p_vOrigin, Vector *p_vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody);


// Very simple and little functions that can be inlined

/*
=================
EV_MuzzleFlash

Flag weapon/view model for muzzle flash
=================
*/
inline void EV_MuzzleFlash( void )
{
	if( gHUD.cl_lw->value )
		return;

	// Add muzzle flash to current weapon model
	cl_entity_t *ent = gEngfuncs.GetViewModel();
	if ( !ent )
		return;

	// Or in the muzzle flash
	ent->curstate.effects |= EF_MUZZLEFLASH;

	gMobileAPI.pfnVibrate(0.1f, false);
}

/*
=================
EV_IsPlayer

Is the entity's index in the player range?
=================
*/
inline bool EV_IsPlayer( int idx )
{
	if ( idx >= 1 && idx <= gEngfuncs.GetMaxClients() )
		return true;

	return false;
}


/*
=================
EV_IsLocal

Is the entity == the local player
=================
*/
inline bool EV_IsLocal( int idx )
{
	// check if we are in some way in first person spec mode
	if ( IS_FIRSTPERSON_SPEC  )
		return (g_iUser2 == idx);
	else
		return gEngfuncs.pEventAPI->EV_IsLocal( idx - 1 ) ? true : false;
}

/*
=================
EV_EjectBrass

Bullet shell casings
=================
*/
inline void EV_EjectBrass( float *origin, float *velocity, float rotation, int model, int soundtype, float life = 2.5f )
{
	Vector angles(0.0f, 0.0f, rotation);
	gEngfuncs.pEfxAPI->R_TempModel( origin, velocity, angles, life, model, soundtype );
}


#endif // EVENTSCRIPTS_H
