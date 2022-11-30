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

namespace cl::event::vulcanus7 {

enum vulcanus7_e
{
	ANIM_IDLEA = 0,
	ANIM_IDLEB,
	ANIM_IDLEA_EMPTY,
	ANIM_IDLEB_EMPTY,
	ANIM_CHANGE,
	ANIM_CHANGE_EMPTY,
	ANIM_SHOOTA1,
	ANIM_SHOOTB1,
	ANIM_SHOOTB2,
	ANIM_SHOOTC1_CHANGE,
	ANIM_SHOOTC1_CHANGE_EMPTY,
	ANIM_RELOADA,
	ANIM_RELOADB,
	ANIM_DRAWA,
	ANIM_DRAWB,
	ANIM_DRAWA_EMPTY,
	ANIM_DRAWB_EMPTY,
};

void EV_FireVulcanus7(event_args_s *args)
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

	cl_entity_t* ent;

	ent = gEngfuncs.GetViewModel();

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;
		int iAnim;
		if (args->bparam1)
		{
			iAnim = ANIM_SHOOTB2;
		}
		else
		{
			iAnim = args->bparam2 ? ANIM_SHOOTB1 : ANIM_SHOOTA1;
		}

		gEngfuncs.pEventAPI->EV_WeaponAnimation(iAnim, 2);
		EV_MuzzleFlash();

		float vecScale[3] = { 20.0, -10.0, -13.0 };

		if (gHUD.cl_righthand->value)
			vecScale[2] = 0 - vecScale[2];

		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

		VectorScale(ShellVelocity, 1.2, ShellVelocity);
		ShellVelocity[2] -= 75;

		VectorCopy(ent->attachment[1], ShellOrigin);
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, 0);
	}


	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;

	if (args->bparam1)
	{
		PLAY_EVENT_SOUND("weapons/vulcanus7-2.wav");

		EV_HLDM_FireBullets(idx,
			forward, right, up,
			3, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_556MM,
			1);
	}
	else
	{
		PLAY_EVENT_SOUND("weapons/vulcanus7-2.wav");

		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_556MM,
			2);
	}

}

}
