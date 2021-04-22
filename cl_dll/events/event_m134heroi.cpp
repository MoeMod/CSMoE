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

namespace cl::event::m134heroi {

static const char *SOUND_NAME = "weapons/m134-1.wav";

	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_DRAW,
		ANIM_RELOAD,
		ANIM_FIRE_READY,
		ANIM_SHOOT,
		ANIM_FIRE_AFTER,
		ANIM_FIRE_CHANGE,
		ANIM_IDLE_CHANGE,
		ANIM_DRAW_OVERHEAT,
		ANIM_FIRE_AFTER_OVERHEAT,
		ANIM_IDLE_OVERHEAT,
		ANIM_OVERHEAT_END,
		ANIM_FIRE_READY_SHOOTB,
		ANIM_SHOOT2,
		ANIM_FIRE_AFTER_SHOOTB,
	};

void EV_FireM134HeroI(event_args_s *args)
{
	vec3_t ShellVelocity, ShellOrigin;
	vec3_t vecSrc, vecAiming;

	int    idx = args->entindex;
	Vector origin( args->origin );
	Vector angles(
		args->iparam1 / 100.0f + args->angles[0],
		args->iparam2 / 100.0f + args->angles[1],
		args->angles[2]
		);
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
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, -11.0, 0);
		}
		else
		{
			EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -10.0, 11.0, 0);
		}
	}
	else
	{
		EV_GetDefaultShellInfo( args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20.0, -12.0, 4.0, 0);
	}


	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[ YAW ], g_iRShell, TE_BOUNCE_SHELL);

	PLAY_EVENT_SOUND(SOUND_NAME);

	EV_GetGunPosition( args, vecSrc, origin );
	VectorCopy( forward, vecAiming );
	Vector vSpread;
	
	vSpread.x = args->fparam1;
	vSpread.y = args->fparam2;
	EV_HLDM_FireBullets( idx,
		forward, right,	up,
		1, vecSrc, vecAiming,
		vSpread, 8192.0, BULLET_PLAYER_556MM,
		2 );
}


//======================
//	    EGON END
//======================

enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE};

#define	EGON_PRIMARY_VOLUME		450
#define EGON_BEAM_SPRITE		"sprites/ef_gungnir_xbeam.spr"
#define EGON_FLARE_SPRITE		"sprites/xspark1.spr"
#define EGON_SOUND_OFF			"weapons/egon_off1.wav"
#define EGON_SOUND_RUN			"weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP		"weapons/egon_windup2.wav"

BEAM *pBeam;
BEAM *pBeam2;

void EV_FireM134HeroI_EgonFire( event_args_t *args )
{
	int idx, iFireState, iFireMode;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	iFireState = args->iparam1;
	iFireMode = args->iparam2;
	int iStartup = args->bparam1;
	int iContinous = args->bparam2;


	if ( iStartup )
	{
		if ( iFireMode == FIRE_WIDE )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125 );
		else
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100 );
	}
	else if ( !iContinous )
	{
		if ( iFireMode == FIRE_WIDE )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.98, ATTN_NORM, 0, 125 );
		else
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.9, ATTN_NORM, 0, 100 );
	}

	//Only play the weapon anims if I shot it.
	if ( EV_IsLocal( idx ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation ( ANIM_SHOOT2, 1 );


    vec3_t vecSrc, vecEnd, angles, forward, right, up;
    pmtrace_t tr;

    cl_entity_t *pl = gEngfuncs.GetEntityByIndex( idx );

    if ( pl )
    {
        VectorCopy( gHUD.m_vecAngles, angles );

        AngleVectors( angles, forward, right, up );

        EV_GetGunPosition( args, vecSrc, pl->origin );

        VectorMA( vecSrc, 2048, forward, vecEnd );

        gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

        // Store off the old count
        gEngfuncs.pEventAPI->EV_PushPMStates();

        // Now add in all of the players.
        gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );

        gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
        gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );

        gEngfuncs.pEventAPI->EV_PopPMStates();

        int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( EGON_BEAM_SPRITE );

        float r = 30.0f;
        float g = 30.0f;
        float b = 64.0f;

        vecEnd = vecSrc + forward * 2048;
        if ( iStartup == 1 && EV_IsLocal( idx ) && !pBeam && !pBeam2 && gHUD.cl_lw->value ) //Adrian: Added the cl_lw check for those lital people that hate weapon prediction.
        {
            pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx, vecEnd, iBeamModelIndex, 99999, 3.5, 0.8, 0.7, 55, 0, 0, r, g, b);

            if (pBeam)
                pBeam->flags |= (FBEAM_SINENOISE);

            pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx, vecEnd, iBeamModelIndex, 99999, 5.0, 0.08, 0.7, 25, 0, 0, r, g, b);
        }
        else
        {
            if(pBeam && pBeam2)
            {
                pBeam->source = vecSrc;
                pBeam2->source = vecSrc;
                pBeam->target = vecEnd;
                pBeam2->target = vecEnd;
            }
        }
    }
}

void EV_FireM134HeroI_EgonStop( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy ( args->origin, origin );

	gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, EGON_SOUND_RUN );

	if ( args->iparam1 )
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_OFF, 0.98, ATTN_NORM, 0, 100 );

	if ( EV_IsLocal( idx ) )
	{
        gEngfuncs.pEventAPI->EV_WeaponAnimation ( ANIM_FIRE_AFTER_SHOOTB, 1 );

		if ( pBeam )
		{
			pBeam->die = 0.0;
			pBeam = NULL;
		}


		if ( pBeam2 )
		{
			pBeam2->die = 0.0;
			pBeam2 = NULL;
		}
	}
}
//======================
//	    EGON END
//======================


}
