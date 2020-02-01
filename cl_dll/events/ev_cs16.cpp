/*
ev_cs16.cpp
Copyright (C) 2020 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifdef _WIN32
#include "port.h"
#endif



#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"

extern "C"
{
#include "pm_defs.h"
#include "pm_materials.h"
}

#include "pm_shared.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "triangleapi.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <string.h>

#include "r_studioint.h"
#include "com_model.h"

#include <assert.h>

extern float g_flRoundTime;

namespace cl {

// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play
void EV_HLDM_NewExplode( float x, float y, float z, float ScaleExplode1 )
{

	int  iNewExplode = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/dexplo.spr");
	TEMPENTITY *pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z + 5 ),
														 Vector( 0, 0, 0 ),
														 ScaleExplode1, iNewExplode, kRenderTransAdd, kRenderFxNone, 1.0, 0.5, FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEKILL );

	if(pTemp)
	{
		pTemp->fadeSpeed = 90.0;
		pTemp->entity.curstate.framerate = 37.0;
		pTemp->entity.curstate.renderamt = 155;
		pTemp->entity.curstate.rendercolor.r = 255;
		pTemp->entity.curstate.rendercolor.g = 255;
		pTemp->entity.curstate.rendercolor.b = 255;
	}

	iNewExplode = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/fexplo.spr");
	pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z + 10),
											 Vector( 0, 0, 0 ),
											 ScaleExplode1, iNewExplode, kRenderTransAdd, kRenderFxNone, 1.0, 0.5, FTENT_SPRANIMATE | FTENT_FADEOUT | FTENT_COLLIDEKILL );

	if(pTemp)
	{
		pTemp->fadeSpeed = 90.0;
		pTemp->entity.curstate.framerate = 35.0;
		pTemp->entity.curstate.renderamt = 150;
		pTemp->entity.curstate.rendercolor.r = 255;
		pTemp->entity.curstate.rendercolor.g = 255;
		pTemp->entity.curstate.rendercolor.b = 255;
		pTemp->entity.angles = Vector( 90, 0, 0 );
	}

	for( int i = 1; i <= 10; i++ )
	{
		int  iSmokeSprite = gEngfuncs.pEventAPI->EV_FindModelIndex ("sprites/smoke.spr");
		TEMPENTITY *pTemp = gEngfuncs.pEfxAPI->R_TempSprite( Vector( x, y, z ),
															 Vector( (int)Com_RandomLong( -100, 100 ), (int)Com_RandomLong( -100, 100 ), (int)Com_RandomLong( -100, 100 ) ),
															 5, iSmokeSprite, kRenderTransAlpha, kRenderFxNone, 1.0, 0.5, FTENT_FADEOUT | FTENT_PERSIST );

		if(pTemp)
		{
			pTemp->fadeSpeed = 0.6;
			pTemp->entity.curstate.framerate = 1;
			pTemp->entity.curstate.renderamt = 255;
			int Color = Com_RandomLong( 0, 140 );
			pTemp->entity.curstate.rendercolor.r = Color;
			pTemp->entity.curstate.rendercolor.g = Color;
			pTemp->entity.curstate.rendercolor.b = Color;
		}
	}

}

char EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType )
{
	// hit the world, try to play sound based on texture material type
	char chTextureType = CHAR_TEX_CONCRETE;
	float fvol;
	const char *rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entity;
	char *pTextureName;
	char texname[ 64 ];
	char szbuffer[ 64 ];

	entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( ptr );

	// FIXME check if playtexture sounds movevar is set
	//

	chTextureType = 0;

	// Player
	if ( entity >= 1 && entity <= gEngfuncs.GetMaxClients() )
	{
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	}
	else if ( entity == 0 )
	{
		// get texture from entity or world (world is ent(0))
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( ptr->ent, vecSrc, vecEnd );

		if ( pTextureName )
		{
			strncpy( texname, pTextureName, sizeof( texname ) );
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+')
			{
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
			{
				pTextureName++;
			}

			// '}}'
			strncpy( szbuffer, pTextureName, sizeof(szbuffer) );
			szbuffer[ CBTEXTURENAMEMAX - 1 ] = 0;

			// get texture type
			chTextureType = PM_FindTextureType( szbuffer );
		}
	}

	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE:
	{
		fvol = 0.9;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	}
	case CHAR_TEX_METAL:
	{
		fvol = 0.9;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	}
	case CHAR_TEX_DIRT:
	{
		fvol = 0.9;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	}
	case CHAR_TEX_VENT:
	{
		fvol = 0.5;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	}
	case CHAR_TEX_GRATE:
	{
		fvol = 0.9;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	}
	case CHAR_TEX_TILE:
	{
		fvol = 0.8;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	}
	case CHAR_TEX_SLOSH:
	{
		fvol = 0.9;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	}
	case CHAR_TEX_SNOW:
	{
		fvol = 0.7;
		rgsz[0] = "debris/pl_snow1.wav";
		rgsz[1] = "debris/pl_snow2.wav";
		rgsz[2] = "debris/pl_snow3.wav";
		rgsz[3] = "debris/pl_snow4.wav";
		cnt = 4;
		break;
	}
	case CHAR_TEX_WOOD:
	{
		fvol = 0.9;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	}
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
	{
		fvol = 0.8;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	}
	case CHAR_TEX_FLESH:
	{
		fvol = 1.0;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}
	}

	// play material hit sound
	gEngfuncs.pEventAPI->EV_PlaySound( 0, ptr->endpos, CHAN_STATIC, rgsz[Com_RandomLong(0,cnt-1)], fvol, fattn, 0, 96 + Com_RandomLong(0,0xf) );

	return chTextureType;
}

char *EV_HLDM_DamageDecal( physent_t *pe )
{
	static char decalname[ 32 ];
	int idx;

	if ( pe->classnumber == 1 )
	{
		idx = Com_RandomLong( 0, 2 );
		sprintf( decalname, "{break%i", idx + 1 );
	}
	else if ( pe->rendermode != kRenderNormal )
	{
		sprintf( decalname, "{bproof1" );
	}
	else
	{
		idx = Com_RandomLong( 0, 4 );
		sprintf( decalname, "{shot%i", idx + 1 );
	}
	return decalname;
}

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, char chTextureType )
{
	int iRand;
	physent_t *pe;

	gEngfuncs.pEfxAPI->R_BulletImpactParticles( pTrace->endpos );


	iRand = Com_RandomLong(0,0x7FFF);
	if ( iRand < (0x7fff/2) )// not every bullet makes a sound.
	{
		if( chTextureType == CHAR_TEX_VENT || chTextureType == CHAR_TEX_METAL )
		{
			switch( iRand % 2 )
			{
			case 0: gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric_metal-1.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM); break;
			case 1: gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric_metal-2.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM); break;
			}
		}
		else
		{
			switch( iRand % 7)
			{
			case 0:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 1:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 2:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 3:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 4:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 5: gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric_conc-1.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM); break;
			case 6: gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric_conc-2.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM); break;
			}
		}

	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent( pTrace->ent );

	// Only decal brush models such as the world etc.
	if (  decalName && decalName[0] && pe && ( pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP ) )
	{
		if ( CVAR_GET_FLOAT( "r_decals" ) )
		{
			gEngfuncs.pEfxAPI->R_DecalShoot(
						gEngfuncs.pEfxAPI->Draw_DecalIndex( gEngfuncs.pEfxAPI->Draw_DecalIndexFromName( decalName ) ),
						gEngfuncs.pEventAPI->EV_IndexFromTrace( pTrace ), 0, pTrace->endpos, 0 );

		}
	}
}

void EV_SmokeRise_Wind(tempent_s *pEntity, float frametime, float currenttime)
{
	static vec3_t velocity;
	static int iSmokeDirectionX = 1, iSmokeDirectionY = 1;	// Default = 1, not constant...
	if (pEntity->entity.curstate.frame > 7.0)
	{
		pEntity->entity.baseline.origin *= 0.97;
		pEntity->entity.baseline.origin.z += 0.7;

		if (pEntity->entity.baseline.origin.z > 70.0)
		{
			pEntity->entity.curstate.origin.z = 70.0;
		}
	}
	if (pEntity->entity.curstate.frame > 6.0)
	{
		velocity.x += 0.075;
		if (velocity.x > 5.0)
			velocity.x = 5.0;

		if (!iSmokeDirectionX)
			pEntity->entity.baseline.origin.x -= velocity.x;
		else
			pEntity->entity.baseline.origin.x += velocity.x;

		velocity.y += 0.075;
		if (velocity.y > 5.0)
			velocity.y = 5.0;

		if (!iSmokeDirectionY)
			pEntity->entity.baseline.origin.y -= velocity.y;
		else
			pEntity->entity.baseline.origin.y += velocity.y;

		if (!gEngfuncs.pfnRandomLong(0, 10))
		{
			if (velocity.y > 3.0)
			{
				velocity.y = 0.0;
				iSmokeDirectionY = 1 - iSmokeDirectionY;
			}
		}

		if (!gEngfuncs.pfnRandomLong(0, 10))
		{
			if (velocity.x > 3.0)
			{
				velocity.x = 0.0;
				iSmokeDirectionX = 1 - iSmokeDirectionX;
			}
		}
	}
}

void EV_SmokeRise_Wind_Expand(tempent_s *pEntity, float frametime, float currenttime)
{
	if (pEntity->entity.curstate.renderamt >= 75)
		pEntity->entity.curstate.renderamt -= gEngfuncs.pfnRandomLong(0, 9) == 0;
	if (pEntity->entity.curstate.scale < pEntity->entity.baseline.fuser1)
		pEntity->entity.curstate.scale *= 1.025;
}

void EV_SmokeRise(tempent_s *pEntity, float frametime, float currenttime)
{
	if (pEntity->entity.curstate.frame > 7.0)
	{
		pEntity->entity.baseline.origin *= 0.97;
		pEntity->entity.baseline.origin.z += 0.7;

		if (pEntity->entity.baseline.origin.z > 70.0)
		{
			pEntity->entity.curstate.origin.z = 70.0;
		}
	}
}

void EV_HugWalls(tempent_s *pEntity, pmtrace_t *tr)
{
	float flLength = pEntity->entity.baseline.origin.Length();

	if (flLength > 2000.0)
		flLength = 2000.0;
	vec3_t vecResult = pEntity->entity.baseline.origin.Normalize();

	pEntity->entity.baseline.origin.x = ((tr->plane.normal.x * vecResult.z - tr->plane.normal.z * vecResult.x) * tr->plane.normal.z - (tr->plane.normal.y * vecResult.z - tr->plane.normal.x * vecResult.y) * tr->plane.normal.y) * flLength * 1.5;
	pEntity->entity.baseline.origin.y = ((tr->plane.normal.y * vecResult.z - tr->plane.normal.x * vecResult.y) * tr->plane.normal.x - (tr->plane.normal.z * vecResult.y - tr->plane.normal.y * vecResult.z) * tr->plane.normal.z) * flLength * 1.5;
	pEntity->entity.baseline.origin.z = ((tr->plane.normal.z * vecResult.y - tr->plane.normal.y * vecResult.z) * tr->plane.normal.y - (tr->plane.normal.x * vecResult.z - tr->plane.normal.z * vecResult.x) * tr->plane.normal.x) * flLength * 1.5;
}

void EV_CS16Client_CreateSmoke(int type, Vector origin, Vector dir, int speed, float scale, int r, int g, int b , bool wind, Vector velocity, int framerate )
{
	TEMPENTITY *te = NULL;
	void ( *callback )( struct tempent_s *ent, float frametime, float currenttime ) = NULL;
	char path[64];

	switch( type )
	{
		case SMOKE_WALLPUFF:
			if( !gHUD.fastsprites->value )
			{
				strcpy( path, "sprites/wall_puff1.spr" );

				path[17] += Com_RandomLong(0, 3); // randomize a bit
			}
			else
			{
				strcpy( path, "sprites/fast_wallpuff1.spr" );
				te = gEngfuncs.pEfxAPI->R_DefaultSprite( origin,
														 gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/fast_wallpuff1.spr"), 30.0f );
			}
			break;
		case SMOKE_RIFLE:
			strcpy( path, "sprites/rifle_smoke1.spr" );
			path[19] += Com_RandomLong(0, 2); // randomize a bit


			break;
		case SMOKE_PISTOL:
			strcpy( path, "sprites/pistol_smoke1.spr" );
			path[20] += Com_RandomLong(0, 1);  // randomize a bit


			break;
		case SMOKE_BLACK:
			strcpy( path, "sprites/black_smoke1.spr" );
			path[19] += Com_RandomLong(0, 3); // randomize a bit


			break;
		default:
			assert(("Unknown smoketype!"));
	}

	if( wind )
		callback = EV_SmokeRise_Wind;
	else
		callback = EV_SmokeRise;


	te = gEngfuncs.pEfxAPI->R_DefaultSprite( origin, gEngfuncs.pEventAPI->EV_FindModelIndex( path ), framerate );

	if( te )
	{
		te->callback = callback;
		te->hitcallback = EV_HugWalls;
		te->flags |= FTENT_COLLIDEALL | FTENT_CLIENTCUSTOM;
		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.rendercolor.r = r;
		te->entity.curstate.rendercolor.g = g;
		te->entity.curstate.rendercolor.b = b;
		te->entity.curstate.renderamt = Com_RandomLong( 100, 180 );
		te->entity.curstate.scale = scale;
		te->entity.baseline.origin = speed * dir;

		if( !velocity.IsNull() )
		{
			velocity.x *= 0.5;
			velocity.y *= 0.5;
			velocity.z *= 0.9;
			te->entity.baseline.origin = te->entity.baseline.origin + velocity;
		}
	}
}

void EV_HLDM_CreateSmoke(float *origin, float *dir, int speed, float scale, int r, int g, int b, int iSmokeType, float *base_velocity, bool bWind, int framerate)
{
	char model[256];
	int model_index;
	TEMPENTITY *te;
	switch (iSmokeType)
	{
		case 1:
			strcpy(model, "sprites/black_smoke1.spr");

			switch (gEngfuncs.pfnRandomLong(0, 3))
			{
				case 0:
					model[19] = '1';
					break;
				case 1:
					model[19] = '2';
					break;
				case 2:
					model[19] = '3';
					break;
				case 3:
					model[19] = '4';
					break;
			}

			model_index = gEngfuncs.pEventAPI->EV_FindModelIndex(model);

			if (gEngfuncs.pfnGetCvarFloat("fastsprites") > 1.0)
			{
				model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/fast_wallpuff1.spr");
			}

			if (!model_index)
				model_index = 293;
			break;

		case 2:
			switch (gEngfuncs.pfnRandomLong(0, 3))
			{
				case 0:
					model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/wall_puff1.spr");
					break;
				case 1:
					model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/wall_puff2.spr");
					break;
				case 2:
					model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/wall_puff3.spr");
					break;
				case 3:
					model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/wall_puff4.spr");
					break;
			}
			break;

		case 3:
			if (gEngfuncs.pfnRandomLong(0, 1))
				model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/pistol_smoke1.spr");
			else
				model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/pistol_smoke2.spr");
			break;

		case 4:
			switch (gEngfuncs.pfnRandomLong(0, 2))
			{
				case 0:
					model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/rifle_smoke1.spr");
				case 1:
					model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/rifle_smoke2.spr");
				case 2:
					model_index = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/rifle_smoke3.spr");
			}
			break;
	}

	te = gEngfuncs.pEfxAPI->R_DefaultSprite(origin, model_index, framerate);

	if (te)
	{
		te->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD;
		te->entity.curstate.scale = scale + gEngfuncs.pfnRandomFloat(0.0, scale / 2.0);

		if (iSmokeType > 1)
		{
			te->callback = bWind ? EV_SmokeRise_Wind : EV_SmokeRise;
			te->entity.curstate.rendermode = kRenderTransAdd;
			te->entity.curstate.renderamt = 150;
			te->entity.curstate.renderfx = kRenderFxFadeSlow;
		}
		else
		{
			if (gEngfuncs.pfnGetCvarFloat("fastsprites") == 1.0)
			{
				te->entity.curstate.rendermode = kRenderTransAdd;
				te->entity.curstate.renderamt = 200;
			}
			else
			{
				if (gEngfuncs.pfnGetCvarFloat("fastsprites") > 1.0)
				{
					te->entity.curstate.rendermode = kRenderNormal;
				}
				else
				{
					te->entity.curstate.rendermode = kRenderTransAlpha;
					te->entity.curstate.renderamt = 200;
				}
			}

			te->callback = EV_SmokeRise_Wind_Expand;
			te->entity.baseline.fuser1 = (iSmokeType == 1) ? 1.0 : 8.5;

			if (gEngfuncs.pfnGetCvarFloat("fastsprites") > 1.0)
				te->entity.baseline.fuser1 -= 3.0;
		}

		te->hitcallback = EV_HugWalls;
		te->entity.curstate.rendercolor.r = r;
		te->entity.curstate.rendercolor.g = g;
		te->entity.curstate.rendercolor.b = b;

		if (speed)
		{
			speed += gEngfuncs.pfnRandomLong(0, 5);
		}

		if (origin[0] != dir[0] && origin[1] != dir[1] && origin[2] != dir[2])
		{
			te->entity.origin.x = origin[0];
			te->entity.origin.y = origin[1];
			te->entity.origin.z = origin[2];

			te->entity.baseline.origin.x = dir[0] * speed;
			te->entity.baseline.origin.y = dir[1] * speed;
			te->entity.baseline.origin.z = dir[2] * speed;

			if (base_velocity)
			{
				te->entity.baseline.origin.x = dir[0] * speed + 0.9 * base_velocity[0];
				te->entity.baseline.origin.y = dir[1] * speed + 0.9 * base_velocity[1];
				te->entity.baseline.origin.z = dir[2] * speed + 0.5 * base_velocity[2];
			}
		}
	}
}

void EV_HLDM_DecalGunshot(pmtrace_t *pTrace, int iBulletType, float scale, int r, int g, int b, bool bStreakSplash, char cTextureType)
{
	int iColorIndex;
	char *decalname;

	physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);

	if (pe && pe->solid == SOLID_BSP)
	{
		if (bStreakSplash)
		{
			int iStreakCount = gEngfuncs.pfnRandomLong(15, 30);

			switch (iBulletType)
			{
				default:
					iColorIndex = 30;
					break;
			}

			gEngfuncs.pEfxAPI->R_StreakSplash(pTrace->endpos, gEngfuncs.pfnRandomFloat(4.0, 10.0) * pTrace->plane.normal, iColorIndex, iStreakCount, gEngfuncs.pfnRandomFloat(4.0, 10.0) * pTrace->plane.normal[2], -75, 75);
		}

		switch (iBulletType)
		{
			default:
				decalname = EV_HLDM_DamageDecal(pe);
				break;
		}

		EV_HLDM_GunshotDecalTrace(pTrace, decalname, cTextureType);
		EV_HLDM_CreateSmoke(pTrace->endpos + pTrace->plane.normal * 5.0, pTrace->plane.normal, 25, scale, r, g, b, 2, NULL, true, 35);
		//EV_CS16Client_CreateSmoke( SMOKE_WALLPUFF, pTrace->endpos, pTrace->plane.normal, 25, 0.5, r, g, b, true );
	}
}

/*
============
EV_DescribeBulletTypeParameters

Sets iPenetrationPower and flPenetrationDistance.
If iBulletType is unknown, calls assert() and sets these two vars to 0
============
*/
void EV_DescribeBulletTypeParameters(int iBulletType, int &iPenetrationPower, float &flPenetrationDistance)
{
	switch (iBulletType)
	{
	case BULLET_PLAYER_9MM:
	{
		iPenetrationPower = 21;
		flPenetrationDistance = 800;
		break;
	}

	case BULLET_PLAYER_45ACP:
	{
		iPenetrationPower = 15;
		flPenetrationDistance = 500;
		break;
	}

	case BULLET_PLAYER_50AE:
	{
		iPenetrationPower = 30;
		flPenetrationDistance = 1000;
		break;
	}

	case BULLET_PLAYER_762MM:
	{
		iPenetrationPower = 39;
		flPenetrationDistance = 5000;
		break;
	}

	case BULLET_PLAYER_556MM:
	{
		iPenetrationPower = 35;
		flPenetrationDistance = 4000;
		break;
	}

	case BULLET_PLAYER_338MAG:
	{
		iPenetrationPower = 45;
		flPenetrationDistance = 8000;
		break;
	}

	case BULLET_PLAYER_57MM:
	{
		iPenetrationPower = 30;
		flPenetrationDistance = 2000;
		break;
	}

	case BULLET_PLAYER_357SIG:
	{
		iPenetrationPower = 25;
		flPenetrationDistance = 800;
		break;
	}

	default:
	{
		iPenetrationPower = 0;
		flPenetrationDistance = 0;
		break;
	}
	}
}

