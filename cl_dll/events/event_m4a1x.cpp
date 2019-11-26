/*
event_m4a1x.cpp
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

namespace cl {

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

void EV_FireM4A1X( event_args_t *args )
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
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
		vSpread, 8192.0, BULLET_PLAYER_556MM,
		2 );

	if(args->bparam1)
	{
		const Vector vecDirShooting = (forward + right * vSpread.x + up * vSpread.y).Normalize();
		Vector vecEnd = vecSrc + vecDirShooting * 2048;

		int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/lgtning.spr");
		BEAM* pBeam1 = gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, vecEnd, iBeamModelIndex, 0.2f, 2.0f, 0.0f, 60.0f, 2.5f, 0.0f, 0.1f, 42, 212, 255);
		BEAM* pBeam2 = gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, vecEnd, iBeamModelIndex, 0.4f, 2.5f, 0.0f, 40.0f, 2.5f, 0.0f, 0.1f, 42, 212, 255);
		BEAM* pBeam3 = gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, vecEnd, iBeamModelIndex, 0.6f, 3.0f, 0.0f, 20.0f, 2.5f, 0.0f, 0.1f, 42, 212, 255);
		//if (pBeam1) pBeam1->flags |= FBEAM_FADEIN;
		//if (pBeam2) pBeam2->flags |= FBEAM_FADEIN;
		//if (pBeam3) pBeam3->flags |= FBEAM_FADEIN;
	}
}

}
