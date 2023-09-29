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

namespace cl::event::dgaxe {
	void EV_FireDgaxe(event_args_t* args)
	{
		int idx = args->entindex;
		auto origin = args->origin;


		if (EV_IsLocal(idx))
		{
			int iChangeModelIndex;

			if (args->bparam1)
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "weapons/dgaxeex_ready.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				iChangeModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_dgaxeex_change.spr");
			}		
			else
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "weapons/dgaxe_ready.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				iChangeModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_dgaxe_change.spr");
			}

			gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 1, 0.5, iChangeModelIndex, 30.0, 255, kRenderTransAdd, 0.09, 9, FTENT_SPRANIMATE, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

			gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 2, 0.5, iChangeModelIndex, 30.0, 255, kRenderTransAdd, 0.09, 9, FTENT_SPRANIMATE, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);		
		}
	}

};
