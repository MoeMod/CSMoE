//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined ( EV_HLDMH )
#define EV_HLDMH

// bullet types
#include "weapons_const.h"

enum
{
	SMOKE_WALLPUFF = 0,
	SMOKE_RIFLE,
	SMOKE_PISTOL,
	SMOKE_BLACK
};

namespace cl {

void EV_HLDM_GunshotDecalTrace(pmtrace_t *pTrace, char *decalName , char chTextureType);
void EV_HLDM_DecalGunshot(pmtrace_t *pTrace, int iBulletType, float scale, int r, int g, int b, bool bCreateSparks, char cTextureType, bool isSky);
void EV_HLDM_FireBullets(int idx,
						 const vec3_t forward, const vec3_t right, const vec3_t up,
						 int cShots,
						 vec3_t vecSrc, const vec3_t vecDirShooting, const vec3_t vecSpread,
						 float flDistance, int iBulletType, int iPenetration);
void EV_HLDM_FireBullets(int idx, const vec3_t forward, const vec3_t right, const vec3_t up, int cShots, vec3_t vecSrc, const vec3_t vecDirShooting, const vec3_t vecSpread, float flDistance, int iBulletType, int iTracerFreq, int* tracerCount, int iPenetration = 1);
void EV_CS16Client_KillEveryRound( struct tempent_s *te, float frametime, float currenttime );
void EV_CS16Client_CreateSmoke(int type, Vector origin, Vector dir, int speed, float scale, int r, int g, int b , bool wind, Vector velocity = Vector(0, 0, 0), int framerate = 35 );
void EV_HLDM_CreateSmoke(const vec3_t origin, const vec3_t dir, int speed, float scale, int r, int g, int b, int iSmokeType, const vec3_t base_velocity, bool bWind, int framerate);

static int tracerCount[32];
}

#endif // EV_HLDMH
