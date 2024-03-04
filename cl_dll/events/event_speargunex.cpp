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

namespace cl::event::speargunex {
	
enum speargunex_e
{
	SPEARGUNEX_IDLE1,
	SPEARGUNEX_SHOOT1,
	SPEARGUNEX_RELOAD,
	SPEARGUNEX_DRAW,
	SPEARGUNEX_DRAW_EMPTY,
	SPEARGUNEX_IDLE_EMPTY,
	SPEARGUNEX_CHARGE_START,
	SPEARGUNEX_CHARGE_LOOP,
	SPEARGUNEX_CHARGE_FINISH,
	SPEARGUNEX_CHARGE_FINISH_LOOP,
	SPEARGUNEX_CHARGE_SHOOT,
};

void EV_FireSpearGunEX( event_args_t *args )
{
	int idx = args->entindex;
	auto origin = args->origin;
	int iClip = args->iparam2;

	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? SPEARGUNEX_SHOOT1 : SPEARGUNEX_CHARGE_SHOOT, 2);
	}
		
	PLAY_EVENT_SOUND(args->bparam1 ? "weapons/speargunex-1.wav" : "weapons/speargunex-2.wav");

}

}
