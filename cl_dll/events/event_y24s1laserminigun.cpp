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

namespace cl::event::y24s1laserminigun {

enum y24s1laserminigun_e
{
	ANIM_IDLE1 = 0,
	ANIM_CHARGE_ORIGIN,
	ANIM_CHARGE_START,
	ANIM_CHARGE_LOOP,
	ANIM_CHARGE_SHOOT,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3,
};

void EV_FireY24S1LaserMiniGun(event_args_s *args)
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
	cl_entity_t* ent = gEngfuncs.GetViewModel();
	Vector vecBeamStart, vecBeamEnd;

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;
		gEngfuncs.pEventAPI->EV_WeaponAnimation(RandomInt(ANIM_SHOOT1, ANIM_SHOOT3), 2);
		EV_MuzzleFlash();

		vecBeamStart = ent->attachment[0];

		vecBeamEnd = vecBeamStart + (forward + right * args->fparam1 + up * args->fparam2) * 40;

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

	PLAY_EVENT_SOUND("weapons/laserminigun-1.wav");

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;

	int iModelLaser;

	iModelLaser = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_laser1.spr");

	BEAM* pBeam = gEngfuncs.pEfxAPI->R_BeamPoints_Tracer(vecBeamStart, vecBeamEnd, iModelLaser, 0.2, 2, 40.0f, 0.0f, 255.0f, 360.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);


	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_556MM,
		1 );
}

}
