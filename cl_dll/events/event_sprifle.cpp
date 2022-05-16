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

namespace cl::event::sprifle {

enum sg550_e
{
	SG550_IDLE,
	SG550_SHOOT,
	SG550_SHOOT2,
	SG550_RELOAD,
	SG550_DRAW
};

void EV_FireSprifle( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t* ent;
	int righthand;
	vec3_t vSpread;

	ent = gEngfuncs.GetViewModel();
	idx = args->entindex;
	righthand = gHUD.cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();


		gEngfuncs.pEventAPI->EV_WeaponAnimation(SG550_SHOOT + Com_RandomLong(0, 1), 2);

		if (gHUD.cl_gunsmoke->value)
		{
			if (ent)
			{
				EV_CS16Client_CreateSmoke(SMOKE_WALLPUFF, ent->attachment[0], forward, 35, 0.35, 30, 30, 30, false, velocity);
				EV_CS16Client_CreateSmoke(SMOKE_WALLPUFF, ent->attachment[0], forward, 70, 0.3, 25, 25, 25, false, velocity);
				EV_CS16Client_CreateSmoke(SMOKE_RIFLE, ent->attachment[0], forward, 3, 0.3, 35, 35, 35, false, velocity);
			}
		}

		EV_CS16Client_CreateSmoke(SMOKE_WALLPUFF, ent->attachment[1], up, 0, 0.15, 55, 50, 50, false, velocity + up * 50, 30);
		EV_CS16Client_CreateSmoke(SMOKE_WALLPUFF, ent->attachment[1], up, 0, 0.2, 25, 20, 20, false, velocity + up * 50, 25);
		EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[1], up, 0, 0.4, 30, 25, 25, false, velocity + up * 50, 25);
		EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[1], up, 0, 0.15, 30, 25, 25, false, velocity + up * 80, 35);
		EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[1], up, 0, 0.3, 65, 65, 65, false, velocity + up * 80, 35);
		EV_CS16Client_CreateSmoke(SMOKE_WALLPUFF, ent->attachment[1], up, 0, 0.45, 20, 15, 15, false, velocity + up * 80, 25);
		EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[1], up, 0, 0.1, 50, 50, 50, false, velocity, 30);
		EV_CS16Client_CreateSmoke(SMOKE_WALLPUFF, ent->attachment[1], up, 0, 0.12, 35, 30, 35, false, velocity, 30);
		EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[1], up, 0, 0.15, 30, 30, 25, false, velocity, 30);
		EV_CS16Client_CreateSmoke(SMOKE_WALLPUFF, ent->attachment[1], up, 0, 0.18, 35, 40, 40, false, velocity, 30);
	}

	if (EV_IsLocal(idx))
	{
		float vecScale[3] = { 20.0, -8.0, -10.0 };

		if (righthand == 0)
		{
			vecScale[0] -= 3.0;
			vecScale[2] = 0 - vecScale[2];
		}

		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.5, ShellVelocity);
		ShellVelocity[2] -= 50.0;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iRShell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sprifle-1.wav", 1.0, 0.4, 0, 94 + Com_RandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_338MAG,
		2 );
}

}
