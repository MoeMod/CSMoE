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

namespace cl {

enum desperado_e
{
	ANIM_IDLE_M = 0,
	ANIM_RUN_START_M,
	ANIM_RUN_IDLE_M,
	ANIM_RUN_END_M,
	ANIM_DRAW_M,
	ANIM_SHOOT_M,
	ANIM_RELOAD_M,
	ANIM_SWAB_M,	// Gold
	ANIM_IDLE_W,
	ANIM_RUN_START_W,
	ANIM_RUN_IDLE_W,
	ANIM_RUN_END_W,
	ANIM_DRAW_W,
	ANIM_SHOOT_W,
	ANIM_RELOAD_W,
	ANIM_SWAB_W	// Blue
};

void EV_FireDesperado( event_args_t *args )
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;

	bool empty		 = !args->bparam1;
	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity( args->velocity );
	Vector forward, right, up;
	cl_entity_t *ent = gEngfuncs.GetViewModel();
	AngleVectors( angles, forward, right, up );

	/*if (effect & (1 << EVENT_BUFFAKM4HIT))
	{
		HudAmmo().HitForBuff(cl.time);
	}
	*/
	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		
		EV_MuzzleFlash();

		if (!args->bparam2)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_M, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_W, 2);

		if (gHUD.cl_gunsmoke->value)
		{
			if (ent)
			{
				EV_CS16Client_CreateSmoke(SMOKE_BLACK, ent->attachment[0], forward, 0, 0.25, 10, 10, 10, false, velocity);
				EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[0], forward, 25, 0.3, 15, 15, 15, false, velocity);
				EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[0], forward, 50, 0.2, 25, 25, 25, false, velocity);
			}
		}

		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, 1);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, 1);
		}

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 0.75, ShellVelocity);
		ShellVelocity[2] += 25;
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}


	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iPShell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/dprd-1.wav", 1.0, 0.6, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_50AE,
		2 );
}

}
