/*
event_buffm4a1.cpp
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "events.h"

namespace cl::event::buffm4 {

enum
{
	ANIM_IDLE = 0,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3
};

static const char *SOUNDS_NAME[] =
{
	"weapons/m4a1buff-1.wav",
	"weapons/m4a1buff-2.wav"
};

void EV_FireBuffM4A1( event_args_t *args )
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming, vecBeamStart, vecEnd;
	cl_entity_t* ent = gEngfuncs.GetViewModel();

	int    sequence, idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity( args->velocity );
	Vector forward, right, up;

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		sequence = Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT3);
		gEngfuncs.pEventAPI->EV_WeaponAnimation(sequence, 2);
		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, 0);
		}
		ShellOrigin = ent->attachment[2];
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}


	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND( args->bparam1 ? SOUNDS_NAME[1] : SOUNDS_NAME[0]);

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_762MM,
		2 );

	if (EV_IsLocal(idx))
	{
		vecBeamStart = vecSrc + vecSpread.x * right + vecSpread.y * up;
		vecEnd = vecSrc + 8196.0f * forward + right * vecSpread.x + up * vecSpread.y;
	}
	else
	{
		vecBeamStart = vecSrc + forward * 40.0 + vecSpread.x * right + vecSpread.y * up - up * 15.0;
		vecEnd = vecSrc + 8196.0f * forward + right * vecSpread.x + up * vecSpread.y - up * 15.0;
	}

	if(args->bparam1)
	{
		int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");

		BEAM* pBeam = gEngfuncs.pEfxAPI->R_BeamCirclePoints(0, vecBeamStart, vecEnd, iBeamModelIndex, 1.0f, 0.75f, 0.0f, 40.0f / 255.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		//if (pBeam) pBeam->flags |= FBEAM_FADEIN;
	}
}

}
