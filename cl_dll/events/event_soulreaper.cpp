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



}

namespace cl::event::soulreaper {

enum soulreaper_e
{
	ANIM_IDLEA = 0,
	ANIM_SHOOTA,
	ANIM_RELOADA,
	ANIM_DRAWA,
	ANIM_SHOOTBA,
	ANIM_SHOOTBC,
	ANIM_IDLEC,
	ANIM_SHOOTC,
	ANIM_RELOADC,
	ANIM_CHANGECA,
};

void EV_FireSoulReaper(event_args_s *args)
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
	Vector vecBeamStart;
	Vector vecBeamEnd;

	ent = gEngfuncs.GetViewModel();
	idx = args->entindex;
	righthand = gHUD.cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);

	if (!args->bparam2 && args->bparam1)
	{
		if (EV_IsLocal(idx))
		{
			EV_MuzzleFlash();
		
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTC, 2);

			vecBeamStart = ent->attachment[0];

			vecBeamEnd = vecBeamStart + forward  * 8192;
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

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/soulreaper-3.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));

		//CreateBeam TODO
		int iBeamIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_soulreaper_laser.spr");

		BEAM* pBeam;

		gEngfuncs.pEfxAPI->R_BeamPoints_Stretch(vecBeamStart, vecBeamEnd, iBeamIndex, 1.0, 20.0, 255, 0, 50.0, 255, 255, 255);

		//CreateFX
		pmtrace_t tr;

		vecEnd = vecSrc + forward * 2048;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
		gEngfuncs.pEventAPI->EV_PushPMStates();
		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);
		gEngfuncs.pEventAPI->EV_PopPMStates();

		float flDis = (tr.endpos - vecSrc).Length();

		static constexpr const float flDisBetweenC02 = 100.0f;

		int iCountFX = floor(flDis / flDisBetweenC02);

		Vector vecAngle;
		VectorAngles(forward, vecAngle);

		int iShootFX = gEngfuncs.pEventAPI->EV_FindModelIndex("models/ef_soulreaper_shoot_c.mdl");

		Vector vecStart = vecSrc + forward * 10;

		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempCustomModel(vecStart, vecAngle, { 0, 0, 0 }, 1.0, iShootFX, 0, 1.0, true, 225, kRenderTransAdd, 0, 30, false, 0, 1.0, 0, FTENT_NOCULL, 0, 0);

		int iShootFX02 = gEngfuncs.pEventAPI->EV_FindModelIndex("models/ef_soulreaper_shoot_c02.mdl");

		for (int i = 1; i < iCountFX; i++)
		{
			float dis = flDisBetweenC02 * i;
			Vector vecC02Start = vecStart + forward * dis;

			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempCustomModel(vecC02Start, vecAngle, { 0, 0, 0 }, 1.0, iShootFX02, 0, 1.0, true, 225, kRenderTransAdd, 0, 30, false, 0, 1.0, 0, FTENT_NOCULL, 0, 0);
		}

		

		return;
	}

	if (EV_IsLocal(idx))
	{
		EV_MuzzleFlash();
		if (!args->bparam2)
		{
		
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTA, 2);
		}
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? ANIM_SHOOTBC : ANIM_SHOOTBA, 2);
		}

		if (gHUD.cl_gunsmoke->value)
		{
			cl_entity_t* ent = gEngfuncs.GetViewModel();

			if (ent)
			{
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.3, 16, 16, 16, 3, velocity, false, 35);
			}
		}
	}

	if (!args->bparam2)
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/soulreaper-1.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/soulreaper-2.wav", VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));
	}

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, forward, Vector(args->fparam1, args->fparam2, 0.0), 8192, BULLET_PLAYER_ETHEREAL, 0, &tracerCount[idx - 1], 2);
}

}
