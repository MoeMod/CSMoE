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

namespace cl::event::voidpistol {

extern engine_studio_api_t IEngineStudio;

enum voidpistol_e
{
	VOIDPISTOL_IDLEA,
	VOIDPISTOL_IDLEB,
	VOIDPISTOL_IDLEC,
	VOIDPISTOL_SHOOTA,
	VOIDPISTOL_SHOOTB,
	VOIDPISTOL_SHOOTC,
	VOIDPISTOL_SHOOT_BLACKHOLE_A,
	VOIDPISTOL_SHOOT_BLACKHOLE_B,
	VOIDPISTOL_RELOADA,
	VOIDPISTOL_RELOADB,
	VOIDPISTOL_RELOADC,
	VOIDPISTOL_SCANNING_ON,
	VOIDPISTOL_SCANNING_OFF,
	VOIDPISTOL_CHANGEAC,
	VOIDPISTOL_CHANGEBC,
	VOIDPISTOL_DRAWA,
	VOIDPISTOL_DRAWB,
	VOIDPISTOL_DRAWC
};

enum blackhole_anim
{
	BLACKHOLE_START,
	BLACKHOLE_LOOP,
	BLACKHOLE_END
};


void EV_FireVoidpistol( event_args_t *args )
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector velocity( args->velocity );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
	);
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		
		if((args->iparam2))
			gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? VOIDPISTOL_SHOOTC : VOIDPISTOL_SHOOTA, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(args->bparam1 ? VOIDPISTOL_SHOOTC : VOIDPISTOL_SHOOTB, 2);

		if (!gHUD.cl_righthand->value)
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, 0);
		}

			EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iPShell, TE_BOUNCE_SHELL);
			if (args->fparam1 == 2.0)
			{
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/voidpistol-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, "weapons/voidpistol_beep.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
			else
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/voidpistol-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			if (args->iparam1)
			{
				EV_GetGunPosition(args, vecSrc, origin);
				Vector vSpread(args->fparam1, args->fparam2, 0.0f);

				EV_HLDM_FireBullets(idx,
					forward, right, up,
					1, vecSrc, forward,
					vSpread, 8192.0, BULLET_PLAYER_50AE,
					2);
			}
		
	}
	else
	{
		if (args->bparam2)
		{
			switch (args->iparam2)
			{
			case BLACKHOLE_START:
			{
				//blackhole_projectile spr
				struct model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_blackhole_start.spr", false);
				if (pModel)
				{
					gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/voidpistol_blackhole_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
					pEnt->entity.curstate.frame = 0;
					pEnt->entity.curstate.framerate = 30.0;
					
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.scale = 0.5;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->die = gHUD.m_flTime + 1.5;
					pEnt->flags |= FTENT_SPRANIMATE;
				}
				break;
			}
			case BLACKHOLE_LOOP:
			{
				//blackhole_projectile spr
				struct model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_blackhole_loop.spr", false);
				if (pModel)
				{
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
					pEnt->entity.curstate.frame = 0;
					pEnt->entity.curstate.framerate = 30.0;
					
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.scale = 0.5;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->die = gHUD.m_flTime + 4.0;
					pEnt->flags |= FTENT_SPRANIMATELOOP | FTENT_SPRANIMATE;
				}
				break;
			}
			case BLACKHOLE_END:
			{
				struct model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_blackhole_end.spr", false);
				if (pModel)
				{
					gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/voidpistol_blackhole_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
					pEnt->entity.curstate.frame = 0;
					pEnt->entity.curstate.framerate = 30.0;
					
					pEnt->entity.curstate.scale = 0.5;
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->die = gHUD.m_flTime + 1.53;
					pEnt->flags |= FTENT_SPRANIMATE;

				}
				break;
			}
			default:
			{
				//blackhole_projectile spr
				struct model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_blackhole_projectile.spr", false);
				if (pModel)
				{
					
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
					pEnt->entity.curstate.frame = 0;
					pEnt->entity.curstate.framerate = 30.0;
					pEnt->entity.curstate.vuser1 = { args->angles[0] ,args->angles[1] ,args->angles[2] };
					pEnt->entity.curstate.scale = 0.1;
					pEnt->clientIndex = idx;
					pEnt->entity.curstate.rendermode = kRenderTransAdd;
					pEnt->entity.curstate.renderamt = 255;

					pEnt->frameMax = 30;
					pEnt->tentOffset = Vector(0, 0, 0);
					pEnt->die = gHUD.m_flTime + 4.0;
					pEnt->callback = [](tempent_s* ent, float frametime, float currenttime){
					
						float	traceFraction = 1;
						vec3_t	traceNormal;
						pmtrace_t pmtrace;
						physent_t* pe;
						Vector vecForward = ent->entity.curstate.vuser1;
						gEngfuncs.pEventAPI->EV_SetTraceHull(2);
						gEngfuncs.pEventAPI->EV_PlayerTrace(ent->entity.prevstate.origin, ent->entity.origin + vecForward * 40, PM_STUDIO_BOX, -1, &pmtrace);

						if (pmtrace.fraction != 1)
						{
							pe = gEngfuncs.pEventAPI->EV_GetPhysent(pmtrace.ent);

							if (!pmtrace.ent || (pe->info != ent->clientIndex))
							{
								traceFraction = pmtrace.fraction;
								VectorCopy(pmtrace.plane.normal, traceNormal);

								if (ent->hitcallback)
								{
									(*ent->hitcallback)(ent, &pmtrace);
								}
							}
						}
					};
					pEnt->hitcallback = [](tempent_s* ent, pmtrace_t* tr)
					{
						ent->flags &= ~FTENT_FADEOUT;
						ent->die = gHUD.m_flTime;
					};

					pEnt->flags |= FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP | FTENT_SPRANIMATE | FTENT_CLIENTCUSTOM;
				}
				break;
			}
			}

		}
	}
	

	
}

}
