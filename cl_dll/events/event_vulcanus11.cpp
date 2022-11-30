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

namespace cl::event::vulcanus11 {

enum vulcanus11_e
{
	ANIM_IDLE,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_INSERT,
	ANIM_AFTER_RELOAD,
	ANIM_START_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT3,
	ANIM_FUNC_CHANGE1,
	ANIM_FUNC_CHANGE2,
	ANIM_IDLE2,
	ANIM_INSERT2,
	ANIM_AFTER_RELOAD2,
	ANIM_START_RELOAD2,
	ANIM_DRAW2,
};

static const char* SOUNDS_NAME[] =
{
	"weapons/vulcanus11-1.wav",
	"weapons/vulcanus11-2.wav"
};

void EV_FireVulcanus11( event_args_t *args )
{
	vec3_t vecSrc, vecAiming;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity(args->velocity);
	Vector forward, right, up;
	cl_entity_t* ent;

	AngleVectors( angles, forward, right, up );

	int shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl");
	ent = gEngfuncs.GetViewModel();
	idx = args->entindex;
	int righthand = gHUD.cl_righthand->value;

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		if(args->bparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT3, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT2), 2);

		if (gHUD.cl_gunsmoke->value)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.45, 15, 15, 15, 3, velocity, false, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.35, 9, 9, 9, 2, velocity, false, 35);
		}
	}

	if (!args->bparam1)
	{
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
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, false);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL, 3);
	}
	
	if (args->bparam1)
		PLAY_EVENT_SOUND(SOUNDS_NAME[1]);
	else
	{
		PLAY_EVENT_SOUND(SOUNDS_NAME[0]);

		EV_GetGunPosition(args, vecSrc, origin);
		VectorCopy(forward, vecAiming);
		Vector vSpread;

		vSpread.x = args->fparam1;
		vSpread.y = args->fparam2;
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			args->bparam2, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
			1);
	}
	
}

}
