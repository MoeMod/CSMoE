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

namespace cl {

enum cannon_e
{
	ANIM_IDLE1,
	ANIM_SHOOT,
	ANIM_DRAW,
};

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

void EV_CannonFireEffect(vec3_t vecSrc, vec3_t vecForward, vec3_t vecVelocity, int idx, int iModel)
{
	if (!iModel)
		iModel = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr");
	TEMPENTITY *ent;
	int i;
	int ran;
	float c, s;
	for (i = 0; i < 12; i++)
	{
		ent = gEngfuncs.pEfxAPI->R_DefaultSprite(vecSrc, iModel, 20);
		if (!ent)
			break;
		ent->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEALL;
		ent->entity.angles.x = gEngfuncs.pfnRandomFloat(-512.0, 511.0);
		ent->entity.angles.y = gEngfuncs.pfnRandomFloat(-256.0, 255.0);
		ent->entity.angles.z = gEngfuncs.pfnRandomFloat(-256.0, 255.0);
		ent->flags |= FTENT_ROTATE;
		ent->entity.curstate.scale = gEngfuncs.pfnRandomFloat(0.3, 0.8);
		ent->entity.baseline.fuser1 = gEngfuncs.pfnRandomFloat(1.5, 2.0);	// CSO: baseline.fuser2
		ent->callback = [](tempent_s *ent, float frametime, float currenttime)
		{
			if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
				ent->entity.curstate.scale += 0.03;
		};

		ent->hitcallback = [](tempent_s *ent, pmtrace_t *tr)
		{
			ent->entity.baseline.origin = ent->entity.baseline.origin * 0.01f;
			ent->entity.curstate.framerate = 60;
		};

		ent->entity.curstate.rendermode = kRenderTransAdd;
		ent->entity.curstate.renderamt = 150;
		ent->entity.curstate.renderfx = kRenderFxFadeSlow;
		ran = 10 * gEngfuncs.pfnRandomLong(0, 4) - 2;
		c = cos(ran * M_PI / 180);
		s = sin(ran * M_PI / 180);
		ent->entity.origin = vecSrc;
		ent->entity.baseline.origin.x = (c * vecForward.x - s * vecForward.y) * gEngfuncs.pfnRandomLong(130, 400);
		ent->entity.baseline.origin.y = (s * vecForward.x + c * vecForward.y) * gEngfuncs.pfnRandomLong(130, 400);
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

void EV_FireCannon( event_args_t *args )
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;

	int idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2] );
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( args->entindex ) )
	{
		++g_iShotsFired;
		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT, 2);
		EV_MuzzleFlash();
		if( !gHUD.cl_righthand->value )
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, -10.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -8.0, 10.0, 0);
		}

		if( gHUD.cl_gunsmoke->value )
		{
			cl_entity_t *ent = gEngfuncs.GetViewModel();

			if( ent )
			{
				EV_CS16Client_CreateSmoke( SMOKE_RIFLE, ent->attachment[0], forward, 3, 0.3, 20, 20, 20, false, velocity );
			}
		}
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}


	//EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND( "weapons/cannon-1.wav" );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );

	Vector vSpread( args->fparam1, args->fparam2, 0.0f );
	cl_entity_t *ent = gEngfuncs.GetViewModel();

	if (EV_IsLocal(idx))
		EV_CannonFireEffect(ent->attachment[0] + 7.0 * forward, forward, velocity, idx, 0);
	else
		EV_CannonFireEffect(vecSrc + 40.0 * forward, forward, velocity, idx, 0);
}

}
