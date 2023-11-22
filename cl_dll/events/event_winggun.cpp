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
#include "r_studioint.h"
#include "studio.h"

namespace cl {
	extern engine_studio_api_t IEngineStudio;

	extern int g_iWingGunIdleB;
	extern int g_iWingGunShootB;
}

namespace cl::event::winggun {

enum xm1014_e
{
	ANIM_IDLE,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_WINGSTART,
	ANIM_WINGLOOP,
	ANIM_WINGEND,
	ANIM_WINGSHOOT1,
	ANIM_WINGSHOOT2,
	ANIM_WINGRELOAD,//USELESS
	ANIM_WINGSPECIALSHOT,
	ANIM_WINGRELOAD2
};

TEMPENTITY* g_WingGunRing = nullptr;

void CreateAttachmentEntity(model_s* pModel, cl_entity_t* viewent, int iAttachment, float framerate, float framemax, float time, float scale, int flags)
{
	if (pModel)
	{
		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], pModel);

		if (!pEnt)
			return;

		if (!viewent)
			return;

		int clientindex = viewent->index;
		cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

		if (!parent)
			return;

		pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
		pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | flags;
		pEnt->die = gHUD.m_flTime + time;
		pEnt->entity.curstate.frame = 0.0f;
		pEnt->entity.curstate.framerate = framerate;
		pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
		pEnt->entity.curstate.aiment = clientindex;
		pEnt->entity.curstate.solid = SOLID_NOT;
		pEnt->entity.curstate.rendermode = kRenderTransAdd;
		pEnt->entity.curstate.renderfx = kRenderFxNone;
		pEnt->frameMax = framemax;
		pEnt->entity.curstate.scale = scale;
		pEnt->entity.curstate.renderamt = 255;
		pEnt->entity.curstate.body = iAttachment + 1;
		pEnt->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
		pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

		pEnt->callback = [](tempent_s* ent, float frametime, float currenttime)
		{
			cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);
			if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
			{
				ent->entity.curstate.renderamt = 0;
				ent->die = gHUD.m_flTime;
			}
		};
	}
}

void EV_FireWingGun(event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;

	int shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/winggun_bullet.mdl");

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t* viewent = gEngfuncs.GetViewModel();
	int righthand;
	vec3_t vSpread;

	idx = args->entindex;
	righthand = gHUD.cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

	if (args->iparam1 == 2)
	{
		idx -= 1;

		if (args->iparam2 == 1)
		{
			g_iWingGunShootB &= ~(1 << idx);
			g_iWingGunIdleB |= (1 << idx);
		}
		else if (args->iparam2 == 2)
		{
			g_iWingGunIdleB &= ~(1 << idx);
			g_iWingGunShootB |= (1 << idx);
		}
		else
		{
			g_iWingGunIdleB &= ~(1 << idx);
			g_iWingGunShootB &= ~(1 << idx);
		}
	
		return;
	}

	if (EV_IsLocal(idx))
	{
		if (args->bparam2)
		{
			if (args->bparam1)
			{
				if (g_WingGunRing)
				{
					g_WingGunRing->die = gHUD.m_flTime;
					g_WingGunRing = nullptr;
				}

				return;
			}
			model_s* pModel;

			pModel = IEngineStudio.Mod_ForName("sprites/ef_winggun_ring.spr", false);
			if (pModel)
			{
				if (g_WingGunRing)
				{
					g_WingGunRing->die = gHUD.m_flTime;
					g_WingGunRing = nullptr;
				}

				if (!viewent)
					return;

				int clientindex = viewent->index;
				cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

				if (!parent)
					return;

				int iAttachment = 1;
				g_WingGunRing = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], pModel);

				if (g_WingGunRing)
				{
					g_WingGunRing->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
					g_WingGunRing->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
					g_WingGunRing->entity.curstate.frame = 0.0f;
					g_WingGunRing->entity.curstate.framerate = 30.0f;
					g_WingGunRing->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_WingGunRing->entity.curstate.aiment = clientindex;
					g_WingGunRing->entity.curstate.solid = SOLID_NOT;
					g_WingGunRing->entity.curstate.rendermode = kRenderTransAdd;
					g_WingGunRing->entity.curstate.renderfx = kRenderFxNone;
					g_WingGunRing->frameMax = 30.0;
					g_WingGunRing->entity.curstate.scale = 0.05f;
					g_WingGunRing->entity.curstate.renderamt = 255;
					g_WingGunRing->entity.curstate.body = iAttachment + 1;
					g_WingGunRing->die = gHUD.m_flTime + 999;
					g_WingGunRing->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
					g_WingGunRing->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

					g_WingGunRing->callback = [](tempent_s* pEnt, float frametime, float currenttime)
					{
						cl_entity_t* parent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.aiment);
						if (pEnt->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
						{
							pEnt->entity.curstate.renderamt = 0;
							pEnt->die = gHUD.m_flTime;
							g_WingGunRing = nullptr;
						}
					};
				}

			}		

			pModel = IEngineStudio.Mod_ForName("sprites/ef_winggun_charging.spr", false);
			if (pModel)
				CreateAttachmentEntity(pModel, viewent, 1, 30.0, 25.0, 1.0, 0.05, 0);

			pModel = IEngineStudio.Mod_ForName("sprites/ef_winggun_charging2.spr", false);
			if (pModel)
			{
				CreateAttachmentEntity(pModel, viewent, 4, 30.0, 44.0, 2.5, 0.1, FTENT_SPRANIMATELOOP);
				CreateAttachmentEntity(pModel, viewent, 5, 30.0, 44.0, 2.5, 0.1, FTENT_SPRANIMATELOOP);
			}
			return;
		}
	}

	if (args->iparam1 == 1)
	{
		Vector vecBeamStart, vecBeamEnd;

		int iBeamIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_winggun_laserbeam.spr");

		if (EV_IsLocal(idx))
		{
			vecBeamStart = viewent->attachment[1];
		}
		else
		{
			if (args->ducking)
			{
				vecBeamStart = vecSrc + forward * 40 - up * 3;
			}
			else
			{
				vecBeamStart = vecSrc + forward * 40 - up * 9;
			}		
		}

		vecBeamEnd = vecBeamStart + forward * 8192;

		gEngfuncs.pEfxAPI->R_BeamPoints_Stretch(vecBeamStart, vecBeamEnd, iBeamIndex, 1.0, 20.0, 255, 0, 50.0, 255, 255, 255);

		int index = idx - 1;
		g_iWingGunIdleB &= ~(1 << index);
		g_iWingGunShootB |= (1 << index);

		return;
	}

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();

		if(args->bparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_WINGSHOOT1 + Com_RandomLong(0, 1), 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT1 + Com_RandomLong(0, 1), 2);

		if (gHUD.cl_gunsmoke->value)
		{
			if (viewent)
			{
				EV_HLDM_CreateSmoke(viewent->attachment[0], forward, 3, 0.3, 35, 35, 35, 4, velocity, false, 35);
				EV_HLDM_CreateSmoke(viewent->attachment[0], forward, 35, 0.35, 30, 30, 30, 2, velocity, false, 35);
				EV_HLDM_CreateSmoke(viewent->attachment[0], forward, 70, 0.3, 25, 25, 25, 2, velocity, false, 35);
			}
		}
	}

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;

		float vecScale[3] = { 22.0, -9.0, -11.0 };

		if (righthand == 0)
			vecScale[2] = 0 - vecScale[2];

		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

		VectorCopy(viewent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 50;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/winggun-1.wav", 1.0, 0.52, 0, 94 + Com_RandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam2;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		7, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_WINGGUN_BUCKSHOT,
		1 );
}

}
