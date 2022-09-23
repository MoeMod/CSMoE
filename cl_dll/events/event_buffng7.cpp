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

namespace cl::event::buffng7 {


void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
{
	cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);

	if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
	{
		ent->entity.curstate.renderamt = 0;
		ent->die = currenttime;
	}
}

enum buffng7_e
{
	ANIM_IDLE1 = 0,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_B_MODE,
	ANIM_A_MODE,
	ANIM_IDLE_B,
	ANIM_SHOOT1_B,
	ANIM_SHOOT2_B,
	ANIM_RELOAD_B,
	ANIM_DRAW_B,
};


static const char* SOUNDS_NAME[] =
{
	"weapons/buffng7-1.wav",
	"weapons/buffng7-2.wav",
	"weapons/buffng7-3.wav",
};

void EV_FireBUFFNG7(event_args_s *args)
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

	if (EV_IsLocal(idx))
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		if (!righthand)
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, -13.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, 13.0, 0);
		}

		VectorCopy(ent->attachment[1], ShellOrigin);
		ShellVelocity[2] -= 75;
	}
	else
	{
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], g_iRShell, args->bparam2 ? TE_BOUNCE_SHOTSHELL : TE_BOUNCE_SHELL);

	if (!args->bparam2)
	{
		if (EV_IsLocal(idx))
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(ANIM_SHOOT1, ANIM_SHOOT2), 2);

			if (gHUD.cl_gunsmoke->value)
			{
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.45, 15, 15, 15, 3, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.35, 9, 9, 9, 2, velocity, false, 35);
			}
		}

		PLAY_EVENT_SOUND(SOUNDS_NAME[0]);

		EV_GetGunPosition(args, vecSrc, origin);
		VectorCopy(forward, vecAiming);
		Vector vSpread;

		vSpread.x = args->fparam1;
		vSpread.y = args->fparam2;
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, vecAiming,
			vSpread, 8192.0, BULLET_PLAYER_BUFFNG7_762MM,
			2);
	}
	else
	{
		if (EV_IsLocal(idx))
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(ANIM_SHOOT1_B, ANIM_SHOOT2_B), 2);

			if (gHUD.cl_gunsmoke->value)
			{
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 3, 0.45, 15, 15, 15, 3, velocity, false, 35);
				EV_HLDM_CreateSmoke(ent->attachment[0], forward, 40, 0.35, 9, 9, 9, 2, velocity, false, 35);
			}
		}

		if (args->bparam1)
		{
			int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_buffng7_charging_beam.spr");
		
			TEMPENTITY* ReloadModel = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 7, 1.0, iBeamModelIndex, 30, 255, kRenderTransAdd, 0.08, 30, FTENT_CLIENTCUSTOM, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);
			if (ReloadModel)
				ReloadModel->callback = ResetMuzz;

			Vector vecBeamStart, vecBeamEnd;

			int iBeamIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_buffng7_charging_shot.spr");

			if (EV_IsLocal(idx))
			{
				vecBeamStart = ent->attachment[7];
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
			}

			vecBeamEnd = vecBeamStart + forward * 3000;

			gEngfuncs.pEfxAPI->R_BeamPoints_Stretch(vecBeamStart, vecBeamEnd, iBeamIndex, 1.0, 20.0, 255, 0, 50.0, 255, 255, 255);

			PLAY_EVENT_SOUND(SOUNDS_NAME[2]);
		}		
		else
		{
			PLAY_EVENT_SOUND(SOUNDS_NAME[1]);

			EV_GetGunPosition(args, vecSrc, origin);
			VectorCopy(forward, vecAiming);
			Vector vSpread;

			vSpread.x = args->fparam1;
			vSpread.y = args->fparam2;
			EV_HLDM_FireBullets(idx,
				forward, right, up,
				args->iparam2, vecSrc, vecAiming,
				vSpread, 8192.0, BULLET_PLAYER_BUFFNG7_BUCKSHOT,
				1);
		}
		
	}
}

}
