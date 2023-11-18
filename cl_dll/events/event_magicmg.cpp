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

namespace cl::event::magicmg {
	
enum
{
	ANIM_IDLE1 = 0,
	ANIM_IDLE2,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3,
};

void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);

	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
	}
}

void EV_FireMagicMG( event_args_t *args )
{
	int    idx = args->entindex;
	Vector origin( args->origin );

	int righthand = gHUD.cl_righthand->value;

	if (!args->iparam2)
	{
		if (EV_IsLocal(idx))
		{
			dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
			if (dl)
			{
				dl->origin = args->origin;
				dl->radius = 50;
				dl->decay = 200;
				dl->color.r = 100;
				dl->color.g = 220;
				dl->color.b = 150;
				dl->die = gEngfuncs.GetClientTime() + 0.1;
			}

			if (args->bparam1)
			{
				int iShoot2ModelIndex = righthand ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgshoot2.spr") : gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgshoot2_left.spr");

				TEMPENTITY* Shoot2Model = gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 0, 1.0, iShoot2ModelIndex, 30, 255, kRenderTransAdd, 0.08, 24, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
				if (Shoot2Model)
					Shoot2Model->callback = ResetMuzz;
			}

			gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? ANIM_SHOOT3 : Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT2), 2);
		}

		PLAY_EVENT_SOUND(args->bparam1 ? "weapons/magicmg-2.wav" : "weapons/magicmg-1.wav");

		return;
	}

	if (EV_IsLocal(idx))
	{
		if (args->iparam2 == 1)
		{
			int iDrawModelIndex = righthand ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgdraw.spr") : gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgdraw_left.spr");

			TEMPENTITY* DrawModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 0, 1.0, iDrawModelIndex, 30, 255, kRenderTransAdd, 0.08, 14, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if (DrawModel)
				DrawModel->callback = ResetMuzz;

		}
		else if (args->iparam2 == 2)
		{
			int iReloadStartModelIndex = righthand ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgreloadstart.spr") : gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgreloadstart_left.spr");

			TEMPENTITY* ReloadStartModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 0, 1.0, iReloadStartModelIndex, 15, 255, kRenderTransAdd, 0.08, 14, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if (ReloadStartModel)
				ReloadStartModel->callback = ResetMuzz;
		}
		else if (args->iparam2 == 3)
		{
			int iIdleModelIndex = righthand ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgidle.spr") : gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgidle_left.spr");

			TEMPENTITY* IdleModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 0, 1.0, iIdleModelIndex, 30, 255, kRenderTransAdd, 0.08, 29, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if (IdleModel)
				IdleModel->callback = ResetMuzz;
		}
		else if (args->iparam2 == 4)
		{
			int iReloadChargeModelIndex = righthand ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgreloadcharge.spr") : gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgreloadcharge_left.spr");

			TEMPENTITY* ReloadChargeModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 1, 1.0, iReloadChargeModelIndex, 24, 255, kRenderTransAdd, 0.1, 19, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if (ReloadChargeModel)
				ReloadChargeModel->callback = ResetMuzz;
		}
		else if (args->iparam2 == 5)
		{
			int iShoot2ModelIndex = righthand ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgshoot2.spr") : gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_magicmgshoot2_left.spr");

			TEMPENTITY* Shoot2Model = gEngfuncs.pEfxAPI->R_AttachTentToModel(idx, 0, 1.0, iShoot2ModelIndex, 30, 255, kRenderTransAdd, 0.08, 24, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if (Shoot2Model)
				Shoot2Model->callback = ResetMuzz;
		}
	}


}

}
