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

namespace cl::event::blockas {

enum blockas1_e
{
	BLOCKAS1_IDLE,
	BLOCKAS1_SHOOT1,
	BLOCKAS1_SHOOT2,
	BLOCKAS1_SHOOT3,
	BLOCKAS1_CHANGE1,
	BLOCKAS1_CHANGE2,
	BLOCKAS1_RELOAD_START,
	BLOCKAS1_RELOAD_LOOP,
	BLOCKAS1_RELOAD_END,
	BLOCKAS1_DRAW,
};

void EV_FireBlockAS(event_args_s *args)
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
	int righthand = gHUD.cl_righthand->value;

	AngleVectors( angles, forward, right, up );
	int shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/blockas1_bullet.mdl");

	if (args->bparam1)
	{
		gEngfuncs.pEfxAPI->R_BreakModel(ent->origin + forward * 35, Vector(10, 10, 10), Vector(0, 0, 0), 100.0, 3.0, 10, shell, 0x40);
	}
	else
	{
		if (EV_IsLocal(args->entindex))
		{
			g_iShotsFired++;
			EV_MuzzleFlash();

			gEngfuncs.pEventAPI->EV_WeaponAnimation(BLOCKAS1_SHOOT1 + Com_RandomLong(0, 2), 2);

			if (gHUD.cl_gunsmoke->value)
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.35, 20, 20, 20, 4, velocity, false, 35);

			float vecScale[3] = { 22.0, -9.0, -11.0 };

			if (righthand == 0)
				vecScale[2] = 0 - vecScale[2];

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

			VectorCopy(ent->attachment[1], ShellOrigin);
			VectorScale(ShellVelocity, 1.25, ShellVelocity);
			ShellVelocity[2] -= 50;
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
		}

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/blockas1-2.wav", 1.0, 0.4, 0, 94 + Com_RandomLong(0, 0xf));

		EV_GetGunPosition(args, vecSrc, origin);
		VectorCopy(forward, vecAiming);

		Vector vSpread(0.0725, 0.0725, 0.0f);
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			6, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
			1);
	}
}

}
