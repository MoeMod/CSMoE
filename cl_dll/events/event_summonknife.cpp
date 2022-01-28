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

namespace cl::event::summonknife {
enum summonknife_e
{
	ANIM_IDLE_A,
	ANIM_SLASH1,
	ANIM_SLASH2,
	ANIM_SLASH3,
	ANIM_SLASH4,
	ANIM_SLASH_END,
	ANIM_DRAW_A,
	ANIM_IDLE_B,
	ANIM_STAB1,
	ANIM_STAB2,
	ANIM_STAB_END,
	ANIM_DRAW_B,
	ANIM_CHARGING_START,
	ANIM_CHARGING_LOOP,
	ANIM_CHARGING_RELEASE,
	ANIM_SUMMONING_A,
};

void EV_FireSummonKnife(event_args_s *args)
{
	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );
	cl_entity_t* ent;
	Vector vecBeamStart;
	Vector vecBeamEnd;

	ent = gEngfuncs.GetViewModel();

	if (!ent)
		return;

	if ( EV_IsLocal(idx) )
	{	
		switch (args->iparam1)
		{
		case ANIM_CHARGING_START:
		{
			model_s* pModel;
			const char* pModelName;
			if (!gHUD.cl_righthand->value)
				pModelName = "sprites/ef_summon_charging_left.spr";
			else
				pModelName = "sprites/ef_summon_charging.spr";

			pModel = IEngineStudio.Mod_ForName(pModelName, false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[6], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.aiment = ent->index;

				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 33.0;
				pEnt->entity.curstate.scale = 0.17f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.body = 7;	//attachment + 1
				pEnt->entity.curstate.weaponmodel = ent->curstate.weaponmodel;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				pEnt->die = gHUD.m_flTime + 1.1;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					/*if (viewent->curstate.sequence != 0)
						pEnt->die = gHUD.m_flTime;*/
				};
			}
			break;
		}
		case ANIM_STAB1:
		case ANIM_STAB2:
		{
			model_s* pModel;
			const char* pModelName;

			if (!gHUD.cl_righthand->value)
				pModelName = "sprites/ef_summon_stab1_left.spr";
			else
				pModelName = "sprites/ef_summon_stab1.spr";

			pModel = IEngineStudio.Mod_ForName(pModelName, false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[7], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.aiment = ent->index;

				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 30.0;
				pEnt->entity.curstate.scale = 0.029f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.body = 8;	//attachment + 1
				pEnt->entity.curstate.weaponmodel = ent->curstate.weaponmodel;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				pEnt->die = gHUD.m_flTime + 1.0;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					/*if (viewent->curstate.sequence != 0)
						pEnt->die = gHUD.m_flTime;*/
				};
			}
			break;
		}
		case ANIM_SUMMONING_A:
		{
			model_s* pModel;
			const char* pModelName;
			int iAttachment = 1;
			if (!gHUD.cl_righthand->value)
			{
				pModelName = "sprites/ef_summonl_left.spr";
				iAttachment = 0;
			}
			else
				pModelName = "sprites/ef_summonl.spr";

			pModel = IEngineStudio.Mod_ForName(pModelName, false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[iAttachment], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.aiment = ent->index;

				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 33.0;
				pEnt->entity.curstate.scale = 0.16f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.body = iAttachment + 1;	//attachment + 1
				pEnt->entity.curstate.weaponmodel = ent->curstate.weaponmodel;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				pEnt->die = gHUD.m_flTime + 1.1;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					/*if (viewent->curstate.sequence != 0)
						pEnt->die = gHUD.m_flTime;*/
				};
			}

			if (!gHUD.cl_righthand->value)
			{
				pModelName = "sprites/ef_summonr_left.spr";
				iAttachment = 1;
			}
			else
			{
				pModelName = "sprites/ef_summonr.spr";
				iAttachment = 0;
			}

			pModel = IEngineStudio.Mod_ForName(pModelName, false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[iAttachment], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.aiment = ent->index;

				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 41.0;
				pEnt->entity.curstate.scale = 0.2f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.body = iAttachment + 1;	//attachment + 1
				pEnt->entity.curstate.weaponmodel = ent->curstate.weaponmodel;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				pEnt->die = gHUD.m_flTime + 1.1;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					/*if (viewent->curstate.sequence != 0)
						pEnt->die = gHUD.m_flTime;*/
				};
			}
			break;
		}
		case ANIM_CHARGING_LOOP:
		{
			model_s* pModel;
			const char* pModelName;

			pModel = IEngineStudio.Mod_ForName("sprites/ef_summon_charging_loop.spr", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[6], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.aiment = ent->index;

				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 15.0;
				pEnt->entity.curstate.scale = 0.05f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.body = 7;	//attachment + 1
				pEnt->entity.curstate.weaponmodel = ent->curstate.weaponmodel;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				pEnt->die = gHUD.m_flTime + 9999.0;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					if (viewent->curstate.sequence != ANIM_CHARGING_LOOP)
						pEnt->die = gHUD.m_flTime;
				};
			}
			break;
		}
		case ANIM_CHARGING_RELEASE:
		{
			model_s* pModel;
			int iAttachment = 0;
			if (!gHUD.cl_righthand->value)
				iAttachment = 1;

			pModel = IEngineStudio.Mod_ForName("sprites/ef_summon_stab2.spr", false);

			if (pModel)
			{
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[4], pModel);

				if (!pEnt)
					return;

				pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				pEnt->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.frame = 0.0f;
				pEnt->entity.curstate.framerate = 30.0f;
				pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
				pEnt->entity.curstate.aiment = ent->index;

				pEnt->entity.curstate.solid = SOLID_NOT;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.renderfx = kRenderFxNone;
				pEnt->frameMax = 46.0;
				pEnt->entity.curstate.scale = 0.1f;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.body = 5;	//attachment + 1
				pEnt->entity.curstate.weaponmodel = ent->curstate.weaponmodel;
				pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				pEnt->die = gHUD.m_flTime + 1.53;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					/*if (viewent->curstate.sequence != 0)
						pEnt->die = gHUD.m_flTime;*/
				};
			}
			break;
		}
		}
	}

}

}
