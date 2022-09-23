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

enum skull2_e
{
	SKULL2_IDLE1,
	SKULL2_RELOAD1,
	SKULL2_DRAW1,
	SKULL2_SHOOT1,
	SKULL2_SHOOT1_EMPTY,
	SKULL2_IDLE2,
	SKULL2_IDLE2_LEFTEMPTY,
	SKULL2_SHOOT_LEFT1,
	SKULL2_SHOOT_LEFT2,
	SKULL2_SHOOT_LEFTLAST,
	SKULL2_SHOOT_RIGHT1,
	SKULL2_SHOOT_RIGHT2,
	SKULL2_SHOOT_RIGHTLAST,
	SKULL2_RELOAD2,
	SKULL2_DRAW2,
	SKULL2_DRAW3,
	SKULL2_DRAW4,
};

namespace cl::event::skull2_left {
void EV_FireSKULL2Left(event_args_s *args)
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
	vec3_t vSpread;

	ent = gEngfuncs.GetViewModel();
	idx = args->entindex;

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

		if (args->bparam2 > 1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(SKULL2_SHOOT_LEFT1 + Com_RandomLong(0, 1), 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(SKULL2_SHOOT_LEFTLAST, 2);

		if (gHUD.cl_gunsmoke->value)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		float vecScale[3] = { 20.0, -8.0, -10.0 };

		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

		VectorCopy(ent->attachment[2], ShellOrigin);
		ShellVelocity[2] -= 45;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iPShell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/skull2-1.wav", 1.0, 0.52, 0, 94 + Com_RandomLong(0, 15));

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
}

}
namespace cl::event::skull2_right {
	void EV_FireSKULL2Right(event_args_s* args)
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
		vec3_t vSpread;

		ent = gEngfuncs.GetViewModel();
		idx = args->entindex;

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

			if (args->bparam2 > 1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(SKULL2_SHOOT_RIGHT1 + Com_RandomLong(0, 1), 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(SKULL2_SHOOT_RIGHTLAST, 2);

			if (gHUD.cl_gunsmoke->value)
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
		}

		if (EV_IsLocal(idx))
		{
			float vecScale[3] = { 20.0, -8.0, 10.0 };

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

			VectorCopy(ent->attachment[3], ShellOrigin);
			ShellVelocity[2] -= 45;
		}
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iPShell, TE_BOUNCE_SHELL);

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/skull2-1.wav", 1.0, 0.52, 0, 94 + Com_RandomLong(0, 15));

		EV_GetGunPosition(args, vecSrc, origin);

		VectorCopy(forward, vecAiming);

		vSpread[0] = args->fparam1;
		vSpread[1] = args->fparam2;
		vSpread[2] = 0;

		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
	}

}
namespace cl::event::skull2 {
	void EV_FireSKULL2(event_args_s* args)
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
		vec3_t vSpread;

		ent = gEngfuncs.GetViewModel();
		idx = args->entindex;

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

			if (args->bparam1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(SKULL2_SHOOT1, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(SKULL2_SHOOT1_EMPTY, 2);

			if (gHUD.cl_gunsmoke->value)
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
		}

		if (EV_IsLocal(idx))
		{
			float vecScale[3] = { 20.0, -8.0, 10.0 };

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

			VectorCopy(ent->attachment[3], ShellOrigin);
			ShellVelocity[2] -= 45;
		}
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iPShell, TE_BOUNCE_SHELL);

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/skull2-1.wav", 1.0, 0.52, 0, 94 + Com_RandomLong(0, 15));

		EV_GetGunPosition(args, vecSrc, origin);

		VectorCopy(forward, vecAiming);

		vSpread[0] = args->fparam1;
		vSpread[1] = args->fparam2;
		vSpread[2] = 0;

		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
	}

}
