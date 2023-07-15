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

namespace cl::event::m3dragonex {

enum m3_e
{
	M3DRAGONEX_IDLE,
	M3DRAGONEX_FIRE1,
	M3DRAGONEX_FIRE2,
	M3DRAGONEX_INSERT,
	M3DRAGONEX_AFTER_RELOAD,
	M3DRAGONEX_BEFORE_RELOAD,
	M3DRAGONEX_DRAW,
	M3DRAGONEX_SECONDARY_READY,
	M3DRAGONEX_SECONDARY_SHOOT,
	M3DRAGONEX_SECONDARY_INSERT,
	M3DRAGONEX_SECONDARY_AFTER_RELOAD,
	M3DRAGONEX_SECONDARY_BEFORE_RELOAD,
	M3DRAGONEX_SECONDARY_DRAW,
	M3DRAGONEX_SHOOT_SPECIAL,
	M3DRAGONEX_CHARGE_START,
	M3DRAGONEX_CHARGE_IDLE,
	M3DRAGONEX_CHARGE_ATTACK,
	M3DRAGONEX_CHARGE_STARTB,
	M3DRAGONEX_CHARGE_IDLEB,
	M3DRAGONEX_CHARGE_ATTACKB,
};

static const char* SOUNDS_NAME[] =
{
	"weapons/m3dragonex-1.wav",
	"weapons/m3dragonex-2.wav",
	"weapons/m3dragonex-3.wav",
};

void EV_CannonFireEffect(vec3_t vecSrc, vec3_t vecForward, vec3_t vecVelocity, int idx, int iModel)
{
	if (!iModel)
		iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_m3dragonex_shoot.spr");
	TEMPENTITY* ent;
	int i;
	int ran;
	float c, s;
	for (i = 0; i < 4; i++)
	{
		ent = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, iModel, 20);
		if (!ent)
			break;
		ent->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEALL;
		ent->entity.angles.x = gEngfuncs.pfnRandomFloat(-512.0, 511.0);
		ent->entity.angles.y = gEngfuncs.pfnRandomFloat(-256.0, 255.0);
		ent->entity.angles.z = gEngfuncs.pfnRandomFloat(-256.0, 255.0);
		ent->flags |= FTENT_ROTATE;
		ent->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.2, 0.3);
		ent->entity.baseline.fuser1 = gEngfuncs.pfnRandomFloat(0.6, 0.8);	// CSO: baseline.fuser2
		ent->callback = [](tempent_s* ent, float frametime, float currenttime)
		{
			if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
				ent->entity.curstate.scale += 0.03;
		};

		ent->hitcallback = [](tempent_s* ent, pmtrace_t* tr)
		{
			ent->entity.baseline.origin = ent->entity.baseline.origin * 0.01f;
			ent->entity.curstate.framerate = 60;
		};

		ent->entity.curstate.rendermode = kRenderTransAdd;
		ent->entity.curstate.renderamt = 150;
		ent->entity.curstate.renderfx = kRenderFxFadeSlow;
		ran = 10 * gEngfuncs.pfnRandomLong(0, 4) - 2;
		c = cos(DEG2RAD(ran));
		s = sin(DEG2RAD(ran));
		ent->entity.origin = vecSrc;
		ent->entity.baseline.origin.x = (c * vecForward.x - s * vecForward.y) * gEngfuncs.pfnRandomLong(50, 200);
		ent->entity.baseline.origin.y = (s * vecForward.x + c * vecForward.y) * gEngfuncs.pfnRandomLong(50, 200);
		ent->entity.baseline.origin.z = vecForward.z * gEngfuncs.pfnRandomLong(30, 200);

		if (vecVelocity)
		{
			ent->entity.baseline.origin.x += vecVelocity.x * 0.9;
			ent->entity.baseline.origin.y += vecVelocity.y * 0.9;
			ent->entity.baseline.origin.z += vecVelocity.z * 0.3;
		}

		ent->clientIndex = idx;
	}
}

void EV_FireM3DragonEX( event_args_t *args )
{
	vec3_t vecSrc, vecAiming;
	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
	Vector forward, right, up;
	Vector velocity(args->velocity);
	int righthand = gHUD.cl_righthand->value;
	cl_entity_t* viewent = gEngfuncs.GetViewModel();
	int iAnim;

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		if (args->bparam2)
		{
			PLAY_EVENT_SOUND(SOUNDS_NAME[1]);
			gEngfuncs.pEventAPI->EV_WeaponAnimation(M3DRAGONEX_SECONDARY_SHOOT, 2);
		}
		else
		{
			iAnim = Com_RandomLong(M3DRAGONEX_FIRE1, M3DRAGONEX_FIRE2);
			PLAY_EVENT_SOUND(SOUNDS_NAME[iAnim - 1]);
			gEngfuncs.pEventAPI->EV_WeaponAnimation(iAnim, 2);
		}
	}
	else
		PLAY_EVENT_SOUND(SOUNDS_NAME[Com_RandomLong(0, 1)]);

	

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = 0.0725;
	vSpread.y = 0.0725;

	cl_entity_t* ent = gEngfuncs.GetViewModel();

	if (EV_IsLocal(idx))
		EV_CannonFireEffect(ent->attachment[0] + 7.0 * forward, forward, velocity, idx, 0);
	else
		EV_CannonFireEffect(vecSrc + 40.0 * forward, forward, velocity, idx, 0);

	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		10, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
		1 );
}

}
