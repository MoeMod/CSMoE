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

namespace cl::event::skull4 {

enum elite_e
{
	ANIM_IDLE1,
	ANIM_IDLE_LEFTEMPTY,
	ANIM_SHOOTLEFT1,
	ANIM_SHOOTLEFT2,
	ANIM_SHOOTLEFTLAST,
	ANIM_SHOOTRIGHT1,
	ANIM_SHOOTRIGHT2,
	ANIM_SHOOTRIGHTLAST,
	ANIM_RELOAD,
	ANIM_DRAW
};

static const char *SOUNDS_NAME = "weapons/skull4-1.wav";

void EV_FireSKULL4( event_args_s *args, int sequence )
{
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	vec3_t vecSrc, vecAiming;

	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles( args->angles );
	Vector velocity( args->velocity );
	Vector forward, right, up;
	AngleVectors( angles, forward, right, up );
	int bullets_left = args->bparam1;
	int animleft = args->bparam2;	//true left
	cl_entity_t* ent = gEngfuncs.GetViewModel();

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
	
		if (bullets_left <= 1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(animleft ? ANIM_SHOOTRIGHTLAST : ANIM_SHOOTLEFTLAST, 2);
		else
			gEngfuncs.pEventAPI->EV_WeaponAnimation(sequence, 2);

		if( sequence >= ANIM_SHOOTLEFTLAST)
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, 0);
		}

		if (animleft && ent)
			ShellOrigin = ent->attachment[3];
		else
			ShellOrigin = ent->attachment[4];
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iPShell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND( SOUNDS_NAME );

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;

	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_9MM,
		2 );
}
}

namespace cl::event::skull4_left {
void EV_FireSKULL4_Left(event_args_s *args)
{
    using namespace cl::event::skull4;
	EV_FireSKULL4( args, Com_RandomLong(ANIM_SHOOTLEFT1, ANIM_SHOOTLEFT2));
}

}
namespace cl::event::skull4_right {
void EV_FireSKULL4_Right( event_args_s *args )
{
    using namespace cl::event::skull4;
	EV_FireSKULL4( args, Com_RandomLong(ANIM_SHOOTRIGHT1, ANIM_SHOOTRIGHT2));
}

}
