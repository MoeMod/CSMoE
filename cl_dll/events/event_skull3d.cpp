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

enum skull3d_e
{
	ANIM_IDLE1,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3,
	ANIM_IDLE,
	ANIM_IDLE_LEFTEMPTY,
	ANIM_SHOOT_LEFT1,
	ANIM_SHOOT_LEFT2,
	ANIM_SHOOT_LEFTLAST,
	ANIM_SHOOT_RIGHT1,
	ANIM_SHOOT_RIGHT2,
	ANIM_SHOOT_RIGHTLAST,
	ANIM_RELOAD2,
	ANIM_DRAW2,
	ANIM_DRAW3,
	ANIM_DRAW4,
};

namespace cl::event::skull3d_left {
void EV_FireDualSKULL3DLeft(event_args_s *args)
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
	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		if (args->bparam2 > 1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_LEFT1 + Com_RandomLong(0, 1), 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_LEFTLAST, 2);

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

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND("weapons/skull3-1.wav");

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
}

}
namespace cl::event::skull3d_right {
	void EV_FireDualSKULL3DRight(event_args_s* args)
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
		shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/pshell.mdl");

		if (EV_IsLocal(idx))
		{
			g_iShotsFired++;
			EV_MuzzleFlash();

			if (args->bparam2 > 1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_RIGHT1 + Com_RandomLong(0, 1), 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_RIGHTLAST, 2);

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

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

		PLAY_EVENT_SOUND("weapons/skull3-1.wav");

		EV_GetGunPosition(args, vecSrc, origin);

		VectorCopy(forward, vecAiming);

		vSpread[0] = args->fparam1;
		vSpread[1] = args->fparam2;
		vSpread[2] = 0;

		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_556MM, 0, &tracerCount[idx - 1], 2);
	}
}

