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

namespace cl::event::laserfist {

void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);

	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
	}
}

enum laserfist_e
{
	ANIM_IDLE1 = 0,
	ANIM_SHOOTA_EMPTY_LOOP,
	ANIM_SHOOTA_EMPTY_END,
	ANIM_SHOOTA_LOOP,
	ANIM_SHOOTA_END,
	ANIM_SHOOTB_READY,
	ANIM_SHOOTB_LOOP,
	ANIM_SHOOTB_SHOOT,
	ANIM_RELOAD,
	ANIM_DRAW,
};

void EV_FireLaserFist(event_args_s *args)
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
	cl_entity_t* ent;
	Vector vecBeamStartL, vecBeamStartR;
	Vector vecBeamEnd;

	int righthand = gHUD.cl_righthand->value;

	ent = gEngfuncs.GetViewModel();

	AngleVectors( angles, forward, right, up );

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);

	if (args->fparam2 == 2.0)
	{
		if (EV_IsLocal(args->entindex))
		{
			int iSmokeRModelIndex, iSmokeLModelIndex;
			int iAttachmentR, iAttachmentL;
			iAttachmentL = 5;
			iAttachmentR = 4;
				
			iSmokeLModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_laserfist_charge_left.spr");
			iSmokeRModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_laserfist_charge.spr");
			
			TEMPENTITY* SmokeLModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, iAttachmentL, 1.0, iSmokeLModelIndex, 30, 255, kRenderTransAdd, 0.05, 24, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

			TEMPENTITY* iSmokeRModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, iAttachmentR, 1.0, iSmokeRModelIndex, 30, 255, kRenderTransAdd, 0.05, 24, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

		}

		return;
	}

	if (args->bparam2)
	{
		pmtrace_t tr;

		vecBeamEnd = vecSrc + (forward + right * args->fparam1 + up * args->fparam2) * 8192;

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecBeamEnd, PM_NORMAL, -1, &tr);

		vecBeamEnd = tr.endpos;
	}
	else
	{
		vecBeamEnd = vecSrc + (forward + right * args->fparam1 + up * args->fparam2) * 50;
	}
	
	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();

		if (args->bparam2)
		{
			vecBeamStartL = ent->attachment[3];

			vecBeamStartR = ent->attachment[2];
		}
		else
		{
			vecBeamStartL = ent->attachment[Com_RandomLong(11, 13)];

			vecBeamStartR = ent->attachment[Com_RandomLong(7, 9)];
		}
	}
	else
	{
		if (args->ducking)
		{
			vecBeamStartR = vecSrc + forward * 40 - up * 3;
			vecBeamStartL = vecSrc + forward * 40 - up * 3;
		}
		else
		{
			vecBeamStartR = vecSrc + forward * 40 - up * 9;
			vecBeamStartL = vecSrc + forward * 40 - up * 3;
		}

	}

	if (args->bparam2)
	{
		int iModel;

		iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_laserfist_laserbeam.spr");

		BEAM* pBeamR;

		pBeamR = gEngfuncs.pEfxAPI->R_BeamPoints_Stretch(vecBeamStartR, vecBeamEnd, iModel, 1.0, 10.0, 600, 0, 15.0, 255, 255, 255);


		if (pBeamR)
			pBeamR->flags |= FBEAM_SHADEOUT;

		BEAM* pBeamL;

		pBeamL = gEngfuncs.pEfxAPI->R_BeamPoints_Stretch(vecBeamStartL, vecBeamEnd, iModel, 1.0, 10.0, 600, 0, 15.0, 255, 255, 255);


		if (pBeamL)
			pBeamL->flags |= FBEAM_SHADEOUT;

		PLAY_EVENT_SOUND("weapons/laserfist_shootb-1.wav");
	}	
	else
	{
		int iModelLaser;

		iModelLaser = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_laserfist_laser.spr");

		Vector vecStart;
		vecStart = args->bparam1 ? vecBeamStartL : vecBeamStartR;

		vecBeamEnd = vecStart + (forward + right * args->fparam1 + up * args->fparam2) * 50;

		BEAM* pBeam = gEngfuncs.pEfxAPI->R_BeamPoints_Tracer(vecStart, vecBeamEnd, iModelLaser, 5.0, 8.0, 60.0f, 0.0f, 255.0f, 1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

		Vector vSpread(args->fparam1, args->fparam2, 0.0f);
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_9MM,
			1);

		PLAY_EVENT_SOUND("weapons/laserfist_shoota-1.wav");
	}
		
}

}
