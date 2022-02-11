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

namespace cl::event::revivegun {
	
enum revivegun_e
{
	REVIVEGUN_IDLE,
	REVIVEGUN_RELOAD,
	REVIVEGUN_DRAW,
	REVIVEGUN_SHOOTA,
	REVIVEGUN_SHOOTB_START,
	REVIVEGUN_SHOOTB,
	REVIVEGUN_REVIVE_IDLE,
	REVIVEGUN_REVIVE_RELOAD,
	REVIVEGUN_REVIVE_DRAW,
	REVIVEGUN_REVIVE_SHOOTA,
	REVIVEGUN_REVIVE_SHOOTB_START,
	REVIVEGUN_REVIVE_SHOOTB,
	REVIVEGUN_REVIVE
};

void EV_FireReviveGun( event_args_t *args )
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2] );
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );

	cl_entity_t* viewent = gEngfuncs.GetViewModel();
	if (!viewent)
		return;

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;
		if (args->bparam1)
		{
			if (args->bparam2)
			{
				//viewent->curstate.skin = 2;
				gEngfuncs.pEventAPI->EV_WeaponAnimation(REVIVEGUN_REVIVE_SHOOTB, 2);
				return;
			}
			else
			{
				//viewent->curstate.skin = 0;
				gEngfuncs.pEventAPI->EV_WeaponAnimation(REVIVEGUN_REVIVE_SHOOTA, 2);
			}
		}
		else
		{
			if (args->bparam2)
			{
				//viewent->curstate.skin = 2;
				gEngfuncs.pEventAPI->EV_WeaponAnimation(REVIVEGUN_SHOOTB, 2);
				return;
			}
			else
			{
				//viewent->curstate.skin = 0;
				gEngfuncs.pEventAPI->EV_WeaponAnimation(REVIVEGUN_SHOOTA, 2);
			}
		}
			
		EV_MuzzleFlash();
		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, 0);
		}
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}


	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	PLAY_EVENT_SOUND("weapons/revivegun-1.wav");

	Vector vSpread( args->fparam1, args->fparam2, 0.0f );
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_762MM,
		2 );
}

}
