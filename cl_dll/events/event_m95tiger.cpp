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

namespace cl::event::m95tiger {

enum m95tiger_e
{
	M95TIGER_IDLE,
	M95TIGER_SHOOT,
	M95TIGER_RELOADA,
	M95TIGER_DRAW,
	M95TIGER_RELOADB,
	M95TIGER_SHOOTNET,
};

static const char* SOUNDS_NAME[2] =
{
	"weapons/m95tiger-1.wav",
	"weapons/m95tiger-2.wav",
};

//TEMPENTITY* g_M95TigerEye = NULL;

void EV_FireM95Tiger( event_args_t *args )
{
	vec3_t vecSrc, vecAiming;
	vec3_t ShellVelocity, ShellOrigin;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector forward, right, up;
	Vector velocity(args->velocity);
	cl_entity_t* viewent;
	Vector vecBeamStart;
	Vector vecBeamEnd;

	viewent = gEngfuncs.GetViewModel();

	AngleVectors( angles, forward, right, up );
	EV_GetGunPosition(args, vecSrc, origin);

	if (EV_IsLocal(idx))
	{
		vecBeamStart = viewent->attachment[0];
#if 0
		if (args->bparam1)
		{
			model_s* pModel;
			char pModelName[32];
			int index = args->iparam1;

			sprintf(pModelName, "sprites/ef_m95tiger_eye%i.spr", index);

			pModel = IEngineStudio.Mod_ForName(pModelName, false);
			if (pModel)
			{
				if (g_M95TigerEye)
				{
					g_M95TigerEye->die = gHUD.m_flTime;
					g_M95TigerEye = nullptr;
				}

				if (!viewent)
					return;

				int clientindex = viewent->index;
				cl_entity_t* parent = gEngfuncs.GetEntityByIndex(clientindex);

				if (!parent)
					return;

				g_M95TigerEye = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[3], pModel);

				if (!g_M95TigerEye)
					return;

				g_M95TigerEye->flags &= ~(FTENT_COLLIDEWORLD | FTENT_GRAVITY);
				g_M95TigerEye->flags |= FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP;
				g_M95TigerEye->entity.curstate.frame = 0.0f;
				g_M95TigerEye->entity.curstate.framerate = 30.0f;
				g_M95TigerEye->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_M95TigerEye->entity.curstate.aiment = clientindex;
				g_M95TigerEye->entity.curstate.solid = SOLID_NOT;
				g_M95TigerEye->entity.curstate.rendermode = kRenderTransAdd;
				g_M95TigerEye->entity.curstate.renderfx = kRenderFxNone;
				g_M95TigerEye->frameMax = 15.0;
				g_M95TigerEye->entity.curstate.scale = 0.03f;
				g_M95TigerEye->entity.curstate.renderamt = 255;
				g_M95TigerEye->entity.curstate.body = 4;
				g_M95TigerEye->die = gHUD.m_flTime + 999;
				g_M95TigerEye->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
				g_M95TigerEye->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL;

				g_M95TigerEye->callback = [](tempent_s* pEnt, float frametime, float currenttime)
				{
					cl_entity_t* parent = gEngfuncs.GetEntityByIndex(pEnt->entity.curstate.aiment);
					if (gHUD.m_iFOV != 90)
						pEnt->entity.curstate.renderamt = 0;
					else
						pEnt->entity.curstate.renderamt = 255;
					if (pEnt->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
					{
						pEnt->entity.curstate.renderamt = 0;
						pEnt->die = gHUD.m_flTime;
						g_M95TigerEye = nullptr;
					}
				};
			}
			return;
		}
#endif
		if (args->bparam2)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M95TIGER_SHOOTNET, 2);
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M95TIGER_SHOOT, 2);
			++g_iShotsFired;
			EV_MuzzleFlash();
			if (!gHUD.cl_righthand->value)
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, 0);
			}
			else
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, 0);
			}

			if (gHUD.cl_gunsmoke->value)
			{
				EV_CS16Client_CreateSmoke(SMOKE_RIFLE, viewent->attachment[0], forward, 3, 0.3, 20, 20, 20, false, velocity);
			}
			VectorCopy(viewent->attachment[1], ShellOrigin);
		}
					
		vecBeamEnd = vecBeamStart + (forward + right * args->fparam1 + up * args->fparam2) * 8192;
	}
	else
	{
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);

		if (args->ducking)
		{
			vecBeamStart = vecSrc + forward * 40 - up * 3;
		}
		else
		{
			vecBeamStart = vecSrc + forward * 40 - up * 9;
		}

		vecBeamEnd = vecBeamStart + forward * 8192;
	}

	if (args->bparam2)
	{
		PLAY_EVENT_SOUND(SOUNDS_NAME[1]);
		return;
	}		
	else
		PLAY_EVENT_SOUND(SOUNDS_NAME[0]);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iRShell, TE_BOUNCE_SHELL);

	int iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");

	BEAM* pBeam;

	pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(vecBeamStart, vecBeamEnd,
		iModel,
		0.5,
		2.5,
		0.0,
		500.0,
		0.0,
		0,
		0,
		1.0,
		0.7,
		0.0
	);

	if (pBeam)
		pBeam->flags |= FBEAM_FADEIN | FBEAM_FADEOUT | FBEAM_SHADEOUT;

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_338MAG,
		9 );
}

}
