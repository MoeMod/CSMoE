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

namespace cl::event::fglauncher {

	void EV_FireFgLauncher(event_args_t* args)
	{
		int idx = args->entindex;
		auto origin = args->origin;

		if (EV_IsLocal(idx))
			gEngfuncs.pEventAPI->EV_WeaponAnimation(1, 2);

		PLAY_EVENT_SOUND("weapons/fglauncher-1.wav");

	}

}
