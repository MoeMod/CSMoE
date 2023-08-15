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

namespace cl::event::balrog7 {

enum balrog7_e
{
	BALROG7_IDLE1,
	BALROG7_SHOOT1,
	BALROG7_SHOOT2,
	BALROG7_SHOOT3,
	BALROG7_RELOAD,
	BALROG7_DRAW
};

static const char *SOUNDS_NAME[] =
{
	 "weapons/balrog7-1.wav", "weapons/balrog7-exp.wav"
};

void EV_FireBalrog7(event_args_s *args)
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;

	int    idx = args->entindex;
	
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );
	Vector origin(args->origin);

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;
		if (args->bparam2)
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BALROG7_SHOOT3, 2);
		}
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(BALROG7_SHOOT1, BALROG7_SHOOT2), 2);
		PLAY_EVENT_SOUND(SOUNDS_NAME[0]);
		EV_MuzzleFlash();
		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, -13.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, 13.0, 0);
		}
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}
	

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_556MM,
		2 );

	if (args->bparam2)
	{
		pmtrace_t tr;
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + vecAiming * 8192, PM_NORMAL, -1, &tr);

		Vector VecOrigin = tr.endpos;

		struct model_s* pModel;
		TEMPENTITY* pEnt;

		pModel = IEngineStudio.Mod_ForName("sprites/balrogcritical.spr", 0);
		
		pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(VecOrigin, pModel);

		if (pEnt)
		{
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
		}
		

		int id;
		gEngfuncs.CL_LoadModel("sprites/fexplo.spr", &id);
		VecOrigin[2] -= 15.0;
		gEngfuncs.pEfxAPI->R_Explosion(VecOrigin, id, 0.45, 30, TE_EXPLFLAG_NONE);

		//gEngfuncs.pEfxAPI->R_TracerEffect(gEngfuncs.GetEntityByIndex(idx)->origin, origin);
	}
}

}
