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

namespace cl::event::huntbow {

enum huntbow_e
{
	BOW_IDLE,
	BOW_IDLE_EMPTY,
	BOW_SHOOT1,
	BOW_SHOOT1_EMPTY,
	BOW_DRAW,
	BOW_DRAW_EMPTY,
	BOW_CHARGE_START1,
	BOW_CHARGE_FINISH1,
	BOW_CHARGE_IDLE1,
	BOW_CHARGE_IDLE2,
	BOW_CHARGE_SHOOT1,
	BOW_CHARGE_SHOOT1_EMPTY,
	BOW_CHARGE_SHOOT2,
	BOW_CHARGE_SHOOT2_EMPTY
};

void EV_FireHuntbow(event_args_s *args)
{
	int idx, effect;
	vec3_t origin;
	vec3_t angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vecSpread;

	cl_entity_s *ent;
	ent = gEngfuncs.GetViewModel();

	idx = args->entindex;
	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	
	angles.x += args->iparam1 / 100.0;

	AngleVectors(angles, forward, right, up);
	EV_GetGunPosition(args, vecSrc, origin);

	if (EV_IsLocal(idx))
	{
		if (!args->bparam2)
		{
			if (args->bparam1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(BOW_SHOOT1, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(BOW_SHOOT1_EMPTY, 2);

			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/failnaught-1.wav", VOL_NORM, ATTN_NORM, 0, 93 + Com_RandomLong(0, 15));

			return;
		}

		if (args->iparam2)
		{
			if (args->bparam1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(BOW_CHARGE_SHOOT2, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(BOW_CHARGE_SHOOT2_EMPTY, 2);
		}
		else
		{
			if (args->bparam1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(BOW_CHARGE_SHOOT1, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(BOW_CHARGE_SHOOT2_EMPTY, 2);
		}
	}

	if (!args->bparam2)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/failnaught-1.wav", VOL_NORM, ATTN_NORM, 0, 93 + Com_RandomLong(0, 15));

		return;
	}

	if (args->iparam2 != 0)
	{
		Vector vecBeamStart;
		Vector vecBeamEnd = vecSrc + forward * 8192;

		if (EV_IsLocal(idx))
		{
			vecBeamStart = ent->attachment[0];
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
		}


		int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_huntbow_trail.spr");
		BEAM* pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(vecBeamStart, vecBeamEnd, iBeamModelIndex, 3.0f, 15.0f, 0.0f, 255.0f, 0.0f, 0.0f, 10.0f, 1.0f, 1.0f, 1.0f);

		if (pBeam)
			pBeam->flags |= FBEAM_FADEIN;

		Vector vecDirShooting = forward;

		vecSpread.x = args->fparam1;
		vecSpread.y = args->fparam2;

		EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecDirShooting, vecSpread, 8192, BULLET_PLAYER_338MAG, 3);
	}

	if (args->bparam1)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/failnaught-2.wav", VOL_NORM, ATTN_NORM, 0, 93 + Com_RandomLong(0, 15));
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/failnaught_shoot1_empty.wav", VOL_NORM, ATTN_NORM, 0, 93 + Com_RandomLong(0, 15));


}

}
