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

namespace cl::event::speargun {
	
enum speargun_e
{
	SPEARGUN_IDLE1,
	SPEARGUN_SHOOT1,
	SPEARGUN_RELOAD,
	SPEARGUN_DRAW,
	SPEARGUN_DRAW_EMPTY,
	SPEARGUN_IDLE_EMPTY
};

void EV_FireSpearGun( event_args_t *args )
{
	int idx = args->entindex;
	auto origin = args->origin;

	if ( EV_IsLocal( idx ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SPEARGUN_SHOOT1, 2);

	PLAY_EVENT_SOUND("weapons/speargun-1.wav");

}

}
