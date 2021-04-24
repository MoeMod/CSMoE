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

namespace cl::event::infinityx {

enum infinityx_e
{
	ANIM_IDLE = 0,
	ANIM_IDLE_LEFTEMPTY,
	ANIM_SHOOT_LEFT,
	ANIM_SHOOT_LEFTLAST,
	ANIM_SHOOT_RIGHT,
	ANIM_SHOOT_RIGHTLAST,
	ANIM_SP_SHOOT_LEFT1,
	ANIM_SP_SHOOT_LEFT2,
	ANIM_SP_SHOOT_RIGHT1,
	ANIM_SP_SHOOT_RIGHT2,
	ANIM_SP_SHOOT_LEFTLAST,
	ANIM_SP_SHOOT_RIGHTLAST,
	ANIM_RELOAD,
	ANIM_DRAW,
};

static const char *SOUNDS_NAME = "weapons/dinfinity-1.wav";

void EV_FireInfinityX( event_args_s *args, int sequence, int direction )
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

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(sequence, 2);
		if(direction)
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, -16.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 35.0, -11.0, 16.0, 0);
		}
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

namespace cl::event::infinityx_left {
using namespace cl::event::infinityx;
void EV_FireInfinityX_Left(event_args_s *args)
{
	if(!args->bparam1)
		EV_FireInfinityX(args, ANIM_SHOOT_LEFT, 0);
	else if(args->bparam2)
		EV_FireInfinityX(args, ANIM_SP_SHOOT_LEFT1, 0);
	else
		EV_FireInfinityX(args, ANIM_SP_SHOOT_LEFT2, 0);
}

}

namespace cl::event::infinityx_right {
using namespace cl::event::infinityx;
void EV_FireInfinityX_Right( event_args_s *args )
{
	if (!args->bparam1)
		EV_FireInfinityX(args, ANIM_SHOOT_RIGHT, 1);
	else if (args->bparam2)
		EV_FireInfinityX(args, ANIM_SP_SHOOT_RIGHT1, 1);
	else
		EV_FireInfinityX(args, ANIM_SP_SHOOT_RIGHT2, 1);
}

}
