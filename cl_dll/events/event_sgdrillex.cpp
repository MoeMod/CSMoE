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
}

namespace cl::event::sgdrillex {

	enum sgdrillex_e
	{
		SGDRILLEX_IDLE,
		SGDRILLEX_FIRE,
		SGDRILLEX_SLASH,
		SGDRILLEX_RELOAD,
		SGDRILLEX_DRAW,
		SGDRILLEX_CHARGESLASH
	};

	static const char* SOUNDS_NAME = "weapons/sgdrillex-1.wav";

	TEMPENTITY* g_SgDrillSmoke = NULL;
	TEMPENTITY* g_SgDrillSlash = NULL;

	void EV_FireSgdrillEx(event_args_s* args)
	{
		vec3_t ShellVelocity;
		vec3_t ShellOrigin;
		vec3_t vecSrc, vecAiming;
		int    idx = args->entindex;
		Vector origin(args->origin);
		Vector angles(
			args->iparam1 / 100.0f + args->angles[0],
			args->iparam2 / 100.0f + args->angles[1],
			args->angles[2]
		);

		cl_entity_t* viewent;
		viewent = gEngfuncs.GetViewModel();
		Vector vecBeamStart;
		Vector vecBeamEnd;

		Vector velocity(args->velocity);
		Vector forward, right, up;

		AngleVectors(angles, forward, right, up);

		if (!args->bparam1 && !args->bparam1)
		{
			if (args->iparam1 == 1)
			{
				model_s* pModel;
				char pModelName[32];

				sprintf(pModelName, "sprites/ef_sgdrillex_smoke.spr");

				pModel = IEngineStudio.Mod_ForName(pModelName, false);

				if (pModel)
				{
					if (g_SgDrillSmoke)
					{
						g_SgDrillSmoke->die = gHUD.m_flTime;
						g_SgDrillSmoke = nullptr;
					}

					if (!viewent)
						return;

					int clientindex = viewent->index;
					cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

					if (!parent)
						return;

					g_SgDrillSmoke = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[2], pModel);

					if (!g_SgDrillSmoke)
						return;

					g_SgDrillSmoke->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
					g_SgDrillSmoke->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
					g_SgDrillSmoke->entity.curstate.frame = 0.0f;
					g_SgDrillSmoke->entity.curstate.framerate = 30.0f;
					g_SgDrillSmoke->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_SgDrillSmoke->entity.curstate.aiment = clientindex;
					g_SgDrillSmoke->entity.curstate.solid = SOLID_NOT;
					g_SgDrillSmoke->entity.curstate.rendermode = kRenderTransAdd;
					g_SgDrillSmoke->entity.curstate.renderfx = kRenderFxNone;
					g_SgDrillSmoke->frameMax = 30.0;
					g_SgDrillSmoke->entity.curstate.scale = 0.015f;
					g_SgDrillSmoke->entity.curstate.renderamt = 255;
					g_SgDrillSmoke->entity.curstate.body = 3;
					g_SgDrillSmoke->die = gHUD.m_flTime + 999;
					g_SgDrillSmoke->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
					g_SgDrillSmoke->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

					g_SgDrillSmoke->entity.curstate.sequence = 1;

					g_SgDrillSmoke->callback = [](tempent_s* pEnt, float frametime, float currenttime)
					{
						cl_entity_t* parent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.aiment);

						if (pEnt->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
						{
							pEnt->entity.curstate.renderamt = 0;
							pEnt->die = gHUD.m_flTime;
							g_SgDrillSmoke = nullptr;
						}
						if (!pEnt->entity.curstate.sequence)
						{
							pEnt->entity.curstate.renderamt = 0;
							pEnt->die = gHUD.m_flTime;
							g_SgDrillSmoke = nullptr;
						}
					};
				}
			}
			else
			{
				if (!g_SgDrillSmoke)
					return;
				g_SgDrillSmoke->entity.curstate.sequence = 0;
			}
		}
		else if (args->bparam1 && args->bparam2)
		{
			if (EV_IsLocal(idx))
			{
				++g_iShotsFired;
				EV_MuzzleFlash();
				gEngfuncs.pEventAPI->EV_WeaponAnimation(SGDRILLEX_FIRE, 2);
				if (!gHUD.cl_righthand->value)
				{
					EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, -11.0, 0);
				}
				else
				{
					EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, 11.0, 0);
				}
			}
			else
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
			}

			int shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell_sgdrill.mdl");
			EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

			PLAY_EVENT_SOUND(SOUNDS_NAME);

			EV_GetGunPosition(args, vecSrc, origin);
			VectorCopy(forward, vecAiming);
			Vector vSpread;

			vSpread.x = 0.0725;
			vSpread.y = 0.0725;
			EV_HLDM_FireBullets(idx,
				forward, right, up,
				6, vecSrc, vecAiming,
				vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
				1);

		}
		else
		{
			model_s* pModel;
			char pModelName[32];

			sprintf(pModelName, "sprites/ef_sgdrillex_slash.spr");

			pModel = IEngineStudio.Mod_ForName(pModelName, false);

			if (pModel)
			{
				if (g_SgDrillSlash)
				{
					g_SgDrillSlash->die = gHUD.m_flTime;
					g_SgDrillSlash = nullptr;
				}

				if (!viewent)
					return;

				int clientindex = viewent->index;
				cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

				if (!parent)
					return;

				g_SgDrillSlash = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[3], pModel);

				if (!g_SgDrillSlash)
					return;

				g_SgDrillSlash->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				g_SgDrillSlash->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
				g_SgDrillSlash->entity.curstate.frame = 0.0f;
				g_SgDrillSlash->entity.curstate.framerate = 30.0f;
				g_SgDrillSlash->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_SgDrillSlash->entity.curstate.aiment = clientindex;
				g_SgDrillSlash->entity.curstate.solid = SOLID_NOT;
				g_SgDrillSlash->entity.curstate.rendermode = kRenderTransAdd;
				g_SgDrillSlash->entity.curstate.renderfx = kRenderFxNone;
				g_SgDrillSlash->frameMax = 40.0;
				g_SgDrillSlash->entity.curstate.scale = 0.15f;
				g_SgDrillSlash->entity.curstate.renderamt = 255;
				g_SgDrillSlash->entity.curstate.body = 4;
				g_SgDrillSlash->die = gHUD.m_flTime + 1.1;
				g_SgDrillSlash->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
				g_SgDrillSlash->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;


				g_SgDrillSlash->callback = [](tempent_s* pEnt, float frametime, float currenttime)
				{
					cl_entity_t* parent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.aiment);

					if (pEnt->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
					{
						pEnt->entity.curstate.renderamt = 0;
						pEnt->die = gHUD.m_flTime;
						g_SgDrillSlash = nullptr;
					}
				};
			}
		}
	}
}
