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

namespace cl {
	extern int g_iVulcanus9State;
}

namespace cl::event::vulcanus9 {
	void EV_FireVulcanus9LightMove(event_args_t* args)
	{
		int idx = args->entindex;
		auto origin = args->origin;

		if (args->bparam1)
		{
			g_iVulcanus9State |= (1<<idx);

			if (EV_IsLocal(idx))
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/vulcanus9_idle.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 0xf));
			}
		}
		else
			g_iVulcanus9State &= ~(1 << idx);
	}

};
