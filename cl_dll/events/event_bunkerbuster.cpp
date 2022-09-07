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

namespace cl::event::bunkerbuster {

	extern engine_studio_api_t IEngineStudio;


	void EV_FireBunkerBuster(event_args_t* args)
	{

		vec3_t origin;
		int ent = args->entindex;
		VectorCopy(args->origin, origin);

		static char szSoundBunkerbuster[64];

		if (!args->bparam1)
		{
			switch (args->iparam1)
			{
			case 0:
			{
				gEngfuncs.pEventAPI->EV_PlaySound(ent, origin, CHAN_VOICE, "weapons/bunkerbuster_fly.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
				break;
			}
			case 1:
			{
				sprintf(szSoundBunkerbuster, "weapons/bunkerbuster_whistling%d.wav", gEngfuncs.pfnRandomLong(1, 3));
				gEngfuncs.pEventAPI->EV_PlaySound(ent, origin, CHAN_STREAM, szSoundBunkerbuster, 1.0, ATTN_NORM, 0, PITCH_NORM);
				break;
			}
			case 2:
			{
				sprintf(szSoundBunkerbuster, "weapons/bunkerbuster_explosion_%s1st.wav", args->iparam2 == 1 ? "after_" : "");
				gEngfuncs.pEventAPI->EV_PlaySound(ent, origin, CHAN_VOICE, szSoundBunkerbuster, 1.0, ATTN_NORM, 0, PITCH_NORM);
				gEngfuncs.pEventAPI->EV_PlaySound(ent, origin, CHAN_VOICE, "weapons/bunkerbuster_fire.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

				break;
			}
			}

		}
		else
		{
			if(!args->bparam2)
				gEngfuncs.pEventAPI->EV_PlaySound(ent, origin, CHAN_VOICE, "weapons/bunkerbuster_target_siren.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

		}
		
}
}