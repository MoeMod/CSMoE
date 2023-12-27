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

enum deagleel_e
{
	ANIM_IDLE1,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_RUN,
	ANIM_STAB,
	ANIM_CHANGEB,
	ANIM_SHOOTB_R1,
	ANIM_SHOOTB_R2,
	ANIM_SHOOTB_R3,
	ANIM_SHOOTB_R4,
	ANIM_SHOOTB_L1,
	ANIM_SHOOTB_L2,
	ANIM_SHOOTB_L3,
	ANIM_SHOOTB_L4,
};


namespace cl::event::deagleel {

	void EV_FireDeagleEL(event_args_t* args)
	{
		vec3_t ShellVelocity;
		vec3_t ShellOrigin;
		vec3_t vecSrc;

		int idx = args->entindex;
		Vector origin(args->origin);
		Vector velocity(args->velocity);
		Vector angles(
			args->iparam1 / 100.0f + args->angles[0],
			args->iparam2 / 100.0f + args->angles[1],
			args->angles[2]
		);
		Vector forward, right, up;
		AngleVectors(angles, forward, right, up);
		cl_entity_t* ent = gEngfuncs.GetViewModel();

		if (EV_IsLocal(idx))
		{
			++g_iShotsFired;
			EV_MuzzleFlash();
			if (args->bparam1)
			{
				gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT2), 2);
			}
			else
			{
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT3, 2);
			}

			if (gHUD.cl_gunsmoke->value)
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);

			if (!gHUD.cl_righthand->value)
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, 0);
			}
			else
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, 0);
			}

			VectorCopy(ent->attachment[1], ShellOrigin);
			ShellVelocity[2] -= 45;
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
		}

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iPShell, TE_BOUNCE_SHELL);

		PLAY_EVENT_SOUND("CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_SHOOT_1.WAV");

		EV_GetGunPosition(args, vecSrc, origin);
		Vector vSpread(args->fparam1, args->fparam2, 0.0f);

		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, forward,
			vSpread, 8192.0, BULLET_PLAYER_50AE,
			2);
	}
}

namespace cl::event::deagleel_left {
void EV_FireDeagleELLEFT(event_args_s *args)
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

		int iAnim = args->bparam2;

		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTB_L1 + iAnim, 2);

		if (gHUD.cl_gunsmoke->value)
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
	}

	if (EV_IsLocal(idx))
	{
		float vecScale[3] = { 20.0, -8.0, -10.0 };

		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

		VectorCopy(ent->attachment[3], ShellOrigin);
		ShellVelocity[2] -= 45;
	}
	else
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND("CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_SHOOT_1.WAV");

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
}

}
namespace cl::event::deagleel_right {
	void EV_FireDeagleELRight(event_args_s* args)
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

			int iAnim = args->bparam2;

			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTB_R1 + iAnim, 2);

			if (gHUD.cl_gunsmoke->value)
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.2, 16, 16, 16, 4, velocity, false, 35);
		}

		if (EV_IsLocal(idx))
		{
			float vecScale[3] = { 20.0, -8.0, 10.0 };

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

			VectorCopy(ent->attachment[1], ShellOrigin);
			ShellVelocity[2] -= 45;
		}
		else
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

		PLAY_EVENT_SOUND("CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_SHOOT_1.WAV");

		EV_GetGunPosition(args, vecSrc, origin);

		VectorCopy(forward, vecAiming);

		vSpread[0] = args->fparam1;
		vSpread[1] = args->fparam2;
		vSpread[2] = 0;

		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_50AE, 0, &tracerCount[idx - 1], 2);
	}
}

