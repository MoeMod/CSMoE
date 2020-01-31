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

enum bow_e
{
	ANIM_IDLE,
	ANIM_IDLE_EMPTY,
	ANIM_SHOOT1,
	ANIM_SHOOT1_EMPTY,
	ANIM_DRAW,
	ANIM_DRAW_EMPTY,
	ANIM_CHARGE_START1,
	ANIM_CHARGE_FINISH1,
	ANIM_CHARGE_IDLE1,
	ANIM_CHARGE_IDLE2,
	ANIM_CHARGE_SHOOT1,
	ANIM_CHARGE_SHOOT1_EMPTY,
	ANIM_CHARGE_SHOOT2,
	ANIM_CHARGE_SHOOT2_EMPTY
};

void EV_FireBow(event_args_s *args)
{
	int idx, effect;
	vec3_t origin;
	vec3_t angles;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	
	cl_entity_s *ent;
	ent = gEngfuncs.GetViewModel();

	idx = args->entindex;
	VectorCopy(args->origin, origin);

	if (args->bparam2)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "sound/weapons/bow-shoot1", 1.0, ATTN_NORM, 0, PITCH_NORM);

		if (EV_IsLocal(idx) && args->bparam2)
			g_iShotsFired++;
		return;
	}
	VectorCopy(args->angles, angles);

	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, EV_IsLocal(idx) ? ent->attachment[0] : vecSrc, vecAiming, Vector(0, 0, 0), 8192, BULLET_PLAYER_338MAG, 0);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, args->iparam1 ? "sound/weapons/bow_charge_shoot2.wav" : "sound/weapons/weapons/bow_charge_shoot1_empty.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");

	vec3_t vecDir, vecEnd;

	vecDir = vecAiming;
	vecEnd = vecSrc + 8196.0f * vecDir;

	BEAM *pBeam;
	if (EV_IsLocal(idx))
		pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(ent->attachment[0], vecEnd, iBeamModelIndex, 1.0f, 3.0f, 0.0f, 128.0f, 0.0f, 0.0f, 0.0f, 0.78f, 0.37f, 0.29f);
	else
		pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, vecEnd, iBeamModelIndex, 1.0f, 3.0f, 0.0f, 128.0f, 0.0f, 0.0f, 0.0f, 0.78f, 0.37f, 0.29f);

	if (pBeam)
		pBeam->flags |=  FBEAM_FADEIN;
}

}
