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

namespace cl::event::balrog11 {

enum xm1014_e
{
	BALROG11_IDLE,
	BALROG11_SHOOT1,
	BALROG11_SHOOT2,
	BALROG11_SHOOTBCS,
	BALROG11_INSERT,
	BALROG11_AFTER_RELOAD,
	BALROG11_START_RELOAD,
	BALROG11_DRAW
};

void EV_FireBalrog11(event_args_s *args)
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
	int shell;
	cl_entity_t* ent;

	AngleVectors( angles, forward, right, up );

	if (args->bparam1)
		shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell_bcs.mdl");
	else
		shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl");

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
	/*	if (args->bparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BALROG11_SHOOTBCS, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BALROG11_SHOOT1 + Com_RandomLong(0, 1), 2);*/

		if (!args->bparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BALROG11_SHOOT1 + Com_RandomLong(0, 1), 2);

		float vecScale[3] = { 22.0, -9.0, -11.0 };

		if (!gHUD.cl_righthand->value)
			vecScale[2] = 0 - vecScale[2];

		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

		ent = gEngfuncs.GetViewModel();
		if (ent)
		{
			ShellOrigin = ent->attachment[1];
			ShellVelocity *= 1.25;
			ShellVelocity[2] -= 50;
		}
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], shell, TE_BOUNCE_SHOTSHELL);

	if (args->bparam1)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/balrog11-2.wav", args->fparam1, 0.52, 0, 94 + Com_RandomLong(0, 0xf));

	EV_GetGunPosition( args, vecSrc, origin );

	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = 0.0735;
	vSpread.y = 0.0735;
	if (!args->bparam1)
	{
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			6, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
			1);
	}
}

}
