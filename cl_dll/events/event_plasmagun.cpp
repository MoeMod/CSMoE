/*
event_PlasmaGun.cpp - Port of Zombie IV Mod
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

namespace cl::event::plasmagun {
	
enum
{
	ANIM_IDLE1 = 0,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3
};

void EV_FirePlasmaGun( event_args_t *args )
{
	int    idx = args->entindex;
	Vector origin( args->origin );

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		
		/*
		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
		if(dl)
		{
			dl->origin = args->origin;
			dl->radius = 50;
			dl->decay = 200;
			dl->color.r = 100;
			dl->color.g = 220;
			dl->color.b = 150;
			dl->die = gEngfuncs.GetClientTime() + 0.1;
		}
		*/

		gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT3), 2);
	}

	PLAY_EVENT_SOUND("weapons/plasmagun-1.wav");
}

}
