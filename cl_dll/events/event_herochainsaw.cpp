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

namespace cl::event::herochainsaw {

enum chainsaw_e
{
	ANIM_IDLE,
    ANIM_DRAW,
    ANIM_DRAW_EMPTY,
    ANIM_ATTACK_BEGIN,
    ANIM_ATTACK_LOOP,
    ANIM_ATTACK_END,
    ANIM_RELOAD,
    ANIM_SLASH1,
    ANIM_SLASH2,
    ANIM_SLASH3,
    ANIM_SLASH4,
    ANIM_IDLE_EMPTY
};

void EV_FireHeroChainsaw(event_args_s *args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	vec3_t vSpread;

	cl_entity_t* ent = gEngfuncs.GetViewModel();
	idx = args->entindex;
	int righthand = gHUD.cl_righthand->value;

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/rshell.mdl");

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
	}
	
	if (!args->bparam1)
	{
		if (!args->bparam2)
		{
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/herochainsaw_attack_start.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 0xf));
			return;
		}

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/herochainsaw_attack_loop.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 0xf));
	}
	

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy(forward, vecAiming);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;
	EV_HLDM_FireBullets(idx,
		forward, right, up,
		1, vecSrc, vecAiming,
		vSpread, 85.0, BULLET_NONE,
		2);
}

}
