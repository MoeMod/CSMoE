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

namespace cl::event::spsg {

enum qbarrel_e
{
	ANIM_IDLE,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_FIRE1,
	ANIM_FIRE2,
	
};

void EV_FireSPSG(event_args_s *args)
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;
	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity( args->velocity );
	Vector forward, right, up;
	cl_entity_t* ent;

	ent = gEngfuncs.GetViewModel();

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(args->iparam1 == 2 ? ANIM_FIRE2 : ANIM_FIRE1, 2);

		if (gHUD.cl_gunsmoke->value)
		{
			if (ent)
			{
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.45, 15, 15, 15, 3, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.35, 9, 9, 9, 2, velocity, false, 35);
			}
		}

		if (args->iparam1 == 2)
		{
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.2, 55, 50, 50, 2, velocity + up * 50, false, 30);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.3, 25, 20, 20, 2, velocity + up * 50, false, 25);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.4, 30, 25, 25, 3, velocity + up * 50, false, 25);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.5, 30, 25, 25, 3, velocity + up * 80, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.4, 65, 65, 65, 3, velocity + up * 80, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.4, 20, 15, 15, 2, velocity + up * 80, false, 25);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.1, 50, 50, 50, 3, velocity, false, 30);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.12, 35, 30, 35, 2, velocity, false, 30);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.15, 30, 30, 25, 3, velocity, false, 30);
			EV_HLDM_CreateSmoke(ent->attachment[2], up, 0, 0.18, 35, 40, 40, 2, velocity, false, 30);
		}

		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, -11.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 22.0, -9.0, 11.0, 0);
		}
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iShotgunShell, TE_BOUNCE_SHELL);

	if (args->iparam1 == 2)
		PLAY_EVENT_SOUND("weapons/spsg-2.wav");
	else
		PLAY_EVENT_SOUND( "weapons/spsg-1.wav" );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam1;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		args->iparam2, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
		1 );
}

}
