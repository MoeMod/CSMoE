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

namespace cl::event::janus1 {

enum janus1_e
{
	ANIM_IDLE1,
	ANIM_DRAW,
	ANIM_SHOOT1_1,
	ANIM_SHOOT1_2,
	ANIM_SHOOT1_SIGNAL,
	ANIM_CHANGE1,
	ANIM_IDLE2,
	ANIM_DRAW2,
	ANIM_SHOOT2_1,
	ANIM_SHOOT2_2,
	ANIM_CHANGE2,
	ANIM_SIGNAL,
	ANIM_DRAW_SIGNAL,
	ANIM_SHOOT1_2_SIGNAL,
};

void EV_FireJANUS1(event_args_s *args)
{
	if (EV_IsLocal(args->entindex))
	{
		g_iShotsFired++;
		EV_MuzzleFlash();

		int iAnim;

		if (args->bparam2)
			iAnim = Com_RandomLong(ANIM_SHOOT2_1, ANIM_SHOOT2_2);
		else
		{
			if (args->iparam1)
			{
				iAnim = args->bparam1 ? ANIM_SHOOT1_SIGNAL : ANIM_SHOOT1_2_SIGNAL;
			}
			else
			{
				iAnim = args->bparam1 ? ANIM_SHOOT1_1 : ANIM_SHOOT1_2;
			}
		}

		gEngfuncs.pEventAPI->EV_WeaponAnimation(iAnim, 2);
	}
}

}