int EV_HLDM_CheckTracer(int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount)
{
	int tracer = 0;
	int i;
	qboolean player = idx >= 1 && idx <= gEngfuncs.GetMaxClients() ? true : false;

	if (iTracerFreq != 0 && ((*tracerCount)++ % iTracerFreq) == 0)
	{
		vec3_t vecTracerSrc;

		if (player)
		{
			vec3_t offset(0, 0, -4);

			// adjust tracer position for player
			for (i = 0; i < 3; i++)
			{
				vecTracerSrc[i] = vecSrc[i] + offset[i] + right[i] * 2 + forward[i] * 16;
			}
		}
		else
		{
			VectorCopy(vecSrc, vecTracerSrc);
		}

		if (iTracerFreq != 1)		// guns that always trace also always decal
			tracer = 1;

		gEngfuncs.pEfxAPI->R_TracerEffect(vecTracerSrc, end);
	}

	return tracer;
}


void EV_HLDM_FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, int iPenetration, int iAttachment, bool lefthand, float srcofs)
{
	int i;
	pmtrace_t tr;
	int tracer;
	int iPenetrationPower;
	float flPenetrationDistance;
	float flCurrentDistance;
	vec3_t vecStart;
	char chTextureType = CHAR_TEX_GRATE;
	float flScale = 0.4;
	float flTempDistance;
	int iSourcePenetration = iPenetration;
	int r = 40, g = 40, b = 40;
	BOOL bCreatedEffects = FALSE;

	EV_DescribeBulletTypeParameters(iBulletType, iPenetrationPower, flPenetrationDistance);

	if (lefthand == 0)
	{
		vecSrc[0] -= right[0] * srcofs;
		vecSrc[1] -= right[1] * srcofs;
		vecSrc[2] -= right[2] * srcofs;
	}
	else
	{
		vecSrc[0] += right[0] * srcofs;
		vecSrc[1] += right[1] * srcofs;
		vecSrc[2] += right[2] * srcofs;
	}

	int iPower = iPenetrationPower * 2;
	bool bStreakSplash = false;

	int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");

	if (cShots >= 1)
	{
		VectorCopy(vecSrc, vecStart);
		int iShots = cShots;
		float x, y, z;
		vec3_t vecDir, vecEnd;

		while (iShots > 0)
		{
			if (iBulletType == BULLET_PLAYER_BUCKSHOT)
			{
				do
				{
					x = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
					y = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
					z = x*x + y*y;
				} while (z > 1);
				for (i = 0; i < 3; i++)
				{
					vecDir[i] = vecDirShooting[i] + x * vecSpread[0] * right[i] + y * vecSpread[1] * up[i];
					vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
				}
			}
			else
			{
				for (i = 0; i < 3; i++)
				{
					vecDir[i] = vecDirShooting[i] + vecSpread[0] * right[i] + vecSpread[1] * up[i];
					vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
				}
			}

			iPenetration = iSourcePenetration;
			bCreatedEffects = FALSE;

			gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
			gEngfuncs.pEventAPI->EV_PushPMStates();
			gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
			gEngfuncs.pEventAPI->EV_SetTraceHull(2);
			bStreakSplash = false;

			while (iPenetration > 0)
			{
				gEngfuncs.pEventAPI->EV_PlayerTrace(vecStart, vecEnd, PM_NORMAL, -1, &tr);

				tracer = EV_HLDM_CheckTracer(idx, vecStart, vecEnd, forward, right, iBulletType, iTracerFreq, tracerCount);
				flCurrentDistance = tr.fraction * flDistance;
				if (flCurrentDistance <= 0.0)
					break;

				if (!bCreatedEffects)
				{
					bCreatedEffects = TRUE;
					vec3_t source;
					if (EV_IsLocal(idx))
						source = gEngfuncs.GetViewModel()->attachment[iAttachment];
					else
						source = gEngfuncs.GetEntityByIndex(idx)->attachment[0];

					if (cl_tracereffect->value)
						gEngfuncs.pEfxAPI->R_TracerParticles(source, (tr.endpos - source).Normalize() * 6000, (tr.endpos - source).Length() / 6000);
					if (cl_gunbubbles->value)
					{
						Vector end, dir;
						dir = vecDir;

						for (int i = 0; i < 3; i++)
						{
							end[i] = tr.endpos[i] - 5.0 * vecDir[i];
						}

						if (gEngfuncs.PM_PointContents(end, NULL) == CONTENTS_WATER)
						{
							int sModelIndexBubbles = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/bubble.spr");

							if (cl_gunbubbles->value == 2)
							{
								cl_entity_t *ent;
								vec3_t src(vecSrc);

								if (EV_IsLocal(idx))
								{
									ent = gEngfuncs.GetViewModel();
									src = ent->attachment[iAttachment];
									dir = (end - src).Normalize();
								}

								vec3_t dir = (end - src).Normalize();

								float height, distance;

								if (gEngfuncs.PM_PointContents(src, NULL) == CONTENTS_WATER)
								{
									height = src.z - end.z;
									distance = (end - src).Length();

									if (height < 20)
										height = 20;
								}
								else
								{
									Vector test = end;

									while (gEngfuncs.PM_PointContents(test, NULL) == CONTENTS_WATER)
									{
										test.z += 2;
									}

									height = test.z - end.z - 5 - 2;
									distance = (end - src).Length();

									src = end - dir * (height / (src.z - end.z) * distance);
								}

								int count = (src - end).Length() / 8;

								if (count && height)
									gEngfuncs.pEfxAPI->R_BubbleTrail(src, end, height, sModelIndexBubbles, count, 2.0);
							}
							else
							{
								Vector mins, maxs;
								mins.x = tr.endpos[0] - 2.0;
								mins.y = tr.endpos[1] - 2.0;
								mins.z = tr.endpos[2] - 2.0;
								maxs.x = tr.endpos[0] + 15.0;
								maxs.y = tr.endpos[1] + 15.0;
								maxs.z = tr.endpos[2] + 15.0;

								gEngfuncs.pEfxAPI->R_Bubbles(mins, maxs, 20, sModelIndexBubbles, 10, 2.0);
							}
						}
					}
				}

				iPenetration--;
				if (flCurrentDistance > flPenetrationDistance)
					iPenetration = 0;
				flTempDistance = flCurrentDistance;
				switch (iBulletType)
				{
					case BULLET_PLAYER_9MM:
						if (!tracer)
							chTextureType = EV_HLDM_PlayTextureSound(idx, &tr, vecStart, vecEnd, iBulletType);
						break;
					case BULLET_PLAYER_BUCKSHOT:
						break;
					case BULLET_PLAYER_MP5:
						if (!tracer)
							chTextureType = EV_HLDM_PlayTextureSound(idx, &tr, vecStart, vecEnd, iBulletType);
						break;
					case BULLET_PLAYER_357:
						if (!tracer)
							chTextureType = EV_HLDM_PlayTextureSound(idx, &tr, vecStart, vecEnd, iBulletType);
						break;
					default:
						chTextureType = EV_HLDM_PlayTextureSound(idx, &tr, vecStart, vecEnd, iBulletType);
						break;
				}

				switch (chTextureType)
				{
					case CHAR_TEX_METAL:
						bStreakSplash = true;
						flCurrentDistance = iPower * 0.15;
						iPower = flCurrentDistance;
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
						break;
					case CHAR_TEX_CONCRETE:
						bStreakSplash = true;
						r = g = b = 65;
						flCurrentDistance = iPower * 0.25;
						iPower = flCurrentDistance;
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
						break;
					case CHAR_TEX_GRATE:
					case CHAR_TEX_VENT:
						bStreakSplash = true;
						flCurrentDistance = iPower * 0.5;
						iPower = flCurrentDistance;
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
						break;
					case CHAR_TEX_TILE:
						bStreakSplash = true;
						flCurrentDistance = iPower * 0.65;
						iPower = flCurrentDistance;
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
						break;
					case CHAR_TEX_COMPUTER:
						bStreakSplash = true;
						flCurrentDistance = iPower * 0.4;
						iPower = flCurrentDistance;
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
						break;
					case CHAR_TEX_WOOD:
						bStreakSplash = false;
						r = 75;
						g = 42;
						b = 15;
						flScale = 0.5;
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
						break;
					default:
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
						break;
				}

				if (!iPenetration)
					break;
				flDistance = (flDistance - flTempDistance) * 0.5;
				vecStart = iPower * vecDir + tr.endpos - vecDir;
				vecEnd = vecDir * flDistance + vecStart;

				float flDepth = 1.0;

				if (iPower > 1)
				{
					while (1)
					{
						gEngfuncs.pEventAPI->EV_SetTraceHull(2);
						gEngfuncs.pEventAPI->EV_PlayerTrace(tr.endpos + vecDir * flDepth, tr.endpos + vecDir * flDepth * 2, PM_NORMAL, -1, &tr);
						if (tr.startsolid)
						{
							if (tr.inopen)
								break;
						}
						flDepth += 1.0;
						if (flDepth >= iPower)
							break;
					}
					if (flDepth < iPower)
						EV_HLDM_DecalGunshot(&tr, iBulletType, flScale, r, g, b, bStreakSplash, chTextureType);
				}
				continue;
			}
			gEngfuncs.pEventAPI->EV_PopPMStates();
			iShots--;
		}
	}
}

