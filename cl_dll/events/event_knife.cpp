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

TEMPENTITY* g_DeathKnightChargeEffect = NULL;

namespace cl::event::knife {

enum knife_e
{
	KNIFE_IDLE1 = 0,
	KNIFE_SLASH1,
	KNIFE_SLASH2,
	KNIFE_DRAW,
	KNIFE_STAB,
	KNIFE_STAB_MISS,
	KNIFE_MIDSLASH1,
	KNIFE_MIDSLASH2
};

static const char *SOUNDS_NAME = "weapons/knife_miss1.wav";

void ResetChargeModel(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);
	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = gHUD.m_flTime;
	}
}

void EV_Knife( struct event_args_s *args )
{
	int    idx = args->entindex;
	Vector origin( args->origin );

	if (!args->iparam2)
	{
		int index = args->iparam1 - 2;
		char pModelName[64];
		if (index == 0)
		{
			if (g_DeathKnightChargeEffect)
			{
				g_DeathKnightChargeEffect->die = gHUD.m_flTime;
			}

			return;
		}

		sprintf(pModelName, "sprites/ef_deathknight_charge%i.spr", index);

		model_s* pModel = IEngineStudio.Mod_ForName(pModelName, false);
		if (pModel)
		{
			cl_entity_t* viewent = gEngfuncs.GetViewModel();

			if (g_DeathKnightChargeEffect)
			{
				g_DeathKnightChargeEffect->die = gHUD.m_flTime;
			}

			if (!viewent)
				return;

			int clientindex = viewent->index;
			cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

			if (!parent)
				return;

			g_DeathKnightChargeEffect = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[0], pModel);

			if (g_DeathKnightChargeEffect)
			{

				g_DeathKnightChargeEffect->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				g_DeathKnightChargeEffect->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
				g_DeathKnightChargeEffect->entity.curstate.frame = 0.0f;
				g_DeathKnightChargeEffect->entity.curstate.framerate = 30.0f;
				g_DeathKnightChargeEffect->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_DeathKnightChargeEffect->entity.curstate.aiment = clientindex;
				g_DeathKnightChargeEffect->entity.curstate.solid = SOLID_NOT;
				g_DeathKnightChargeEffect->entity.curstate.rendermode = kRenderTransAdd;
				g_DeathKnightChargeEffect->entity.curstate.renderfx = kRenderFxNone;
				g_DeathKnightChargeEffect->frameMax = 30.0;
				g_DeathKnightChargeEffect->entity.curstate.scale = 0.07f;
				g_DeathKnightChargeEffect->entity.curstate.renderamt = 255;
				g_DeathKnightChargeEffect->entity.curstate.body = 1;
				g_DeathKnightChargeEffect->die = gHUD.m_flTime + 999;
				g_DeathKnightChargeEffect->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
				g_DeathKnightChargeEffect->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				g_DeathKnightChargeEffect->callback = ResetChargeModel;
			}

		}
		return;
	}

	if( EV_IsLocal( idx ))
		gEngfuncs.pEventAPI->EV_WeaponAnimation( args->iparam1, 2 );

	//Play Swing sound
	PLAY_EVENT_SOUND( SOUNDS_NAME );
}

}
