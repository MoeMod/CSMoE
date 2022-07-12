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

namespace cl::event::kronos12 {

enum kronos12_e
{
	ANIM_IDLE,
	ANIM_IDLE2,
	ANIM_IDLE3,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3,
	ANIM_RELOAD,
	ANIM_RELOAD2,
	ANIM_DRAW,
	ANIM_DRAW2,
	ANIM_SCAN_ACTIVATE,
	ANIM_ZOOM,
	ANIM_SCAN_EACTIVATE,
};

void EV_FireKronos12(event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t* ent = gEngfuncs.GetViewModel();
	int righthand;
	vec3_t vSpread;

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
		EV_MuzzleFlash();

		if (args->bparam2)
		{
			if(args->bparam1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT3, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT2, 2);
		}
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT1, 2);
		}

		if (gHUD.cl_gunsmoke->value)
		{
			if (ent)
			{
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.3, 35, 35, 35, 4, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 35, 0.35, 30, 30, 30, 2, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 70, 0.3, 25, 25, 25, 2, velocity, false, 35);
			}
		}
	}

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;

		float vecScale[3] = { 22.0, -9.0, -11.0 };

		if (righthand == 0)
			vecScale[2] = 0 - vecScale[2];

		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

		VectorCopy(ent->attachment[1], ShellOrigin);
		VectorScale(ShellVelocity, 1.25, ShellVelocity);
		ShellVelocity[2] -= 50;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iShotgunShell, TE_BOUNCE_SHOTSHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/kronos12-1.wav", 1.0, 0.52, 0, 94 + Com_RandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam2;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		args->bparam2, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
		1 );
}

}
