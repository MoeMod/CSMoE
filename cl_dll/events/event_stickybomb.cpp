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

namespace cl {
extern int g_iStickyBombSkinG;
extern int g_iStickyBombSkinW;
}

namespace cl::event::stickybomb {

	void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
	{
		cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);

		if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
		{
			ent->entity.curstate.renderamt = 0;
			ent->die = currenttime;
		}
	}

	void EV_FireStickyBomb(event_args_t* args)
	{
		int idx = args->entindex;
		int iWeaponState = args->iparam1;

		cl_entity_t* ent;
		Vector vecBeamStart;
		Vector vecBeamEnd;

		int righthand = gHUD.cl_righthand->value;
		ent = gEngfuncs.GetViewModel();

		if (EV_IsLocal(idx))
		{
			if (args->bparam1)
			{
				int iTransformModelIndex;
				int iFrameMax;
				switch (iWeaponState)
				{
				case 1: 
					iTransformModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_transform_g.spr");
					g_iStickyBombSkinW &= ~(1 << idx);
					g_iStickyBombSkinG |= (1 << idx);
					iFrameMax = 11;
					break;
				case 2:
					iTransformModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_transform_w.spr");
					g_iStickyBombSkinG &= ~(1 << idx);
					g_iStickyBombSkinW |= (1 << idx);
					iFrameMax = 12;
					break;
				default:
					iTransformModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_transform_r.spr");
					g_iStickyBombSkinW &= ~(1 << idx);
					g_iStickyBombSkinG &= ~(1 << idx);
					iFrameMax = 13;
					break;
				}

				TEMPENTITY* pTransformModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 0, 1.0, iTransformModelIndex, 30, 255, kRenderTransAdd, 0.03, iFrameMax, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
				if (pTransformModel)
					pTransformModel->callback = ResetMuzz;
			}

			if (args->bparam2)
			{
				int iHandModelIndex;
				int iFrameMax;
				switch (iWeaponState)
				{
				case 1:
					if (!righthand)
					{
						iHandModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_hand_g2.spr");

					}
					else
					{
						iHandModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_hand_g.spr");
					}
					iFrameMax = 12;
					break;
				case 2:
					if (!righthand)
					{
						iHandModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_hand_w2.spr");

					}
					else
					{
						iHandModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_hand_w.spr");
					}
					iFrameMax = 8;
					break;
				default:
					if (!righthand)
					{
						iHandModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_hand_r2.spr");

					}
					else
					{
						iHandModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_stickybomb_hand_r.spr");
					}
					iFrameMax = 10;
					break;
				}

				TEMPENTITY* pHandModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 1, 1.0, iHandModelIndex, 30, 255, kRenderTransAdd, 0.03, iFrameMax, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
				if (pHandModel)
					pHandModel->callback = ResetMuzz;
			}
			
		}
	}

};
