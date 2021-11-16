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

namespace cl::event::buffm249 {

enum buffm249_e
{
	BUFFM249_IDLE1,
	BUFFM249_SHOOT1,
	BUFFM249_SHOOT2,
	BUFFM249_RELOAD,
	BUFFM249_DRAW
};

static const char *SOUNDS_NAME[] =
{
	 "weapons/buffm249-1.wav", "weapons/buffm249-2.wav"
};

void EV_FireBUFFM249(event_args_s *args)
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
		
		if (args->bparam1)
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BUFFM249_SHOOT2, 2);
			vecBeamStart = ent->attachment[0];
			vecBeamEnd = vecBeamStart + (forward + right * args->fparam1 + up * args->fparam2) * 8192;
		}
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(BUFFM249_SHOOT1, 2);
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

		VectorCopy(ent->attachment[1], ShellOrigin);
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

	if (args->bparam1)
	{
		PLAY_EVENT_SOUND(SOUNDS_NAME[1]);

		int iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");

		BEAM* pBeam;

		pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(vecBeamStart, vecBeamEnd,
			iModel,
			1.0,
			3.0,
			0.0,
			128.0,
			0.0,
			0,
			0,
			1.0,
			0.4,
			0.0
		);

		if (pBeam)
			pBeam->flags |= FBEAM_FADEIN;
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
	EV_HLDM_FireBullets(idx,
		forward, right, up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_556MM,
		2);
}

}
