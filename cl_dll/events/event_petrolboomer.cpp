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

namespace cl::event::petrolboomer {
	
enum petrolboomer_e
{
	ANIM_IDLE1 = 0,
	ANIM_SHOOT1,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_DRAW_EMPTY,
	ANIM_IDLE_EMPTY,
};

void EV_FirePetrolBoomer( event_args_t *args )
{
	int idx = args->entindex;
	auto origin = args->origin;

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT1, 2);
	}
		
	PLAY_EVENT_SOUND("weapons/petrolboomer-1.wav");
}

}
