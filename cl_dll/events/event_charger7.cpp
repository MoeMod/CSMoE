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

namespace cl::event::charger7 {

enum charger7_e
{
	ANIM_IDLE1,
	ANIM_IDLE2,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT_LASER1,
	ANIM_SHOOT_LASER2,
	ANIM_RELOAD1,
	ANIM_RELOAD2,
	ANIM_DRAW1,
	ANIM_DRAW2,
};

static const char *SOUNDS_NAME[] =
{
	 "weapons/charger7-1.wav", "weapons/charger7-2.wav"
};

void EV_FireCharger7(event_args_s *args)
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
	Vector vecBeamStart;
	Vector vecBeamEnd;

	ent = gEngfuncs.GetViewModel();

	if ( EV_IsLocal(idx) )
	{
		++g_iShotsFired;
		
		if (args->bparam2)
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? ANIM_SHOOT_LASER2 : ANIM_SHOOT_LASER1, 2);
			vecBeamStart = ent->attachment[1];
			vecBeamEnd = vecBeamStart + (forward + right * args->fparam1 + up * args->fparam2) * 8192;
		}
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? ANIM_SHOOT2 : ANIM_SHOOT1, 2);

		}
		EV_MuzzleFlash();
			
		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, -13.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, 13.0, 0);
		}

		VectorCopy(ent->attachment[2], ShellOrigin);
	}
	else
	{
		if (args->bparam1)
		{
			if (args->ducking)
			{
				vecBeamStart = vecSrc + forward * 40 - up * 3;
			}
			else
			{
				vecBeamStart = vecSrc + forward * 40 - up * 9;
			}

			vecBeamEnd = vecBeamStart + forward * 8192;
		}
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	if (args->bparam2)
	{
		PLAY_EVENT_SOUND(SOUNDS_NAME[1]);

		int iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");

		BEAM* pBeam;

		pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(vecBeamStart, vecBeamEnd,
			iModel,
			1.0,
			3.0,
			0.0,
			700.0f,
			0.0,
			0,
			0,
			0.0,
			1.0,
			1.0
		);


		if (pBeam)
			pBeam->flags |= FBEAM_FADEIN | FBEAM_FADEOUT | FBEAM_SHADEOUT;
	}
	else
	{
		PLAY_EVENT_SOUND(SOUNDS_NAME[0]);
	}

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);
	Vector vSpread;

	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;

	if(args->bparam2)
		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_RAILBEAM, 0, &tracerCount[idx - 1], 3);
	else
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_762MM,
			2);
}

}
