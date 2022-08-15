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

namespace cl::event::lockongun {


void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);

	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
	}
}

enum lockongun_e
{
	ANIM_IDLE1 = 0,
	ANIM_IDLE_ON,
	ANIM_SHOOTA,
	ANIM_SHOOTA_ON,
	ANIM_RELOAD,
	ANIM_RELOAD_ON,
	ANIM_DRAW,
	ANIM_DRAW_ON,
	ANIM_SCOPE_ON,
	ANIM_ZOOM_IN,
	ANIM_ZOOM_IDLE,
	ANIM_ZOOM_OUT,
	ANIM_SHOOTB,
	ANIM_SHOOTB_ON,
};

void EV_FireLockOnGun(event_args_s *args)
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
	int righthand = gHUD.cl_righthand->value;

	ent = gEngfuncs.GetViewModel();

	if (args->iparam1 == 2)
	{
		if (EV_IsLocal(args->entindex))
		{
			int iSmokeRModelIndex, iSmokeLModelIndex;
			int iAttachmentR, iAttachmentL;
			if (!righthand)
			{		
				iAttachmentL = 2;
				iAttachmentR = 3;
				iSmokeLModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lockongun_smokel_left.spr");
				iSmokeRModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lockongun_smoker_left.spr");
			}
			else
			{
				iAttachmentL = 2;
				iAttachmentR = 3;
				iSmokeLModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lockongun_smokel.spr");
				iSmokeRModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lockongun_smoker.spr");
			}
			

			TEMPENTITY* SmokeLModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, iAttachmentL, 1.2, iSmokeLModelIndex, 30, 255, kRenderTransAdd, 0.15, 32, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

			TEMPENTITY* iSmokeRModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, iAttachmentR, 1.2, iSmokeRModelIndex, 30, 255, kRenderTransAdd, 0.15, 32, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

		}

		PLAY_EVENT_SOUND("weapons/lockongun_shootb.wav");

		return;
	}

	if (args->iparam1 == 3)
	{
		if (EV_IsLocal(args->entindex))
		{
			int iReloadModelIndex;
			int iAttachment;
			if (!righthand)
			{
				iAttachment = 1;
				iReloadModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lockongun_reload_left.spr");

			}
			else
			{
				iAttachment = 1;
				iReloadModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lockongun_reload.spr");
			}

			TEMPENTITY* ReloadModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, iAttachment, 0.8, iReloadModelIndex, 28, 255, kRenderTransAdd, 0.05, 16, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if(ReloadModel)
				ReloadModel->callback = ResetMuzz;
		}
		return;
	}

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;
		
		if (args->bparam2)
		{
			if (args->bparam1)
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_ZOOM_IDLE, 2);
			else
				gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTA_ON, 2);
		}
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTA, 2);
		}

		EV_MuzzleFlash();
		if( !righthand)
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, -13.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, 13.0, 0);
		}

		VectorCopy(ent->attachment[1], ShellOrigin);
		ShellVelocity[2] -= 75;
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}


	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND("weapons/lockongun-1.wav");

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_556MM,
		3 );
}

}
