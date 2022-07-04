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

namespace cl::event::whipsword {
enum whipsword_e
{
	ANIM_IDLE,
	ANIM_SLASH1,
	ANIM_SLASH1_END,
	ANIM_SLASH_LOOP,
	ANIM_SLASH_END,
	ANIM_SLASH_SKILL,
	ANIM_STAB1,
	ANIM_STAB2,
	ANIM_STAB12_END,
	ANIM_STAB_LOOP,
	ANIM_STAB_END,
	ANIM_STAB_SKILL_START,
	ANIM_STAB_SKILL_LOOP,
	ANIM_STAB_SKILL_END,
	ANIM_DRAW,
};

TEMPENTITY* g_WhipSwordSlashEffect = NULL;
TEMPENTITY* g_WhipSwordStabEffect = NULL;

void EV_FireWhipSword(event_args_t* args)
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

	switch (args->iparam1)
	{
	case 1:
	{
		if (g_WhipSwordSlashEffect)
			g_WhipSwordSlashEffect->die = gHUD.m_flTime;

		struct model_s* pModel = IEngineStudio.Mod_ForName("models/ef_whipsword_slash_skill.mdl", false);
		if (pModel)
		{
			int id = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(id);

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			if (pEnt)
			{
				pEnt->entity.angles = pPlayer->angles;
				pEnt->entity.angles.z = 0;
				pEnt->entity.curstate.sequence = 0;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1.0;
				pEnt->entity.curstate.effects |= EF_NOCULL;
				//pEnt->tentOffset = Vector(0, 0, -32);
				pEnt->die = gHUD.m_flTime + 1.0;
				pEnt->clientIndex = idx;
				pEnt->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.iuser1 = id;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime)
				{
					cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.iuser1);
					pEnt->entity.angles = pPlayer->angles;
					pEnt->entity.angles.z = 0;
				};
			}
		}
		break;
	}
	case 2:
	{
		if (EV_IsLocal(idx))
			return;

		struct model_s* pModel = IEngineStudio.Mod_ForName("models/ef_whipsword_slash.mdl", false);
		if (pModel)
		{
			int id = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(id);

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			if (pEnt)
			{
				pEnt->entity.angles = pPlayer->angles;
				pEnt->entity.angles.z = 0;
				pEnt->entity.curstate.sequence = 0;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.curstate.framerate = 1.0;
				//pEnt->entity.curstate.effects |= EF_NOCULL;
				pEnt->tentOffset = Vector(0, 0, 12);
				pEnt->die = gHUD.m_flTime + 0.7;
				pEnt->clientIndex = idx;
				pEnt->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.iuser1 = id;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime)
				{
					cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.iuser1);
					pEnt->entity.angles = pPlayer->angles;
					pEnt->entity.angles.z = 0;
				};
			}
		}

		break;
	}
	case 3:
	{
		if (EV_IsLocal(idx))
			return;

		struct model_s* pModel = IEngineStudio.Mod_ForName("models/ef_whipsword_stab.mdl", false);
		if (pModel)
		{
			int id = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(id);

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			if (pEnt)
			{
				pEnt->entity.angles = pPlayer->angles;
				pEnt->entity.angles.z = 0;
				pEnt->entity.curstate.sequence = args->iparam2;
				pEnt->entity.curstate.animtime = gHUD.m_flTime;
				pEnt->entity.curstate.frame = 0;
				pEnt->entity.curstate.framerate = 1.0;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->tentOffset = Vector(0, 0, 12);
				pEnt->die = gHUD.m_flTime + 0.7;
				pEnt->clientIndex = idx;
				pEnt->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM;
				pEnt->entity.curstate.iuser1 = id;

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime)
				{
					cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.iuser1);
					pEnt->entity.angles = pPlayer->angles;
					pEnt->entity.angles.z = 0;
				};
			}
		}

		break;
	}
	case 4:
	{
		if (EV_IsLocal(idx))
			return;

		struct model_s* pModel = IEngineStudio.Mod_ForName("models/ef_whipsword_stab_loop.mdl", false);
		if (pModel)
		{
			if (g_WhipSwordStabEffect)
				g_WhipSwordStabEffect->die = gHUD.m_flTime;

			int id = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(id);

			g_WhipSwordStabEffect = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			if (g_WhipSwordStabEffect)
			{
				g_WhipSwordStabEffect->entity.angles = pPlayer->angles;
				g_WhipSwordStabEffect->entity.angles.z = 0;
				g_WhipSwordStabEffect->entity.curstate.sequence = 0;
				g_WhipSwordStabEffect->entity.curstate.animtime = gHUD.m_flTime;
				g_WhipSwordStabEffect->entity.curstate.frame = 0;
				g_WhipSwordStabEffect->entity.curstate.framerate = 1.0;
				g_WhipSwordStabEffect->entity.curstate.rendermode = kRenderTransAdd;
				g_WhipSwordStabEffect->tentOffset = Vector(0, 0, 12);
				g_WhipSwordStabEffect->die = gHUD.m_flTime + 5.0;
				g_WhipSwordStabEffect->clientIndex = idx;
				g_WhipSwordStabEffect->entity.curstate.iuser1 = id;
				g_WhipSwordStabEffect->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM;
			}

			g_WhipSwordStabEffect->callback = [](tempent_s* pEnt, float frametime, float currenttime)
			{
				cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(g_WhipSwordStabEffect->entity.curstate.iuser1);
				pEnt->entity.angles = pPlayer->angles;
				pEnt->entity.angles.z = 0;
			};
		}

		break;
	}
	case 5:
	{
		if (EV_IsLocal(idx))
			return;

		struct model_s* pModel = IEngineStudio.Mod_ForName("models/ef_whipsword_slash_loop.mdl", false);
		if (pModel)
		{
			if (g_WhipSwordSlashEffect)
				g_WhipSwordSlashEffect->die = gHUD.m_flTime;

			int id = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(id);

			g_WhipSwordSlashEffect = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
			if (g_WhipSwordSlashEffect)
			{
				g_WhipSwordSlashEffect->entity.angles = pPlayer->angles;
				g_WhipSwordSlashEffect->entity.angles.z = 0;
				g_WhipSwordSlashEffect->entity.curstate.sequence = 0;
				g_WhipSwordSlashEffect->entity.curstate.animtime = gHUD.m_flTime;
				g_WhipSwordSlashEffect->entity.curstate.frame = 0;
				g_WhipSwordSlashEffect->entity.curstate.framerate = 1.0;
				//g_WhipSwordSlashEffect->entity.curstate.effects |= EF_NOCULL;
				g_WhipSwordSlashEffect->tentOffset = Vector(0, 0, 12);
				g_WhipSwordSlashEffect->die = gHUD.m_flTime + 5.0;
				g_WhipSwordSlashEffect->clientIndex = idx;
				g_WhipSwordSlashEffect->entity.curstate.rendermode = kRenderTransAdd;
				g_WhipSwordSlashEffect->entity.curstate.iuser1 = id;
				g_WhipSwordSlashEffect->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM;
			}

			g_WhipSwordSlashEffect->callback = [](tempent_s* pEnt, float frametime, float currenttime)
			{
				cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(g_WhipSwordSlashEffect->entity.curstate.iuser1);
				pEnt->entity.angles = pPlayer->angles;
				pEnt->entity.angles.z = 0;
			};
		}

		break;
	}
	case 6:
	{
		if (g_WhipSwordStabEffect)
			g_WhipSwordStabEffect->die = gHUD.m_flTime;
		break;
	}
	}
		
}

}
