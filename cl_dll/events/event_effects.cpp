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

enum event_effect
{
	EVENT_GATLINGRX,
	EVENT_DUALSWORD,
	EVENT_VOIDPISTOL
};

namespace cl {

extern engine_studio_api_t IEngineStudio;

static const char* SOUNDS_NAME[] =
{
	"weapons/gatlingex-1.wav", "weapons/gatlingex-2.wav"
};

void EV_WpnEffects(event_args_s* args)
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
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

		switch (args->iparam1)
		{
		case EVENT_GATLINGRX:
		{
			vec3_t vStartOrigin;
			VectorCopy(args->origin, vStartOrigin);

			for (int i = 0; i < 2; i++)
			{
				float flspeed = gEngfuncs.pfnRandomFloat(5.0, 80.0);
				vec3_t vEndOrigin, vBalOrigin, vVelocity;
				VectorCopy(vStartOrigin, vEndOrigin);
				vEndOrigin[0] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);//Caculate velocity
				vEndOrigin[1] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);
				vEndOrigin[2] += gEngfuncs.pfnRandomFloat(-50.0, 500.0);
				VectorSubtract(vEndOrigin, vStartOrigin, vBalOrigin);
				VectorScale(vBalOrigin, sqrt(flspeed * flspeed / (vBalOrigin[0] * vBalOrigin[0] + vBalOrigin[1] * vBalOrigin[1] + vBalOrigin[2] * vBalOrigin[2])), vVelocity);

				model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_gatlingex_star.spr", false);
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
				pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.baseline.origin = vVelocity;
				pEnt->entity.curstate.framerate = 2.0;
				pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.11, 0.13);
				pEnt->die = gHUD.m_flTime + gEngfuncs.pfnRandomFloat(1.2, 2.2);

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					float gravity = -frametime * 140.0;
					pEnt->entity.baseline.origin[2] += gravity;
				};

				pEnt->hitcallback = [](tempent_s* pEnt, pmtrace_s* ptr) {
					pEnt->entity.baseline.origin = Vector(0, 0, 0);
				};
			}

			break;
		}
		case EVENT_DUALSWORD:
		{
				vec3_t vStartOrigin;
				VectorCopy(args->angles, angles);
				VectorCopy(args->origin, vStartOrigin);

				const char* szSprite;
				if (args->bparam1)
				{
					if (args->bparam2)
						szSprite = "sprites/leaf01_dualsword.spr";
					else
						szSprite = "sprites/leaf02_dualsword.spr";
				}
				else
				{
					if (args->bparam2)
						szSprite = "sprites/petal01_dualsword.spr";
					else
						szSprite = "sprites/petal02_dualsword.spr";
				}



				for (int i = 0; i < 10; i++)
				{
					angles[0] = gEngfuncs.pfnRandomFloat(-90.0, 90.0);
					angles[1] = gEngfuncs.pfnRandomFloat(-90.0, 90.0);
					angles[2] = gEngfuncs.pfnRandomFloat(-90.0, 90.0);
					float flspeed = gEngfuncs.pfnRandomFloat(5.0, 80.0);
					vec3_t vEndOrigin, vBalOrigin, vVelocity;
					VectorCopy(vStartOrigin, vEndOrigin);
					vEndOrigin[0] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);//Caculate velocity
					vEndOrigin[1] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);
					vEndOrigin[2] += gEngfuncs.pfnRandomFloat(-50.0, 500.0);
					VectorSubtract(vEndOrigin, vStartOrigin, vBalOrigin);
					VectorScale(vBalOrigin, sqrt(flspeed * flspeed / (vBalOrigin[0] * vBalOrigin[0] + vBalOrigin[1] * vBalOrigin[1] + vBalOrigin[2] * vBalOrigin[2])), vVelocity);

					model_s* pModel = IEngineStudio.Mod_ForName(szSprite, false);
					TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(vStartOrigin, pModel);
					pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;
					pEnt->entity.curstate.renderamt = 255;
					pEnt->entity.curstate.rendermode = kRenderNormal;
					pEnt->entity.baseline.origin = vVelocity;
					pEnt->entity.angles = angles;
					pEnt->entity.curstate.framerate = 2.0;
					pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.11, 0.13);
					pEnt->die = gHUD.m_flTime + gEngfuncs.pfnRandomFloat(1.2, 2.2);
					/*gEngfuncs.Con_Printf("vVelocity is x= %f ,y= %f ,z=%f\n", vVelocity[0], vVelocity[1], vVelocity[2]);
					gEngfuncs.Con_Printf("angles is x= %f ,y= %f ,z=%f\n", angles[0], angles[1], angles[2]);*/

					pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
						float gravity = -frametime * 96.0;
						pEnt->entity.baseline.origin[2] += gravity;
						if (pEnt->entity.baseline.origin[1])
							pEnt->entity.angles -= Vector(6.0, 6.0, 6.0);
					};
					pEnt->hitcallback = [](tempent_s* pEnt, pmtrace_s* ptr) {
						pEnt->entity.baseline.origin = Vector(0, 0, 0);

					};
				}

			
			break;
		}
		case EVENT_VOIDPISTOL:
		{
			vec3_t vStartOrigin;
			VectorCopy(args->origin, vStartOrigin);

			for (int i = 0; i < 3; i++)
			{
				float flspeed = gEngfuncs.pfnRandomFloat(5.0, 80.0);
				vec3_t vEndOrigin, vBalOrigin, vVelocity;
				VectorCopy(vStartOrigin, vEndOrigin);
				vEndOrigin[0] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);//Caculate velocity
				vEndOrigin[1] += gEngfuncs.pfnRandomFloat(-400.0, 400.0);
				vEndOrigin[2] += gEngfuncs.pfnRandomFloat(-50.0, 500.0);
				VectorSubtract(vEndOrigin, vStartOrigin, vBalOrigin);
				VectorScale(vBalOrigin, sqrt(flspeed * flspeed / (vBalOrigin[0] * vBalOrigin[0] + vBalOrigin[1] * vBalOrigin[1] + vBalOrigin[2] * vBalOrigin[2])), vVelocity);

				model_s* pModel = IEngineStudio.Mod_ForName("sprites/ef_blackhole_star.spr", false);
				TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(origin, pModel);
				pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;
				pEnt->entity.curstate.renderamt = 255;
				pEnt->entity.curstate.rendermode = kRenderTransAdd;
				pEnt->entity.baseline.origin = vVelocity;
				pEnt->entity.curstate.framerate = 2.0;
				pEnt->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.11, 0.13);	
				pEnt->die = gHUD.m_flTime + gEngfuncs.pfnRandomFloat(1.2, 2.2);

				pEnt->callback = [](tempent_s* pEnt, float frametime, float currenttime) {
					float gravity = -frametime * 300.0;
					pEnt->entity.baseline.origin[2] += gravity;
				};

				pEnt->hitcallback = [](tempent_s* pEnt, pmtrace_s* ptr) {
					pEnt->entity.baseline.origin = Vector(0, 0, 0);
				};
			}

			break;
		}
		default:break;
		}
}
}
