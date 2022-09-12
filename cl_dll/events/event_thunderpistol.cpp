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

namespace cl::event::thunderpistol {
	

void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);

	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
	}
}

enum thunderpistol_e
{
	ANIM_IDLE = 0,
	ANIM_SHOOTA,
	ANIM_SHOOTB,
	ANIM_RELOAD,
	ANIM_DRAW,
};

void EV_FireThunderPistol( event_args_t *args )
{
	vec3_t vecSrc, vecAiming;

	int idx = args->entindex;
	Vector origin(args->origin);
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
	);
	Vector forward, right, up;

	cl_entity_t* ent;
	Vector vecBeamStart;
	Vector vecBeamEnd;

	int righthand = gHUD.cl_righthand->value;
	ent = gEngfuncs.GetViewModel();

	if (args->bparam2)
	{
		if (EV_IsLocal(idx))
		{
			int iTransformModelIndex;
			if (!righthand)
			{
				iTransformModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_thunderpistol_transform_left.spr");

			}
			else
			{
				iTransformModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_thunderpistol_transform.spr");
			}

			TEMPENTITY* pTransformModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 1, 1.0, iTransformModelIndex, 45, 255, kRenderTransAdd, 0.03, 45, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if (pTransformModel)
				pTransformModel->callback = ResetMuzz;


			return;
		}
	}

	AngleVectors(angles, forward, right, up);
	EV_GetGunPosition(args, vecSrc, origin);

	if (EV_IsLocal(idx))
	{
		vecBeamStart = ent->attachment[0];
		if (args->bparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTB, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTA, 2);

		vecBeamEnd = vecBeamStart + (forward + right * args->fparam1 + up * args->fparam2) * 8192;
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

	int iModel;

	if (args->bparam1)
		iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_thunderpistol_laserbeamb.spr");
	else
		iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_thunderpistol_laserbeam.spr");

	BEAM* pBeam;

	pBeam = gEngfuncs.pEfxAPI->R_BeamPoints(vecBeamStart, vecBeamEnd,
		iModel,
		1.0,
		20.0,
		0.0,
		500.0,
		0.0,
		0,
		60.0,
		1.0,
		1.0,
		1.0
	);

	if (pBeam)
		pBeam->flags |= FBEAM_FADEIN | FBEAM_FADEOUT | FBEAM_SHADEOUT;

	if (args->bparam1)
		PLAY_EVENT_SOUND("weapons/thundergun-1-1.wav");
	else
		PLAY_EVENT_SOUND("weapons/thundergun-1.wav");

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy(forward, vecAiming);
	Vector vSpread;

	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, vSpread, 8192, BULLET_PLAYER_338MAG, 0, &tracerCount[idx - 1], args->bparam1 ? 1 : 10);
}

}
