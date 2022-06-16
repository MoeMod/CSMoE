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

namespace cl::event::ethereal {

enum lightzg_e
{
	ANIM_IDLE1,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3,
};

void EV_FireEthereal(event_args_s *args)
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

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT1 + Com_RandomLong(0, 2), 2);

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

	const float minParticleDistance = 20.0f;
	const float traceDistance = 8192.0f;
	const float particleVel = 6900.0f;

	if (EV_IsLocal(idx))
	{
		vecEnd = ent->attachment[0] + (forward + right * args->fparam1 + up * args->fparam2) * traceDistance;

		pmtrace_t tr;

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(ent->attachment[0], vecEnd, PM_NORMAL, -1, &tr);

		if (tr.fraction > (minParticleDistance / traceDistance))
		{
			if (righthand)
			{
				dir = right;
			}
			else
			{
				dir = -right;
			}

			Vector vecStart = ent->attachment[0] + dir * 10.0 + up * -8.0 + forward * -30.0;
			Vector vecDir = vecEnd - vecStart;
			float flLength = vecDir.NormalizeInPlace();

			vecStart += vecDir * float(rand() % 20 + 30);
			vecDir *= particleVel;

			particle_t* pparticle = gEngfuncs.pEfxAPI->R_TracerParticles(vecStart, vecDir, flLength / particleVel);

			if (pparticle)
			{
				pparticle->type = pt_clientcustom;
				pparticle->color = 12;
				pparticle->ramp = 0.9;
			}
		}
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/ethereal-1.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, forward, Vector(args->fparam1, args->fparam2, 0.0), 8192, BULLET_PLAYER_ETHEREAL, 0, &tracerCount[idx - 1], 2);
}

}
