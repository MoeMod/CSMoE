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

namespace cl::event::m3dragon {

enum m3_e
{
	M3DRAGON_IDLE,
	M3DRAGON_FIRE1,
	M3DRAGON_FIRE2,
	M3DRAGON_INSERT,
	M3DRAGON_AFTER_RELOAD,
	M3DRAGON_BEFORE_RELOAD,
	M3DRAGON_DRAW,
	M3DRAGON_SECONDARY_READY,
	M3DRAGON_SECONDARY_SHOOT,
	M3DRAGON_SECONDARY_INSERT,
	M3DRAGON_SECONDARY_AFTER_RELOAD,
	M3DRAGON_SECONDARY_BEFORE_RELOAD,
	M3DRAGON_SECONDARY_DRAW,
};

TEMPENTITY* g_M3DragonFlameLeft = NULL;
TEMPENTITY* g_M3DragonFlameRight = NULL;

static const char* SOUNDS_NAME[] =
{
	"weapons/m3dragon-1_1.wav",
	"weapons/m3dragon-1_2.wav",
};

void EV_FireM3Dragon( event_args_t *args )
{
	vec3_t vecSrc, vecAiming;
	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector forward, right, up;
	int righthand = gHUD.cl_righthand->value;
	cl_entity_t* viewent = gEngfuncs.GetViewModel();
	int iAnim;

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		if (args->iparam1)
		{
			if (args->bparam2)
			{
				model_s* pModel;

				pModel = IEngineStudio.Mod_ForName("sprites/m3dragon_flame.spr", false);
				if (pModel)
				{
					if (g_M3DragonFlameLeft)
					{
						g_M3DragonFlameLeft->die = gHUD.m_flTime;
						g_M3DragonFlameLeft = nullptr;
					}

					if (!viewent)
						return;

					int clientindex = viewent->index;
					cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

					if (!parent)
						return;

					int iAttachment = righthand ? 2 : 3;
					g_M3DragonFlameLeft = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], pModel);

					if (!g_M3DragonFlameLeft)
						return;

					g_M3DragonFlameLeft->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
					g_M3DragonFlameLeft->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
					g_M3DragonFlameLeft->entity.curstate.frame = 0.0f;
					g_M3DragonFlameLeft->entity.curstate.framerate = 30.0f;
					g_M3DragonFlameLeft->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_M3DragonFlameLeft->entity.curstate.aiment = clientindex;
					g_M3DragonFlameLeft->entity.curstate.solid = SOLID_NOT;
					g_M3DragonFlameLeft->entity.curstate.rendermode = kRenderTransAdd;
					g_M3DragonFlameLeft->entity.curstate.renderfx = kRenderFxNone;
					g_M3DragonFlameLeft->frameMax = 20.0;
					g_M3DragonFlameLeft->entity.curstate.scale = 0.08f;
					g_M3DragonFlameLeft->entity.curstate.renderamt = 255;
					g_M3DragonFlameLeft->entity.curstate.body = iAttachment + 1;
					g_M3DragonFlameLeft->die = gHUD.m_flTime + 999;
					g_M3DragonFlameLeft->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
					g_M3DragonFlameLeft->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

					g_M3DragonFlameLeft->callback = [](tempent_s* pEnt, float frametime, float currenttime)
					{
						cl_entity_t* parent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.aiment);
						if (pEnt->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
						{
							pEnt->entity.curstate.renderamt = 0;
							pEnt->die = gHUD.m_flTime;
							g_M3DragonFlameLeft = nullptr;
						}
					};
				}

				//right
				pModel = IEngineStudio.Mod_ForName("sprites/m3dragon_flame2.spr", false);
				if (pModel)
				{
					if (g_M3DragonFlameRight)
					{
						g_M3DragonFlameRight->die = gHUD.m_flTime;
						g_M3DragonFlameRight = nullptr;
					}

					if (!viewent)
						return;

					int clientindex = viewent->index;
					cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

					if (!parent)
						return;

					int iAttachment = righthand ? 3 : 2;

					g_M3DragonFlameRight = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], pModel);

					if (!g_M3DragonFlameRight)
						return;

					g_M3DragonFlameRight->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
					g_M3DragonFlameRight->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
					g_M3DragonFlameRight->entity.curstate.frame = 0.0f;
					g_M3DragonFlameRight->entity.curstate.framerate = 30.0f;
					g_M3DragonFlameRight->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_M3DragonFlameRight->entity.curstate.aiment = clientindex;
					g_M3DragonFlameRight->entity.curstate.solid = SOLID_NOT;
					g_M3DragonFlameRight->entity.curstate.rendermode = kRenderTransAdd;
					g_M3DragonFlameRight->entity.curstate.renderfx = kRenderFxNone;
					g_M3DragonFlameRight->frameMax = 20.0;
					g_M3DragonFlameRight->entity.curstate.scale = 0.08f;
					g_M3DragonFlameRight->entity.curstate.renderamt = 255;
					g_M3DragonFlameRight->entity.curstate.body = iAttachment + 1;
					g_M3DragonFlameRight->die = gHUD.m_flTime + 999;
					g_M3DragonFlameRight->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
					g_M3DragonFlameRight->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;

					g_M3DragonFlameRight->callback = [](tempent_s* pEnt, float frametime, float currenttime)
					{
						cl_entity_t* parent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.aiment);
						if (pEnt->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
						{
							pEnt->entity.curstate.renderamt = 0;
							pEnt->die = gHUD.m_flTime;
							g_M3DragonFlameRight = nullptr;
						}
					};
				}
				return;
			}
			else
			{
				if (g_M3DragonFlameLeft)
				{
					g_M3DragonFlameLeft->die = gHUD.m_flTime;
					g_M3DragonFlameLeft = nullptr;
				}

				if (g_M3DragonFlameRight)
				{
					g_M3DragonFlameRight->die = gHUD.m_flTime;
					g_M3DragonFlameRight = nullptr;
				}
				return;
			}
		}

		++g_iShotsFired;
		EV_MuzzleFlash();
		iAnim = Com_RandomLong(M3DRAGON_FIRE1, M3DRAGON_FIRE2);
		PLAY_EVENT_SOUND(SOUNDS_NAME[iAnim - 1]);
		gEngfuncs.pEventAPI->EV_WeaponAnimation(iAnim, 2);
	}
	else
		PLAY_EVENT_SOUND(SOUNDS_NAME[Com_RandomLong(0, 1)]);

	

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = 0.0725;
	vSpread.y = 0.0725;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		10, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
		1 );
}

}
