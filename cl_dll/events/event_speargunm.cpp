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

namespace cl::event::speargunm {
	
enum speargunm_e
{
	SPEARGUNM_IDLE1,
	SPEARGUNM_IDLE2,
	SPEARGUNM_IDLE3,
	SPEARGUNM_SHOOT1,
	SPEARGUNM_SHOOT2,
	SPEARGUNM_SHOOT3,
	SPEARGUNM_RELOAD,
	SPEARGUNM_DRAW1,
	SPEARGUNM_DRAW2,
	SPEARGUNM_DRAW3,
	SPEARGUNM_DRAW_EMPTY,
	SPEARGUNM_IDLE_EMPTY,
	SPEARGUNM_SHOOT_EMPTY
};

void EV_FireSpearGunm( event_args_t *args )
{
	int idx = args->entindex;
	auto origin = args->origin;
	int iClip = args->iparam2;

	if (EV_IsLocal(idx))
	{
		int iAnim;

		if (iClip >= 2)
			iAnim = SPEARGUNM_SHOOT1;
		else if (iClip == 1)
			iAnim = SPEARGUNM_SHOOT2;
		else
			iAnim = SPEARGUNM_SHOOT3;

		gEngfuncs.pEventAPI->EV_WeaponAnimation(iAnim, 2);
	}
		

	PLAY_EVENT_SOUND("weapons/speargunm-1.wav");

}

}
