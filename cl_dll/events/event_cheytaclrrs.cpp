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

namespace cl::event::cheytaclrrs {

enum cheytaclrrs_e
{
	ANIM_IDLE1 = 0,
	ANIM_SHOOT,
	ANIM_DRAW,
	ANIM_SHOOT_EMPTY,
};

void ResetMuz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);
	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel) {
		ent->entity.curstate.renderamt = 0;
		ent->die = gHUD.m_flTime;
	}
}

void PlayMuzzleFlash(cl_entity_t* ent, int iType, int iId)
{
	float Scale = 0.2;
	float FrameCount = 2;
	float FrameRate = 30.0;
	float die = 0.2;
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(iId);
	if (!parent)
		return;

	model_s* pModel = IEngineStudio.Mod_ForName("sprites/muz_cheytaclrrs.spr", false);
	if (iType)
	{
		pModel = IEngineStudio.Mod_ForName("sprites/smoke_cheytaclrrs.spr", false);
		FrameCount = 10;
		Scale = 0.08;
		FrameRate = 30.0;
		die = 1.0;
	}
		

	if (pModel)
	{
		TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(ent->attachment[0], pModel);
		if (pTemp)
		{
			pTemp->flags |= FTENT_SPRANIMATE;
			pTemp->entity.curstate.rendermode = kRenderTransAdd;
			pTemp->entity.curstate.renderamt = 255;
			pTemp->entity.curstate.framerate = FrameRate;
			pTemp->entity.curstate.renderfx = 0;
			pTemp->die = gHUD.m_flTime + die;
			pTemp->entity.curstate.frame = 0;
			pTemp->entity.curstate.movetype = MOVETYPE_FOLLOW;
			pTemp->entity.curstate.aiment = iId;
			pTemp->frameMax = FrameCount;
			pTemp->entity.curstate.body = 1;
			pTemp->entity.curstate.weaponmodel = parent->curstate.weaponmodel;
			pTemp->entity.curstate.scale = Scale;
			pTemp->flags |= FTENT_CLIENTCUSTOM;
			pTemp->callback = ResetMuz;
		}
		
	}
	
}

void EV_FireCheytaclrrs( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	cl_entity_t* ent;
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
		g_iShotsFired++;
		PlayMuzzleFlash(ent, 0, idx);
		PlayMuzzleFlash(ent, 1, idx);

		if(args->bparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_EMPTY, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT, 2);

		if (gHUD.cl_gunsmoke->value)
		{
			if (ent)
			{
				EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[0], forward, 3, 0.5, 20, 20, 20, false, velocity);
				EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[0], forward, 40, 0.5, 15, 15, 15, false, velocity);
				EV_CS16Client_CreateSmoke(SMOKE_PISTOL, ent->attachment[0], forward, 80, 0.45, 10, 10, 10, false, velocity);
			}
		}
	}

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/cheytaclrrs-1.wav", VOL_NORM, 0.28, 0, 94 + Com_RandomLong(0, 0xf));

	EV_GetGunPosition(args, vecSrc, origin);

	vSpread[0] = args->fparam1;
	vSpread[1] = args->fparam2;
	vSpread[2] = 0;

	VectorCopy(forward, vecAiming);
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_338MAG,
		9 );
}

}
