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

namespace cl::event::malorian3516 {

enum malorian3516_e
{
	ANIM_IDLE1,
	ANIM_SHOOT0,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOT3,
	ANIM_SHOOT4,
	ANIM_RELOAD0,
	ANIM_RELOAD0_EMPTY,
	ANIM_RELOAD1,
	ANIM_RELOAD1_EMPTY,
	ANIM_RELOAD2,
	ANIM_RELOAD2_EMPTY,
	ANIM_DRAW,
	ANIM_MELEE,
	ANIM_WALK,
};


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

void EV_CannonFlames(Vector vecSrc, Vector vecDir, Vector vecVelocity, int idx, int modelIndex)
{
	int model = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr");

	if (modelIndex)
		model = modelIndex;

	for (int i = 0; i < 12; i++)
	{
		TEMPENTITY* ent = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, model, 20);

		if (!ent)
			break;

		ent->flags |= (FTENT_CLIENTCUSTOM | FTENT_COLLIDEALL);

		ent->entity.angles.x = Com_RandomFloat(-512.0, 511.0);
		ent->entity.angles.y = Com_RandomFloat(-256.0, 255.0);
		ent->entity.angles.z = Com_RandomFloat(-256.0, 255.0);

		ent->flags |= FTENT_ROTATE;

		ent->entity.curstate.scale = Com_RandomFloat(0.2, 0.5);
		ent->entity.baseline.fuser2 = Com_RandomFloat(1.5, 2.0);
		ent->callback = SpriteGrow;
		ent->hitcallback = SpriteBlocked;

		ent->entity.curstate.rendermode = kRenderTransAdd;
		ent->entity.curstate.renderamt = 150;
		ent->entity.curstate.renderfx = kRenderFxFadeSlow;
		ent->entity.curstate.rendercolor.r = ent->entity.curstate.rendercolor.g = ent->entity.curstate.rendercolor.b = 255;

		int degree = 10 * (Com_RandomLong(0, 4) - 2);
		float c = cos(degree * M_PI / 180.0);
		float s = sin(degree * M_PI / 180.0);

		ent->entity.origin = vecSrc;

		ent->entity.baseline.origin.x = c * vecDir.x - s * vecDir.y;
		ent->entity.baseline.origin.y = s * vecDir.x + c * vecDir.y;
		ent->entity.baseline.origin.z = vecDir.z;

		ent->entity.baseline.origin.x *= Com_RandomLong(130, 400);
		ent->entity.baseline.origin.y *= Com_RandomLong(130, 400);
		ent->entity.baseline.origin.z *= Com_RandomLong(30, 200);

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
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
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

		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_MELEE, 2);

		int iMuzzModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke_thanatos9.spr");

		float framerate = Com_RandomLong(30, 35);
		float scale = Com_RandomFloat(0.2, 0.3);
		int brightness = Com_RandomLong(200, 230);

		gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 3, 0.7, iMuzzModelIndex, framerate, brightness, kRenderTransAdd, scale, 17, FTENT_SPRANIMATE, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);


		EV_MuzzleFlash();

		if (!Com_RandomLong(0, 5))
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.26, 15, 15, 15, 3, velocity, true, 35);
	}

	EV_GetGunPosition(args, vecSrc, origin);

	if (EV_IsLocal(idx))
	{
		vecSrc = ent->attachment[2] + forward * 7.0 + velocity * 0.015;
	}
	else
	{
		vecSrc = vecSrc + forward * 40.0 - up * (args->ducking ? 3.0 : 9.0);
	}

	EV_CannonFlames(vecSrc, forward, velocity, idx, modelIndex);

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

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, szSound, 1.5, ATTN_NORM, 0, 94 + Com_RandomLong(0, 15));
}

void EV_FireMalorian3516(event_args_s *args)
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;

	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );
	cl_entity_t* ent;
	Vector vecBeamStart;
	Vector vecBeamEnd;

	ent = gEngfuncs.GetViewModel();
	int shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell762.mdl");

	if (args->bparam1)
	{
		EV_FireCannonCommon(args, "weapons/malorian_arms_3516/malorian_powerdown_1.wav");
	}
	else
	{
		if (EV_IsLocal(idx))
		{
			++g_iShotsFired;

			//gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(ANIM_SHOOT0, ANIM_SHOOT4), 2);
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT0, 2);

			EV_MuzzleFlash();

			if (!gHUD.cl_righthand->value)
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, -13.0, 0);
			}
			else
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, 13.0, 0);
			}

			VectorCopy(ent->attachment[1], ShellOrigin);
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
		}

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

		int iMuzzModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke_ia.spr");

		float framerate = Com_RandomLong(30, 35);
		float scale = Com_RandomFloat(0.05, 0.1);
		int brightness = Com_RandomLong(150, 180);

		//gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 3, 1.0, iMuzzModelIndex, framerate, brightness, kRenderTransAdd, scale, 30, FTENT_SPRANIMATE, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

		gEngfuncs.pEfxAPI->R_TempCustomModel(ent->attachment[3], { 0, 0, 0 }, { 0, 0, 0 }, 1.0, iMuzzModelIndex, 0, framerate, false, brightness, kRenderTransAdd, 0, 0, false, 0, scale, 30, FTENT_SPRANIMATE);

		framerate = Com_RandomLong(30, 35);
		scale = Com_RandomFloat(0.05, 0.1);
		brightness = Com_RandomLong(150, 180);

		//gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 4, 1.0, iMuzzModelIndex, framerate, brightness, kRenderTransAdd, scale, 30, FTENT_SPRANIMATE, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

		gEngfuncs.pEfxAPI->R_TempCustomModel(ent->attachment[4], {0, 0, 0}, {0, 0, 0}, 1.0, iMuzzModelIndex, 0, framerate, false, brightness, kRenderTransAdd, 0, 0, false, 0, scale, 30, FTENT_SPRANIMATE);
		
		if(args->bparam2)
			PLAY_EVENT_SOUND("weapons/malorian_arms_3516/malorian-1.wav");
		else
			PLAY_EVENT_SOUND("weapons/malorian_arms_3516/malorian-2.wav");

		EV_GetGunPosition(args, vecSrc, origin);
		VectorCopy(forward, vecAiming);
		Vector vSpread;

		vSpread.x = args->fparam1;
		vSpread.y = args->fparam2;


		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_50AE,
			4);
	}
}

}
