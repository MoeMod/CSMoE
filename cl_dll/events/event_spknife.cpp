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
#include "r_studioint.h"
#include "studio.h"

namespace cl::event::spknife {
	void EV_FireSPKnife(event_args_t* args)
	{
		int idx = args->entindex;
		auto origin = args->origin;

		if (args->bparam1)
		{
			if (EV_IsLocal(idx))
			{
			}

			dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);

			dl->origin = origin;
			dl->radius = 300;
			dl->decay = 0;
			dl->color.r = 200;
			dl->color.g = 200;
			dl->color.b = 50;
			dl->die = gHUD.m_flTime + 1.0;
		}
	}

};
