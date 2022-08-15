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

namespace cl::event::sfsniper {

enum sfsniper_e
{
	ANIM_IDLE = 0,
	ANIM_SHOOT,
	ANIM_DRAW
};

static const char *SOUNDS_NAME = "weapons/awp1.wav";

void EV_FireSFSniper( event_args_t *args )
{
	vec3_t vecSrc, vecAiming;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector forward, right, up;

	cl_entity_t* ent;
	Vector vecBeamStart;
	Vector vecBeamEnd;

	ent = gEngfuncs.GetViewModel();

	AngleVectors( angles, forward, right, up );
	EV_GetGunPosition(args, vecSrc, origin);

	if (EV_IsLocal(idx))
	{
		if (args->bparam1)
		{
			if (args->ducking)
			{
				vecBeamStart = args->origin;
			}
			else
			{
				vecBeamStart = args->origin;
			}
		}
		else
			vecBeamStart = ent->attachment[0];

		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT, 2);

		vecBeamEnd = vecBeamStart + (forward + right * args->fparam1 + up * args->fparam2) * 8192;
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
		0.0,
		0.0,
		1.0
	);

	if (pBeam)
		pBeam->flags |= FBEAM_FADEIN | FBEAM_FADEOUT | FBEAM_SHADEOUT;

	PLAY_EVENT_SOUND("weapons/sfsniper-1.wav");

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_RAILBEAM, 0, &tracerCount[idx - 1], 5);
}

}
