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

namespace cl::event::flamethrower {

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

void EV_FlameThrower(vec3_t vecSrc, vec3_t vecDir, vec3_t vecBaseVelocity, int idx)
{
	int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr");

	TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, iModelIndex, 15);

	if (!pTemp)
		return;

	pTemp->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;

	pTemp->entity.angles.x = Com_RandomFloat(-512.0, 511.0);
	pTemp->entity.angles.y = Com_RandomFloat(-256.0, 255.0);
	pTemp->entity.angles.z = Com_RandomFloat(-256.0, 255.0);

	pTemp->flags |= FTENT_ROTATE;
	pTemp->entity.curstate.scale = 0.15;
	pTemp->entity.baseline.fuser2 = 1.5;

	pTemp->callback = SpriteGrow;
	pTemp->hitcallback = SpriteBlocked;

	pTemp->entity.curstate.rendermode = kRenderTransAdd;
	pTemp->entity.curstate.renderamt = 150;
	pTemp->entity.curstate.renderfx = kRenderFxNone;
	pTemp->entity.curstate.rendercolor.r = 255;
	pTemp->entity.curstate.rendercolor.g = 255;
	pTemp->entity.curstate.rendercolor.b = 255;

	pTemp->entity.origin = vecSrc;

	float flSpeed = 400 + Com_RandomLong(0, 5);

	pTemp->entity.baseline.origin = vecDir;
	VectorScale(pTemp->entity.baseline.origin, flSpeed, pTemp->entity.baseline.origin);

	if (vecBaseVelocity)
	{
		pTemp->entity.baseline.origin.x += vecBaseVelocity[0] * 0.9;
		pTemp->entity.baseline.origin.y += vecBaseVelocity[1] * 0.9;
		pTemp->entity.baseline.origin.z += vecBaseVelocity[2] * 0.5;
	}

	pTemp->clientIndex = idx;
}


void EV_FireFlameThrower(event_args_s* args)
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
	AngleVectors( angles, forward, right, up );

	angles.x += args->iparam1 / 100.0;
	angles.y += args->iparam2 / 100.0;

	EV_GetGunPosition( args, vecSrc, origin );

	cl_entity_t *ent = gEngfuncs.GetViewModel();

	if (EV_IsLocal(idx))
	{
		vecSrc = ent->attachment[0] + forward * 7 + velocity * 0.015;
	}
	else
	{
		vecSrc = vecSrc + forward - up * (args->ducking ? 3.0 : 9.0);
	}

	float flRange = args->bparam1;

	EV_FlameThrower(vecSrc, forward, velocity, idx);

	if (EV_IsLocal(idx))
	{
		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);

		dl->origin = vecSrc;
		dl->radius = 50.0;
		dl->decay = 25.0;
		dl->color.r = 225;
		dl->color.g = 190;
		dl->color.b = 80;
		dl->die = gHUD.m_flTime + 0.05;
	}
}

}
