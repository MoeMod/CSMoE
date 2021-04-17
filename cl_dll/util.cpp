/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// util.cpp
//
// implementation of class-less helper functions
//

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "hud.h"
#include "cl_util.h"
#include <string.h>

int HUD_GetSpriteIndexByName( const char *sz )
{
	return gHUD.GetSpriteIndex(sz);
}

HSPRITE HUD_GetSprite( int index )
{
	return gHUD.GetSprite(index);
}

wrect_t HUD_GetSpriteRect( int index )
{
	return gHUD.GetSpriteRect( index );
}

vec3_t g_ColorBlue	= { 0.6, 0.8, 1.0 };
vec3_t g_ColorcsgoBlue = { 0.45, 0.63, 0.82 };
vec3_t g_ColorcsgoRed		= { 0.93, 0.78, 0.38 };
vec3_t g_ColorRed = { 1.0, 0.25, 0.25 };
vec3_t g_ColorGreen	= { 0.6, 1.0, 0.6 };
vec3_t g_ColorYellow	= { 1.0, 0.7, 0.0 };
vec3_t g_ColorGrey	= { 0.8, 0.8, 0.8 };

float *GetClientColor( int clientIndex )
{
	switch ( g_PlayerExtraInfo[clientIndex].teamnumber )
	{
	case TEAM_TERRORIST:  return g_ColorRed;
	case TEAM_CT:         return g_ColorBlue;
	case TEAM_SPECTATOR:
	case TEAM_UNASSIGNED: return g_ColorYellow;
	case 4:               return g_ColorGreen;
	default:              return g_ColorGrey;
	}
}


