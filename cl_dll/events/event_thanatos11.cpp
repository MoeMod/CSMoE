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

namespace cl::event::thanatos11 {

enum m3_e
{
	ANIM_IDLEA = 0,
	ANIM_IDLEB1,
	ANIM_IDLEB2,
	ANIM_IDLEB_EMPTY,
	ANIM_SHOOTA,
	ANIM_SHOOTB,
	ANIM_SHOOTB_EMPTY,
	ANIM_CHANGEA,
	ANIM_CHANGEA_EMPTY,
	ANIM_CHANGEB,
	ANIM_CHANGEB_EMPTY,
	ANIM_INSERT,
	ANIM_AFTER_RELOAD,
	ANIM_START_RELOAD,
	ANIM_DRAW,
	ANIM_IDLEB_RELOAD
};

void EV_FireThanatos11( event_args_t *args )
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
	int righthand = gHUD.cl_righthand->value;
	cl_entity_t* viewent = gEngfuncs.GetViewModel();
	int iAnim;

	AngleVectors( angles, forward, right, up );

	if ( EV_IsLocal( idx ) )
	{
		++g_iShotsFired;
		EV_MuzzleFlash();
		PLAY_EVENT_SOUND("weapons/thanatos11-1.wav");
		gEngfuncs.pEventAPI->EV_WeaponAnimation(ANIM_SHOOTA, 2);
	}

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = 0.0683;
	vSpread.y = 0.0683;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		9, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_BUCKSHOT,
		1 );
}

}