void EV_HLDM_FireBullets(int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float *vecSpread, float flDistance, int iBulletType, int iPenetration)
{
    return EV_HLDM_FireBullets(idx, forward, right, up, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, 0, 0, iPenetration, 0, false, 0.0f);
}

void EV_CS16Client_KillEveryRound( TEMPENTITY *te, float frametime, float current_time )
{
	if( g_flRoundTime > te->entity.curstate.fuser4 )
	{
		// Mark it die on next TempEntUpdate
		te->die = 0.0f;
		// Set null renderamt, so it will be invisible now
		// Also it will die immediately, if FTEMP_FADEOUT was set
		te->entity.curstate.renderamt = 0;
	}
}

void RemoveBody(TEMPENTITY *te, float frametime, float current_time)
{
	// go underground...
	/*if ( current_time >= 2 * te->entity.curstate.fuser2 + 5.0 )
		te->entity.origin.z -= 5.0 * frametime;*/
}

void HitBody(TEMPENTITY *ent, pmtrace_s *ptr)
{
	/*if ( ptr->plane.normal.z > 0.0 )
		ent->flags |= FTENT_NONE;*/
}


void CreateCorpse(Vector *p_vOrigin, Vector *p_vAngles, const char *pModel, float flAnimTime, int iSequence, int iBody)
{
	int modelIdx = gEngfuncs.pEventAPI->EV_FindModelIndex(pModel);
	vec3_t null(0, 0, 0);
	TEMPENTITY *model = gEngfuncs.pEfxAPI->R_TempModel( (float*)p_vOrigin,
														null,
														(float*)p_vAngles,
														gEngfuncs.pfnGetCvarFloat("cl_corpsestay"),
														modelIdx,
														0 );

	if(model)
	{
		//model->frameMax = -1;
		model->entity.curstate.animtime = flAnimTime;
		model->entity.curstate.framerate = 1.0;
		model->entity.curstate.frame = 0;
		model->entity.curstate.sequence = iSequence;
		model->entity.curstate.body = iBody;
		model->entity.curstate.fuser1 = gHUD.m_flTime + 1.0;
		model->entity.curstate.fuser2 = gEngfuncs.pfnGetCvarFloat("cl_corpsestay") + gHUD.m_flTime;
		model->hitcallback = HitBody;
		model->callback = RemoveBody;
	}
}

}
