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

namespace cl::event::y22s1holyswordmb {
enum y22s1holyswordmb_e
{
	ANIM_IDLE,
	ANIM_DRAW,
	ANIM_SLASH1,
	ANIM_SLASH2,
	ANIM_SLASH3,
	ANIM_CHARGE_START,
	ANIM_CHARGE_LOOP,
	ANIM_CHARGE_SLASH,
	ANIM_CHARGE_SLASH2,
	ANIM_PARING,
	ANIM_PARING_SLASH,
	ANIM_PARING_SLASH2,
};

void EV_FireY22S1HolySwordMB(event_args_t *args)
{
	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );

	if (args->iparam1)
	{
		cl_entity_t* viewent = gEngfuncs.GetViewModel();
		if (!viewent)
			return;

		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempModel(viewent->origin, Vector(0, 0, 0), Vector(0, 0, 0), 0.35, gEngfuncs.pEventAPI->EV_FindModelIndex("models/ef_holystack.mdl"), 0);

		if (!pEnt)
			return;

		pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
		pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_SPRANIMATE | FTENT_FADEOUT;

		pEnt->entity.angles = viewent->angles;
		pEnt->clientIndex = viewent->index;
		pEnt->entity.curstate.skin = 0;
		pEnt->entity.curstate.frame = 0;
		pEnt->fadeSpeed = 3;
		pEnt->entity.curstate.rendermode = kRenderTransAdd;
		pEnt->entity.baseline.renderamt = 128;
		pEnt->frameMax = 256;
		pEnt->entity.curstate.animtime = gHUD.m_flTime;
		pEnt->entity.curstate.framerate = 1.0;
		pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

		pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
			cl_entity_t* viewent = gEngfuncs.GetViewModel();
			pEnt->entity.origin = viewent->origin;
			pEnt->entity.angles = viewent->angles;
		};
		return;
	}

	if (args->bparam1)
	{
		const char* pModelName;
		pModelName = "models/ef_holysword_parryattack.mdl";

		struct model_s* pModel = IEngineStudio.Mod_ForName(pModelName, false);

		if (pModel)
		{
			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			if (pEnt)
			{
				pEnt->entity.curstate.sequence = 0;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1.0;
				pEnt->tentOffset = Vector(0, 0, -32);
				pEnt->die = gHUD.m_flTime + 0.5;
				pEnt->flags |= FTENT_NONE;
			}
		}

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_AUTO, "weapons/holysword_parryattack.wav", 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
	}
	else
	{
		if (EV_IsLocal(idx))
		{
			model_s* pModel;
			char pModelName[32];
			int index = args->iparam2;

			if (args->bparam2)
			{
				pModel = IEngineStudio.Mod_ForName("sprites/ef_holysword_charge.spr", false);
				if (pModel)
				{
					cl_entity_t* viewent = gEngfuncs.GetViewModel();

					if (!viewent)
						return;

					int clientindex = viewent->index;
					cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

					if (!parent)
						return;

					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[0], pModel);

					pEnt->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
					pEnt->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_CLIENTCUSTOM;
					pEnt->entity.curstate.frame = 0.0f;
					pEnt->entity.curstate.framerate = 30.0f;
					pEnt->entity.curstate.movetype = MOVETYPE_FOLLOW;
					pEnt->entity.curstate.aiment = clientindex;
					pEnt->entity.curstate.solid = SOLID_NOT;
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.renderfx = kRenderFxNone;
					pEnt->frameMax = 11.0;
					pEnt->entity.curstate.scale = 0.04f;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->entity.curstate.body = 1;
					pEnt->die = gHUD.m_flTime + 999;
					pEnt->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
					pEnt->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

					pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime)
					{
						cl_entity_t* viewent = gEngfuncs.GetViewModel();
						if (viewent->curstate.sequence != ANIM_CHARGE_START && viewent->curstate.sequence != ANIM_CHARGE_LOOP)
						{
							pEnt->entity.curstate.renderamt = 0;
							pEnt->die = gHUD.m_flTime;
						}

						cl_entity_t* parent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.aiment);
						if (pEnt->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
						{
							pEnt->entity.curstate.renderamt = 0;
							pEnt->die = gHUD.m_flTime;
						}
					};
				}
			}
		}
	}	
}

}
