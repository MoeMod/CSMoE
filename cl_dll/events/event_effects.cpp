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
#include <string>


namespace cl {
    extern engine_studio_api_t IEngineStudio;
}

namespace cl::event::wpneffects {

enum event_effect
{
	EVENT_GATLINGRX,
	EVENT_DUALSWORD,
	EVENT_VOIDPISTOL,
	EVENT_BUFFSG552EX,
	EVENT_DGAXE,
	EVENT_DUALSWORD_FX,
	EVENT_HUNTBOW,
	EVENT_BALROG11_FOLLOW_ENTITY,
	EVENT_LANCE,
	EVENT_BUFFM249,
	EVENT_DUALSWORD_SKILL,
	EVENT_DUALSWORD_EYE,
	EVENT_DRAGONSWORDFX,
	EVENT_RUNEBLADEFX,
	EVENT_RUNEBLADE_EYE,
	EVENT_SUMMONER_STAB,
	EVENT_SUMMONER_SLASH,
	EVENT_SUMMONER,
	EVENT_REVIVEGUN,
	EVENT_VOIDPISTOL_BLACKHOLE,
	EVENT_FOLLOW_ENT,
	EVENT_BALROGCRITICAL,
	EVENT_M32VENOM,
	EVENT_M95TIGERNET,
	EVENT_M95TIGER,
};

enum buffsg552ex_slash_e
{
	EF_SLASH1,
	EF_SLASH2,
	EF_SLASH3,
	EF_SLASH_BIG,
};

void CreateBalrog11CannonSingleProjectile(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType)
{
	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(iAimingEntity);
	if (!ent)
		return;
	static int iModelIndex;
	if(iType)
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_hwater.spr");
	else
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr");

	TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_DefaultSprite(origin, iModelIndex, 48);
	if (pEnt)
	{
		pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEKILL;
		pEnt->entity.angles.y = gEngfuncs.pfnRandomFloat(-512, 511);
		pEnt->entity.angles.x = gEngfuncs.pfnRandomFloat(-256, 255);
		pEnt->entity.angles.z = gEngfuncs.pfnRandomFloat(-256, 255);
		pEnt->flags |= FTENT_ROTATE;
		pEnt->entity.curstate.scale = 0.01;
		pEnt->entity.baseline.fuser1 = 0.3;
		pEnt->entity.curstate.iuser2 = iType;
	
		pEnt->entity.curstate.rendermode = kRenderTransAdd;
		pEnt->entity.curstate.renderamt = 40.0f;
		pEnt->entity.curstate.renderfx = kRenderFxNone;

		pEnt->entity.baseline.origin = Vector(0, 0, 0);
		pEnt->entity.baseline.iuser1 = ent->index;
		pEnt->entity.curstate.iuser1 = 0;
		pEnt->clientIndex = ent->curstate.owner;

		pEnt->callback = [](tempent_s* ent, float frametime, float currenttime) {
			if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
				ent->entity.curstate.scale += 0.03;

			cl_entity_t* pEntity;
			pEntity = gEngfuncs.GetEntityByIndex(ent->entity.baseline.iuser1);

			if (!pEntity)
			{
				ent->die = currenttime;
				return;
			}

			if (ent->die - currenttime <= frametime * 100 && !ent->entity.curstate.iuser1)
			{
				if ((ent->entity.origin - pEntity->origin).Length() < 5)
					return;
				CreateBalrog11CannonSingleProjectile(ent, pEntity->origin, pEntity->index, ent->entity.curstate.iuser2);
				ent->entity.curstate.iuser1 = 1;
			}
		};
		pEnt->hitcallback = [](tempent_s* ent, pmtrace_s* ptr) {
			ent->entity.baseline.origin *= 0.01f;
			ent->entity.curstate.framerate = 60;

		};
	}
}

void EV_WpnEffects(event_args_s* args)
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
	Vector velocity(args->velocity);
	Vector forward, right, up;

	AngleVectors(angles, forward, right, up);

