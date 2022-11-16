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

namespace cl::event::voidpistol {

#define WPNSTATE_VOIDPISTOLEX_MODEB	 (1 << 0)
#define WPNSTATE_VOIDPISTOLEX_MODEC	 (1 << 1)
#define WPNSTATE_VOIDPISTOLEX_SCAN_ON (1 << 2)

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

static const char* SOUNDS_NAME[] =
{
	"weapons/voidpistol-1.wav",
	"weapons/voidpistol-2.wav",
	"weapons/voidpistol_beep.wav",
};

void EV_FireVoidPistol( event_args_t *args )
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

		int iAnim;
		if (args->bparam2 & WPNSTATE_VOIDPISTOLEX_MODEC)
			iAnim = VOIDPISTOL_SHOOTC;
		else if (args->bparam2 & WPNSTATE_VOIDPISTOLEX_MODEB)
			iAnim = VOIDPISTOL_SHOOTB;
		else
			iAnim = VOIDPISTOL_SHOOTA;

		gEngfuncs.pEventAPI->EV_WeaponAnimation(iAnim, 2);
	}

	PLAY_EVENT_SOUND(SOUNDS_NAME[0]);

	if (!args->bparam1)
	{
		EV_GetGunPosition(args, vecSrc, origin);
		Vector vSpread(args->fparam1, args->fparam2, 0.0f);

		EV_HLDM_FireBullets(idx,
			forward, right, up,
			1, vecSrc, forward,
			vSpread, 8192.0, BULLET_PLAYER_VOIDPISTOL,
			3);
	}
	else
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, SOUNDS_NAME[2], VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
}

}
