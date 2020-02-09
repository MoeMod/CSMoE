/*
event_gunkata.cpp
Copyright (C) 2020 Moemod Haoyuan

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
#include "studio.h"
#include "r_studioint.h"
#include <string>

namespace cl {

extern engine_studio_api_t IEngineStudio;

enum
{
	ANIM_IDLE = 0, //6.0
	ANIM_IDLE2, //6.0
	ANIM_SHOOT1, //0.5
	ANIM_SHOOT1_EMPTY, //0.5
	ANIM_SHOOT2, //0.7
	ANIM_SHOOT2_EMPTY, //0.5
	ANIM_RELOAD, //2.0
	ANIM_RELOAD2, //2.0
	ANIM_DRAW1, //1.0
	ANIM_DRAW2, //1.0
	ANIM_SKILL1, //0.7
	ANIM_SKILL2, //0.7
	ANIM_SKILL3, //1.0
	ANIM_SKILL4, //1.0
	ANIM_SKILL5, //1.0
	ANIM_SKILL_LAST //1.0
};

void EV_FireGunkata(struct event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t *ent;
	vec3_t vSpread;


	ent = gEngfuncs.GetViewModel();
	idx = args->entindex;
	empty = args->bparam1 != false;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	gEngfuncs.pfnAngleVectors(angles, forward, right, up);
	if (!args->bparam2)
	{

		if (EV_IsLocal(idx))
		{
			g_iShotsFired++;
			EV_MuzzleFlash();
			//gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1, 2);
		}

		if (EV_IsLocal(idx))
		{
			float vecScale[3] = { 35.0, -11.0, -16.0 };

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

			VectorCopy(ent->attachment[1], ShellOrigin);
			VectorScale(ShellVelocity, 0.75, ShellVelocity);
			ShellVelocity[2] += 25;
		}
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iPShell, TE_BOUNCE_SHELL, idx);

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/gunkata-1.wav", 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

		EV_GetGunPosition(args, vecSrc, origin);

		VectorCopy(forward, vecAiming);

		vSpread[0] = args->fparam1;
		vSpread[1] = args->fparam2;
		vSpread[2] = 0;

        EV_HLDM_FireBullets( idx,
                             forward, right,	up,
                             1, vecSrc, forward,
                             vSpread, 8192.0, BULLET_PLAYER_50AE,
                             2 );
	}
	else
	{
		if (args->iparam1 && args->iparam1<=15 && args->fparam2 == 0.0f)
		{
			if (EV_IsLocal(idx))
			{
				//g_flBloodhunterAnimTime = gHUD.m_flTime;
				//g_iBloodhunterSecAnim = args->iparam1;
				//gEngfuncs.pEventAPI->EV_WeaponAnimation(args->iparam1 + 1, 2);
				float time = gHUD.m_flTime;
				// !R_AddExtraViewModel({ time, time + args->fparam1, args->iparam1, kRenderNormal, 255 });
				//R_AddExtraViewModel({ gHUD.m_flTime + 0.2f, gHUD.m_flTime + args->fparam1 + 0.2f, args->iparam1, kRenderNormal, 255 });

				//gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, WeaponData.szSound, 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			}

			struct model_s *pModel = IEngineStudio.Mod_ForName("models/ef_gunkata.mdl", false);
			if(pModel)
			{
				TEMPENTITY *pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1;
				pEnt->tentOffset = Vector(0, 0, 0);
				pEnt->die = gHUD.m_flTime + 0.9;
				pEnt->clientIndex = idx;
				pEnt->flags |= FTENT_PLYRATTACHMENT;
			}

		}
		else if(args->fparam1 == 1.0f && args->fparam2 > 0.2f && args->fparam2 < 0.23f) // end
		{
			struct model_s *pModel = IEngineStudio.Mod_ForName("models/ef_scorpion_hole.mdl", false);
			if (pModel)
			{
				TEMPENTITY *pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
				pEnt->entity.curstate.sequence = 1;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1.0;
				pEnt->tentOffset = Vector(0, 0, -32);
				pEnt->die = gHUD.m_flTime + 0.9;
				pEnt->clientIndex = idx;
				pEnt->flags |= FTENT_PLYRATTACHMENT;
			}
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_AUTO, "weapons/gunkata_skill_last_exp.wav", 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		}
		else if (args->fparam1 == 10.0f && args->fparam2 == 2.0f) // playermodel
		{
			struct model_s *pModel = IEngineStudio.Mod_ForName("models/ef_gunkata_man.mdl", false);

			if (pModel)
			{
				TEMPENTITY *pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
				pEnt->entity.curstate.sequence = args->iparam1;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1.0;
				pEnt->entity.angles = angles;

				pEnt->die = gHUD.m_flTime + args->fparam2;
				pEnt->entity.curstate.fuser1 = gHUD.m_flTime;
				pEnt->clientIndex = idx;
				pEnt->flags |= FTENT_FADEOUT | FTENT_CLIENTCUSTOM;

				pEnt->entity.curstate.rendermode = kRenderTransTexture;
				pEnt->entity.curstate.renderamt = 100;

				pEnt->callback = [](tempent_s *pEnt, float frametime, float currenttime) {
					float delta = currenttime - pEnt->entity.curstate.fuser1;
					if (delta > 1.0)
						pEnt->entity.curstate.renderamt = 100 - (delta - 1.0) * 100.0f;
				};

				int i = args->iparam1 % 6;
				if(i)
					gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_AUTO, ("weapons/gunkata_skill_0" + std::to_string(i) + ".wav").c_str(), 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			}
		}
	}


}

}
