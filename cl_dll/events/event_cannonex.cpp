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


namespace cl::event::cannonex {

enum cannon_e
{
	ANIM_IDLE1,
	ANIM_SHOOT,
	ANIM_DRAW,
};

void DefaultScaleThink(tempent_s* ent, float frametime, float currenttime)
{
	if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
		ent->entity.curstate.scale += 0.03;
}

void DefaultFramerateTouch(tempent_s* ent, pmtrace_t* tr)
{
	ent->entity.baseline.origin *= 0.01f;
	ent->entity.curstate.framerate = 60;
}

void EV_CreateFlameThrowerFlame(vec3_t vecSrc, vec3_t vecDir, vec3_t base_velocity, int idx)
{
	int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr");

	TEMPENTITY* pEntity = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, iModelIndex, 48);

	if (pEntity)
	{
		pEntity->flags |= FTENT_COLLIDEALL | FTENT_CLIENTCUSTOM;
		pEntity->entity.angles.x = gEngfuncs.pfnRandomFloat(-512, 511);
		pEntity->entity.angles.x = gEngfuncs.pfnRandomFloat(-256, 255);
		pEntity->entity.angles.z = gEngfuncs.pfnRandomFloat(-256, 255);
		pEntity->flags |= FTENT_ROTATE;
		pEntity->entity.curstate.scale = 0.15;
		pEntity->entity.baseline.fuser1 = 2.5;
		pEntity->callback = DefaultScaleThink;
		pEntity->hitcallback = DefaultFramerateTouch;
		pEntity->entity.curstate.rendermode = kRenderTransAdd;
		pEntity->entity.curstate.renderamt = 150;
		pEntity->entity.curstate.renderfx = kRenderFxFadeSlow;
		pEntity->entity.origin = vecSrc;
		pEntity->entity.baseline.origin = vecDir * (400 + gEngfuncs.pfnRandomLong(0, 5));
		if (base_velocity.Length())
		{
			pEntity->entity.baseline.origin.x += 0.9 * base_velocity.x;
			pEntity->entity.baseline.origin.y += 0.9 * base_velocity.y;
			pEntity->entity.baseline.origin.z += 0.5 * base_velocity.z;
		}
		pEntity->clientIndex = idx;
	}
}

void EV_CannonexFireEffect(vec3_t vecSrc, vec3_t vecForward, vec3_t vecVelocity, int idx, int iModel)
{
	if (!iModel)
		iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr");
	TEMPENTITY *ent;
	int i;
	int ran;
	float c, s;
	for (i = 0; i < 12; i++)
	{
		ent = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, iModel, 20);
		if (!ent)
			break;
		ent->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEALL;
		ent->entity.angles.x = gEngfuncs.pfnRandomFloat(-512.0, 511.0);
		ent->entity.angles.y = gEngfuncs.pfnRandomFloat(-256.0, 255.0);
		ent->entity.angles.z = gEngfuncs.pfnRandomFloat(-256.0, 255.0);
		ent->flags |= FTENT_ROTATE;
		ent->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.3, 0.8);
		ent->entity.baseline.fuser1 = gEngfuncs.pfnRandomFloat(1.5, 2.0);	// CSO: baseline.fuser2
		ent->callback = [](tempent_s *ent, float frametime, float currenttime)
		{
			if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
				ent->entity.curstate.scale += 0.03;
		};

		ent->hitcallback = [](tempent_s *ent, pmtrace_t *tr)
		{
			ent->entity.baseline.origin = ent->entity.baseline.origin * 0.01f;
			ent->entity.curstate.framerate = 60;
		};

		ent->entity.curstate.rendermode = kRenderTransAdd;
		ent->entity.curstate.renderamt = 150;
		ent->entity.curstate.renderfx = kRenderFxFadeSlow;
		ran = 10 * gEngfuncs.pfnRandomLong(0, 4) - 2;
		c = cos(DEG2RAD(ran));
		s = sin(DEG2RAD(ran));
		ent->entity.origin = vecSrc;
		ent->entity.baseline.origin.x = (c * vecForward.x - s * vecForward.y) * gEngfuncs.pfnRandomLong(130, 400);
		ent->entity.baseline.origin.y = (s * vecForward.x + c * vecForward.y) * gEngfuncs.pfnRandomLong(130, 400);
		ent->entity.baseline.origin.z = vecForward.z * gEngfuncs.pfnRandomLong(30, 200);

		if (vecVelocity)
		{
			ent->entity.baseline.origin.x += vecVelocity.x * 0.9;
			ent->entity.baseline.origin.y += vecVelocity.y * 0.9;
			ent->entity.baseline.origin.z += vecVelocity.z * 0.3;
		}

		ent->clientIndex = idx;
	}
}


void EV_FireCannonCommon(struct event_args_s* args, const char* szSound, int modelIndex = 0)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc;
	vec3_t up, right, forward;
	cl_entity_t* ent;
	int righthand;

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
		g_iShotsFired++;

		EV_MuzzleFlash();

		if (!Com_RandomLong(0, 5))
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.26, 15, 15, 15, 3, velocity, true, 35);
	}

	EV_GetGunPosition(args, vecSrc, origin);

	if (EV_IsLocal(idx))
	{
		vecSrc = ent->attachment[0] + forward * 7.0 + velocity * 0.015;
	}
	else
	{
		vecSrc = vecSrc + forward * 40.0 - up * (args->ducking ? 3.0 : 9.0);
	}

	EV_CannonexFireEffect(vecSrc, forward, velocity, idx, modelIndex);

	if (EV_IsLocal(idx))
	{
		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);

		dl->origin = vecSrc;
		dl->radius = 50;
		dl->decay = 25;
		dl->color.r = 225;
		dl->color.g = 190;
		dl->color.b = 80;
		dl->die = gHUD.m_flTime + 0.05;
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, szSound, VOL_NORM, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));
}

void EV_FireCannonEX( event_args_t *args )
{
	if (args->iparam1 == 4)
	{
		int idx;
		vec3_t origin;
		vec3_t angles;
		vec3_t velocity;
		vec3_t up, right, forward;
		cl_entity_t* ent;

		idx = args->entindex;
		ent = gEngfuncs.GetEntityByIndex(idx);
		VectorCopy(args->origin, origin);
		VectorCopy(args->angles, angles);
		VectorCopy(args->velocity, velocity);

		angles.y -= 18;

		gEngfuncs.pfnAngleVectors(angles, forward, right, up);

		AngleVectors(args->angles, forward, right, up);

		Vector vecVelocity;
		VectorCopy(args->velocity, vecVelocity);
		vecVelocity[2] -= 240.0;

		EV_CreateFlameThrowerFlame(ent->attachment[0] + forward * 7 + up * 20, forward, vecVelocity, idx);

		gEngfuncs.pEventAPI->EV_PlaySound(idx, args->origin, CHAN_AUTO, "weapons/flamegun-1.wav", 1.0, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 15));
		return;
	}

	if (args->bparam1)
	{
		EV_FireCannonCommon(
			args,
			args->bparam2 ? "weapons/cannonex_ice_shootA.wav" : "weapons/cannonex_shootA.wav",
			args->bparam2 ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_cannonex_breath_d3a3.spr") : 0
		);
	}
	else
	{
		EV_FireCannonCommon(
			args,
			args->bparam2 ? "weapons/cannonex_ice_D_reload1.wav" : "weapons/cannonex_D_reload1.wav",
			args->bparam2 ? gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_cannonex_breath_d3a3.spr") : 0
		);
	}
}

}
