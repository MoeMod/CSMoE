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

extern float g_flTimeM134FireAfter;

}

namespace cl::event::m134heroFA {

void EV_M134HeroFireAfter(struct event_args_s* args)
{
	if (EV_IsLocal(args->entindex))
		g_flTimeM134FireAfter = 0.0f;
}
}

namespace cl::event::m134hero {

enum m134hero_e
{
	ANIM_IDLE1,
	ANIM_DRAW,
	ANIM_RELOAD,
	ANIM_FIRE_READY,
	ANIM_SHOOT1,
	ANIM_FIRE_AFTER,
	ANIM_FIRE_CHANGE,
	ANIM_IDLE_CHANGE,
	ANIM_DRAW_OVERHEAT,
	ANIM_FIRE_AFTER_OVERHEAT,
	ANIM_IDLE_OVERHEAT,
	ANIM_OVERHEAT_END,
	ANIM_FIRE_READY_SHOOTB,
	ANIM_SHOOT2,
	ANIM_FIRE_AFTER_SHOOTB
};

void EV_FireM134Hero(event_args_s *args)
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
	AngleVectors( angles, forward, right, up );
	int shell[2];

	shell[0] = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell762_m134.mdl");
	shell[1] = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell762_m134_01.mdl");

	cl_entity_t* ent;

	ent = gEngfuncs.GetViewModel();

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;

		if (gHUD.m_flTime >= g_flTimeM134FireAfter)
		{
			if (gHUD.m_flTime > g_flTimeM134FireAfter + 1.0)
			{
				g_flTimeM134FireAfter = gHUD.m_flTime;

				gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? ANIM_SHOOT2 : ANIM_SHOOT1, 2);
			}
		}
		else
		{
			g_flTimeM134FireAfter = gHUD.m_flTime;
		}

		EV_MuzzleFlash();
	}
	
	for (int i = 0; i < 2; i++)
	{
		if (EV_IsLocal(idx))
		{
			float vecScale[3] = { 20.0, -10.0, -13.0 };

			if (gHUD.cl_righthand->value)
				vecScale[2] = 0 - vecScale[2];

			if (i)
				vecScale[2] = 0 - vecScale[2];

			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

			VectorScale(ShellVelocity, 1.2, ShellVelocity);
			ShellVelocity[2] -= 75;

			VectorCopy(ent->attachment[1 + i], ShellOrigin);

			EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell[i], TE_BOUNCE_SHELL);
		}
	}

	PLAY_EVENT_SOUND("weapons/m134-1.wav");

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_762MM,
		2 );
}

}
