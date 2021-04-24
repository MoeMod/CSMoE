/*
event_gungnir.cpp
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

#include "events.h"

namespace cl::event::gungnir {

enum
{
	ANIM_IDLE1,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT_START,
	ANIM_SHOOT_LOOP,
	ANIM_SHOOT_END,
	ANIM_SHOOT_B,
	ANIM_SHOOT_B_CHARGE,
	ANIM_CHARGE_SHOOT,
	ANIM_CHARGE_LOOP
};

void EV_FireGungnir( event_args_t *args )
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2] );
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;

		switch (args->iparam1)
		{
		case 0:
		{

		}
		case 1:
		{
			PLAY_EVENT_SOUND("weapons/gungnir_shoot_loop.wav");
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_LOOP, 2);
			break;
		}
		case 2:
		{
			PLAY_EVENT_SOUND("weapons/gungnir_shoot_end.wav");
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_END, 2);
			break;
		}
		case 3:
		{
			PLAY_EVENT_SOUND("weapons/gungnir_shoot_b.wav");
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_B, 2);
			break;
		}
		case 4:
		{
			PLAY_EVENT_SOUND("weapons/gungnir_charge_shoot1.wav");
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_CHARGE_SHOOT, 2);
			break;
		}
		}
			

		EV_MuzzleFlash();
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


	//EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	

}

}
