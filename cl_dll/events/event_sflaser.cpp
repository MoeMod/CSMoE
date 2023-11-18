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
#include "studio.h"
#include "r_studioint.h"
#include <string>

namespace cl {
	extern engine_studio_api_t IEngineStudio;
}

namespace cl::event::sflaser {

void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);
	int index = ent->entity.curstate.iuser1;
	float die = ent->entity.curstate.fuser1;
}

enum sflaser_e
{
	ANIM_IDLEA,
	ANIM_IDLEB,
	ANIM_IDLEC,
	ANIM_SHOOTA,
	ANIM_SHOOTB,
	ANIM_SHOOTC,
	ANIM_SHOOT_EMPTYA,
	ANIM_SHOOT_EMPTYB,
	ANIM_SHOOT_EMPTYC,
	ANIM_SHOOT_ENDA,
	ANIM_SHOOT_ENDB,
	ANIM_SHOOT_ENDC,
	ANIM_RELOADA,
	ANIM_RELOADB,
	ANIM_RELOADC,
	ANIM_CHANGEA,
	ANIM_CHANGEB,
	ANIM_DRAWA,
	ANIM_DRAWB,
	ANIM_DRAWC,
};

void EV_FireSFLaser(struct event_args_s* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int iBeamModelIndex;
	int iMuzzModelIndex;
	vec3_t vecSrc, vecEnd;
	vec3_t up, right, forward;
	cl_entity_t* ent;
	int righthand;
	pmtrace_t tr;

	ent = gEngfuncs.GetViewModel();

	idx = args->entindex;
	empty = args->bparam1 == false;
	righthand = gHUD.cl_righthand->value;

	VectorCopy(args->origin, origin);
	VectorCopy(args->angles, angles);
	VectorCopy(args->velocity, velocity);

	angles[0] += args->iparam1 / 100.0;
	angles[1] += args->iparam2 / 100.0;

	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);

	vecEnd = vecSrc + forward * 8192;

	iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	gEngfuncs.pEventAPI->EV_PushPMStates();
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);
	gEngfuncs.pEventAPI->EV_PopPMStates();

	int sModelMuzzleflash;
	gEngfuncs.CL_LoadModel("sprites/smoke_ia.spr", &sModelMuzzleflash);

	TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempCustomModel(tr.endpos, { 0, 0, 0 }, { 0, 0, 0 }, 0.3, sModelMuzzleflash, 0, 30, false, 225, kRenderTransAdd, 0, 0, false, 0, 0.08, 11, FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP, 0, 0);
	if (pEnt)
	{
		pEnt->entity.curstate.rendercolor.r = 185;
		pEnt->entity.curstate.rendercolor.g = 250;
		pEnt->entity.curstate.rendercolor.b = 30;
	}

	if (args->iparam2 == 1)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sflaser-2.wav", 1.0, ATTN_NORM, (1 << 7), PITCH_NORM);
	else if (args->iparam2 == 2)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sflaser-3.wav", 1.0, ATTN_NORM, (1 << 7), PITCH_NORM);
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sflaser-1.wav", 1.0, ATTN_NORM, (1 << 7), PITCH_NORM);

}


};