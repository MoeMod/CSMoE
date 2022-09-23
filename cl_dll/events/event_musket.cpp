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

namespace cl::event::musket {

enum musket_e
{
	ANIM_IDLE1,
	ANIM_IDLE2,
	ANIM_SHOOTSTART,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_DRAW,
};

void EV_FireMusket( event_args_t *args )
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
	cl_entity_t* ent;

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		if (args->bparam1)	//highnoon
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT1, 2);
			
		else		
			gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOT2, 2);
	}

	PLAY_EVENT_SOUND( "weapons/musket-1.wav" );

	EV_GetGunPosition( args, vecSrc, origin );
	Vector vSpread( args->fparam1, args->fparam2, 0.0f );
	
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, forward,
		vSpread, 8192.0, BULLET_PLAYER_50AE,
		7 );
}

}
