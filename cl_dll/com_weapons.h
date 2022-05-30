//========= Copyright пїЅ 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
// com_weapons.h
// Shared weapons common function prototypes
#if !defined( COM_WEAPONSH )
#define COM_WEAPONSH


#include "hud_iface.h"

#define PLAYER_CAN_SHOOT (1 << 0)
#define PLAYER_FREEZE_TIME_OVER ( 1 << 1 )
#define PLAYER_IN_BOMB_ZONE (1 << 2)
#define PLAYER_HOLDING_SHIELD (1 << 3)

#define CROSSHAIR_

#include "weapons_const.h"

namespace cl {
extern "C"
{
	void _DLLEXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
}



void			COM_Log( char *pszFile, char *fmt, ...);
bool			CL_IsDead();

float			UTIL_SharedRandomFloat( unsigned int seed, float low, float high );
int				UTIL_SharedRandomLong( unsigned int seed, int low, int high );


int				HUD_GetWeaponAnim( void );
void			HUD_SendWeaponAnim(int iAnim, int iWeaponId, int iBody, int iForce = 0, float framerate = 1.0f );
int				HUD_GetWeapon( void );
void			HUD_PlaySound( char *sound, float volume );
void			HUD_PlaybackEvent( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, const vec3_t origin, const vec3_t angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
//void			HUD_SetMaxSpeed( const struct edict_s *ed, float speed );
WeaponCrosshairData				GetWeaponCrosshairData( void );

extern cvar_t *cl_lw;

extern int g_runfuncs;

extern int g_iWeaponFlags;
extern bool g_bInBombZone;
extern int g_iFreezeTimeOver;
extern bool g_bHoldingShield;
extern bool g_bHoldingKnife;
extern int g_iPlayerFlags;
extern float g_flPlayerSpeed;
extern Vector g_vPlayerVelocity;
extern struct local_state_s *g_curstate;
extern struct local_state_s *g_finalstate;
extern int g_iShotsFired;

extern float g_lastFOV;
extern vec3_t v_angles;

}

#endif
