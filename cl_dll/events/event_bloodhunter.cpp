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

namespace cl {

extern float g_flBloodhunterAnimTime;
extern int g_iBloodhunterSecAnim;
extern int g_iBloodhunterState;

extern engine_studio_api_t IEngineStudio;
}

namespace cl::event::bloodhunter {


enum bloodhunter_e
{
	ANIM_IDLE_RIGHT = 0,
	ANIM_SHOOT_RIGHT,
	ANIM_SHOOT_RIGHT_EF,
	ANIM_SHOOT,
	ANIM_SHOOTA,
	ANIM_SHOOTB,
	ANIM_SHOOTC,
	ANIM_THROW,
	ANIM_THROWA,
	ANIM_THROWB,
	ANIM_THROWC,
	ANIM_RELOAD,
	ANIM_RELOADA,
	ANIM_RELOADB,
	ANIM_RELOADC,
	ANIM_DRAW,
	ANIM_DRAWA,
	ANIM_DRAWB,
	ANIM_DRAWC,
	ANIM_IDLE_LEFT,
	ANIM_IDLE_LEFTA,
	ANIM_IDLE_LEFTB,
	ANIM_IDLE_LEFTC,
	ANIM_CHANGEA,
	ANIM_CHANGEB,
	ANIM_CHANGEC,
};

static const char *SOUNDS_NAME[] =
{
	"weapons/balrog1-1.wav",
	"weapons/balrog1-2.wav"
};

void EV_BloodhunterEffectCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	Vector forward, right, up;
	AngleVectors(gEngfuncs.GetLocalPlayer()->angles, forward, right, up);

	int righthand = gHUD.cl_righthand->value;

	if (righthand)
		ent->tentOffset = ent->entity.curstate.vuser1 + forward * 25 + right * 5 - up * 1;
	else
		ent->tentOffset = ent->entity.curstate.vuser1 + forward * 25 - right * 5 - up * 15;
}

void EV_CreateBloodhunterEffect(event_args_t* args)
{
	Vector vecSrc;
	int iModelIndex;
	struct model_s* pModel;
	if (Com_RandomLong(0, 1))
	{
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_bloodhunter1.spr");
	}
	else
	{
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_bloodhunter2.spr");
	}
	
	TEMPENTITY* pEnt;

	EV_GetGunPosition(args, vecSrc, args->origin);
	pEnt = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, iModelIndex, 30.0);
	if (pEnt)
	{
		pEnt->entity.curstate.rendermode = kRenderTransAdd;
		pEnt->entity.curstate.renderamt = 250;
		pEnt->entity.curstate.scale = 0.125;

		if (!gHUD.cl_righthand->value)
			pEnt->entity.angles[2] -= 180.0f;

		pEnt->clientIndex = args->entindex;

		pEnt->entity.curstate.vuser1 = vecSrc - gEngfuncs.GetLocalPlayer()->origin;

		pEnt->die = gHUD.m_flTime + 1.5;
		pEnt->callback = EV_BloodhunterEffectCallback;
		pEnt->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM | FTENT_SPRANIMATE;
	}
	
}

void EV_FireBloodHunter( event_args_t *args )
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

	int righthand = gHUD.cl_righthand->value;
	cl_entity_t* ent;
	ent = gEngfuncs.GetViewModel();

	if (args->bparam1)
	{
		if (EV_IsLocal(idx))
		{
			EV_CreateBloodhunterEffect( args );
		}
		return;
	}

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		if( args->bparam2 )
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT_RIGHT_EF, 2 );
			g_flBloodhunterAnimTime = gHUD.m_flTime;
			g_iBloodhunterSecAnim = 23 + g_iBloodhunterState - 1;
		}
		else if (g_iBloodhunterSecAnim && gHUD.m_flTime - g_flBloodhunterAnimTime < 1.32)
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(1, 2);
		}
		else
		{
			gEngfuncs.pEventAPI->EV_WeaponAnimation(3 + g_iBloodhunterState, 2);
			g_flBloodhunterAnimTime = 0.0;
			g_iBloodhunterSecAnim = 0;
		}

		if (gHUD.cl_gunsmoke->value)
		{
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 0, 0.25, 10, 10, 10, 3, velocity, 0, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 25, 0.3, 15, 15, 15, 2, velocity, 0, 35);
			EV_HLDM_CreateSmoke(ent->attachment[0], forward, 50, 0.2, 25, 25, 25, 2, velocity, 0, 35);
		}

		if (!righthand)
			EV_GetDefaultShellInfo(args,
				origin,
				velocity,
				ShellVelocity,
				ShellOrigin,
				forward,
				right,
				up,
				35.0,
				-11.0,
				-16.0,
				false
			);
		else
			EV_GetDefaultShellInfo(args,
				origin,
				velocity,
				ShellVelocity,
				ShellOrigin,
				forward,
				right,
				up,
				35.0,
				-11.0,
				16.0,
				false
			);

		ShellVelocity *= 0.75;
		ShellOrigin = ent->attachment[1];
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iPShell, TE_BOUNCE_SHELL);

	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/bloodhunter-1.wav", VOL_NORM, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 15));

	EV_GetGunPosition( args, vecSrc, origin );
	Vector vSpread( args->fparam1, args->fparam2, 0.0f );
	
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, forward,
		vSpread, 8192.0, BULLET_PLAYER_50AE,
		3 );
}

}
