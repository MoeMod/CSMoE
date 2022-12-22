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
		Vector origin(args->origin);
		Vector angles(
			args->iparam1 / 100.0f + args->angles[0],
			args->iparam2 / 100.0f + args->angles[1],
			args->angles[2]);
		Vector velocity(args->velocity);
		Vector forward, right, up;
		AngleVectors(angles, forward, right, up);
		cl_entity_t* ent = gEngfuncs.GetViewModel();

		if (args->bparam1)
		{
			//secondaryattack
			if (EV_IsLocal(idx))
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/spknife_steam.wav", 1.0, 0.4, 0, 94 + Com_RandomLong(0, 0xf));

				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.15, 55, 50, 50, 2, velocity + up * 50, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.2, 25, 20, 20, 2, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.4, 30, 25, 25, 3, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.15, 30, 25, 25, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.3, 65, 65, 65, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.45, 20, 15, 15, 2, velocity + up * 80, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.1, 50, 50, 50, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.12, 35, 30, 35, 2, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.15, 30, 30, 25, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.18, 35, 40, 40, 2, velocity, false, 30);

				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.15, 55, 50, 50, 2, velocity + up * 50, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.2, 25, 20, 20, 2, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.4, 30, 25, 25, 3, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.15, 30, 25, 25, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.3, 65, 65, 65, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.45, 20, 15, 15, 2, velocity + up * 80, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.1, 50, 50, 50, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.12, 35, 30, 35, 2, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.15, 30, 30, 25, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.18, 35, 40, 40, 2, velocity, false, 30);
			}

			dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);

			dl->origin = origin;
			dl->radius = 300;
			dl->decay = 0;
			dl->color.r = 200;
			dl->color.g = 200;
			dl->color.b = 50;
			dl->die = gHUD.m_flTime + 0.5;
		}
		else
		{
			if (EV_IsLocal(idx))
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/spknife_steam.wav", 1.0, 0.4, 0, 94 + Com_RandomLong(0, 0xf));

				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.15, 55, 50, 50, 2, velocity + up * 50, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.2, 25, 20, 20, 2, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.4, 30, 25, 25, 3, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.15, 30, 25, 25, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.3, 65, 65, 65, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.45, 20, 15, 15, 2, velocity + up * 80, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.1, 50, 50, 50, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.12, 35, 30, 35, 2, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.15, 30, 30, 25, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[0], up, 0, 0.18, 35, 40, 40, 2, velocity, false, 30);

				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.15, 55, 50, 50, 2, velocity + up * 50, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.2, 25, 20, 20, 2, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.4, 30, 25, 25, 3, velocity + up * 50, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.15, 30, 25, 25, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.3, 65, 65, 65, 3, velocity + up * 80, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.45, 20, 15, 15, 2, velocity + up * 80, false, 25);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.1, 50, 50, 50, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.12, 35, 30, 35, 2, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.15, 30, 30, 25, 3, velocity, false, 30);
				EV_HLDM_CreateSmoke(ent->attachment[1], up, 0, 0.18, 35, 40, 40, 2, velocity, false, 30);
			}
		}
	}

};
