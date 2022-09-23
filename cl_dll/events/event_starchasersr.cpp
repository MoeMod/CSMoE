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

namespace cl::event::starchasersr {

enum starchasersr_e
{
	ANIM_IDLE1 = 0,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_RELOAD,
	ANIM_DRAW,
};

static const char *SOUNDS_NAME = "weapons/starchasersr-1.wav";

void EV_FireStarChaserSR( event_args_t *args )
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;
	vec3_t velocity;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector forward, right, up;
	cl_entity_t* ent;
	int righthand;
	pmtrace_t tr;

	ent = gEngfuncs.GetViewModel();
	righthand = gHUD.cl_righthand->value;
	Vector vecBeamStart;
	Vector vecBeamEnd;

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();

		vecBeamStart = ent->attachment[0];
		vecBeamEnd = vecBeamStart + (forward + right * args->fparam1 + up * args->fparam2) * 8192;

		gEngfuncs.pEventAPI->EV_WeaponAnimation( Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT2), 2 );
	}
	else
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

	//if (EV_IsLocal(idx))
	//{
	//	float vecScale[3] = { 20.0, -8.0, -10.0 };

	//	if (righthand == 0)
	//	{
	//		vecScale[2] = 0 - vecScale[2];
	//		vecScale[0] = 17.0;
	//	}

	//	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, vecScale[0], vecScale[1], vecScale[2], true);

	//	VectorCopy(ent->attachment[1], ShellOrigin);
	//	VectorScale(ShellVelocity, 1.5, ShellVelocity);
	//	ShellVelocity[2] -= 50.0;
	//}
	//else
	//	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, -4.0, false);

	//EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND( SOUNDS_NAME );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	gEngfuncs.pEventAPI->EV_PushPMStates();
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecBeamStart, vecBeamEnd, PM_STUDIO_BOX, -1, &tr);
	gEngfuncs.pEventAPI->EV_PopPMStates();

	vecBeamEnd = tr.endpos;
	int iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_starchasersr_line.spr");
	Vector vecforward = (vecBeamEnd - vecBeamStart) / 17.5;

	int iCount = round((vecBeamEnd - vecBeamStart).Length() / 17.5);
	int iCount2 = iCount;
	while (iCount)
	{
		float fPrecent = float(iCount / iCount2);	
		gEngfuncs.pEfxAPI->R_Explosion(vecBeamStart, iModel, 0.1, 15 + round(15.0 * fPrecent), TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
		vecBeamStart += vecforward;
		iCount--;
	}

	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_338MAG,
		1 );
}

}
