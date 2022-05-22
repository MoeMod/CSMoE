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

namespace cl::event::spsmg {

void EV_FireSPSMG( event_args_t *args )
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;
	int    idx = args->entindex;
	Vector origin(args->origin);
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
	);
	Vector velocity(args->velocity);
	Vector forward, right, up;
	Vector vecSpread;
	cl_entity_t* ent;

	ent = gEngfuncs.GetViewModel();
	int righthand = gHUD.cl_righthand->value;
	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	if (args->bparam1)
	{
		if (EV_IsLocal(args->entindex))
		{
			++g_iShotsFired;
			gEngfuncs.pEventAPI->EV_WeaponAnimation(4, 5);
			EV_MuzzleFlash();

			if (gHUD.cl_gunsmoke->value)
			{
				if (ent)
				{
					EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.45, 15, 15, 15, 3, velocity, false, 35);
					EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.35, 9, 9, 9, 2, velocity, false, 35);
				}
			}	
			
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
		vecSpread = Vector(args->fparam2, args->fparam2, 0);

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/spsmg-2.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 0xf));
	}
	else
	{
		if (EV_IsLocal(args->entindex))
		{
			++g_iShotsFired;
			gEngfuncs.pEventAPI->EV_WeaponAnimation(3, 2);
			EV_MuzzleFlash();
			
			if (gHUD.cl_gunsmoke->value)
			{
				EV_CS16Client_CreateSmoke(SMOKE_RIFLE, ent->attachment[0], forward, 3, 0.35, 20, 20, 20, false, velocity);
			}

			if (righthand)
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, 0);
			else
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, 0);

			ShellVelocity.z -= 75;
			ShellOrigin = ent->attachment[1];

		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
		}
		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iRShell, TE_BOUNCE_SHELL);

		vecSpread = Vector(args->fparam1, args->fparam2, 0);

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/spsmg-1.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 0xf));
	}

	
	int shots = args->bparam2;

	shots = min(shots, 5);

	Vector vSpread;

	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets(idx,
		forward, right, up,
		shots, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_MP5,
		2);
}

}
