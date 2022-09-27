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
#include "r_studioint.h"
#include "studio.h"

namespace cl {
	extern engine_studio_api_t IEngineStudio;
}

namespace cl::event::wondercannon {
#if 0
	TEMPENTITY* g_pWonderCannonParticles[4];

	void ResetMuzz(struct tempent_s* ent, float frametime, float currenttime)
	{
		cl_entity_t* parent = gEngfuncs.GetEntityByIndex(ent->entity.curstate.aiment);
		int index = ent->entity.curstate.iuser1;

		if (ent->entity.curstate.weaponmodel != parent->curstate.weaponmodel)
		{
			ent->entity.curstate.renderamt = 0;
			ent->die = currenttime;
			g_pWonderCannonParticles[index] = nullptr;
		}
	}
#endif

	enum wondercannon_e
	{
		WONDERCANNON_IDLE1,
		WONDERCANNON_RELOAD,
		WONDERCANNON_DRAW,
		WONDERCANNON_SHOOT1,
		WONDERCANNON_SHOOT2,
		WONDERCANNON_SHOOT3,
		WONDERCANNON_BMOD_ON,
		WONDERCANNON_CMOD_START,
		WONDERCANNON_CMOD_IDLE,
		WONDERCANNON_CMOD_SHOOT
	};


	void EV_FireWonderCannon(event_args_s* args)
	{
		vec3_t ShellVelocity, ShellOrigin;
		vec3_t vecSrc, vecAiming;

		int    idx = args->entindex;
		Vector origin(args->origin);
		Vector angles(
			args->iparam1 / 100.0f + args->angles[0],
			args->iparam2 / 100.0f + args->angles[1],
			args->angles[2]
		);
		Vector velocity(args->velocity);
		Vector forward, right, up;
		AngleVectors(angles, forward, right, up);
		cl_entity_t* ent;

		ent = gEngfuncs.GetViewModel();

		if (EV_IsLocal(idx))
		{
#if 0
			if (args->bparam2)
			{
				if (!args->bparam1)
				{
					for (int i = 0; i <= 3; i++)
					{
						if (g_pWonderCannonParticles[i])
						{
							g_pWonderCannonParticles[i]->die = gHUD.m_flTime;
							g_pWonderCannonParticles[i] = nullptr;
						}
					}
					return;
				}

				int iMuzzModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_wondercannon_bomb_set.spr");

				for (int i = 0; i <= 3; i++)
				{
					if (!g_pWonderCannonParticles[i])
						g_pWonderCannonParticles[i] = gEngfuncs.pEfxAPI->R_AttachTentToModel(ent->index, 8 + i, 990, iMuzzModelIndex, 24, 210, kRenderTransAdd, 0.01, 0, FTENT_CLIENTCUSTOM | FTENT_SPRANIMATELOOP, EFLAG_DEPTH_CHANGED | EFLAG_AFTER_VIEWMODEL);

					if (g_pWonderCannonParticles[i])
					{
						g_pWonderCannonParticles[i]->entity.curstate.frame = Com_RandomLong(0, 5);
						g_pWonderCannonParticles[i]->callback = ResetMuzz;
						g_pWonderCannonParticles[i]->entity.curstate.iuser1 = i;
					}
				}
				return;
			}
#endif
			if (args->bparam1)
			{
				++g_iShotsFired;
				EV_MuzzleFlash();

				gEngfuncs.pEventAPI->EV_WeaponAnimation(Com_RandomLong(WONDERCANNON_SHOOT1, WONDERCANNON_SHOOT3), 2);
			}

			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/wondercannon-1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

			EV_GetGunPosition(args, vecSrc, origin);
			VectorCopy(forward, vecAiming);
			Vector vSpread;

			vSpread.x = args->fparam1;
			vSpread.y = args->fparam2;
			EV_HLDM_FireBullets(idx,
				forward, right, up,
				1, vecSrc, vecAiming,
				vSpread, 8192.0, BULLET_PLAYER_556MM,
				2);
		}
	}
}