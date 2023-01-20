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
	extern int g_iHaloGunShootB;
	extern int g_iHaloGunChargingShootB;
}

namespace cl::event::halogun {

enum lightzg_e
{
	ANIM_IDLE = 0,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOTA,
	ANIM_SHOOTB_START,
	ANIM_SHOOTB_LOOP,
	ANIM_SHOOTB_END,
	ANIM_SHOOTB_CHARGING_LOOP,
	ANIM_SHOOTB_CHARGING_END,
	ANIM_SHOOTC,
	ANIM_SHOOTB_CHANGE,
};

void EV_FireHaloGun(event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t* ent;
	vec3_t dir, vecEnd;
	int righthand;
	vec3_t vSpread;

	ent = gEngfuncs.GetViewModel();
	idx = args->entindex;
	righthand = gHUD.cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

	if (args->bparam2)
	{
		if (args->iparam2 > 2)
		{
			g_iHaloGunShootB &= ~(1 << idx);
			g_iHaloGunChargingShootB |= (1 << idx);
		}
		else if (args->iparam2 > 0)
		{
			g_iHaloGunChargingShootB &= ~(1 << idx);
			g_iHaloGunShootB |= (1 << idx);
		}
		else
		{
			g_iHaloGunChargingShootB &= ~(1 << idx);
			g_iHaloGunShootB &= ~(1 << idx);
		}

		return;
	}

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? ANIM_SHOOTC : ANIM_SHOOTA, 2);

		if (gHUD.cl_gunsmoke->value)
		{
			cl_entity_t* ent = gEngfuncs.GetViewModel();

			if (ent)
			{
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.3, 16, 16, 16, 3, velocity, false, 35);
			}
		}
	}

	EV_GetGunPosition(args, vecSrc, origin);

	g_iHaloGunChargingShootB &= ~(1 << idx);
	g_iHaloGunShootB &= ~(1 << idx);

	if (args->bparam1)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/halogun-3.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));
	}	
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/halogun-1.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, forward, Vector(args->fparam1, args->fparam2, 0.0), 8192, BULLET_PLAYER_ETHEREAL, 0, &tracerCount[idx - 1], 2);
}

}
