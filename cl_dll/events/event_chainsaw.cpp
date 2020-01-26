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


void EV_FireChainsaw(event_args_s *args)
{
	vec3_t vecSrc, vecAiming;

	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles( args->origin);
	Vector forward, right, up;

	AngleVectors( angles, forward, right, up );

	// Of all conditions, fparam1 means the distance to trace to, iparam2 means if the attack is mode 2 or 1
	// When attack with mode 1
	// bparam2 means if the player clicked left button just now
	// fparam2 means the shake power from server
	// iparam1 means a condition to choose which sound should we play
	// When attack with mode 2
	// fparam2 means if we should play the hit sound or just a slash sound
	// iparam1 means which condition of the animation we need to play
	// bparam1 means if the player hitted a player or an entity that we can hurt it
	// bparam2 means if the weapon has clip (m_iClip > 0)
	
	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	if (args->iparam2 == 2)
	{
		if (!EV_IsLocal(idx))
		{
			gEngfuncs.pfnPlaySoundByNameAtLocation("weapons/chainsaw_hit2.wav", gEngfuncs.pfnRandomFloat(0.8f, 0.9f), origin);
		}
		else
		{
			if(args->bparam2)
			{
				//have ammo
				gEngfuncs.pEventAPI->EV_WeaponAnimation(args->iparam1 ? ANIM_SLASH1 : ANIM_SLASH2, 2);
				//sound on attacker													hit player ? (random ? hit sound : (!anim ? (slash2 clip ? yes :no) :(slash1 clip ? yes: no))):(anim ? (slash1 clip ? yes :no) :(slash2 clip ? yes: no))
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, args->bparam1 ? "weapons/chainsaw_hit2.wav"  : (args->iparam1 ? (args->bparam2 ? "weapons/chainsaw_slash1.wav" : "weapons/chainsaw_slash3.wav") : (args->bparam2 ? "weapons/chainsaw_slash2.wav" : "weapons/chainsaw_slash4.wav")), 1.0f, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 15));
			}
			else
			{
				//no ammo
				gEngfuncs.pEventAPI->EV_WeaponAnimation(args->iparam1 ?  ANIM_SLASH3 :  ANIM_SLASH4, 2);
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, args->iparam1 ? (args->bparam2 ? "weapons/chainsaw_slash1.wav" : "weapons/chainsaw_slash3.wav") : (!args->bparam2 ? "weapons/chainsaw_slash2.wav" : "weapons/chainsaw_slash4.wav"), 1.0, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 15));
			}
		}
	}
	else if (args->iparam2 == 1)
	//sound on hit
	{
		if (!EV_IsLocal(idx))
		{
			cl_entity_t *gPlayer = gEngfuncs.GetEntityByIndex(idx);
			gEngfuncs.pEventAPI->EV_PlaySound(idx, gPlayer->origin, CHAN_AUTO, args->bparam2 ? "weapons/chainsaw_hit2.wav" : (args->bparam1 ? "weapons/chainsaw_hit4.wav" : "weapons/chainsaw_hit3.wav"), 1.0, ATTN_NORM, 0, gEngfuncs.pfnRandomLong(0, 15) + 94);
		}
		/*int iPlayerBits = args->iparam1;
		if (iPlayerBits)
		{
			for (int iPlayer = 1; iPlayer < 33; iPlayer++)
			{
				if (iPlayerBits & (1 << iPlayer))
				{
					cl_entity_t *gPlayer = gEngfuncs.GetEntityByIndex(iPlayer);
					if (!gPlayer)
						continue;
					gEngfuncs.pEventAPI->EV_PlaySound(iPlayer, gPlayer->origin, CHAN_AUTO, args->bparam2 ? "weapons/chainsaw_hit2.wav" : (args->bparam1 ? "weapons/chainsaw_hit4.wav" : "weapons/chainsaw_hit3.wav"), 1.0, ATTN_NORM, 0, gEngfuncs.pfnRandomLong(0, 15) + 94);
				}
			}
		}*/
	}
	else
	{
		if (!args->bparam2)
		{
			if (EV_IsLocal(idx))
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_ATTACK_BEGIN, 2);
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/chainsaw_attack1_start.wav", 1.0f, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 15));
		}
		else
		{
			if (EV_IsLocal(idx))
			{
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_ATTACK_LOOP, 2);
			}
			if (args->bparam1)
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/chainsaw_hit1.wav", 1.0f, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 15));
			}
			else
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/chainsaw_attack1_loop.wav", 1.0f, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 15));
			}
		}
	}

	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		Vector(0, 0, 0), args->fparam1, BULLET_PLAYER_CROWBAR,
		0 );
}

}
