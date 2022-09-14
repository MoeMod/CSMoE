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

namespace cl::event::m32 {
	
enum
{
	M32VENOM_IDLE,
	M32VENOM_SHOOT1,
	M32VENOM_SHOOT2,
	M32VENOM_INSERT,
	M32VENOM_AFTER_RELOAD,
	M32VENOM_START_RELOAD,
	M32VENOM_DRAW
};

void EV_FireM32( event_args_t *args )
{
	int idx = args->entindex;
	auto origin = args->origin;

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();

		gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(M32VENOM_SHOOT1, M32VENOM_SHOOT2), 2);
	}

	if (args->bparam2)
		PLAY_EVENT_SOUND("weapons/m32venom-1.wav");
	else
		PLAY_EVENT_SOUND("weapons/m32-1.wav");
}

}
