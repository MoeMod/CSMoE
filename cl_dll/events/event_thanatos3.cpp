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

namespace cl::event::thanatos3 {

enum thanatos3_e
{
	ANIM_IDLE,
	ANIM_IDLE_W1,
	ANIM_IDLE_W2,
	ANIM_IDLE_W3,
	ANIM_SHOOT,
	ANIM_SHOOT_W1,
	ANIM_SHOOT_W2,
	ANIM_SHOOT_W3,
	ANIM_FLY_W1,
	ANIM_FLY_W2,
	ANIM_FLY_W3,
	ANIM_RELOAD,
	ANIM_RELOAD_W1,
	ANIM_RELOAD_W2,
	ANIM_RELOAD_W3,
	ANIM_SPREAD_W1,
	ANIM_SPREAD_W2,
	ANIM_SPREAD_W3,
	ANIM_DRAW,
	ANIM_DRAW_W1,
	ANIM_DRAW_W2,
	ANIM_DRAW_W3,
};

void EV_FireThanatos3(event_args_s *args)
{
	vec3_t ShellVelocity, ShellOrigin;
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
	int iWeaponState = args->bparam1;

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;

		if (args->bparam2)
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SPREAD_W1 + iWeaponState, 2);
		}
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT + iWeaponState, 2);
		}
		

		EV_MuzzleFlash();
		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, -11.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, 11.0, 0);
		}
		ShellVelocity[2] -= 50;

		VectorCopy(ent->attachment[1], ShellOrigin);
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iPShell, TE_BOUNCE_SHELL);

	if (args->bparam2)
	{
		switch (iWeaponState)
		{
		case 1: PLAY_EVENT_SOUND("weapons/thanatos3_spread_w1.wav"); break;
		case 2: PLAY_EVENT_SOUND("weapons/thanatos3_spread_w2.wav"); break;
		case 3: PLAY_EVENT_SOUND("weapons/thanatos3_spread_w3.wav"); break;	
		}
	}
	else
	{
		PLAY_EVENT_SOUND("weapons/thanatos3-1.wav");
	}
	

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_45ACP,
		1 );
}

}
