/*
event_m4a1razer.cpp - Port of Zombie IV Mod
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
		ANIM_IDLEB = 0,
		ANIM_SHOOTB1,
		ANIM_SHOOTB2,
		ANIM_RELOADB,
		ANIM_DRAWB,
		ANIM_CHANGEA,
		ANIM_IDLE,
		ANIM_SHOOT,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_CHANGEB,
	};

static const char *SOUNDS_NAME[] =
{
	"weapons/m4m203_shoot1.wav",
	"weapons/launch.wav",
	"weapons/balrog9_charge_finish1.wav"
};

void EV_FireM4A1Razer( event_args_t *args )
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
	int    idx = args->entindex;
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
		if (args->bparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam2 ? ANIM_SHOOTB2 : ANIM_SHOOTB1, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT, 2);
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

	if (args->bparam1)
	{
		PLAY_EVENT_SOUND(SOUNDS_NAME[1]);
	}
	else
	{
		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iRShell, TE_BOUNCE_SHELL);

		if (args->bparam2)
			PLAY_EVENT_SOUND(SOUNDS_NAME[2]);
		else
			PLAY_EVENT_SOUND(SOUNDS_NAME[0]);

		EV_GetGunPosition(args, vecSrc, origin);
		VectorCopy(forward, vecAiming);
		Vector vSpread;

		vSpread.x = args->fparam1;
		vSpread.y = args->fparam2;
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_556MM,
			2);
	}
	
}

}
