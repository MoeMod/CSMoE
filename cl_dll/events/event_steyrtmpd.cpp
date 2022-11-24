/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/
#include "events.h"
#include "studio.h"
#include "r_studioint.h"

namespace cl {
	extern engine_studio_api_t IEngineStudio;
}

enum steyrtmpd_e
{
	STEYRTMPD_IDLE,
	STEYRTMPD_RELOAD,
	STEYRTMPD_DRAW,
	STEYRTMPD_SHOOT_PRE,
	STEYRTMPD_SHOOT,
	STEYRTMPD_SHOOT_POST,
	STEYRTMPD_RUN,
	STEYRTMPD_ON_SHOULDER,
	STEYRTMPD_ON_SHOULDER_IDLE,
	STEYRTMPD_ON_SHOULDER_RELOAD,
	STEYRTMPD_ON_SHOULDER_DRAW,
	STEYRTMPD_ON_SHOULDER_SHOOT_PRE,
	STEYRTMPD_ON_SHOULDER_SHOOT,
	STEYRTMPD_ON_SHOULDER_SHOOT_POST,
	STEYRTMPD_ON_SHOULDER_RUN,
	STEYRTMPD_OFF_SHOULDER,
};

namespace cl::event::steyrtmpd {


void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);

	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
	}
}

void EV_FireSteyrTMPD(event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocityL;
	vec3_t ShellVelocityR;
	vec3_t ShellOriginL;
	vec3_t ShellOriginR;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t* ent;
	vec3_t vSpread;

	ent = gEngfuncs.GetViewModel();
	idx = args->entindex;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (args->fparam1 == 2.0)
	{
		model_s* pModel;

		char pModelName[32];

		if (args->iparam1)
		{
			sprintf(pModelName, "sprites/ef_m95tiger_eye1.spr");
		}
		else
		{
			sprintf(pModelName, "sprites/ef_m95tiger_eye2.spr");
		}
			
		for (int i = 4; i <= 6; i=i+2)
		{
			pModel = IEngineStudio.Mod_ForName(pModelName, false);
			if (pModel)
			{
				cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->index);

				if (!parent)
					return;

				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAlloc(ent->attachment[i], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_PERSIST | FTENT_CLIENTCUSTOM | FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1.0;

				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOWMOVE;
				pEnt->entity.curstate.aiment = ent->index;
				pEnt->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
				pEnt->entity.curstate.body = i + 1;
				pEnt->frameMax = 30.0;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.03f, 0.04f);
				pEnt->entity.baseline.renderamt = pEnt->entity.curstate.renderamt = gEngfuncs.pfnRandomFloat(150.0f, 200.0f);
				pEnt->fadeSpeed = 2.0f;
				pEnt->entity.curstate.iuser1 = i;
				pEnt->die = gHUD.m_flTime + 0.3f;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;
				pEnt->callback = ResetMuzz;
			}
		}
		return;
	}

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		
		int iMuzzModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/muzzleflash25.spr");

		gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 0, 0.1, iMuzzModelIndex, 24, 210, kRenderTransAdd, 0.2, 3, FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

		gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 1, 0.1, iMuzzModelIndex, 24, 210, kRenderTransAdd, 0.2, 3, FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);


		if (args->bparam2)
		{
			if (args->bparam1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(STEYRTMPD_ON_SHOULDER_SHOOT_POST, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(STEYRTMPD_ON_SHOULDER_SHOOT, 2);
		}
		else
		{
			if (args->bparam2 > 1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(STEYRTMPD_SHOOT_POST, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(STEYRTMPD_SHOOT, 2);
		}

		if (gHUD.cl_gunsmoke->value)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[1], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
		}
			
	}

	if (EV_IsLocal(idx))
	{
		if (!gHUD.cl_righthand->value)
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocityL, ShellOriginL, forward, right, up, 34.0, -10.0, -11.0, 0);

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocityR, ShellOriginR, forward, right, up, 34.0, -10.0, 11.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocityR, ShellOriginR, forward, right, up, 34.0, -10.0, 11.0, 0);

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocityL, ShellOriginL, forward, right, up, 34.0, -10.0, -11.0, 0);
		}

		VectorCopy(ent->attachment[2], ShellOriginL);
		ShellVelocityL[2] -= 45;

		VectorCopy(ent->attachment[3], ShellOriginR);
		ShellVelocityR[2] -= 45;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocityL, ShellOriginL, forward, right, up, 20.0, -12.0, 4.0, false);

	EV_EjectBrass(ShellOriginL, ShellVelocityL, angles[YAW], shell, TE_BOUNCE_SHELL);

	EV_EjectBrass(ShellOriginR, ShellVelocityR, angles[YAW], shell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/dualuzi-1.wav", 1.0, 0.52, 0, 94 + Com_RandomLong(0, 15));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], 1);
}

}
