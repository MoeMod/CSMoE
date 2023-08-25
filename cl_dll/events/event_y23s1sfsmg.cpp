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

	extern float g_flY23S1SFSMGBeamTime[33];
	extern BEAM* g_pY23S1SFSMGBeams[33];
	extern TEMPENTITY* g_pY23S1SFSMGParticles[33];
}

namespace cl::event::y23s1sfsmg {

void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);
	int index = ent->entity.curstate.iuser1;
	float die = ent->entity.curstate.fuser1;

	if (die <= currenttime)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
		g_pY23S1SFSMGParticles[index] = nullptr;
	}

	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
		g_pY23S1SFSMGParticles[index] = nullptr;
	}
}

enum y23s1sfsmg_e
{
	ANIM_IDLE1,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOTB,
	ANIM_SHOOTB_END,
};

void EV_FireY23S1SFSMG(struct event_args_s* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecAiming;
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

	if (!EV_IsLocal(idx))
	{
		cl_entity_t* pl = gEngfuncs.GetEntityByIndex(idx);

		Vector vecForward, vecRight, vecUp;

		AngleVectors(pl->angles, vecForward, vecRight, vecUp);

		Vector vecDelta = vecForward - forward;

	}

	if (!args->bparam2)
	{
		if (EV_IsLocal(args->entindex))
		{
			++g_iShotsFired;
			gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT2), 2);
			EV_MuzzleFlash();
			if (!gHUD.cl_righthand->value)
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, -11.0, 0);
			}
			else
			{
				EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 34.0, -10.0, 11.0, 0);
			}
			ShellVelocity[2] -= 50;

			VectorCopy(ent->attachment[1], ShellOrigin);
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
		}

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iPShell, TE_BOUNCE_SHELL);

		PLAY_EVENT_SOUND("weapons/y23s1sfsmg-1.wav");

		EV_GetGunPosition(args, vecSrc, origin);
		VectorCopy(forward, vecAiming);
		Vector vSpread;

		vSpread.x = args->fparam1;
		vSpread.y = args->fparam2;
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_45ACP,
			1);

		return;
	}

	EV_GetGunPosition(args, vecSrc, origin);

	vecEnd = vecSrc + forward * 8192;

	iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_y23s1sfsmg_laser.spr");
	iMuzzModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_y23s1sfsmg_laser_hit.spr");

	if (EV_IsLocal(idx))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTB, 2);
	}

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	gEngfuncs.pEventAPI->EV_PushPMStates();
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);
	gEngfuncs.pEventAPI->EV_PopPMStates();

	int sModelMuzzleflash = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/muzzleflash289.spr");

	TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempCustomModel(tr.endpos, { 0, 0, 0 }, { 0, 0, 0 }, 0.3, sModelMuzzleflash, 0, 30, false, 225, kRenderTransAdd, 0, 0, false, 0, 0.08, 11, FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP, 0, 0);

	if (g_flY23S1SFSMGBeamTime[idx] >= gHUD.m_flTime)
	{
		if (g_pY23S1SFSMGBeams[idx])
		{
			g_flY23S1SFSMGBeamTime[idx] = gHUD.m_flTime + 0.15;

			g_pY23S1SFSMGBeams[idx]->die = gHUD.m_flTime + 0.15;
		}

		if (EV_IsLocal(idx))
		{
			if (g_pY23S1SFSMGParticles[idx])
				g_pY23S1SFSMGParticles[idx]->entity.curstate.fuser1 = gHUD.m_flTime + 0.15;
		}

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/y23s1sfsmg-2_loop.wav", 1.0, ATTN_NORM, (1 << 7), PITCH_NORM);
	}
	else
	{
		g_pY23S1SFSMGBeams[idx] = gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, tr.endpos, iBeamModelIndex, 0.15, 1.2, 0, 1.0, 5, 0, 10, 1.0, 1.0, 1.0);

		if (g_pY23S1SFSMGBeams[idx])
			g_flY23S1SFSMGBeamTime[idx] = gHUD.m_flTime + 0.15;
		if (EV_IsLocal(idx))
		{
			g_pY23S1SFSMGParticles[idx] = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 0, 99, iMuzzModelIndex, 24, 255, kRenderTransAdd, 0.05, 10, FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP, 0);

			if (g_pY23S1SFSMGParticles[idx])
			{
				g_pY23S1SFSMGParticles[idx]->callback = ResetMuzz;
				g_pY23S1SFSMGParticles[idx]->entity.curstate.iuser1 = idx;
				g_pY23S1SFSMGParticles[idx]->entity.curstate.fuser1 = gHUD.m_flTime + 0.15;
			}
		}

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/y23s1sfsmg-2_loop.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
}


};