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

namespace cl::event::poisongun {

void SpriteGrow(struct tempent_s* te, float frametime, float current_time)
{
	if (te->entity.baseline.fuser2 > te->entity.curstate.scale)
	{
		te->entity.curstate.scale += 0.03;
	}
}

void SpriteBlocked(TEMPENTITY* te, pmtrace_s* ptr)
{
	te->entity.curstate.framerate = 60;
	te->entity.baseline.origin *= 0.01;
}

void EV_CreatePoison(vec3_t vecSrc, vec3_t vecDir, vec3_t vecVelocity, int idx, const char* modelName, float flScale)
{
	TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, gEngfuncs.pEventAPI->EV_FindModelIndex(modelName), 10);

	if (!pTemp)
		return;

	pTemp->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEALL;
	pTemp->entity.angles.x = Com_RandomFloat(-512.0, 511.0);
	pTemp->entity.angles.y = Com_RandomFloat(-256.0, 255.0);
	pTemp->entity.angles.z = Com_RandomFloat(-256.0, 255.0);

	pTemp->flags |= FTENT_ROTATE;
	pTemp->callback = SpriteGrow;
	pTemp->hitcallback = SpriteBlocked;
	pTemp->entity.curstate.rendermode = kRenderTransAdd;
	pTemp->entity.curstate.renderamt = 125;
	pTemp->entity.curstate.renderfx = kRenderFxFadeFast;
	pTemp->entity.curstate.rendercolor.r = 255;
	pTemp->entity.curstate.rendercolor.g = 255;
	pTemp->entity.curstate.rendercolor.b = 255;
	pTemp->entity.curstate.scale = flScale * 0.15;
	pTemp->entity.baseline.fuser2 = flScale * 2.5;
	pTemp->entity.origin = vecSrc;
	pTemp->entity.baseline.origin = vecDir;
	pTemp->entity.baseline.origin *= Com_RandomLong(0, 5) + 800;
	pTemp->clientIndex = idx;
}


void EV_PoisonEffect(vec3_t vecSrc, vec3_t vecDir, vec3_t vecVelocity, int idx)
{
	for (int i = 5; i; i--)
		EV_CreatePoison(vecSrc, vecDir, vecVelocity, idx, "sprites/spr_boomer.spr", 0.75);

	EV_CreatePoison(vecSrc, vecDir, vecVelocity, idx, "sprites/ef_smoke_poison.spr", 20.0);
}

void EV_FirePoisonGun(event_args_s* args)
{
	vec3_t vecSrc;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2] );
	Vector velocity( args->velocity );
	Vector forward, right, up;
	
	angles.x += args->iparam1 / 100.0;
	angles.y += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

		cl_entity_t *ent = gEngfuncs.GetViewModel();

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;

		if (!Com_RandomLong(0, 5))
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.26, 15, 15, 15, 3, velocity, true, 35);
	}

	EV_GetGunPosition(args, vecSrc, origin);

	if (EV_IsLocal(idx))
	{
		vecSrc = ent->attachment[0] + forward * 7.0;
	}
	else
	{
		vecSrc = vecSrc + forward * 40.0 - up * (args->ducking ? 3.0 : 9.0);
	}

	EV_PoisonEffect(vecSrc, forward, velocity, idx);

	if (EV_IsLocal(idx))
	{
		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);

		dl->origin = vecSrc;
		dl->radius = 250;
		dl->decay = 200;
		dl->color.r = 100;
		dl->color.g = 220;
		dl->color.b = 150;
		dl->die = gHUD.m_flTime + 0.3;
	}
}

}