	switch (args->iparam1)
	{
	case EVENT_GATLINGRX:
	{
		vec3_t vStartOrigin;
		VectorCopy(args->origin, vStartOrigin);

		for (int i = 0; i < 2; i++)
		{
			float flspeed = gEngfuncs.pfnRandomFloat(5.0, 80.0);
			vec3_t vEndOrigin, vBalOrigin, vVelocity;
			VectorCopy(vStartOrigin, vEndOrigin);
			vEndOrigin[0] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);//Caculate velocity
			vEndOrigin[1] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);
			vEndOrigin[2] += gEngfuncs.pfnRandomFloat(-50.0, 500.0);
			VectorSubtract(vEndOrigin, vStartOrigin, vBalOrigin);
			VectorScale(vBalOrigin, sqrt(flspeed * flspeed / (vBalOrigin[0] * vBalOrigin[0] + vBalOrigin[1] * vBalOrigin[1] + vBalOrigin[2] * vBalOrigin[2])), vVelocity);

			model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_gatlingex_star.spr", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

				if (!pEnt)
					return;

				pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.baseline.origin = vVelocity;
				pEnt->entity.curstate.framerate = 2.0;
				pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.11, 0.13);
				pEnt->die = gHUD.m_flTime + gEngfuncs.pfnRandomFloat(1.2, 2.2);

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					float gravity = -frametime * 140.0;
					pEnt->entity.baseline.origin[2] += gravity;
				};

				pEnt->hitcallback = [](tempent_s* pEnt, pmtrace_s* ptr) {
					pEnt->entity.baseline.origin = Vector(0, 0, 0);
				};
			}
		}

		break;
	}
	case EVENT_DUALSWORD:
	{
		vec3_t vStartOrigin;
		VectorCopy(args->angles, angles);
		VectorCopy(args->origin, vStartOrigin);

		const char* szSprite;
		if (args->bparam1)
		{
			if (args->bparam2)
				szSprite = "sprites/leaf01_dualsword.spr";
			else
				szSprite = "sprites/leaf02_dualsword.spr";
		}
		else
		{
			if (args->bparam2)
				szSprite = "sprites/petal01_dualsword.spr";
			else
				szSprite = "sprites/petal02_dualsword.spr";
		}



		for (int i = 0; i < 10; i++)
		{
			angles[0] = gEngfuncs.pfnRandomFloat(-90.0, 90.0);
			angles[1] = gEngfuncs.pfnRandomFloat(-90.0, 90.0);
			angles[2] = gEngfuncs.pfnRandomFloat(-90.0, 90.0);
			float flspeed = gEngfuncs.pfnRandomFloat(5.0, 80.0);
			vec3_t vEndOrigin, vBalOrigin, vVelocity;
			VectorCopy(vStartOrigin, vEndOrigin);
			vEndOrigin[0] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);//Caculate velocity
			vEndOrigin[1] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);
			vEndOrigin[2] += gEngfuncs.pfnRandomFloat(-50.0, 500.0);
			VectorSubtract(vEndOrigin, vStartOrigin, vBalOrigin);
			VectorScale(vBalOrigin, sqrt(flspeed * flspeed / (vBalOrigin[0] * vBalOrigin[0] + vBalOrigin[1] * vBalOrigin[1] + vBalOrigin[2] * vBalOrigin[2])), vVelocity);

			model_s* pModel = IEngineStudio.Mod_ForName(szSprite, false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(vStartOrigin, pModel);

				if (!pEnt)
					return;

				pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.rendermode = kRenderNormal;
				pEnt->entity.baseline.origin = vVelocity;
				pEnt->entity.angles = angles;
				pEnt->entity.curstate.framerate = 2.0;
				pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.11, 0.13);
				pEnt->die = gHUD.m_flTime + gEngfuncs.pfnRandomFloat(1.2, 2.2);
				/*gEngfuncs.Con_Printf("vVelocity is x= %f ,y= %f ,z=%f\n", vVelocity[0], vVelocity[1], vVelocity[2]);
				gEngfuncs.Con_Printf("angles is x= %f ,y= %f ,z=%f\n", angles[0], angles[1], angles[2]);*/

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					float gravity = -frametime * 96.0;
					pEnt->entity.baseline.origin[2] += gravity;
					if (pEnt->entity.baseline.origin[1])
						pEnt->entity.angles -= Vector(6.0, 6.0, 6.0);
				};
				pEnt->hitcallback = [](tempent_s* pEnt, pmtrace_s* ptr) {
					pEnt->entity.baseline.origin = Vector(0, 0, 0);

				};
			}
		}


		break;
	}
	case EVENT_VOIDPISTOL:
	{
		vec3_t vStartOrigin;
		VectorCopy(args->origin, vStartOrigin);

		for (int i = 0; i < 3; i++)
		{
			float flspeed = gEngfuncs.pfnRandomFloat(5.0, 80.0);
			vec3_t vEndOrigin, vBalOrigin, vVelocity;
			VectorCopy(vStartOrigin, vEndOrigin);
			vEndOrigin[0] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);//Caculate velocity
			vEndOrigin[1] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);
			vEndOrigin[2] += gEngfuncs.pfnRandomFloat(-50.0, 500.0);
			VectorSubtract(vEndOrigin, vStartOrigin, vBalOrigin);
			VectorScale(vBalOrigin, sqrt(flspeed * flspeed / (vBalOrigin[0] * vBalOrigin[0] + vBalOrigin[1] * vBalOrigin[1] + vBalOrigin[2] * vBalOrigin[2])), vVelocity);

			model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_blackhole_star.spr", false);
			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

				if (!pEnt)
					return;

				pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.baseline.origin = vVelocity;
				pEnt->entity.curstate.framerate = 2.0;
				pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.11, 0.13);
				pEnt->die = gHUD.m_flTime + gEngfuncs.pfnRandomFloat(1.2, 2.2);

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					float gravity = -frametime * 300.0;
					pEnt->entity.baseline.origin[2] += gravity;
				};

				pEnt->hitcallback = [](tempent_s* pEnt, pmtrace_s* ptr) {
					pEnt->entity.baseline.origin = Vector(0, 0, 0);
				};
			}
		}

		break;
	}
	case EVENT_BUFFSG552EX:
	{
		static const char* SLASH_SOUND[] =
		{
			"weapons/buffsg552ex_claw_1.wav", "weapons/buffsg552ex_claw_2.wav", "weapons/buffsg552ex_claw_3.wav", "weapons/buffsg552ex_claw_4.wav"
		};

		model_s* pModel;
		switch (args->iparam2)
		{
		case EF_SLASH1:
		{
			pModel = IEngineStudio.Mod_ForName("sprites/ef_buffsg552ex_slash1.spr", false); break;
		}
		case EF_SLASH2:
		{
			pModel = IEngineStudio.Mod_ForName("sprites/ef_buffsg552ex_slash2.spr", false); break;
		}
		case EF_SLASH3:
		{
			pModel = IEngineStudio.Mod_ForName("sprites/ef_buffsg552ex_slash3.spr", false); break;
		}
		case EF_SLASH_BIG:
		{
			pModel = IEngineStudio.Mod_ForName("sprites/ef_buffsg552ex_slash_big.spr", false); break;
		}
		default: return;
		}
		if (pModel)
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, SLASH_SOUND[args->iparam2], VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

			if (!pEnt)
				return;

			pEnt->flags |= FTENT_SPRANIMATE;
			pEnt->entity.curstate.frame = 0;
			pEnt->entity.curstate.framerate = 30.0;

			pEnt->entity.curstate.rendermode = kRenderTransAlpha;
			pEnt->entity.curstate.scale = 0.5;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->die = gHUD.m_flTime + 0.7;

		}
		break;
	}
	case EVENT_DGAXE:
	{
		if (!args->iparam2)
		{
			model_s* pModel = IEngineStudio.Mod_ForName("models/ef_scorpion_hole.mdl", false);
			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

				if (!pEnt)
					return;

				pEnt->entity.curstate.sequence = 1;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.framerate = 1.0;
				pEnt->tentOffset = Vector(0, 0, -32);
				pEnt->die = gHUD.m_flTime + 0.2;
				pEnt->clientIndex = idx;
				pEnt->flags |= FTENT_PLYRATTACHMENT;
			}

		}
		else if(args->iparam2 == 1)
		{
			model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_dgaxe_smoke.spr", false);
			if (pModel)
			{
				//origin[2] += 32;
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

				if (!pEnt)
					return;

				pEnt->flags |= FTENT_SPRANIMATE;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 30.0;
				pEnt->tentOffset = Vector(0, 0, -32);

				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.scale = 1.0;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 0.67;

			}
		}
		else
		{
			model_s* pModel;
			pModel = IEngineStudio.Mod_ForName("sprites/ef_dgaxe_change.spr", false);
			if (pModel)
			{
				cl_entity_t* viewent = gEngfuncs.GetViewModel();
				TEMPENTITY* pEntAttach1 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[1], pModel);

				if (!pEntAttach1)
					return;

				pEntAttach1->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEntAttach1->flags |= FTENT_CLIENTCUSTOM | FTENT_PERSIST | FTENT_SPRANIMATE;
				pEntAttach1->entity.curstate.frame = 0;
				pEntAttach1->entity.curstate.framerate = 30.0;
				pEntAttach1->entity.curstate.rendermode = kRenderTransAdd;
				pEntAttach1->entity.curstate.renderfx = kRenderFxNone;
				pEntAttach1->entity.curstate.scale = 0.09;
				pEntAttach1->entity.curstate.renderamt = 255;
				pEntAttach1->die = gHUD.m_flTime + 0.5;
				pEntAttach1->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				pEntAttach1->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();
					pEnt->entity.origin = viewent->attachment[1];
				};

				TEMPENTITY* pEntAttach2 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[2], pModel);

				if (!pEntAttach2)
					return;

				pEntAttach2->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEntAttach2->flags |= FTENT_CLIENTCUSTOM | FTENT_PERSIST | FTENT_SPRANIMATE;
				pEntAttach2->entity.curstate.frame = 0;
				pEntAttach2->entity.curstate.framerate = 30.0;
				pEntAttach2->entity.curstate.rendermode = kRenderTransAdd;
				pEntAttach2->entity.curstate.renderfx = kRenderFxNone;
				pEntAttach2->entity.curstate.scale = 0.11;
				pEntAttach2->entity.curstate.renderamt = 255;
				pEntAttach2->die = gHUD.m_flTime + 0.5;
				pEntAttach2->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;
			
				pEntAttach2->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();
					pEnt->entity.origin = viewent->attachment[2];
				};
			}

			break;
		}
		break;
	}
	case EVENT_DUALSWORD_FX:
	{
		if (!EV_IsLocal(idx))
			break;

		auto viewent = gEngfuncs.GetViewModel();

		if (!viewent)
			return;

		model_s* pModel = IEngineStudio.Mod_ForName("models/dualswordfx.mdl", false);

		if (pModel)
		{
			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

			if (!pEnt)
				return;

			pEnt->entity.curstate.body = viewent->curstate.body;
			pEnt->entity.curstate.frame = 0.0f;
			pEnt->entity.curstate.framerate = 1.0f;
			pEnt->entity.curstate.animtime = gHUD.m_flTime;
			pEnt->entity.curstate.sequence = args->iparam2;
			pEnt->tentOffset = Vector(0, 0, 0);
			pEnt->bounceFactor = 3;	// ??
			pEnt->fadeSpeed = 512;
			pEnt->frameMax = 31;

			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->entity.baseline.angles = Vector(0, 0, 0);
			pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

			if (args->iparam2 != 3)
			{
				pEnt->die = gHUD.m_flTime + 1.0;
				pEnt->entity.curstate.fuser1 = gHUD.m_flTime;
				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();
					pEnt->entity.origin = viewent->origin;
					pEnt->entity.angles = viewent->angles;
					if (currenttime > pEnt->entity.curstate.fuser1)
						pEnt->entity.curstate.renderamt = 255 - (currenttime - pEnt->entity.curstate.fuser1) * 255.0f;
				};
			}
			else
			{
				pEnt->die = gHUD.m_flTime + 1.5;
				pEnt->entity.curstate.fuser1 = gHUD.m_flTime;
				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();
					pEnt->entity.origin = viewent->origin;
					pEnt->entity.angles = viewent->angles;
					if (currenttime > pEnt->entity.curstate.fuser1 + 1.0)
						pEnt->entity.curstate.renderamt = 255 - (currenttime - pEnt->entity.curstate.fuser1 - 1.0) * 2.0f * 255.0f;
				};
			}


			pEnt->clientIndex = idx;
			pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
			pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_FADEOUT | FTENT_PERSIST;
		}

		break;
	}
	case EVENT_HUNTBOW:
	{
		if (args->bparam1)
		{
			idx = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(idx);
			if (pPlayer)
			{
				for (int i = 1; i < 33; i++)
				{
					if (i == idx)
						continue;

					
					cl_entity_t* pEnt = gEngfuncs.GetEntityByIndex(i);
					if (!pEnt)
						continue;

					
					float fDelta = (pPlayer->origin - pEnt->origin).Length();

					if (fDelta < 1500)
					{
						if(g_PlayerExtraInfo[i].zombie)
							pEnt->curstate.renderfx = kRenderFxWallHack;
						else
							pEnt->curstate.renderfx = kRenderFxNone;
					}
					else
						pEnt->curstate.renderfx = kRenderFxNone;
				}
			}
		}
		break;
	}
	case EVENT_BALROG11_FOLLOW_ENTITY:
	{
		CreateBalrog11CannonSingleProjectile(NULL, args->origin, idx, args->iparam2);
		break;
	}
	case EVENT_LANCE:
	{
		if (!EV_IsLocal(idx))
			return;

		model_s* pModel;
		pModel = IEngineStudio.Mod_ForName("sprites/ef_lance_cockpung.spr", false);
		if (pModel)
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_AUTO, "weapons/lance_slash3_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			cl_entity_t* viewent = gEngfuncs.GetViewModel();

			if (!viewent)
				return;

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[0], pModel);

			if (!pEnt)
				return;

			pEnt->flags |= FTENT_SPRANIMATE;
			pEnt->entity.curstate.frame = 0;
			pEnt->entity.curstate.framerate = 30.0;
			pEnt->entity.curstate.movetype = MOVETYPE_NONE;
			pEnt->entity.curstate.solid = SOLID_TRIGGER;
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.scale = 0.5;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->die = gHUD.m_flTime + 0.5;
		}
		
		break;
	}
	case EVENT_BUFFM249:
	{
		for (int i = 2; i <= 14; i = i + 2)
		{
			model_s* pModel;

			char pModelName[32];
			int index = Com_RandomLong(0, 4);
			sprintf(pModelName, "sprites/buffm249_fire0%i.spr", index + 1);

			pModel = IEngineStudio.Mod_ForName(pModelName, false);
			if (pModel)
			{
				cl_entity_t* viewent = gEngfuncs.GetViewModel();
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[i], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_PERSIST | FTENT_CLIENTCUSTOM | FTENT_FADEOUT;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOWMOVE;
				pEnt->entity.curstate.aiment = viewent->index;
				pEnt->entity.curstate.body = i + 1;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.005f, 0.01f);
				pEnt->entity.baseline.renderamt = pEnt->entity.curstate.renderamt = gEngfuncs.pfnRandomFloat(150.0f, 200.0f);
				pEnt->fadeSpeed = 2.0f;
				pEnt->die = gHUD.m_flTime + 0.3f;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;
				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					char pModelName[32];
					int index = Com_RandomLong(0, 4);
					sprintf(pModelName, "sprites/buffm249_fire0%i.spr", index + 1);
					pEnt->entity.curstate.modelindex = gEngfuncs.pEventAPI->EV_FindModelIndex(pModelName);
					pEnt->entity.curstate.frame += frametime * 2;
					if(pEnt->entity.curstate.scale < 0.03f)
						pEnt->entity.curstate.scale += frametime * 0.05f;
				};
			}
		}
		break;
	}
	case EVENT_DUALSWORD_SKILL:
	{
		struct model_s* pModel = nullptr;

		if (!args->bparam1)
		{
			pModel = IEngineStudio.Mod_ForName("models/dualsword_skill.mdl", 0);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

				if (!pEnt)
					return;

				pEnt->die = gHUD.m_flTime + 1.5f;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.framerate = 1.0f;
				pEnt->entity.curstate.sequence = args->bparam2;
				pEnt->clientIndex = args->iparam2;
				pEnt->tentOffset = Vector(0, 0, -20);
				pEnt->flags |= FTENT_PLYRATTACHMENT;
			}
		}
		else
		{
			if (args->iparam2 == 1)
			{
				pModel = IEngineStudio.Mod_ForName("models/dualsword_skillfx1.mdl", 0);

				if (pModel)
				{
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

					if (!pEnt)
						return;
								
					pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
					pEnt->flags |= FTENT_FADEOUT | FTENT_CLIENTCUSTOM;
					pEnt->entity.curstate.solid = SOLID_NOT;
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.renderfx = kRenderFxNone;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->entity.curstate.angles = args->angles;
					pEnt->entity.curstate.movetype = MOVETYPE_NOCLIP;
					pEnt->entity.curstate.animtime = gHUD.m_flTime;
					pEnt->entity.curstate.framerate = 1.0f;
					pEnt->entity.curstate.fuser1 = gHUD.m_flTime + 0.5f;

					pEnt->entity.curstate.sequence = Com_RandomLong(0, 2);
					pEnt->die = gHUD.m_flTime + 0.5f;

					pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
						if (gHUD.m_flTime - pEnt->entity.curstate.fuser1 < 1.0f)
							pEnt->entity.curstate.renderamt = 255 * (1.0f - (gHUD.m_flTime - pEnt->entity.curstate.fuser1));
						else
							pEnt->die = gHUD.m_flTime;
					};
				}
			}
			else
			{
				pModel = IEngineStudio.Mod_ForName("models/dualsword_skillfx2.mdl", 0);

				if (pModel)
				{
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);

					if (!pEnt)
						return;

					Vector vecAngle, vecVelocity;
					vecVelocity = args->angles;
					VectorAngles(vecVelocity, vecAngle);

					pEnt->flags |= FTENT_CLIENTCUSTOM;
					pEnt->entity.curstate.solid = SOLID_NOT;
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.renderfx = kRenderFxNone;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->entity.baseline.origin = vecVelocity;
					pEnt->entity.angles = vecAngle;
					pEnt->entity.curstate.movetype = MOVETYPE_NOCLIP;
					pEnt->entity.curstate.animtime = gHUD.m_flTime;
					pEnt->entity.curstate.framerate = 1.0f;

					pEnt->entity.curstate.vuser1 = origin;	//oldorigin
					pEnt->entity.curstate.sequence = 0;
					pEnt->die = gHUD.m_flTime + 1.0f;

					pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
						if ((pEnt->entity.origin - pEnt->entity.curstate.vuser1).Length2D() > 200)
							pEnt->die = gHUD.m_flTime;					
					};
				}
			}			
		}
		break;
	}
	case EVENT_DUALSWORD_EYE:
	{
		model_s* pModel;
		int index;
		const char* pModelName;
		if (args->bparam1)
		{
			index = 0;
			pModelName = "sprites/ef_dualsword_right.spr";
		}
		else
		{
			index = 1;
			pModelName = "sprites/ef_dualsword_left.spr";
		}
			
		pModel = IEngineStudio.Mod_ForName(pModelName, false);
		if (pModel)
		{
			cl_entity_t* viewent = gEngfuncs.GetViewModel();

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[index], pModel);

			if (!pEnt)
				return;

			pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
			pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
			pEnt->entity.curstate.frame = 0.0f;
			pEnt->entity.curstate.iuser1 = index;
			pEnt->entity.curstate.framerate = 30.0f;
			pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
			pEnt->entity.curstate.aiment = viewent->index;
			pEnt->entity.curstate.solid = SOLID_NOT;
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderfx = kRenderFxNone;
			pEnt->frameMax = 30.0;
			pEnt->entity.curstate.scale = 0.036f;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->entity.curstate.body = index + 1;
			pEnt->entity.curstate.weaponmodel = viewent->curstate.weaponmodel;
			pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED|EFLAG_AFTER_VIEWMODEL;

			if (!index)
				pEnt->die = gHUD.m_flTime + 3.03;
			else
				pEnt->die = gHUD.m_flTime + 4.03;

			pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
				cl_entity_t* viewent = gEngfuncs.GetViewModel();
				if (!pEnt->entity.curstate.iuser1)
				{
					if (viewent->curstate.sequence != 0)
						pEnt->die = gHUD.m_flTime;
				}
				else
				{
					if (viewent->curstate.sequence != 7)
						pEnt->die = gHUD.m_flTime;
				}
			};
		}
		break;
	}
	case EVENT_DRAGONSWORDFX:
	{
		cl_entity_t* viewent = gEngfuncs.GetViewModel();
		if(!viewent)
			return;

		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempModel(viewent->origin + Vector(0, 0, 16), Vector(0, 0, 0), Vector(0, 0, 0), 0.35, gEngfuncs.pEventAPI->EV_FindModelIndex("models/ef_dragonsword.mdl"), 0);

		if (!pEnt)
			return;

		pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
		pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_SPRANIMATE | FTENT_FADEOUT;
		
		pEnt->entity.angles = viewent->angles;
		pEnt->tentOffset.z = 16;
		pEnt->clientIndex = viewent->index;
		pEnt->entity.curstate.skin = 0;
		pEnt->entity.curstate.frame = 0;
		pEnt->fadeSpeed = 3;
		pEnt->entity.curstate.rendermode = kRenderTransAdd;
		pEnt->entity.baseline.renderamt = 128;
		pEnt->frameMax = 256;
		pEnt->entity.curstate.animtime = gHUD.m_flTime + args->fparam2;
		pEnt->entity.curstate.framerate = 1.0;
		pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

		int iType = args->iparam2;

		if (gHUD.cl_righthand->value > 0)
		{
			if (iType == 0)
				pEnt->entity.curstate.sequence = 0;
			else if (iType == 2)
				pEnt->entity.curstate.sequence = 2;
		}
		else if (iType == 0)
		{
			pEnt->entity.curstate.sequence = 1;
		}
		else if (iType == 2)
		{
			pEnt->entity.curstate.sequence = 3;
		}

		pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
			cl_entity_t* viewent = gEngfuncs.GetViewModel();
			pEnt->entity.origin = viewent->origin;
			pEnt->entity.angles = viewent->angles;
		};

		break;
	}
	case EVENT_RUNEBLADEFX:
	{
		cl_entity_t* viewent = gEngfuncs.GetViewModel();
		if (!viewent)
			return;

		model_s* pModel;
		pModel = IEngineStudio.Mod_ForName("models/ef_runeblade_swing.mdl", 0);
		if (pModel)
		{
			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(args->origin, pModel);

			if (!pEnt)
				return;

			pEnt->entity.angles = viewent->angles;
			pEnt->flags &= ~FTENT_GRAVITY;
			pEnt->entity.curstate.sequence = 0;
			pEnt->entity.curstate.frame = 0;
			pEnt->entity.curstate.renderamt = 250;
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.animtime = gHUD.m_flTime;
			pEnt->entity.curstate.framerate = 1.0;
			pEnt->die = gHUD.m_flTime + 0.5;

			
		}
		break;
	}
	case EVENT_RUNEBLADE_EYE:
	{
		model_s* pModel;

		pModel = IEngineStudio.Mod_ForName("sprites/runeblade_eye.spr", false);
		if (pModel)
		{
			cl_entity_t* viewent = gEngfuncs.GetViewModel();

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[0], pModel);

			if (!pEnt)
				return;

			pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
			pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
			pEnt->entity.curstate.frame = 0.0f;
			pEnt->entity.curstate.framerate = 30.0f;
			pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
			pEnt->entity.curstate.aiment = viewent->index;

			pEnt->entity.curstate.solid = SOLID_NOT;
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderfx = kRenderFxNone;
			pEnt->frameMax = 40.0;
			pEnt->entity.curstate.scale = 0.075f;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->entity.curstate.body =  1;
			pEnt->entity.curstate.weaponmodel = viewent->curstate.weaponmodel;
			pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

			pEnt->die = gHUD.m_flTime + 22;

			pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
				cl_entity_t* viewent = gEngfuncs.GetViewModel();

				if (viewent->curstate.sequence != 0)
					pEnt->die = gHUD.m_flTime;
			};
		}
		break;
	}
	case EVENT_SUMMONER_STAB:
	{
		model_s* pModel;
		pModel = IEngineStudio.Mod_ForName("sprites/ef_summoner_stab1.spr", false);

		if (args->iparam2 == 1)
		{
			pModel = IEngineStudio.Mod_ForName("sprites/ef_summon_charging_exp.spr", false);

			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_AUTO, "weapons/plasmagun_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(args->origin, pModel);

				if (!pEnt)
					return;

				pEnt->flags |= FTENT_SPRANIMATE;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 30.0;
				pEnt->entity.curstate.movetype = MOVETYPE_NONE;
				pEnt->entity.curstate.solid = SOLID_TRIGGER;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.scale = 0.6;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->frameMax = 31.0;
				pEnt->die = gHUD.m_flTime + 1.0;
			}
		}
		else
		{
			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(args->origin, pModel);

				if (!pEnt)
					return;

				pEnt->flags |= FTENT_SPRANIMATE;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 30.0;
				pEnt->entity.curstate.movetype = MOVETYPE_NONE;
				pEnt->entity.curstate.solid = SOLID_TRIGGER;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.scale = 0.6;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 0.83;
			}

			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(idx);
			int iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_summoner_stab2.spr");

			BEAM* pBeam;

			pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(ent->attachment[2], args->origin,
				iModel,
				0.83,
				35.0,
				0.0,
				200.0,
				35.0,
				1.0,
				0.0,
				1.0,
				1.0,
				1.0
			);
		}
		
		break;
	}
	case EVENT_SUMMONER_SLASH:
	{
		model_s* pModel;
		pModel = IEngineStudio.Mod_ForName("models/ef_summoner_swing.mdl", false);

		if (pModel)
		{
			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(idx);

			if (!ent)
				return;

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[3], pModel);

			if (!pEnt)
				return;

			pEnt->flags |= FTENT_CLIENTCUSTOM;
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->entity.curstate.animtime = gHUD.m_flTime;
			pEnt->entity.curstate.framerate = 1.0;
			pEnt->entity.curstate.body = 4;
			pEnt->entity.curstate.sequence = args->iparam2;
			pEnt->die = gHUD.m_flTime + 0.8;
			pEnt->entity.curstate.iuser1 = idx;

			pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
				cl_entity_t* ent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.iuser1);

				if (!ent)
				{
					pEnt->die = gHUD.m_flTime;
					return;
				}

				if (pEnt->die >= gHUD.m_flTime)
				{
					if (pEnt->die <= gHUD.m_flTime + 0.3f)
						pEnt->entity.curstate.renderamt = (int)(255.0f * (1.0 - (gHUD.m_flTime + 0.3f - pEnt->die) / 0.3f));
				}
			};
		}
	break;
	}
	case EVENT_SUMMONER:
	{
		switch (args->iparam2)
		{
		case 0:
		{
			//appear
			idx = args->entindex;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(idx);
			if (!pPlayer)
				return;

			model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_summoner_summon01.spr", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin + Vector(0, 0, 125), pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_NONE;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 33.0;
				pEnt->entity.curstate.scale = 1.0f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 1.1;
				pEnt->entity.curstate.fuser1 = gHUD.m_flTime + 0.6;
				pEnt->entity.curstate.iuser1 = idx;
				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {

					if (currenttime > pEnt->entity.curstate.fuser1)
					{
						pEnt->entity.curstate.fuser1 = currenttime + 9999.0;
						int id = pEnt->entity.curstate.iuser1;
						cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(id);
						if (!pPlayer)
							return;

						model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_summoner_summon02.spr", false);

						if (pModel)
						{
							TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin + Vector(0, 0, 125), pModel);

							if (!pEnt)
								return;

							pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
							pEnt->flags |= FTENT_SPRANIMATE;
							pEnt->entity.curstate.frame = 0.0f;
							pEnt->entity.curstate.framerate = 30.0f;
							pEnt->entity.curstate.movetype = MOVETYPE_NONE;
							pEnt->entity.curstate.solid = SOLID_NOT;
							pEnt->entity.curstate.rendermode = kRenderTransAdd;
							pEnt->entity.curstate.renderfx = kRenderFxNone;
							pEnt->frameMax = 46.0;
							pEnt->entity.curstate.scale = 1.5f;
							pEnt->entity.curstate.renderamt = 255;
							pEnt->die = gHUD.m_flTime + 1.53;
						}

						pModel = IEngineStudio.Mod_ForName("models/ef_summoner_summoning.mdl", false);

						if (pModel)
						{
							TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin + Vector(0, 0, -25), pModel);

							if (!pEnt)
								return;

							pEnt->entity.curstate.frame = 0;
							pEnt->entity.curstate.sequence = 0;
							pEnt->entity.curstate.framerate = 1.0f;
							pEnt->entity.curstate.animtime = gHUD.m_flTime;
							pEnt->entity.curstate.movetype = MOVETYPE_NONE;
							pEnt->entity.curstate.solid = SOLID_NOT;
							pEnt->entity.curstate.rendermode = kRenderTransAdd;
							pEnt->entity.curstate.renderfx = kRenderFxNone;
							pEnt->entity.curstate.renderamt = 255;
							pEnt->die = gHUD.m_flTime + 2.7;
							pEnt->flags |= FTENT_CLIENTCUSTOM;
							pEnt->entity.curstate.fuser1 = gHUD.m_flTime + 0.3f;

							pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
								if (gHUD.m_flTime - pEnt->entity.curstate.fuser1 < 2.0f)
									pEnt->entity.curstate.renderamt = 255 * (2.0f - (gHUD.m_flTime - pEnt->entity.curstate.fuser1));
								else
									pEnt->die = gHUD.m_flTime;
							};
						}
					}
				};
			}
			break;
		}
		case 1:
		{
			//appear Trash
			idx = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(idx);
			if (!pPlayer)
				return;

			model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_summoner_summon02.spr", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin + Vector(0, 0, 25), pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_NONE;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 46.0;
				pEnt->entity.curstate.scale = 1.8f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 1.53;
			}

			pModel = IEngineStudio.Mod_ForName("models/ef_summoner_summoning.mdl", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin + Vector(0, 0, -25), pModel);

				if (!pEnt)
					return;

				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.sequence = 0;
				pEnt->entity.curstate.framerate = 1.0f;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.movetype = MOVETYPE_NONE;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 2.7;
				pEnt->flags |= FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.fuser1 = gHUD.m_flTime + 0.3f;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					if (gHUD.m_flTime - pEnt->entity.curstate.fuser1 < 1.2f)
						pEnt->entity.curstate.renderamt = 255 * (1.2f - (gHUD.m_flTime - pEnt->entity.curstate.fuser1));
					else
						pEnt->die = gHUD.m_flTime;
				};
			}
			break;
		}
		case 2:
		{
			//hide
			idx = args->entindex;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(idx);
			if (!pPlayer)
				return;

			model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_summoner_unsummon01.spr", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin + Vector(0, 0, 135), pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_NONE;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 36.0;
				pEnt->entity.curstate.scale = 1.0f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 1.53;
				pEnt->entity.curstate.fuser1 = gHUD.m_flTime + 0.5;
				pEnt->entity.curstate.vuser1 = pPlayer->origin + Vector(0, 0, 135);

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {

					if (currenttime > pEnt->entity.curstate.fuser1)
					{
						pEnt->entity.curstate.fuser1 = currenttime + 9999.0;
						model_s* pModel = pModel = IEngineStudio.Mod_ForName("sprites/ef_summoner_unsummon02.spr", false);

						if (pModel)
						{
							TEMPENTITY* ent = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pEnt->entity.curstate.vuser1, pModel);

							if (!ent)
								return;

							ent->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
							ent->flags |= FTENT_SPRANIMATE;
							ent->entity.curstate.frame = 0.0f;
							ent->entity.curstate.framerate = 30.0f;
							ent->entity.curstate.movetype = MOVETYPE_NONE;
							ent->entity.curstate.solid = SOLID_NOT;
							ent->entity.curstate.rendermode = kRenderTransAdd;
							ent->entity.curstate.renderfx = kRenderFxNone;
							ent->frameMax = 36.0;
							ent->entity.curstate.scale = 1.0f;
							ent->entity.curstate.renderamt = 255;
							ent->die = gHUD.m_flTime + 1.53;
						}
					}
				};
			}
			break;
		}
		case 3:
		{
			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(idx);
			if (!ent)
				return;

			model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_summoner_smoke.spr", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[1], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.aiment = idx;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.body = 1;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 29.0;
				pEnt->entity.curstate.scale = 0.3f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 13;
				pEnt->entity.curstate.iuser1 = idx;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* ent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.iuser1);

					if (!ent)
						pEnt->die = gHUD.m_flTime;
				};
			}
		}
		default:
			break;
		}
	break;
	}
	case EVENT_REVIVEGUN:
	{
		if (EV_IsLocal(args->entindex))
		{
			if (args->bparam2)
			{
				model_s* pModel;

				char pModelName[128];

				if (args->iparam2)
				{
					//draw
					if (args->bparam1)
					{
						if (!gHUD.cl_righthand->value)
							sprintf(pModelName, "sprites/ef_revivegun_draw2_left.spr");
						else
							sprintf(pModelName, "sprites/ef_revivegun_draw2.spr");
					}
					else
					{
						if (!gHUD.cl_righthand->value)
							sprintf(pModelName, "sprites/ef_revivegun_draw1_left.spr");
						else
							sprintf(pModelName, "sprites/ef_revivegun_draw1.spr");
					}

				}
				else
				{
					if (args->bparam1)
					{
						if (!gHUD.cl_righthand->value)
							sprintf(pModelName, "sprites/ef_revivegun_idle2_left.spr");
						else
							sprintf(pModelName, "sprites/ef_revivegun_idle2.spr");
					}
					else
					{
						if (!gHUD.cl_righthand->value)
							sprintf(pModelName, "sprites/ef_revivegun_idle1_left.spr");
						else
							sprintf(pModelName, "sprites/ef_revivegun_idle1.spr");
					}
				}

				pModel = IEngineStudio.Mod_ForName(pModelName, false);
				if (pModel)
				{
					cl_entity_t* viewent = gEngfuncs.GetViewModel();
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[5], pModel);

					if (!pEnt)
						return;

					if (viewent->curstate.sequence != 5 || viewent->curstate.sequence != 11)
						viewent->curstate.skin = 0;

					if (args->iparam2)
					{
						pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
						pEnt->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
						pEnt->entity.curstate.frame = 0;
						pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
						pEnt->entity.curstate.framerate = 30.0f;
						pEnt->entity.curstate.aiment = viewent->index;
						pEnt->entity.curstate.body = 6;
						pEnt->frameMax = 31;

						pEnt->entity.curstate.solid = SOLID_NOT;
						pEnt->entity.curstate.rendermode = kRenderTransAdd;
						pEnt->entity.curstate.renderfx = kRenderFxNone;
						pEnt->entity.curstate.weaponmodel = viewent->curstate.weaponmodel;
						pEnt->entity.curstate.scale = args->bparam1 ? 0.04f : 0.03f;

						pEnt->die = gHUD.m_flTime + 1.0;
						pEnt->entity.curstate.iuser1 = viewent->index;
						pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;
					}
					else
					{
						pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
						pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
						pEnt->entity.curstate.frame = 0;
						pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
						pEnt->entity.curstate.framerate = 30.0f;
						pEnt->entity.curstate.aiment = viewent->index;
						pEnt->entity.curstate.body = 6;
						pEnt->frameMax = args->bparam1 ? 47 : 45;

						pEnt->entity.curstate.solid = SOLID_NOT;
						pEnt->entity.curstate.rendermode = kRenderTransAdd;
						pEnt->entity.curstate.renderfx = kRenderFxNone;
						pEnt->entity.curstate.weaponmodel = viewent->curstate.weaponmodel;
						pEnt->entity.curstate.scale = args->bparam1 ? 0.04f : 0.03f;
						pEnt->entity.curstate.iuser2 = args->bparam1;

						pEnt->die = gHUD.m_flTime + (args->bparam1 ? 1.77f : 1.7f);
						pEnt->entity.curstate.iuser1 = viewent->index;
						pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

						pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
							cl_entity_t* viewent = gEngfuncs.GetViewModel();

							if (viewent)
							{
								if (pEnt->entity.curstate.iuser2)
								{
									if (viewent->curstate.sequence == 7 || viewent->curstate.sequence < 5)
										pEnt->die = gHUD.m_flTime;
								}
								else
								{
									if (viewent->curstate.sequence == 1 || viewent->curstate.sequence > 6)
										pEnt->die = gHUD.m_flTime;
								}
							}
						};
					}
				}

			}
		}

		if (!args->bparam2)
		{
			model_s* pModel = IEngineStudio.Mod_ForName("models/ef_revivegun_field.mdl", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(args->origin + Vector(0, 0, -36), pModel);

				if (!pEnt)
					return;

				pEnt->entity.curstate.scale = 1.0;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.sequence = 0;
				pEnt->entity.curstate.framerate = 1.0f;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.movetype = MOVETYPE_NONE;
				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->die = gHUD.m_flTime + 1.7;
				pEnt->flags |= FTENT_NONE;
			}
		}
		break;
	}
	case EVENT_VOIDPISTOL_BLACKHOLE:
	{
		if (EV_IsLocal(args->entindex))
		{

			char pModelName[128];

			switch (args->iparam2)
			{
			case 1:
			{
				sprintf(pModelName, "sprites/ef_blackhole01.spr"); break;
			}
			case 2:
			{
				sprintf(pModelName, "sprites/ef_blackhole02.spr"); break;
			}
			case 3: 
			{
				sprintf(pModelName, "sprites/ef_blackhole03.spr"); break;
			}
			case 4:
			{
				sprintf(pModelName, "sprites/ef_blackhole04.spr"); break;
			}
			default:
				return;
			}			

			model_s* pModel = IEngineStudio.Mod_ForName(pModelName, false);
			if (pModel)
			{
				cl_entity_t* viewent = gEngfuncs.GetViewModel();
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[1], pModel);

				if (!pEnt)
					return;
			
				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.aiment = viewent->index;
				pEnt->entity.curstate.body = 2;
				pEnt->frameMax = 35;

				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->entity.curstate.weaponmodel = viewent->curstate.weaponmodel;
				if (args->iparam2 == 4)
				{
					pEnt->entity.curstate.scale = 0.05;
					pEnt->die = gHUD.m_flTime + 3.0f;
					pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;
				}
				else
				{
					pEnt->die = gHUD.m_flTime + 1.4f;
					pEnt->entity.curstate.scale = 0.01;
					if(args->iparam2 == 3)
						pEnt->entity.curstate.scale = 0.005;
					pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;
				}
				
				pEnt->entity.curstate.iuser2 = args->iparam2;

				
				pEnt->entity.curstate.iuser1 = viewent->index;
				

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					if (viewent)
					{
						if (pEnt->entity.curstate.iuser2 == 4)
						{
							if (viewent->curstate.sequence != 2 || viewent->curstate.sequence != 5)
								pEnt->die = gHUD.m_flTime;
						}
						else
						{
							if (viewent->curstate.sequence == 2 || (viewent->curstate.sequence >= 5 && viewent->curstate.sequence <= 10) || viewent->curstate.sequence >= 13)
								pEnt->die = gHUD.m_flTime;
						}
					}
				};
					
			}		
		}
		break;
	}
	case EVENT_FOLLOW_ENT:
	{
		cl_entity_t* pFollow = gEngfuncs.GetEntityByIndex(idx);
		if (!pFollow)
			return;

		model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_aksha_fireball.spr", false);
		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(args->origin, pModel);

		if (pEnt)
		{
			pEnt->die = gHUD.m_flTime + 1.0;
			pEnt->entity.curstate.iuser1 = idx;
			pEnt->entity.curstate.iuser2 = args->iparam2;
			pEnt->entity.curstate.fuser1 = args->fparam1;
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderfx = kRenderFxNone;
			pEnt->entity.curstate.frame = 0;
			pEnt->entity.curstate.framerate = 30.0f;
			pEnt->entity.curstate.fuser2 = gHUD.m_flTime + args->fparam2;
			pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
			pEnt->entity.curstate.vuser1 = pFollow->origin;
			pEnt->callback = [](tempent_s* ent, float frametime, float currenttime)
			{
				cl_entity_t* pFollow = gEngfuncs.GetEntityByIndex(ent->entity.curstate.iuser1);
				if (!pFollow)
				{
					ent->die = currenttime;
					return;
				}
				else
				{
					//if (pFollow->origin == ent->entity.curstate.vuser1) // Entity Stoppted = Death
					//{
					//	ent->die = gHUD.m_flTime;
					//	return;
					//}

					ent->entity.curstate.vuser1 = pFollow->origin;

					if (ent->entity.curstate.fuser2 > currenttime)
						return;

					if (ent->entity.curstate.fuser1)
						ent->entity.curstate.fuser2 = currenttime + ent->entity.curstate.fuser1;

					model_t* pModel = NULL;
					float fScale;
					int frameCount;
					float displaytime;
					switch (ent->entity.curstate.iuser2)
					{
						case 0:
						{
							pModel = IEngineStudio.Mod_ForName("sprites/ef_aksha_fireball.spr", 0);
							frameCount = 30;
							displaytime = (float)frameCount / 30.0f;
							fScale = 0.3;
							break;
						}
					}

					if (!pModel)
					{
						ent->die = gHUD.m_flTime;
						return;
					}

					Vector vecOrigin = pFollow->origin;

					vecOrigin.x += gEngfuncs.pfnRandomFloat(-2.0, 2.0);
					vecOrigin.y += gEngfuncs.pfnRandomFloat(-2.0, 2.0);
					vecOrigin.z += gEngfuncs.pfnRandomFloat(-2.0, 2.0);

					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(vecOrigin, pModel);
					if (pEnt)
					{
						pEnt->flags |= FTENT_CLIENTCUSTOM;
						pEnt->die = gHUD.m_flTime + displaytime;
						pEnt->entity.angles[2] = gEngfuncs.pfnRandomFloat(0.0, 360.0);
						pEnt->entity.curstate.rendermode = kRenderTransAdd;
						pEnt->entity.curstate.renderamt = pEnt->entity.curstate.fuser2 = 250;
						pEnt->entity.curstate.scale = fScale;
						pEnt->entity.curstate.frame = 0;
						pEnt->entity.curstate.framerate = 1.0f;
						pEnt->entity.curstate.fuser1 = displaytime;
						pEnt->frameMax = frameCount;

						pEnt->callback = [](tempent_s* ent, float frametime, float currenttime) {
							if (ent->die >= gHUD.m_flTime)
							{
								float percentage = (ent->die - gHUD.m_flTime) / ent->entity.curstate.fuser1;
								ent->entity.curstate.frame = ent->frameMax * (1.0f - percentage);
								ent->entity.curstate.renderamt = ent->entity.curstate.fuser2 * percentage;
							}
						};

						
					}
				}



			};
			
		}
	}
	case EVENT_BALROGCRITICAL:
	{
		int type = args->iparam2;

		if (!type)
		{
			//balrog9
			struct model_s* pModel;
			TEMPENTITY* pEnt;

			pModel = IEngineStudio.Mod_ForName("sprites/balrogcritical.spr", 0);

			pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->entity.curstate.scale = 0.3;
			pEnt->die = gHUD.m_flTime + 0.6f;
			pEnt->flags |= FTENT_CLIENTCUSTOM;
			pEnt->callback = pEnt->callback = [](tempent_s* ent, float frametime, float currenttime) {
				ent->entity.curstate.scale += 0.0013;
				if (ent->die <= gHUD.m_flTime + 0.4)
					ent->entity.curstate.renderamt -= 15;

				if (ent->entity.curstate.renderamt < 0)
					ent->entity.curstate.renderamt = 0;

			};

			int id;
			gEngfuncs.CL_LoadModel("sprites/fexplo.spr", &id);
			origin[2] -= 15.0;
			gEngfuncs.pEfxAPI->R_Explosion(origin, id, 0.45, 60, TE_EXPLFLAG_NONE);
		}
		else
		{
			struct model_s* pModel;
			TEMPENTITY* pEnt;

			pModel = IEngineStudio.Mod_ForName("sprites/balrogcritical.spr", 0);

			pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->entity.curstate.scale = 0.5;
			pEnt->die = gHUD.m_flTime + 0.6f;
			pEnt->flags |= FTENT_CLIENTCUSTOM;
			pEnt->callback = pEnt->callback = [](tempent_s* ent, float frametime, float currenttime) {
				ent->entity.curstate.scale += 0.007;
				if (ent->die <= gHUD.m_flTime + 0.3)
					ent->entity.curstate.renderamt -= 35;
			};

			int id;
			gEngfuncs.CL_LoadModel("sprites/fexplo.spr", &id);
			origin[2] -= 15.0;
			gEngfuncs.pEfxAPI->R_Explosion(origin, id, 0.45, 30, TE_EXPLFLAG_NONE);

			gEngfuncs.pEfxAPI->R_TracerEffect(gEngfuncs.GetEntityByIndex(idx)->origin, origin);
		}
		break;
	}
	case EVENT_M32VENOM:
	{
		struct model_s* pModel;
		TEMPENTITY* pEnt;

		if (args->bparam1)
		{
			//touch
			pModel = IEngineStudio.Mod_ForName("sprites/ef_boomer_ex.spr", 0);
			pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			pEnt->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderamt = 255;
			pEnt->entity.curstate.scale = 0.5f;
			pEnt->die = gHUD.m_flTime + 0.1f;

			pModel = IEngineStudio.Mod_ForName("sprites/ef_m32_poison.spr", 0);
			TEMPENTITY* pEnt2;
			pEnt2 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			pEnt2->entity.curstate.rendermode = kRenderTransAdd;
			pEnt2->entity.curstate.renderamt = 210;
			pEnt2->entity.curstate.scale = 2.0f;
			pEnt2->entity.curstate.fuser1 = args->fparam1;
			pEnt2->die = gHUD.m_flTime + args->fparam1;
			pEnt2->entity.curstate.animtime = gHUD.m_flTime;
			pEnt2->frameMax = 23;
			pEnt2->flags |= FTENT_CLIENTCUSTOM;
			
			pEnt2->callback = [](tempent_s* pEnt, float frametime, float currenttime)
			{
				pEnt->entity.curstate.frame = pEnt->frameMax * (pEnt->die - gHUD.m_flTime) / pEnt->entity.curstate.fuser1;
				pEnt->entity.curstate.renderamt = 200 * (pEnt->die - gHUD.m_flTime) / pEnt->entity.curstate.fuser1;
			};
		}
		else
		{
			//venom after touch && poison
			struct model_s* pModel;
			TEMPENTITY* pEnt;

			if (args->bparam2)
				pModel = IEngineStudio.Mod_ForName("sprites/ef_m32_poison.spr", 0);	//touch		
			else
				pModel = IEngineStudio.Mod_ForName("sprites/ef_smoke_poison.spr", 0);	//poison	
								
			pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			pEnt->entity.curstate.rendermode = kRenderTransAdd;
			pEnt->entity.curstate.renderamt = 210;
			pEnt->entity.curstate.scale = 2.0f;
			pEnt->entity.curstate.fuser1 = args->fparam1;
			pEnt->die = gHUD.m_flTime + args->fparam1;
			pEnt->entity.curstate.animtime = gHUD.m_flTime;
			pEnt->frameMax = args->bparam2 ? 23 : 39;
			pEnt->entity.curstate.framerate = 0.5;
			pEnt->flags |= FTENT_CLIENTCUSTOM;
			if (args->bparam2)
			{
				pEnt->flags |= FTENT_PLYRATTACHMENT;
				pEnt->clientIndex = args->iparam2;
				pEnt->tentOffset = Vector(0, 0, 20);
			}
			pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime)
			{
				pEnt->entity.curstate.frame = pEnt->frameMax * (pEnt->die - gHUD.m_flTime) / pEnt->entity.curstate.fuser1;
				pEnt->entity.curstate.renderamt = 210 * (pEnt->die - gHUD.m_flTime) / pEnt->entity.curstate.fuser1;
			};
		}	
		break;
	}
	case EVENT_M95TIGERNET:
	{
		model_s* pModel;
		pModel = IEngineStudio.Mod_ForName("models/ef_m95tiger_nethit.mdl", false);
		if (pModel)
		{
			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			if (pEnt)
			{
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.sequence = 1;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1.0;
				pEnt->die = gHUD.m_flTime + 1.5;
				pEnt->clientIndex = args->iparam2;
				pEnt->flags |= FTENT_PLYRATTACHMENT;
			}
		}

		break;
	}
	case EVENT_M95TIGER:
	{
		model_s* pModel;
		TEMPENTITY* pEnt;
		if (args->bparam1)
		{
			pModel = IEngineStudio.Mod_ForName("sprites/ef_m95tiger_start.spr", false);
			if (pModel)
			{
				pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
				if (pEnt)
				{
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->entity.curstate.animtime = gHUD.m_flTime;
					pEnt->entity.curstate.frame = 0;
					pEnt->entity.curstate.framerate = 1.0;
					pEnt->die = gHUD.m_flTime + 0.1;
					pEnt->clientIndex = args->iparam2;
					pEnt->flags |= FTENT_NONE;
				}
			}
		}
		else
		{		
			pModel = IEngineStudio.Mod_ForName("sprites/ef_m95tiger_scratch.spr", false);
			if (pModel)
			{
				pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
				if (pEnt)
				{
					pEnt->entity.curstate.animtime = gHUD.m_flTime;
					pEnt->entity.curstate.frame = 0;
					pEnt->entity.curstate.framerate = 1.0;
					pEnt->die = gHUD.m_flTime + 1.5;
					pEnt->clientIndex = args->iparam2;
					pEnt->entity.curstate.fuser1 = gHUD.m_flTime;
					pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP | FTENT_SPRANIMATE;

					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.renderamt = 255;

					pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
						float delta = currenttime - pEnt->entity.curstate.fuser1;
						if (delta > 1.0)
							pEnt->entity.curstate.renderamt = 255 - (delta - 1.0) * 255;
					};
				}
			}
		}

		break;
	}

	default:
		break;
	}
}

}
