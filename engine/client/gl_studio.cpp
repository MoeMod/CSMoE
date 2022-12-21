/*
gl_studio.c - studio model renderer
Copyright (C) 2010 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef XASH_DEDICATED

#include "common.h"
#include "client.h"
#include "mathlib.h"
#include "const.h"
#include "r_studioint.h"
#include "studio.h"
#include "pm_local.h"
#include "gl_local.h"
#include "cl_tent.h"
#include "gl_texlru.h"

#ifdef XASH_RAGDOLL
#include "physics.h"
#endif

#define EVENT_CLIENT	5000	// less than this value it's a server-side studio events
#define MAXARRAYVERTS	20000	// used for draw shadows
#define LEGS_BONES_COUNT	8
#define MAX_LOCALLIGHTS	4

static vec3_t hullcolor[8] = 
{
{ 1.0f, 1.0f, 1.0f },
{ 1.0f, 0.5f, 0.5f },
{ 0.5f, 1.0f, 0.5f },
{ 1.0f, 1.0f, 0.5f },
{ 0.5f, 0.5f, 1.0f },
{ 1.0f, 0.5f, 1.0f },
{ 0.5f, 1.0f, 1.0f },
{ 1.0f, 1.0f, 1.0f },
};

// enumerate all the bones that used for gait animation
const char *legs_bones[] =
{
 "Bip01" ,
 "Bip01 Pelvis" ,
 "Bip01 L Leg" ,
 "Bip01 L Leg1" ,
 "Bip01 L Foot" ,
 "Bip01 R Leg" ,
 "Bip01 R Leg1" ,
 "Bip01 R Foot" ,
};

typedef struct studiolight_item_s
{
	vec3_t vec[MAXSTUDIOBONES];
	vec3_t color;	// ambient light colors
} studiolight_item_t;

typedef struct studiolight_s
{
	vec3_t		lightvec;			// light vector
	vec3_t		lightcolor;		// ambient light color
	vec3_t		lightspot;		// potential coords where placed lightsource

	vec3_t		blightvec[MAXSTUDIOBONES];	// ambient lightvectors per bone
	vec3_t		lightval;

    // elights cache
    int		numlocallights;
    int		lightage[MAXSTUDIOBONES];
    dlight_t		*locallight[MAX_LOCALLIGHTS];
    color24		locallightcolor[MAX_LOCALLIGHTS];
    vec4_t		lightpos[MAXSTUDIOVERTS][MAX_LOCALLIGHTS];
    vec3_t		lightbonepos[MAXSTUDIOBONES][MAX_LOCALLIGHTS];
    float		locallightR2[MAX_LOCALLIGHTS];

    int			ambientlight;	// clip at 128
    int			shadelight;		// clip at 192 - ambientlight
} studiolight_t;

typedef struct sortedmesh_s
{
	mstudiomesh_t	*mesh;
	int		flags;			// face flags
} sortedmesh_t;

convar_t			*r_studio_lerping;
convar_t			*r_studio_lambert;
convar_t			*r_studio_lighting;
convar_t			*r_studio_sort_textures;
convar_t			*r_studio_drawelements;
convar_t			*r_drawviewmodel;
convar_t			*r_customdraw_playermodel;
convar_t			*cl_himodels;
convar_t			*cl_righthand;
cvar_t			r_shadowalpha = { "r_shadowalpha", "0.5", 0, 0.8f };
static r_studio_interface_t	*pStudioDraw;
static float		aliasXscale, aliasYscale;	// software renderer scale
static matrix3x4		g_aliastransform;		// software renderer transform
static matrix3x4		g_rotationmatrix;
static vec3_t		g_chrome_origin;
static vec2_t		g_chrome[MAXSTUDIOVERTS];	// texture coords for surface normals
static sortedmesh_t		g_sortedMeshes[MAXSTUDIOMESHES];
matrix3x4		g_bonestransform[MAXSTUDIOBONES];
matrix3x4		g_lighttransform[MAXSTUDIOBONES];
static matrix3x4		g_rgCachedBonesTransform[MAXSTUDIOBONES];
static matrix3x4		g_rgCachedLightTransform[MAXSTUDIOBONES];
static vec3_t		g_chromeright[MAXSTUDIOBONES];// chrome vector "right" in bone reference frames
static vec3_t		g_chromeup[MAXSTUDIOBONES];	// chrome vector "up" in bone reference frames
static int		g_chromeage[MAXSTUDIOBONES];	// last time chrome vectors were updated
static vec3_t		g_xformverts[MAXSTUDIOVERTS];
static vec3_t		g_xformnorms[MAXSTUDIOVERTS];
static vec3_t		g_xarrayverts[MAXARRAYVERTS];
static vec2_t		g_xarraycoord[MAXARRAYVERTS];
static GLubyte		g_xarraycolor[MAXARRAYVERTS][4];
static unsigned short		g_xarrayelems[MAXARRAYVERTS*6];
static uint		g_nNumArrayVerts;
static uint		g_nNumArrayElems;
static vec3_t		g_lightvalues[MAXSTUDIOVERTS];
static studiolight_t	g_studiolight;
char			g_nCachedBoneNames[MAXSTUDIOBONES][32];
int			g_nCachedBones;		// number of bones in cache
int			g_nStudioCount;		// for chrome update
int			g_iRenderMode;		// currentmodel rendermode
int			g_iBackFaceCull;
vec3_t			studio_mins, studio_maxs;
float			studio_radius;

// global variables
qboolean	g_bCustomStudioRenderer;
qboolean			m_fDoInterp;
qboolean			m_fDoRemap;
mstudiomodel_t		*m_pSubModel;
mstudiobodyparts_t		*m_pBodyPart;
player_info_t		*m_pPlayerInfo;
studiohdr_t		*m_pStudioHeader;
studiohdr_t		*m_pTextureHeader;
float			m_flGaitMovement;
pmtrace_t			g_shadowTrace;
vec3_t			g_mvShadowVec;
int			g_nTopColor, g_nBottomColor;	// remap colors
int			g_nFaceFlags, g_nForceFaceFlags;

// quick cache aligned
vec3_t pstudioverts[MAXSTUDIOVERTS]; // copied from m_pStudioHeader + m_pSubModel->vertindex
vec3_t pstudionorms[MAXSTUDIOVERTS]; // copied from m_pStudioHeader + m_pSubModel->normindex


/*
====================
R_StudioInit

====================
*/
void R_StudioInit( void )
{
	float	pixelAspect;

	r_studio_lambert = Cvar_Get( "r_studio_lambert", "2", CVAR_ARCHIVE, "bonelighting lambert value" );
	r_studio_lerping = Cvar_Get( "r_studio_lerping", "1", CVAR_ARCHIVE, "enables studio animation lerping" );
	r_drawviewmodel = Cvar_Get( "r_drawviewmodel", "1", 0, "draw firstperson weapon model" );
	cl_himodels = Cvar_Get( "cl_himodels", "1", CVAR_ARCHIVE, "draw high-resolution player models in multiplayer" );
	r_studio_lighting = Cvar_Get( "r_studio_lighting", "1", CVAR_ARCHIVE, "studio lighting models ( 0 - normal, 1 - extended, 2 - experimental )" );
	r_studio_sort_textures = Cvar_Get( "r_studio_sort_textures", "0", CVAR_ARCHIVE, "sort additive and normal textures for right drawing" );
	r_studio_drawelements = Cvar_Get( "r_studio_drawelements", "1", CVAR_ARCHIVE, "Use glDrawElements for studio render" );
	// NOTE: some mods with custom studiomodel renderer may cause error when menu trying draw player model out of the loaded game
	r_customdraw_playermodel = Cvar_Get( "r_customdraw_playermodel", "0", CVAR_ARCHIVE, "allow to drawing playermodel in menu with client renderer" );

	// recalc software X and Y alias scale (this stuff is used only by HL software renderer but who knews...)
	pixelAspect = ((float)scr_height->integer / (float)scr_width->integer);
	if( scr_width->integer < 640 )
		pixelAspect *= (320.0f / 240.0f);
	else pixelAspect *= (640.0f / 480.0f);

	if( RI.refdef.fov_y != 0 )
	{
		aliasXscale = (float)scr_width->integer / RI.refdef.fov_y;
		aliasYscale = aliasXscale * pixelAspect;
	}

	Matrix3x4_LoadIdentity( g_aliastransform );
	Matrix3x4_LoadIdentity( g_rotationmatrix );

	g_nStudioCount = 0;
	m_fDoRemap = false;
}

/*
===============
R_GenAlignedVertAndNormCache

===============
*/
static void R_GenAlignedVertAndNormCache()
{
    // for cache friendly
    const float* pstudioverts_unaligned = (const float*)((const byte*)m_pStudioHeader + m_pSubModel->vertindex);
    const float* pstudionorms_unaligned = (const float*)((const byte*)m_pStudioHeader + m_pSubModel->normindex);
    float *pstudioverts_out = (float *)pstudioverts;
    float *pstudionorms_out = (float *)pstudionorms;
    for(int i = 0; i < m_pSubModel->numverts; ++i)
    {
        pstudioverts_out[i * 4 + 0] = pstudioverts_unaligned[i * 3 + 0];
        pstudioverts_out[i * 4 + 1] = pstudioverts_unaligned[i * 3 + 1];
        pstudioverts_out[i * 4 + 2] = pstudioverts_unaligned[i * 3 + 2];
        pstudioverts_out[i * 4 + 3] = 0;
    }
    for(int i = 0; i < m_pSubModel->numnorms; ++i)
    {
        pstudionorms_out[i * 4 + 0] = pstudionorms_unaligned[i * 3 + 0];
        pstudionorms_out[i * 4 + 1] = pstudionorms_unaligned[i * 3 + 1];
        pstudionorms_out[i * 4 + 2] = pstudionorms_unaligned[i * 3 + 2];
        pstudionorms_out[i * 4 + 3] = 0;
    }
}

/*
================
R_StudioBodyVariations

calc studio body variations
================
*/
static int R_StudioBodyVariations( model_t *mod )
{
	studiohdr_t	*pstudiohdr;
	mstudiobodyparts_t	*pbodypart;
	int		i, count;

	pstudiohdr = (studiohdr_t *)Mod_Extradata( mod );
	if( !pstudiohdr ) return 0;

	count = 1;
	pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex);

	// each body part has nummodels variations so there are as many total variations as there
	// are in a matrix of each part by each other part
	for( i = 0; i < pstudiohdr->numbodyparts; i++ )
		count = count * pbodypart[i].nummodels;

	return count;
}

/*
================
R_StudioExtractBbox

Extract bbox from current sequence
================
*/
qboolean R_StudioExtractBbox( studiohdr_t *phdr, int sequence, vec3_t_ref mins, vec3_t_ref maxs )
{
	mstudioseqdesc_t	*pseqdesc;

	if( !phdr ) return false;

	// check sequence range
	if( sequence < 0 || sequence >= phdr->numseq )
		sequence = 0;

	pseqdesc = (mstudioseqdesc_t *)((byte *)phdr + phdr->seqindex);
	VectorCopy( pseqdesc[sequence].bbmin, mins );
	VectorCopy( pseqdesc[sequence].bbmax, maxs );

	return true;
}

/*
================
R_AllowFlipViewModel
should a flip the viewmodel if cl_righthand is set to 1
================
*/
static qboolean R_AllowFlipViewModel(cl_entity_t* e)
{
	if (cl_righthand && cl_righthand->value > 0)
	{
		constexpr int CSW_KNIFE = 29;
		if (!((e == &clgame.viewent) ^ (cl.frame.client.m_iId != CSW_KNIFE)))
			return true;
	}

	return false;
}

/*
================
R_StudioComputeBBox

Compute a full bounding box for current sequence
================
*/
static qboolean R_StudioComputeBBox( cl_entity_t *e, vec3_t bbox[8] )
{
	vec3_t	tmp_mins, tmp_maxs;
	vec3_t	vectors[3], angles, p1, p2;
	int	i, seq = e->curstate.sequence;

	if( !R_StudioExtractBbox( m_pStudioHeader, seq, tmp_mins, tmp_maxs ))
		return false;

	// copy original bbox
	VectorCopy( m_pStudioHeader->bbmin, studio_mins );
	VectorCopy( m_pStudioHeader->bbmax, studio_maxs );

	// rotate the bounding box
	VectorCopy( e->angles, angles );

	if( e->player ) angles[PITCH] = 0.0f; // don't rotate player model, only aim
	AngleVectors( angles, vectors[0], vectors[1], vectors[2] );

	// compute a full bounding box
	for( i = 0; i < 8; i++ )
	{
  		p1[0] = ( i & 1 ) ? tmp_mins[0] : tmp_maxs[0];
  		p1[1] = ( i & 2 ) ? tmp_mins[1] : tmp_maxs[1];
  		p1[2] = ( i & 4 ) ? tmp_mins[2] : tmp_maxs[2];

		// rotate by YAW
		p2[0] = DotProduct( p1, vectors[0] );
		p2[1] = DotProduct( p1, vectors[1] );
		p2[2] = DotProduct( p1, vectors[2] );

		if( bbox ) VectorAdd( p2, e->origin, bbox[i] );

  		if( p2[0] < studio_mins[0] ) studio_mins[0] = p2[0];
  		if( p2[0] > studio_maxs[0] ) studio_maxs[0] = p2[0];
  		if( p2[1] < studio_mins[1] ) studio_mins[1] = p2[1];
  		if( p2[1] > studio_maxs[1] ) studio_maxs[1] = p2[1];
  		if( p2[2] < studio_mins[2] ) studio_mins[2] = p2[2];
  		if( p2[2] > studio_maxs[2] ) studio_maxs[2] = p2[2];
	}

	studio_radius = RadiusFromBounds( studio_mins, studio_maxs );

	return true;
}

/*
===============
pfnGetCurrentEntity

===============
*/
static cl_entity_t *GAME_EXPORT pfnGetCurrentEntity( void )
{
	return RI.currententity;
}

/*
===============
pfnPlayerInfo

===============
*/
static player_info_t *GAME_EXPORT pfnPlayerInfo( int index )
{
	if( cls.key_dest == key_menu && !index )
		return &menu.playerinfo;

	if( index < 0 || index > cl.maxclients )
		return NULL;
	return &cl.players[index];
}

/*
===============
pfnGetPlayerState

===============
*/
entity_state_t *GAME_EXPORT R_StudioGetPlayerState( int index )
{
	if( index < 0 || index > cl.maxclients )
		return NULL;
	return &cl.frame.playerstate[index];
}

/*
===============
pfnGetViewEntity

===============
*/
static cl_entity_t *GAME_EXPORT pfnGetViewEntity( void )
{
	return &clgame.viewent;
}

/*
===============
pfnGetEngineTimes

===============
*/
static void GAME_EXPORT pfnGetEngineTimes( int *framecount, double *current, double *old )
{
	if( framecount ) *framecount = tr.framecount;
	if( current ) *current = cl.time;
	if( old ) *old = cl.oldtime;
}

/*
===============
pfnGetViewInfo

===============
*/
static void GAME_EXPORT pfnGetViewInfo( float *origin, float *upv, float *rightv, float *forwardv )
{
	if( origin ) VectorCopy( RI.vieworg, origin );
	if( forwardv ) VectorCopy( RI.vforward, forwardv );
	if( rightv ) VectorCopy( RI.vright, rightv );
	if( upv ) VectorCopy( RI.vup, upv );
}

/*
===============
R_GetChromeSprite

===============
*/
static model_t *GAME_EXPORT R_GetChromeSprite( void )
{
	if( cls.hChromeSprite <= 0 || cls.hChromeSprite > ( MAX_IMAGES - 1 ))
		return NULL; // bad sprite
	return &clgame.sprites[cls.hChromeSprite];
}

/*
===============
pfnGetModelCounters

===============
*/
static void GAME_EXPORT pfnGetModelCounters( int **s, int **a )
{
	*s = &g_nStudioCount;
	*a = (int *)&r_stats.c_studio_models_drawn;
}

/*
===============
pfnGetAliasScale

===============
*/
static void GAME_EXPORT pfnGetAliasScale( float *x, float *y )
{
	if( x ) *x = aliasXscale;
	if( y ) *y = aliasYscale;
}

/*
===============
pfnStudioGetBoneTransform

===============
*/
static matrix3x4 (* GAME_EXPORT pfnStudioGetBoneTransform( void ))[MAXSTUDIOBONES]
{
	return &g_bonestransform;
}

/*
===============
pfnStudioGetLightTransform

===============
*/
static matrix3x4 (* GAME_EXPORT pfnStudioGetLightTransform( void ))[MAXSTUDIOBONES]
{
	return &g_lighttransform;
}

/*
===============
pfnStudioGetAliasTransform

===============
*/
static matrix3x4 * GAME_EXPORT pfnStudioGetAliasTransform( void )
{
	return &g_aliastransform;
}

/*
===============
pfnStudioGetRotationMatrix

===============
*/
static matrix3x4 *GAME_EXPORT pfnStudioGetRotationMatrix( void )
{
	return &g_rotationmatrix;
}

/*
====================
CullStudioModel

====================
*/
qboolean R_CullStudioModel( cl_entity_t *e )
{
	vec3_t	origin;

	if( !e || !e->model || !e->model->cache.data )
		return true;

	if( e == &clgame.viewent && ( r_lefthand->integer >= 2 || gl_overview->integer ))
		return true; // hidden

	if( !R_StudioComputeBBox( e, NULL ))
		return true; // invalid sequence

	// NOTE: extract real drawing origin from rotation matrix
	Matrix3x4_OriginFromMatrix( g_rotationmatrix, origin );

	return R_CullModel( e, origin, studio_mins, studio_maxs, studio_radius );
}

/*
====================
StudioEstimateFrame

====================
*/
float R_StudioEstimateFrame( cl_entity_t *e, mstudioseqdesc_t *pseqdesc )
{
	double	dfdt, f;
	
	if( m_fDoInterp )
	{
		if( RI.refdef.time < e->curstate.animtime ) dfdt = 0.0;
		else dfdt = (RI.refdef.time - e->curstate.animtime) * e->curstate.framerate * pseqdesc->fps;
	}
	else dfdt = 0;

	if( pseqdesc->numframes <= 1 ) f = 0.0;
	else f = (e->curstate.frame * (pseqdesc->numframes - 1)) / 256.0;
 
	f += dfdt;

	if( pseqdesc->flags & STUDIO_LOOPING ) 
	{
		if( pseqdesc->numframes > 1 )
			f -= (int)(f / (pseqdesc->numframes - 1)) *  (pseqdesc->numframes - 1);
		if( f < 0 ) f += (pseqdesc->numframes - 1);
	}
	else 
	{
		if( f >= pseqdesc->numframes - 1.001 )
			f = pseqdesc->numframes - 1.001;
		if( f < 0.0 )  f = 0.0;
	}
	return f;
}

/*
====================
StudioSetupChrome

====================
*/
void R_StudioSetupChrome( vec2_t_ref pchrome, int bone, const vec3_t normal )
{
	float	n;

	if( g_chromeage[bone] != g_nStudioCount )
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		vec3_t	chromeupvec;	// g_chrome t vector in world reference frame
		vec3_t	chromerightvec;	// g_chrome s vector in world reference frame
		vec3_t	tmp, v_left;	// vector pointing at bone in world reference frame

		VectorCopy( g_chrome_origin, tmp );
		tmp[0] += g_bonestransform[bone][0][3];
		tmp[1] += g_bonestransform[bone][1][3];
		tmp[2] += g_bonestransform[bone][2][3];

		VectorNormalize( tmp );
		VectorNegate( RI.vright, v_left );

		if( g_nForceFaceFlags & STUDIO_NF_CHROME )
		{
			float	angle, sr, cr;
			int	i;

			angle = anglemod( RI.refdef.time * 40 ) * (M_PI2 / 360.0f);
			SinCos( angle, &sr, &cr );

			for( i = 0; i < 3; i++ )
			{
				chromerightvec[i] = (v_left[i] * cr + RI.vup[i] * sr);
				chromeupvec[i] = v_left[i] * -sr + RI.vup[i] * cr;
			}
		}
		else
		{
			CrossProduct( tmp, v_left, chromeupvec );
			VectorNormalize( chromeupvec );
			CrossProduct( tmp, chromeupvec, chromerightvec );
			VectorNormalize( chromerightvec );
			VectorNegate( chromeupvec, chromeupvec );
		}

		Matrix3x4_VectorIRotate( g_bonestransform[bone], chromeupvec, g_chromeup[bone] );
		Matrix3x4_VectorIRotate( g_bonestransform[bone], chromerightvec, g_chromeright[bone] );
		g_chromeage[bone] = g_nStudioCount;
	}

	// calc s coord
	n = DotProduct( normal, g_chromeright[bone] );
	pchrome[0] = (n + 1.0f) * 32.0f;

	// calc t coord
	n = DotProduct( normal, g_chromeup[bone] );
	pchrome[1] = (n + 1.0f) * 32.0f;
}

/*
===============
pfnStudioSetupModel

===============
*/
static void GAME_EXPORT R_StudioSetupModel( int bodypart, void **ppbodypart, void **ppsubmodel )
{
	int	index;

	if( bodypart > m_pStudioHeader->numbodyparts )
		bodypart = 0;

	m_pBodyPart = (mstudiobodyparts_t *)((byte *)m_pStudioHeader + m_pStudioHeader->bodypartindex) + bodypart;

	// this cannot be equal 0 for valid model
	if( m_pBodyPart->base )
	{
		index = RI.currententity->curstate.body / m_pBodyPart->base;
	}
	else
	{
		MsgDev( D_ERROR, "R_StudioSetupModel: m_pBodyPart->base = 0 in %s\n", RI.currentmodel->name );
		index = 0;
	}

	index = index % m_pBodyPart->nummodels;

	m_pSubModel = (mstudiomodel_t *)((byte *)m_pStudioHeader + m_pBodyPart->modelindex) + index;
    R_GenAlignedVertAndNormCache();

	if( ppbodypart ) *ppbodypart = m_pBodyPart;
	if( ppsubmodel ) *ppsubmodel = m_pSubModel;
}

/*
===============
R_StudioCheckBBox

===============
*/
static int GAME_EXPORT R_StudioCheckBBox( void )
{
	if( R_CullStudioModel( RI.currententity ))
		return false;
	return true;
}

/*
===============
R_StudioDynamicLight

===============
*/
void GAME_EXPORT R_StudioDynamicLight( cl_entity_t *ent, alight_t *lightinfo )
{
    movevars_t	*mv = cl.refdef.movevars;
    vec3_t		lightDir, vecSrc, vecEnd;
    vec3_t		origin, dist, finalLight;
    float		add, radius, total;
    colorVec		light;
    uint		lnum;
    dlight_t		*dl;

    if( !lightinfo || !ent || !ent->model )
        return;

    if( !RI.drawWorld || r_fullbright->value || ( ent->curstate.effects & EF_FULLBRIGHT ))
    {
        lightinfo->shadelight = 0;
        lightinfo->ambientlight = 192;

        VectorSet( *lightinfo->plightvec, 0.0f, 0.0f, -1.0f );
        VectorSet( lightinfo->color, 1.0f, 1.0f, 1.0f );
        return;
    }

    // determine plane to get lightvalues from: ceil or floor
    if(  ent->curstate.effects & EF_INVLIGHT )
        VectorSet( lightDir, 0.0f, 0.0f, 1.0f );
    else VectorSet( lightDir, 0.0f, 0.0f, -1.0f );

    VectorCopy( ent->origin, origin );

    VectorSet( vecSrc, origin[0], origin[1], origin[2] - lightDir[2] * 8.0f );
    light.r = light.g = light.b = light.a = 0;

    if(( mv->skycolor_r + mv->skycolor_g + mv->skycolor_b ) != 0 )
    {
        msurface_t	*psurf = NULL;
        pmtrace_t		trace;

        if( host.features & ENGINE_WRITE_LARGE_COORD )
        {
            vecEnd[0] = origin[0] - mv->skyvec_x * 65536.0f;
            vecEnd[1] = origin[1] - mv->skyvec_y * 65536.0f;
            vecEnd[2] = origin[2] - mv->skyvec_z * 65536.0f;
        }
        else
        {
            vecEnd[0] = origin[0] - mv->skyvec_x * 8192.0f;
            vecEnd[1] = origin[1] - mv->skyvec_y * 8192.0f;
            vecEnd[2] = origin[2] - mv->skyvec_z * 8192.0f;
        }

        trace = CL_TraceLine( vecSrc, vecEnd, PM_WORLD_ONLY );
        if( trace.ent > 0 ) psurf = PM_TraceSurface( &clgame.pmove->physents[trace.ent], vecSrc, vecEnd );
        else psurf = PM_TraceSurface( clgame.pmove->physents, vecSrc, vecEnd );

        if( ( psurf && (psurf->flags & SURF_DRAWSKY )))
        {
            VectorSet( lightDir, mv->skyvec_x, mv->skyvec_y, mv->skyvec_z );

            light.r = ( bound( 0, mv->skycolor_r, 255 ));
            light.g = ( bound( 0, mv->skycolor_g, 255 ));
            light.b = ( bound( 0, mv->skycolor_b, 255 ));
        }
    }

    if(( light.r + light.g + light.b ) < 16 ) // TESTTEST
    {
        colorVec	gcolor;
        float	grad[4];

        VectorScale( lightDir, 2048.0f, vecEnd );
        VectorAdd( vecEnd, vecSrc, vecEnd );

        light = R_LightVec( vecSrc, vecEnd, g_studiolight.lightspot, g_studiolight.lightvec );

        if( VectorIsNull( g_studiolight.lightvec ))
        {
            vec3_t spot, vec;
            vecSrc[0] -= 16.0f;
            vecSrc[1] -= 16.0f;
            vecEnd[0] -= 16.0f;
            vecEnd[1] -= 16.0f;

            gcolor = R_LightVec( vecSrc, vecEnd, spot, vec );
            grad[0] = ( gcolor.r + gcolor.g + gcolor.b ) / 768.0f;

            vecSrc[0] += 32.0f;
            vecEnd[0] += 32.0f;

            gcolor = R_LightVec( vecSrc, vecEnd, spot, vec );
            grad[1] = ( gcolor.r + gcolor.g + gcolor.b ) / 768.0f;

            vecSrc[1] += 32.0f;
            vecEnd[1] += 32.0f;

            gcolor = R_LightVec( vecSrc, vecEnd, spot, vec );
            grad[2] = ( gcolor.r + gcolor.g + gcolor.b ) / 768.0f;

            vecSrc[0] -= 32.0f;
            vecEnd[0] -= 32.0f;

            gcolor = R_LightVec( vecSrc, vecEnd, spot, vec );
            grad[3] = ( gcolor.r + gcolor.g + gcolor.b ) / 768.0f;

            lightDir[0] = grad[0] - grad[1] - grad[2] + grad[3];
            lightDir[1] = grad[1] + grad[0] - grad[2] - grad[3];
            VectorNormalize( lightDir );
        }
        else
        {
            VectorCopy( g_studiolight.lightvec, lightDir );
        }
    }

    VectorSet( finalLight, light.r, light.g, light.b );
    ent->cvFloorColor = light;

    total = max( max( light.r, light.g ), light.b );
    if( total == 0.0f ) total = 1.0f;

    // scale lightdir by light intentsity
    VectorScale( lightDir, total, lightDir );

    for( lnum = 0, dl = cl_dlights; lnum < MAX_DLIGHTS; lnum++, dl++ )
    {
        if( dl->die < RI.refdef.time || !r_dynamic->value )
            continue;

        VectorSubtract( ent->origin, dl->origin, dist );

        radius = VectorLength( dist );
        add = (dl->radius - radius);

        if( add > 0.0f )
        {
            total += add;

            if( radius > 1.0f )
                VectorScale( dist, ( add / radius ), dist );
            else VectorScale( dist, add, dist );

            VectorAdd( lightDir, dist, lightDir );

            finalLight[0] += ( dl->color.r ) * ( add / 256.0f ) * 2.0f;
            finalLight[1] += ( dl->color.g ) * ( add / 256.0f ) * 2.0f;
            finalLight[2] += ( dl->color.b ) * ( add / 256.0f ) * 2.0f;
        }
    }

    add = 0.9f;

    VectorScale( lightDir, add, lightDir );

    lightinfo->shadelight = VectorLength( lightDir );
    lightinfo->ambientlight = total - g_studiolight.shadelight;

    total = max_element(finalLight);

    if( total > 0.0f )
    {
        lightinfo->color[0] = finalLight[0] * ( 1.0f / total );
        lightinfo->color[1] = finalLight[1] * ( 1.0f / total );
        lightinfo->color[2] = finalLight[2] * ( 1.0f / total );
    }
    else VectorSet( lightinfo->color, 1.0f, 1.0f, 1.0f );

    if( lightinfo->ambientlight > 128 )
        lightinfo->ambientlight = 128;

    if( lightinfo->ambientlight + g_studiolight.shadelight > 255 )
        lightinfo->shadelight = 255 - g_studiolight.ambientlight;

    *lightinfo->plightvec = lightDir.Normalize();
}

/*
===============
pfnStudioEntityLight

===============
*/
void GAME_EXPORT R_StudioEntityLight( alight_t *lightinfo )
{
    int		lnum, i, j, k;
    float		minstrength, dist2, f, r2;
    float		lstrength[MAX_LOCALLIGHTS];
    cl_entity_t	*ent = RI.currententity;
    vec3_t		mid, origin, pos;
    dlight_t		*el;

    g_studiolight.numlocallights = 0;

    if( !ent || !r_dynamic->value )
        return;

    for( i = 0; i < MAX_LOCALLIGHTS; i++ )
        lstrength[i] = 0;

    Matrix3x4_OriginFromMatrix( g_rotationmatrix, origin );
    dist2 = 1000000.0f;
    k = 0;

    for( lnum = 0, el = cl_elights; lnum < MAX_ELIGHTS; lnum++, el++ )
    {
        if( el->die < RI.refdef.time || el->radius <= 0.0f )
            continue;

        if(( el->key & 0xFFF ) == ent->index )
        {
            int	att = (el->key >> 12) & 0xF;

            if( att ) VectorCopy( ent->attachment[att], el->origin );
            else VectorCopy( ent->origin, el->origin );
        }

        VectorCopy( el->origin, pos );
        VectorSubtract( origin, el->origin, mid );

        f = DotProduct( mid, mid );
        r2 = el->radius * el->radius;

        if( f > r2 ) minstrength = r2 / f;
        else minstrength = 1.0f;

        if( minstrength > 0.05f )
        {
            if( g_studiolight.numlocallights >= MAX_LOCALLIGHTS )
            {
                for( j = 0, k = -1; j < g_studiolight.numlocallights; j++ )
                {
                    if( lstrength[j] < dist2 && lstrength[j] < minstrength )
                    {
                        dist2 = lstrength[j];
                        k = j;
                    }
                }
            }
            else k = g_studiolight.numlocallights;

            if( k != -1 )
            {
                g_studiolight.locallightcolor[k].r = ( el->color.r ) * (1.0f / 255.0f);
                g_studiolight.locallightcolor[k].g = ( el->color.g ) * (1.0f / 255.0f);
                g_studiolight.locallightcolor[k].b = ( el->color.b ) * (1.0f / 255.0f);
                g_studiolight.locallightR2[k] = r2;
                g_studiolight.locallight[k] = el;
                lstrength[k] = minstrength;

                if( k >= g_studiolight.numlocallights )
                    g_studiolight.numlocallights = k + 1;
            }
        }
    }
}

/*
===============
R_StudioSetupLighting

===============
*/
void GAME_EXPORT R_StudioSetupLighting( alight_t *lightinfo )
{
    float	scale = 1.0f;
	int		i;

	if( !m_pStudioHeader || !lightinfo )
		return;


    if( RI.currententity != NULL )
        scale = RI.currententity->curstate.scale;

    g_studiolight.ambientlight = lightinfo->ambientlight;
    g_studiolight.shadelight = lightinfo->shadelight;
    VectorCopy( *lightinfo->plightvec, g_studiolight.lightvec );

	for( i = 0; i < m_pStudioHeader->numbones; i++ )
    {
		Matrix3x4_VectorIRotate( g_lighttransform[i], *lightinfo->plightvec, g_studiolight.blightvec[i] );
        //if( scale > 1.0f ) VectorNormalize( g_studiolight.blightvec[i] ); // in case model may be scaled
    }

	// copy custom alight color in case of mod-maker changed it
    g_studiolight.lightcolor = lightinfo->color;

	float illum = std::clamp(g_studiolight.ambientlight + g_studiolight.shadelight * 0.8f, 0.0f, 255.0f);
	g_studiolight.lightval = g_studiolight.lightcolor * (illum / 255.0f);
}

/*
===============
R_StudioLighting

===============
*/
void R_StudioLighting( vec3_t_ref lv, int bone, int flags, const vec3_t normal )
{
	float		illum;

	if( !RI.drawWorld || RI.currententity->curstate.effects & EF_FULLBRIGHT || flags & STUDIO_NF_FULLBRIGHT)
	{
		VectorSet( lv, 1.0f, 1.0f, 1.0f );
		return;
	}

	lv = g_studiolight.lightval;
}

/*
====================
R_LightLambert
====================
*/
void R_LightLambert( vec4_t light[MAX_LOCALLIGHTS], const vec3_t normal, const vec3_t color, byte *out )
{
    vec3_t	finalLight = color;

    for( int i = 0; i < g_studiolight.numlocallights; i++ )
    {
        float	r, r2;

        if( tr.fFlipViewModel )
            r = DotProduct( normal, light[i] );
        else r = -DotProduct( normal, light[i] );

        if( r > 0.0f )
        {
            if( light[i][3] == 0.0f )
            {
                r2 = DotProduct( light[i], light[i] );

                if( r2 > 0.0f )
                    light[i][3] = g_studiolight.locallightR2[i] / ( r2 * sqrt( r2 ));
                else light[i][3] = 0.0001f;
            }

            constexpr vec3_t ones = { 1.0f, 1.0f, 1.0f };
            vec3_t col = vec3_t( g_studiolight.locallightcolor[i].r, g_studiolight.locallightcolor[i].g, g_studiolight.locallightcolor[i].b )  / 255.0f;
            vec3_t localLight = col * r * light[i][3];
            finalLight = mins( finalLight + localLight, ones );
        }
    }

    out[0] = finalLight[0] * 255;
    out[1] = finalLight[1] * 255;
    out[2] = finalLight[2] * 255;
}

/*
====================
R_LightStrength
====================
*/
void R_LightStrength( int bone, const vec3_t localpos, vec4_t light[MAX_LOCALLIGHTS] )
{
    int	i;

    if( g_studiolight.lightage[bone] != tr.framecount )
    {
        for( i = 0; i < g_studiolight.numlocallights; i++ )
        {
            dlight_t *el = g_studiolight.locallight[i];
            Matrix3x4_VectorITransform( g_lighttransform[bone], el->origin, g_studiolight.lightbonepos[bone][i] );
        }

        g_studiolight.lightage[bone] = tr.framecount;
    }

    for( i = 0; i < g_studiolight.numlocallights; i++ )
    {
        VectorSubtract( localpos, g_studiolight.lightbonepos[bone][i], light[i] );
        light[i][3] = 0.0f;
    }
}

/*
===============
R_StudioSetupTextureHeader

===============
*/
void R_StudioSetupTextureHeader( void )
{
	m_pTextureHeader = m_pStudioHeader;
}

/*
===============
R_StudioSetupSkin

===============
*/
static void GAME_EXPORT R_StudioSetupSkin( mstudiotexture_t *ptexture, int index )
{
	short	*pskinref;
	int	m_skinnum;

	R_StudioSetupTextureHeader ();

	if( !m_pTextureHeader ) return;

	// NOTE: user can comment call StudioRemapColors and remap_info will be unavailable
	if( m_fDoRemap ) ptexture = CL_GetRemapInfoForEntity( RI.currententity )->ptexture;

	// safety bounding the skinnum
	m_skinnum = bound( 0, RI.currententity->curstate.skin, ( m_pTextureHeader->numskinfamilies - 1 ));
	pskinref = (short *)((byte *)m_pTextureHeader + m_pTextureHeader->skinindex);
	if( m_skinnum != 0 && m_skinnum < m_pTextureHeader->numskinfamilies )
		pskinref += (m_skinnum * m_pTextureHeader->numskinref);

    xe::TexLru_Bind(&ptexture[pskinref[index]]);
}

/*
===============
R_StudioGetTexture

Doesn't changes studio global state at all
===============
*/
mstudiotexture_t *R_StudioGetTexture( cl_entity_t *e )
{
	mstudiotexture_t	*ptexture;
	studiohdr_t	*phdr, *thdr;

	if(( phdr = (studiohdr_t *)Mod_Extradata( e->model )) == NULL )
		return NULL;

	thdr = m_pStudioHeader;

	if( !thdr ) return NULL;	

	if( m_fDoRemap ) ptexture = CL_GetRemapInfoForEntity( e )->ptexture;
	else ptexture = (mstudiotexture_t *)((byte *)thdr + thdr->textureindex);

	return ptexture;
}

void GAME_EXPORT R_StudioSetRenderamt( int iRenderamt )
{
	if( !RI.currententity ) return;

	RI.currententity->curstate.renderamt = iRenderamt;
	RI.currententity->curstate.renderamt = R_ComputeFxBlend( RI.currententity );
}

/*
===============
R_StudioSetCullState

sets true for enable backculling (for left-hand viewmodel)
===============
*/
void GAME_EXPORT R_StudioSetCullState( int iCull )
{
	g_iBackFaceCull = iCull;
}

/*
===============
R_StudioRenderShadow

just a prefab for render shadow
===============
*/
void GAME_EXPORT R_StudioRenderShadow( int iSprite, float *p1, float *p2, float *p3, float *p4 )
{
	if( !p1 || !p2 || !p3 || !p4 )
		return;

	if( TriSpriteTexture( Mod_Handle( iSprite ), 0 ))
	{
		pglEnable( GL_BLEND );
		pglDisable( GL_ALPHA_TEST );
		pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		pglColor4f( 0.0f, 0.0f, 0.0f, 1.0f ); // render only alpha

		pglBegin( GL_QUADS );
			pglTexCoord2f( 0.0f, 0.0f );
			pglVertex3fv( p1 );
			pglTexCoord2f( 0.0f, 1.0f );
			pglVertex3fv( p2 );
			pglTexCoord2f( 1.0f, 1.0f );
			pglVertex3fv( p3 );
			pglTexCoord2f( 1.0f, 0.0f );
			pglVertex3fv( p4 );
		pglEnd();

		pglDisable( GL_BLEND );
		pglDisable( GL_ALPHA_TEST );
	}
}

/*
===============
R_StudioMeshCompare

Sorting opaque entities by model type
===============
*/
static bool R_StudioMeshCompare( const sortedmesh_t &a, const sortedmesh_t &b )
{
	return ((a.flags & STUDIO_NF_TRANSPARENT) && !(a.flags & STUDIO_NF_ADDITIVE)) && !((b.flags & STUDIO_NF_TRANSPARENT) && !(b.flags & STUDIO_NF_ADDITIVE));
}

/*
===============
R_StudioDrawMesh

===============
*/
static void (*R_MeshUVFunc())(const short* ptricmds, vec2_t &uv, float s, float t)
{
	if (g_nFaceFlags & STUDIO_NF_CHROME || (g_nForceFaceFlags & STUDIO_NF_CHROME))
	{
		return [](const short* ptricmds, vec2_t& uv, float s, float t) {
			uv[0] = g_chrome[ptricmds[1]][0] * s;
			uv[1] = g_chrome[ptricmds[1]][1] * t;
		};
	}
	else if (g_nFaceFlags & STUDIO_NF_UV_COORDS)
	{
		return [](const short* ptricmds, vec2_t& uv, float s, float t) {
			uv[0] = HalfToFloat(ptricmds[2]);
			uv[1] = HalfToFloat(ptricmds[3]);
		};
	}
	else [[likely]]
	{
		return [](const short* ptricmds, vec2_t& uv, float s, float t) {
			uv[0] = ptricmds[2] * s;
			uv[1] = ptricmds[3] * t;
		};
	}
}

static void (*R_MeshColorFunc())(const short* ptricmds, GLubyte (&cl)[4], float alpha)
{
	if (RI.currententity->curstate.renderfx == kRenderFxWallHack) //kRenderWallHack
	{
		return [](const short* ptricmds, GLubyte(&cl)[4], float alpha) {
			cl[0] = alpha;
			cl[1] = 0;
			cl[2] = 0;
			cl[3] = 255;
		};
	}
	else if (g_iRenderMode == kRenderTransColor || (g_nForceFaceFlags & STUDIO_NF_CHROME))
	{
		return [](const short* ptricmds, GLubyte(&cl)[4], float alpha) {
			color24* clr;
			clr = &RI.currententity->curstate.rendercolor;
			cl[0] = clr->r;
			cl[1] = clr->g;
			cl[2] = clr->b;
			cl[3] = alpha;
		};
	}
	else if (g_iRenderMode == kRenderTransAdd || (g_nForceFaceFlags & STUDIO_NF_FULLBRIGHT))
	{
		return [](const short* ptricmds, GLubyte(&cl)[4], float alpha) {
			cl[0] = cl[1] = cl[2] = 255;
			cl[3] = alpha;
		};
	}
	else [[likely]]
	{
		return [](const short* ptricmds, GLubyte(&cl)[4], float alpha) {
			vec3_t lv = g_lightvalues[ptricmds[1]];
			R_LightLambert(g_studiolight.lightpos[ptricmds[0]], g_xformnorms[ptricmds[1]], lv, cl);
			cl[3] = alpha;
		};
	}
}

static void R_StudioDrawMesh( short *ptricmds, float s, float t, float a, float scale )
{
	GLubyte alpha = 255 * a;
	int i;
	uint startArrayVerts = g_nNumArrayVerts;
	uint startArrayElems = g_nNumArrayElems;

	const auto R_MeshColor = R_MeshColorFunc();
	const auto R_MeshUV = R_MeshUVFunc();

	while( ( i = *( ptricmds++ ) ) )
	{
		int	vertexState = 0;
		bool tri_strip = i >= 0;

		if( !tri_strip )
		{
			i = -i;
		}

		r_stats.c_studio_polys += (i - 2);

		for( ; i > 0; i--, ptricmds += 4 )
		{
			GLubyte cl[4];
			// build in indices
			if( vertexState++ < 3 )
			{
				g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts;
			}
			else if( tri_strip )
			{
				// flip triangles between clockwise and counter clockwise
				if( vertexState & 1 )
				{
					// draw triangle [n-2 n-1 n]
					g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts - 2;
					g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts - 1;
					g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts;
				}
				else
				{
					// draw triangle [n-1 n-2 n]
					g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts - 1;
					g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts - 2;
					g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts;
				}
			}
			else
			{
				// draw triangle fan [0 n-1 n]
				g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts - ( vertexState - 1 );
				g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts - 1;
				g_xarrayelems[g_nNumArrayElems++] = g_nNumArrayVerts;
			}

			R_MeshUV(ptricmds, g_xarraycoord[g_nNumArrayVerts], s, t);
			R_MeshColor(ptricmds, g_xarraycolor[g_nNumArrayVerts], alpha);

			vec3_t av = g_xformverts[ptricmds[0]];
			if( g_nForceFaceFlags & STUDIO_NF_CHROME )
			{
				vec3_t nv = g_xformnorms[ptricmds[1]];
				VectorMA( av, scale, nv, av );
			}

			ASSERT( g_nNumArrayVerts < MAXARRAYVERTS );
			VectorCopy( av, g_xarrayverts[g_nNumArrayVerts] ); // store off vertex
			g_nNumArrayVerts++;
		}
	}

	pglEnableClientState( GL_VERTEX_ARRAY );
	pglVertexPointer( 3, GL_FLOAT, sizeof(vec3_t), g_xarrayverts );

	pglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	pglTexCoordPointer( 2, GL_FLOAT, 0, g_xarraycoord );

	if( !( g_nForceFaceFlags & STUDIO_NF_CHROME ) )
	{
		pglEnableClientState( GL_COLOR_ARRAY );
		pglColorPointer( 4, GL_UNSIGNED_BYTE, 0, g_xarraycolor );
	}

#if !defined XASH_NANOGL || defined XASH_WES && defined __EMSCRIPTEN__ // WebGL need to know array sizes
	if( pglDrawRangeElements )
		pglDrawRangeElements( GL_TRIANGLES, startArrayVerts, g_nNumArrayVerts,
			g_nNumArrayElems - startArrayElems, GL_UNSIGNED_SHORT, &g_xarrayelems[startArrayElems] );
	else
#endif
		pglDrawElements( GL_TRIANGLES, g_nNumArrayElems - startArrayElems, GL_UNSIGNED_SHORT, &g_xarrayelems[startArrayElems] );
	pglDisableClientState( GL_VERTEX_ARRAY );
	pglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	if( !( g_nForceFaceFlags & STUDIO_NF_CHROME ) )
		pglDisableClientState( GL_COLOR_ARRAY );
}

/*
===============
R_StudioDrawMeshes

===============
*/
static void R_StudioDrawMeshes( mstudiotexture_t *ptexture, short *pskinref, float scale )
{
	int		j;
	mstudiomesh_t	*pmesh;

	for( j = 0; j < m_pSubModel->nummesh; j++ )
	{
		float	s, t, alpha;
		short	*ptricmds;

		pmesh = g_sortedMeshes[j].mesh;
		ptricmds = (short *)((byte *)m_pStudioHeader + pmesh->triindex);

        auto &&attr = xe::TexLru_GetAttr(&ptexture[pskinref[pmesh->skinref]]);
		g_nFaceFlags = attr.flags;
		s = 1.0f / (float)attr.width;
		t = 1.0f / (float)attr.height;

		if( g_iRenderMode != kRenderTransAdd )
			pglDepthMask( GL_TRUE );
		else pglDepthMask( GL_FALSE );

		if( g_nForceFaceFlags & STUDIO_NF_CHROME )
		{
			color24	*clr;
			clr = &RI.currententity->curstate.rendercolor;
			pglColor4ub( clr->r, clr->g, clr->b, 255 );
			alpha = 1.0f;
		}
		else if (g_nForceFaceFlags & STUDIO_NF_OUTLINE)
		{
			GL_Bind(XASH_TEXTURE0, tr.whiteTexture);
			pglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			pglLineWidth(5.0);
			//pglEnable(GL_LINE_SMOOTH);
			if (RI.currententity->curstate.renderfx == kRenderFxGreenOutLine) {
				pglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				RI.currententity->curstate.rendercolor.r = 100;
				RI.currententity->curstate.rendercolor.g = 200;
				RI.currententity->curstate.rendercolor.b = 150;

				alpha = 0.5f;

				pglEnable(GL_STENCIL_TEST);
				pglStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				pglDisable(GL_DEPTH_TEST);
			}
			else if (RI.currententity->curstate.renderfx == kRenderFxRedOutLine) {
				pglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				RI.currententity->curstate.rendercolor.r = 200;
				RI.currententity->curstate.rendercolor.g = 0;
				RI.currententity->curstate.rendercolor.b = 0;

				alpha = 0.5f;

				pglEnable(GL_STENCIL_TEST);
				pglStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				pglDisable(GL_DEPTH_TEST);
			}
			else {
				pglBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				RI.currententity->curstate.rendercolor.r = 1;
				RI.currententity->curstate.rendercolor.g = 1;
				RI.currententity->curstate.rendercolor.b = 1;

				alpha = 1.0f;

				pglEnable(GL_STENCIL_TEST);
				pglStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			}

		}
		else if( g_nFaceFlags & STUDIO_NF_TRANSPARENT && R_StudioOpaque( RI.currententity ))
		{
			GL_SetRenderMode( kRenderTransAlpha );
			pglAlphaFunc( GL_GREATER, 0.0f );
			if (RI.currententity->curstate.rendermode == kRenderTransAlpha)
				alpha = RI.currententity->curstate.renderamt * (1.0f / 255.0f);
			else
				alpha = 1.0f;
		}
		else if( g_nFaceFlags & STUDIO_NF_ADDITIVE )
		{
			GL_SetRenderMode( kRenderTransAdd );
			alpha = RI.currententity->curstate.renderamt * (1.0f / 255.0f);
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE );
			pglDepthMask( GL_FALSE );
		}
		else if( g_nFaceFlags & STUDIO_NF_ALPHA && !( host.features & ENGINE_DISABLE_HDTEXTURES )) // Paranoia2 collision flag
		{
			GL_SetRenderMode( kRenderTransTexture );
			alpha = RI.currententity->curstate.renderamt * (1.0f / 255.0f);
			pglDepthMask( GL_FALSE );
		}
		else
		{
			GL_SetRenderMode( g_iRenderMode );

			if( g_iRenderMode == kRenderNormal )
			{
				if( gl_overbright_studio->integer )
				{
					pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
					pglTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE );
					pglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB );
					pglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE );
					pglTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2 );
				}
				else
					pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
				alpha = 1.0f;
			}
			else alpha = RI.currententity->curstate.renderamt * (1.0f / 255.0f);
		}

		if( !( g_nForceFaceFlags & STUDIO_NF_CHROME ) && !(g_nForceFaceFlags & STUDIO_NF_OUTLINE))
		{
            xe::TexLru_Bind(&ptexture[pskinref[pmesh->skinref]]);
		}

		if (RI.currententity->curstate.renderfx == kRenderFxWallHack) {
			GL_Bind(XASH_TEXTURE0, tr.whiteTexture);
			pglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (g_nForceFaceFlags & STUDIO_NF_FLATSHADE) {
				alpha = 1.0f;
				pglDepthFunc(GL_LEQUAL);  //           //draw front first						
				pglDepthMask(GL_TRUE);
			}
			else {

				alpha = 0.5f;
				pglDepthFunc(GL_GEQUAL);
				pglDepthMask(GL_FALSE);
			}
		}

		if (RI.currententity->syncbase == 1.0f)   //set stencil
		{
			pglEnable(GL_STENCIL_TEST);
			pglStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

			pglStencilMask(0xFF);  // �����޸�ģ�建��			
			pglStencilFunc(GL_ALWAYS, 1, 0xFF); //����Ƭ�ζ�Ҫд��ģ�建��
		}

		R_StudioDrawMesh( ptricmds, s, t, alpha, scale );
	}
}

/*
===============
R_StudioDrawPoints

===============
*/
static void GAME_EXPORT R_StudioDrawPoints( void )
{
	int		m_skinnum;
	byte		*pvertbone;
	byte		*pnormbone;
	mstudiotexture_t	*ptexture;
	mstudiomesh_t	*pmesh;
	short		*pskinref;
	float		*lv, scale = 0.0f;

	R_StudioSetupTextureHeader ();

	g_nNumArrayVerts = g_nNumArrayElems = 0;

	if( !m_pTextureHeader || !m_pTextureHeader->numskinfamilies ) return;
	if( RI.currententity->curstate.renderfx == kRenderFxGlowShell )
		g_nStudioCount++;

	// safety bounding the skinnum
	m_skinnum = bound(0, RI.currententity->curstate.skin, (m_pTextureHeader->numskinfamilies - 1));
	pvertbone = ((byte *)m_pStudioHeader + m_pSubModel->vertinfoindex);
	pnormbone = ((byte *)m_pStudioHeader + m_pSubModel->norminfoindex);

	// NOTE: user can comment call StudioRemapColors and remap_info will be unavailable
	if( m_fDoRemap ) ptexture = CL_GetRemapInfoForEntity( RI.currententity )->ptexture;
	else ptexture = (mstudiotexture_t *)((byte *)m_pTextureHeader + m_pTextureHeader->textureindex);

	ASSERT( ptexture != NULL );

	pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex);

	pskinref = (short *)((byte *)m_pTextureHeader + m_pTextureHeader->skinindex);
	if( m_skinnum != 0 && m_skinnum < m_pTextureHeader->numskinfamilies )
		pskinref += (m_skinnum * m_pTextureHeader->numskinref);

	if( m_pSubModel->numverts > MAXSTUDIOVERTS )
		m_pSubModel->numverts = MAXSTUDIOVERTS;

#ifdef STUDIO_SKINNING_OMP_MIN
#pragma omp parallel for private(i) if(m_pSubModel->numverts > STUDIO_SKINNING_OMP_MIN)
#endif
	for (int i = 0; i < m_pSubModel->numverts; ++i)
	{
		Matrix3x4_VectorTransform(g_bonestransform[pvertbone[i]], pstudioverts[i], g_xformverts[i]);
	}

	if (RI.currententity->curstate.scale != 1.0f)
	{
		//scale multi by Rentro
		float scale_offset = -RI.currententity->curstate.mins[2];
		for (int i = 0; i < m_pSubModel->numverts; ++i) 
		{
			float temp = RI.currententity->curstate.scale - 1.0f;
			g_xformverts[i] = (g_xformverts[i] - RI.currententity->origin) * (temp * 2);
			g_xformverts[i][2] += scale_offset * temp;
		}
	};

	if( g_nForceFaceFlags & STUDIO_NF_CHROME )
	{
		scale = RI.currententity->curstate.renderamt * (1.0f / 255.0f);

		for(int i = 0; i < m_pSubModel->numnorms; i++ )
			Matrix3x4_VectorRotate( g_bonestransform[pnormbone[i]], pstudionorms[i], g_xformnorms[i] );
	}

	for( int j = 0; j < m_pSubModel->nummesh; j++ )
	{
        auto &&attr = xe::TexLru_GetAttr(&ptexture[pskinref[pmesh[j].skinref]]);
		g_nFaceFlags = attr.flags;

		// fill in sortedmesh info
		g_sortedMeshes[j].mesh = &pmesh[j];
		g_sortedMeshes[j].flags = g_nFaceFlags;
		for (int i = 0; i < pmesh[j].numnorms; ++i)
		{
			R_StudioLighting(g_lightvalues[i], pnormbone[i], g_nFaceFlags, pstudionorms[i]);
		};

		if(( g_nFaceFlags & STUDIO_NF_CHROME ) || ( g_nForceFaceFlags & STUDIO_NF_CHROME ))
		{
			for (int i = 0; i < pmesh[j].numnorms; ++i)
			{
				R_StudioSetupChrome(g_chrome[i], pnormbone[i], pstudionorms[i]);
			};
		}
	}

	if( r_studio_sort_textures->integer )
	{
		// sort opaque and translucent for right results
		std::sort( g_sortedMeshes, g_sortedMeshes + m_pSubModel->nummesh, R_StudioMeshCompare );
	}

	R_StudioDrawMeshes( ptexture, pskinref, scale );

	// restore depthmask for next call StudioDrawPoints
	if( g_iRenderMode != kRenderTransAdd )
		pglDepthMask( GL_TRUE );
}

/*
===============
R_StudioDrawHulls

===============
*/
static void GAME_EXPORT R_StudioDrawHulls( void )
{
	int	i, j;
	float	alpha;

	if( r_drawentities->integer == 4 )
		alpha = 0.5f;
	else alpha = 1.0f;

	pglDisable( GL_TEXTURE_2D );

	for( i = 0; i < m_pStudioHeader->numhitboxes; i++ )
	{
		mstudiobbox_t	*pbboxes = (mstudiobbox_t *)((byte *)m_pStudioHeader + m_pStudioHeader->hitboxindex);
		vec3_t		v[8], v2[8], bbmin, bbmax;

		VectorCopy( pbboxes[i].bbmin, bbmin );
		VectorCopy( pbboxes[i].bbmax, bbmax );

		v[0][0] = bbmin[0];
		v[0][1] = bbmax[1];
		v[0][2] = bbmin[2];

		v[1][0] = bbmin[0];
		v[1][1] = bbmin[1];
		v[1][2] = bbmin[2];

		v[2][0] = bbmax[0];
		v[2][1] = bbmax[1];
		v[2][2] = bbmin[2];

		v[3][0] = bbmax[0];
		v[3][1] = bbmin[1];
		v[3][2] = bbmin[2];

		v[4][0] = bbmax[0];
		v[4][1] = bbmax[1];
		v[4][2] = bbmax[2];

		v[5][0] = bbmax[0];
		v[5][1] = bbmin[1];
		v[5][2] = bbmax[2];

		v[6][0] = bbmin[0];
		v[6][1] = bbmax[1];
		v[6][2] = bbmax[2];

		v[7][0] = bbmin[0];
		v[7][1] = bbmin[1];
		v[7][2] = bbmax[2];

		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[0], v2[0] );
		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[1], v2[1] );
		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[2], v2[2] );
		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[3], v2[3] );
		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[4], v2[4] );
		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[5], v2[5] );
		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[6], v2[6] );
		Matrix3x4_VectorTransform( g_bonestransform[pbboxes[i].bone], v[7], v2[7] );

		j = (pbboxes[i].group % 8);

		// set properly color for hull
		pglColor4f( hullcolor[j][0], hullcolor[j][1], hullcolor[j][2], alpha );

		pglBegin( GL_QUAD_STRIP );
		for( j = 0; j < 10; j++ )
			pglVertex3fv( v2[j & 7].data() );
		pglEnd( );
	
		pglBegin( GL_QUAD_STRIP );
		pglVertex3fv( v2[6].data() );
		pglVertex3fv( v2[0].data() );
		pglVertex3fv( v2[4].data() );
		pglVertex3fv( v2[2].data() );
		pglEnd( );

		pglBegin( GL_QUAD_STRIP );
		pglVertex3fv( v2[1].data() );
		pglVertex3fv( v2[7].data() );
		pglVertex3fv( v2[3].data() );
		pglVertex3fv( v2[5].data() );
		pglEnd( );			
	}

	pglEnable( GL_TEXTURE_2D );
}

/*
===============
R_StudioDrawAbsBBox

===============
*/
static void GAME_EXPORT R_StudioDrawAbsBBox( void )
{
	vec3_t	bbox[8];
	int	i;

	// looks ugly, skip
	if( RI.currententity == &clgame.viewent )
		return;

	if( !R_StudioComputeBBox( RI.currententity, bbox ))
		return;

	pglDisable( GL_TEXTURE_2D );
	pglDisable( GL_DEPTH_TEST );

	pglColor4f( 1.0f, 0.0f, 0.0f, 1.0f );	// red bboxes for studiomodels
	pglBegin( GL_LINES );

	for( i = 0; i < 2; i += 1 )
	{
		pglVertex3fv( bbox[i+0].data() );
		pglVertex3fv( bbox[i+2].data() );
		pglVertex3fv( bbox[i+4].data() );
		pglVertex3fv( bbox[i+6].data() );
		pglVertex3fv( bbox[i+0].data() );
		pglVertex3fv( bbox[i+4].data() );
		pglVertex3fv( bbox[i+2].data() );
		pglVertex3fv( bbox[i+6].data() );
		pglVertex3fv( bbox[i*2+0].data() );
		pglVertex3fv( bbox[i*2+1].data() );
		pglVertex3fv( bbox[i*2+4].data() );
		pglVertex3fv( bbox[i*2+5].data() );
	}

	pglEnd();
	pglEnable( GL_TEXTURE_2D );
	pglEnable( GL_DEPTH_TEST );
}

/*
===============
R_StudioDrawBones

===============
*/
static void GAME_EXPORT R_StudioDrawBones( void )
{
	mstudiobone_t	*pbones = (mstudiobone_t *) ((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);
	vec3_t		point;
	int		i;

	pglDisable( GL_TEXTURE_2D );

	for( i = 0; i < m_pStudioHeader->numbones; i++ )
	{
		if( pbones[i].parent >= 0 )
		{
			pglPointSize( 3.0f );
			pglColor3f( 1, 0.7f, 0 );
			pglBegin( GL_LINES );
			
			Matrix3x4_OriginFromMatrix( g_bonestransform[pbones[i].parent], point );
			pglVertex3fv( point.data() );
			Matrix3x4_OriginFromMatrix( g_bonestransform[i], point );
			pglVertex3fv( point.data() );
			
			pglEnd();

			pglColor3f( 0, 0, 0.8f );
			pglBegin( GL_POINTS );
			if( pbones[pbones[i].parent].parent != -1 )
			{
				Matrix3x4_OriginFromMatrix( g_bonestransform[pbones[i].parent], point );
				pglVertex3fv( point.data() );
			}
			Matrix3x4_OriginFromMatrix( g_bonestransform[i], point );
			pglVertex3fv( point.data() );
			pglEnd();
		}
		else
		{
			// draw parent bone node
			pglPointSize( 5.0f );
			pglColor3f( 0.8f, 0, 0 );
			pglBegin( GL_POINTS );
			Matrix3x4_OriginFromMatrix( g_bonestransform[i], point );
			pglVertex3fv( point.data() );
			pglEnd();
		}
	}

	pglPointSize( 1.0f );
	pglEnable( GL_TEXTURE_2D );
}

static void R_StudioDrawAttachments( void )
{
	int	i;
	
	pglDisable( GL_TEXTURE_2D );
	pglDisable( GL_DEPTH_TEST );
	
	for( i = 0; i < m_pStudioHeader->numattachments; i++ )
	{
		mstudioattachment_t	*pattachments;
		vec3_t		v[4];

		pattachments = (mstudioattachment_t *)((byte *)m_pStudioHeader + m_pStudioHeader->attachmentindex);		
		Matrix3x4_VectorTransform( g_bonestransform[pattachments[i].bone], pattachments[i].org, v[0] );
		Matrix3x4_VectorTransform( g_bonestransform[pattachments[i].bone], pattachments[i].vectors[0], v[1] );
		Matrix3x4_VectorTransform( g_bonestransform[pattachments[i].bone], pattachments[i].vectors[1], v[2] );
		Matrix3x4_VectorTransform( g_bonestransform[pattachments[i].bone], pattachments[i].vectors[2], v[3] );
		
		pglBegin( GL_LINES );
		pglColor3f( 1, 0, 0 );
		pglVertex3fv( v[0].data() );
		pglColor3f( 1, 1, 1 );
		pglVertex3fv (v[1].data() );
		pglColor3f( 1, 0, 0 );
		pglVertex3fv (v[0].data() );
		pglColor3f( 1, 1, 1 );
		pglVertex3fv (v[2].data() );
		pglColor3f( 1, 0, 0 );
		pglVertex3fv (v[0].data() );
		pglColor3f( 1, 1, 1 );
		pglVertex3fv( v[3].data() );
		pglEnd();

		pglPointSize( 5.0f );
		pglColor3f( 0, 1, 0 );
		pglBegin( GL_POINTS );
		pglVertex3fv( v[0].data() );
		pglEnd();
		pglPointSize( 1.0f );
	}

	pglEnable( GL_TEXTURE_2D );
	pglEnable( GL_DEPTH_TEST );
}

/*
===============
R_StudioSetRemapColors

===============
*/
void GAME_EXPORT R_StudioSetRemapColors( int newTop, int newBottom )
{
	// update colors for viewentity
	if( RI.currententity == &clgame.viewent )
	{
		player_info_t	*pLocalPlayer;

		// copy top and bottom colors for viewmodel
		if(( pLocalPlayer = pfnPlayerInfo( clgame.viewent.curstate.number - 1 )) != NULL )
		{
			newTop = bound( 0, pLocalPlayer->topcolor, 360 );
			newBottom = bound( 0, pLocalPlayer->bottomcolor, 360 );
		}
	}

	CL_AllocRemapInfo( newTop, newBottom );

	if( CL_GetRemapInfoForEntity( RI.currententity ))
	{
		CL_UpdateRemapInfo( newTop, newBottom );
		m_fDoRemap = true;
	}
}

/*
===============
R_StudioSetupPlayerModel

===============
*/
static model_t *GAME_EXPORT R_StudioSetupPlayerModel( int index )
{
	player_info_t	*info;
	string		modelpath;

	if( cls.key_dest == key_menu && !index )
	{
		// we are in menu.
		info = &menu.playerinfo;
	}
	else
	{
		if( index < 0 || index > cl.maxclients )
			return NULL; // bad client ?
		info = &cl.players[index];
	}


	// set to invisible, skip
	if( !info->model[0] ) return NULL;

	// FS_FileExists is too slow to use it 32 times at every frame
	if( ( cls.key_dest != key_menu || index ) && cl.playermodels[index] )
		return cl.playermodels[index];

	if( !Q_stricmp( info->model, "player" ))
		Q_strncpy( modelpath, "models/player.mdl", sizeof( modelpath ));
	else
		Q_snprintf( modelpath, sizeof( modelpath ), "models/player/%s/%s.mdl", info->model, info->model );

	cl.playermodels[index] = Mod_ForName( modelpath, false );

	if( !cl.playermodels[index] )
		cl.playermodels[index] = Mod_ForName( "models/player.mdl", false );

	return cl.playermodels[index];
}

int GameStudioRenderer_StudioDrawModel(int flags)
{
#ifdef XASH_RAGDOLL
	using namespace physics;
	auto currententity = RI.currententity;

	if((flags & STUDIO_RENDER) &&
		!currententity->player && 
		/*currententity->index &&
		currententity->curstate.messagenum == cl.parsecount &&*/
		currententity->curstate.renderfx != kRenderFxDeadPlayer
		)
	{
		int entindex = currententity->index;
		auto model = currententity->model;

		int iActivityType = GetSequenceActivityType(model, &currententity->curstate);

		auto ragdoll = gPhysicsManager.FindRagdoll(entindex);
		if (!ragdoll)
		{
			auto cfg = gPhysicsManager.LoadRagdollConfig(model);

			if (cfg && cfg->state == 1 && bv_enable->value)
			{
				pStudioDraw->StudioDrawModel(0);

				ragdoll = gPhysicsManager.CreateRagdoll(cfg, entindex, m_pStudioHeader, iActivityType, false);

				goto has_ragdoll;
			}
		}
		else
		{
			/*if (!iActivityType)
			{
				gPhysicsManager.RemoveRagdoll(entindex);
				return pStudioDraw->StudioDrawModel(flags);
			}*/

		has_ragdoll:

			if (gPhysicsManager.UpdateKinematic(ragdoll, iActivityType, &currententity->curstate))
			{
				//Monster don't have barnacle animation
				/*if (ragdoll->m_iActivityType == 2)
				{
					cl_entity_t *barnacleEntity = gCorpseManager.FindBarnacleForPlayer(&currententity->curstate);

					gPhysicsManager.ApplyBarnacle(ragdoll, barnacleEntity);
				}*/
			}

			if (ragdoll->m_iActivityType > 0)
			{
				int iuser4 = currententity->curstate.iuser4;
				currententity->curstate.iuser4 = 114514;

				vec3_t saved_origin;
				VectorCopy(currententity->origin, saved_origin);
				gPhysicsManager.GetRagdollOrigin(ragdoll, currententity->origin);

				int result = pStudioDraw->StudioDrawModel(flags);

				VectorCopy(saved_origin, currententity->origin);

				currententity->curstate.iuser4 = iuser4;

				return result;
			}
			else
			{
				int iuser4 = currententity->curstate.iuser4;
				currententity->curstate.iuser4 = 114515;

				int result = pStudioDraw->StudioDrawModel(flags);

				currententity->curstate.iuser4 = iuser4;

				return result;
			}
		}
	}
#endif
	return pStudioDraw->StudioDrawModel(flags);
}

int GameStudioRenderer_StudioDrawPlayer(int flags, struct entity_state_s* pplayer)
{
#ifdef XASH_RAGDOLL
	using namespace physics;
	if (flags & STUDIO_RENDER)
	{
		auto currententity = RI.currententity;

		int playerindex = pplayer->number;

		auto model = R_StudioSetupPlayerModel(playerindex - 1);

		if (!model)
			return pStudioDraw->StudioDrawPlayer(flags, pplayer);

		int iActivityType = GetSequenceActivityType(model, pplayer);

		auto ragdoll = gPhysicsManager.FindRagdoll(playerindex);

		if (!ragdoll)
		{
			auto cfg = gPhysicsManager.LoadRagdollConfig(model);

			if (cfg && cfg->state == 1 && bv_enable->value)
			{
				pStudioDraw->StudioDrawPlayer(0, pplayer);

				ragdoll = gPhysicsManager.CreateRagdoll(cfg, playerindex, m_pStudioHeader, iActivityType, true);

				goto has_ragdoll;
			}
		}
		else
		{
			//model changed ?
			if (ragdoll->m_studiohdr != Mod_Extradata(model))
			{
				gPhysicsManager.RemoveRagdoll(playerindex);
				return pStudioDraw->StudioDrawPlayer(flags, pplayer);
			}

		has_ragdoll:

			int oldActivityType = ragdoll->m_iActivityType;

			if (gPhysicsManager.UpdateKinematic(ragdoll, iActivityType, pplayer))
			{
				//Transform from whatever to barnacle
				/*if (ragdoll->m_iActivityType == 2)
				{
					cl_entity_t* barnacleEntity = gCorpseManager.FindBarnacleForPlayer(&currententity->curstate);

					if (barnacleEntity)
					{
						gPhysicsManager.ApplyBarnacle(ragdoll, barnacleEntity);
					}
					else
					{
						cl_entity_t* gargantuaEntity = gCorpseManager.FindGargantuaForPlayer(&currententity->curstate);
						if (gargantuaEntity)
						{
							gPhysicsManager.ApplyGargantua(ragdoll, gargantuaEntity);
						}
					}
				}

				//Transform from Death or barnacle to idle
				else */if (oldActivityType > 0 && ragdoll->m_iActivityType == 0)
				{
					pStudioDraw->StudioDrawPlayer(0, pplayer);

					gPhysicsManager.ResetPose(ragdoll, pplayer);
				}

			}

			//Teleport ?
			else if (oldActivityType == 0 && ragdoll->m_iActivityType == 0 &&
				Distance(currententity->curstate.origin, currententity->latched.prevorigin) > 500)
			{
				pStudioDraw->StudioDrawPlayer(0, pplayer);

				gPhysicsManager.ResetPose(ragdoll, pplayer);
			}

			if (ragdoll->m_iActivityType > 0)
			{
				int number = currententity->curstate.number;
				int iuser4 = currententity->curstate.iuser4;
				currententity->curstate.number = pplayer->number;
				currententity->curstate.iuser4 = 1919810;

				vec3_t saved_origin;
				VectorCopy(currententity->origin, saved_origin);
				gPhysicsManager.GetRagdollOrigin(ragdoll, currententity->origin);

				int result = pStudioDraw->StudioDrawPlayer(flags, pplayer);

				VectorCopy(saved_origin, currententity->origin);

				currententity->curstate.number = number;
				currententity->curstate.iuser4 = iuser4;
				return result;
			}
			else
			{
				int number = currententity->curstate.number;
				int iuser4 = currententity->curstate.iuser4;
				currententity->curstate.number = pplayer->number;
				currententity->curstate.iuser4 = 1919811;

				int result = pStudioDraw->StudioDrawPlayer(flags, pplayer);

				currententity->curstate.number = number;
				currententity->curstate.iuser4 = iuser4;
				return result;
			}
		}
	}
#endif
	return pStudioDraw->StudioDrawPlayer(flags, pplayer);
}

/*
===============
R_StudioClientEvents

===============
*/
static void GAME_EXPORT R_StudioClientEvents( void )
{
	mstudioseqdesc_t	*pseqdesc;
	mstudioevent_t	*pevent;
	cl_entity_t	*e = RI.currententity;
	float		f, start;
	int		i;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + e->curstate.sequence;
	pevent = (mstudioevent_t *)((byte *)m_pStudioHeader + pseqdesc->eventindex);

	// no events for this animation or gamepaused
	if( pseqdesc->numevents == 0 || cl.time == cl.oldtime )
		return;

	f = R_StudioEstimateFrame( e, pseqdesc );	// get start offset
	if ( e->latched.sequencetime == e->curstate.animtime && !( pseqdesc->flags & STUDIO_LOOPING ) )
		start = -0.01f;
	else start = f - e->curstate.framerate * host.frametime * pseqdesc->fps;

	for( i = 0; i < pseqdesc->numevents; i++ )
	{
		// ignore all non-client-side events
		if( pevent[i].event < EVENT_CLIENT )
			continue;

		if( (float)pevent[i].frame > start && f >= (float)pevent[i].frame )
			clgame.dllFuncs.pfnStudioEvent( &pevent[i], e );
	}
}

/*
===============
R_StudioGetForceFaceFlags

===============
*/
int GAME_EXPORT R_StudioGetForceFaceFlags( void )
{
	return g_nForceFaceFlags;
}

/*
===============
R_StudioSetForceFaceFlags

===============
*/
void GAME_EXPORT R_StudioSetForceFaceFlags( int flags )
{
	g_nForceFaceFlags = flags;
	int flag = RI.currententity->curstate.renderfx;
	if (!(flag == kRenderFxBlackOutLine || flag == kRenderFxRedOutLine || flag == kRenderFxGreenOutLine)) return;
	if (RI.currententity->syncbase == 1.0f && flags == 0)   //DrawOutLinePause
	{
		pglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		pglLineWidth(1.0);

		pglStencilMask(0x00); // ��ֹ�޸�ģ�建��
		pglDisable(GL_STENCIL_TEST);
		pglEnable(GL_DEPTH_TEST);
	}
	else if (RI.currententity->syncbase == 2.0f && flags == 0)   //DrawOutLineEnd
	{
		pglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		pglLineWidth(1.0);

		pglClear(GL_STENCIL_BUFFER_BIT);
		pglStencilMask(0x00); // ��ֹ�޸�ģ�建��
		pglDisable(GL_STENCIL_TEST);
		pglEnable(GL_DEPTH_TEST);

		RI.currententity->syncbase = 0.0f;
	}
}

/*
===============
pfnStudioSetHeader

===============
*/
void GAME_EXPORT R_StudioSetHeader( studiohdr_t *pheader )
{
	m_pStudioHeader = pheader;

	VectorClear( g_chrome_origin );
	m_fDoRemap = false;
}

/*
===============
R_StudioSetRenderModel

===============
*/
void GAME_EXPORT R_StudioSetRenderModel( model_t *model )
{
	RI.currentmodel = model;
}

/*
===============
R_StudioSetupRenderer

===============
*/
static void GAME_EXPORT R_StudioSetupRenderer( int rendermode )
{
	g_iRenderMode = bound( 0, rendermode, kRenderTransAdd );
	pglShadeModel( GL_SMOOTH );	// enable gouraud shading
	
	if( clgame.ds.cullMode != GL_NONE ) GL_Cull( GL_FRONT );
	if (RI.currententity)
	{
		if(RI.currententity->curstate.effects & EF_NOCULL)
			GL_Cull(0);
	}

	// enable depthmask on studiomodels
	if( glState.drawTrans && g_iRenderMode != kRenderTransAdd )
		pglDepthMask( GL_TRUE );

	pglAlphaFunc( GL_GREATER, 0.0f );

	// was done before, in R_DrawViewModel
	if( g_iBackFaceCull )
		GL_FrontFace( !glState.frontFace );
}

/*
===============
R_StudioRestoreRenderer

===============
*/
static void GAME_EXPORT R_StudioRestoreRenderer( void )
{
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	pglShadeModel( GL_FLAT );

	// restore depthmask state for sprites etc
	if( glState.drawTrans && g_iRenderMode != kRenderTransAdd )
		pglDepthMask( GL_FALSE );
	else pglDepthMask( GL_TRUE );

	// was done before, in R_DrawViewModel
	if( g_iBackFaceCull )
		GL_FrontFace( !glState.frontFace );

	g_iBackFaceCull = false;
	m_fDoRemap = false;
}

/*
===============
R_StudioSetChromeOrigin

===============
*/
void GAME_EXPORT R_StudioSetChromeOrigin( void )
{
	VectorNegate( RI.vieworg, g_chrome_origin );
}

/*
===============
pfnIsHardware

Xash3D is always works in hardware mode
===============
*/
static int GAME_EXPORT pfnIsHardware( void )
{
	return 1;	// 0 is Software, 1 is OpenGL, 2 is Direct3D
}

/*
===============
R_StudioDeformShadow

Deform vertices by specified lightdir
===============
*/
void R_StudioDeformShadow( void )
{
	float		*verts, dist, dist2;

	dist = g_shadowTrace.plane.dist + 1.0f;
	dist2 = -1.0f / DotProduct( g_mvShadowVec, g_shadowTrace.plane.normal );
	VectorScale( g_mvShadowVec, dist2, g_mvShadowVec );

	auto vert_calc_fn = [dist](vec3_t& v)
	{
		float dist3 = DotProduct(v, g_shadowTrace.plane.normal) - dist;
		if (dist3 > 0.0f) VectorMA(v, dist3, g_mvShadowVec, v);
	};

#ifdef XASH_RENDER_PAR
#ifdef XASH_TBBMALLOC
	tbb::parallel_for_each(g_xarrayverts, g_xarrayverts + g_nNumArrayVerts, vert_calc_fn);
#else
	std::for_each(std::execution::par_unseq, g_xarrayverts, g_xarrayverts + g_nNumArrayVerts, vert_calc_fn);
#endif
#else
	std::for_each(g_xarrayverts, g_xarrayverts + g_nNumArrayVerts, vert_calc_fn);
#endif
}

static void R_StudioDrawPlanarShadow( void )
{
	if( RI.currententity->curstate.effects & EF_NOSHADOW )
		return;

	R_StudioDeformShadow ();

	if( glState.stencilEnabled )
		pglEnable( GL_STENCIL_TEST );

	pglEnableClientState( GL_VERTEX_ARRAY );
	pglVertexPointer( 3, GL_FLOAT, sizeof(vec3_t), g_xarrayverts );

#if !defined XASH_NANOGL || defined XASH_WES && defined __EMSCRIPTEN__ // WebGL need to know array sizes
	if( pglDrawRangeElements )
		pglDrawRangeElements( GL_TRIANGLES, 0, g_nNumArrayVerts, g_nNumArrayElems, GL_UNSIGNED_SHORT, g_xarrayelems );
	else
#endif
		pglDrawElements( GL_TRIANGLES, g_nNumArrayElems, GL_UNSIGNED_SHORT, g_xarrayelems );

	if( glState.stencilEnabled )
		pglDisable( GL_STENCIL_TEST );

	pglDisableClientState( GL_VERTEX_ARRAY );
}
	
/*
===============
GL_StudioDrawShadow

NOTE: this code sucessfully working with ShadowHack only in Release build
===============
*/
static void GAME_EXPORT GL_StudioDrawShadow( void )
{
	int	rendermode;
	float	shadow_alpha;
	float	shadow_alpha2;
	GLenum	depthmode;
	GLenum	depthmode2;

	pglDepthMask( GL_TRUE );

	if (RI.currententity->baseline.movetype != MOVETYPE_FLY)
	{
		rendermode = RI.currententity->baseline.rendermode;

		if(rendermode == kRenderNormal && RI.currententity != &clgame.viewent )
		{
			shadow_alpha = 1.0f - r_shadowalpha.value * 0.5f;
			pglDisable( GL_TEXTURE_2D );
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			pglEnable( GL_BLEND );
			pglShadeModel( GL_FLAT );
			shadow_alpha2 = 1.0f - shadow_alpha;

			pglColor4f( 0.0f, 0.0f, 0.0f, shadow_alpha2 );

			depthmode = GL_LESS;
			pglDepthFunc( depthmode );

			R_StudioDrawPlanarShadow();

			depthmode2 = GL_LEQUAL;
			pglDepthFunc( depthmode2 );

			pglEnable( GL_TEXTURE_2D );
			pglDisable( GL_BLEND );

			pglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			pglShadeModel( GL_SMOOTH );
		}
	}
}

/*
=================
R_DrawStudioModel
=================
*/
void R_DrawStudioModelInternal( cl_entity_t *e, qboolean follow_entity )
{
	int	i, flags, result;
	float	prevFrame;

	if( RI.params & RP_ENVVIEW )
		return;

	if( !Mod_Extradata( e->model ))
		return;

	ASSERT( pStudioDraw != NULL );

	if( e == &clgame.viewent )
		flags = STUDIO_RENDER;	
	else flags = STUDIO_RENDER|STUDIO_EVENTS;

	if( e == &clgame.viewent )
		m_fDoInterp = true;	// viewmodel can't properly animate without lerping
	else if( r_studio_lerping->integer )
		m_fDoInterp = (e->curstate.effects & EF_NOINTERP) ? false : true;
	else m_fDoInterp = false;

	prevFrame = e->latched.prevframe;

#ifdef XASH_RAGDOLL
		// select the properly method
		if (e->player)
			result = GameStudioRenderer_StudioDrawPlayer(flags, &e->curstate);
		else result = GameStudioRenderer_StudioDrawModel(flags);
#else
		// select the properly method
		if (e->player)
			result = pStudioDraw->StudioDrawPlayer(flags, &e->curstate);
		else result = pStudioDraw->StudioDrawModel(flags);
#endif
	
	// old frame must be restored
	if( !RP_NORMALPASS( )) e->latched.prevframe = prevFrame;

	if( !result || follow_entity ) return;

	// NOTE: we must draw all followed entities
	// immediately after drawing parent when cached bones is valid
	for( i = 0; i < tr.num_child_entities; i++ )
	{
		if( CL_GetEntityByIndex( tr.child_entities[i]->curstate.aiment ) == e )
		{
			// copy the parent origin for right frustum culling
			VectorCopy( e->origin, tr.child_entities[i]->origin );

			RI.currententity = tr.child_entities[i];
			RI.currentmodel = RI.currententity->model;
			R_DrawStudioModelInternal( RI.currententity, true );
		}
	} 
}

/*
=================
R_DrawStudioModel
=================
*/
void R_DrawStudioModel( cl_entity_t *e )
{
	R_DrawStudioModelInternal( e, false );
}

/*
=================
R_RunViewmodelEvents
=================
*/
void R_RunViewmodelEvents( void )
{
	if( cl.refdef.nextView || cl.thirdperson || RI.params & RP_NONVIEWERREF )
		return;

	if( !Mod_Extradata( clgame.viewent.model ))
		return;

/*#ifndef _MSC_VER
#warning "check this"
#endif
	if( cl_lw->value && cl.frame.client.viewmodel != cl.predicted.viewmodel )
		return;*/

	RI.currententity = &clgame.viewent;
	RI.currentmodel = RI.currententity->model;
	if( !RI.currentmodel ) return;

	if( !cl.weaponstarttime )
		cl.weaponstarttime = cl.time;
	RI.currententity->curstate.animtime = cl.weaponstarttime;
	RI.currententity->curstate.sequence = cl.weaponseq;
	pStudioDraw->StudioDrawModel( STUDIO_EVENTS );

	RI.currententity = NULL;
	RI.currentmodel = NULL;
}

/*
=================
R_DrawViewModel
=================
*/
void R_DrawViewModel( void )
{
	if( RI.refdef.onlyClientDraw || r_drawviewmodel->integer == 0 )
		return;

	// ignore in thirdperson, camera view or client is died
	if( cl.thirdperson || cl.refdef.health <= 0 || cl.refdef.viewentity != ( cl.playernum + 1 ))
		return;

	if( RI.params & RP_NONVIEWERREF )
		return;

	if( !Mod_Extradata( clgame.viewent.model ))
		return;
/*#ifndef _MSC_VER
#warning "check this"
#endif
	if( cl_lw->value && cl.frame.client.viewmodel != cl.predicted.viewmodel )
		return;*/

	RI.currententity = &clgame.viewent;
	RI.currentmodel = RI.currententity->model;
	if( !RI.currentmodel ) return;

	RI.currententity->curstate.renderamt = R_ComputeFxBlend( RI.currententity );

	// hack the depth range to prevent view model from poking into walls
	pglDepthRange( gldepthmin, gldepthmin + 0.3f * ( gldepthmax - gldepthmin ));

	// backface culling for left-handed weapons
	if (R_AllowFlipViewModel(RI.currententity) || g_iBackFaceCull)
	{
		tr.fFlipViewModel = true;
		pglFrontFace(GL_CW);
	}
	
	RI.currententity->curstate.scale = 1.0f;
	RI.currententity->curstate.frame = 0;
	if( !cl.weaponstarttime )
		cl.weaponstarttime = cl.time;
	RI.currententity->curstate.animtime = cl.weaponstarttime;
	RI.currententity->curstate.sequence = cl.weaponseq;
	pStudioDraw->StudioDrawModel( STUDIO_RENDER );

	// restore depth range
	pglDepthRange( gldepthmin, gldepthmax );

	// backface culling for left-handed weapons
	if (R_AllowFlipViewModel(RI.currententity) || g_iBackFaceCull)
	{
		tr.fFlipViewModel = false;
		pglFrontFace(GL_CCW);
	}

	RI.currententity = NULL;
	RI.currentmodel = NULL;
}

/*
====================
R_StudioLoadTexture

load model texture with unique name
====================
*/
static void R_StudioLoadTexture( model_t *mod, const studiohdr_t *phdr, const mstudiotexture_t *ptexture )
{
	size_t		size;
	int		flags = 0;
	qboolean		load_external = false;
	char		texname[128], name[128], mdlname[128];
	imgfilter_t	*filter = NULL;
	texture_t		*tx = NULL;
    int	gl_texturenum = 0;

	if( ptexture->flags & STUDIO_NF_NORMALMAP )
		flags |= (TF_NORMALMAP);

#if defined(__ANDROID__) || ( TARGET_OS_IOS || TARGET_OS_IPHONE )
	// dont build mipmap on iOS to reduce memory usage
	flags |= TF_NOPICMIP|TF_NOMIPMAP;
#endif

#if 0 // Useless feature in CSMoE, removed for optimization
	// store some textures for remapping
	if( !Q_strnicmp( ptexture->name, "DM_Base", 7 ) || !Q_strnicmp( ptexture->name, "remap", 5 ))
	{
		int	i;
		char	val[6];
		const byte	*pixels;

		i = mod->numtextures;
		mod->textures = (texture_t **)Mem_Realloc( mod->mempool, mod->textures, ( i + 1 ) * sizeof( texture_t* ));
		size = ptexture->width * ptexture->height + 768;
		tx = (texture_t *)Mem_ZeroAlloc( mod->mempool, sizeof( *tx ) + size );
		mod->textures[i] = tx;

		// parse ranges and store it
		// HACKHACK: store ranges into anim_min, anim_max etc
		if( !Q_strnicmp( ptexture->name, "DM_Base", 7 ))
		{
			Q_strncpy( tx->name, "DM_Base", sizeof( tx->name ));
			tx->anim_min = PLATE_HUE_START; // topcolor start
			tx->anim_max = PLATE_HUE_END; // topcolor end
			// bottomcolor start always equal is (topcolor end + 1)
			tx->anim_total = SUIT_HUE_END;// bottomcolor end 
		}
		else
		{
			Q_strncpy( tx->name, "DM_User", sizeof( tx->name ));	// custom remapped
			Q_strncpy( val, ptexture->name + 7, 4 );  
			tx->anim_min = bound( 0, Q_atoi( val ), 255 );	// topcolor start
			Q_strncpy( val, ptexture->name + 11, 4 ); 
			tx->anim_max = bound( 0, Q_atoi( val ), 255 );	// topcolor end
			// bottomcolor start always equal is (topcolor end + 1)
			Q_strncpy( val, ptexture->name + 15, 4 ); 
			tx->anim_total = bound( 0, Q_atoi( val ), 255 );	// bottomcolor end
		}

		tx->width = ptexture->width;
		tx->height = ptexture->height;

		// the pixels immediately follow the structures
		pixels = (const byte *)phdr + ptexture->index;
        Mem_VirtualCopy( tx+1, pixels, size );

		ptexture->flags |= STUDIO_NF_COLORMAP;	// yes, this is colormap image
		flags |= TF_FORCE_COLOR;

		mod->numtextures++;	// done
	}
#endif

	Q_strncpy( mdlname, mod->name, sizeof( mdlname ));
	FS_FileBase( ptexture->name, name );
	FS_StripExtension( mdlname );

	// loading texture filter for studiomodel
	if( !( ptexture->flags & STUDIO_NF_COLORMAP ))
		filter = R_FindTexFilter( va( "%s.mdl/%s", mdlname, name )); // grab texture filter

	// NOTE: colormaps must have the palette for properly work. Ignore it.
	if( Mod_AllowMaterials( ) && !( ptexture->flags & STUDIO_NF_COLORMAP ))
	{

		Q_snprintf( texname, sizeof( texname ), "materials/%s/%s.tga", mdlname, name );

		if( FS_FileExists( texname, false ))
		{
            gl_texturenum = xe::TexLru_LoadTextureExternal(texname,  flags, filter);
		}

		if( gl_texturenum )
		{
            xe::TexLru_CreateAttr(ptexture, gl_texturenum);
			load_external = true; // sucessfully loaded
		}
	}

    do {
        // CSO Texture Load
        if (name[0] == '#' || name[0] == '@')
        {
			if (mod->suffix[0])
			{
				sprintf(texname, "models/texture/%s%s.bmp", mod->suffix, name);
				if (FS_FileExists(texname, false))
				{
                    gl_texturenum = xe::TexLru_LoadTextureExternal(texname, flags, filter);
					break;
				}
			}

#ifdef XASH_ASTC
			if(GL_Support(GL_ASTC_EXT) > 0)
			{
				Q_snprintf(texname, sizeof(texname), "ddc/models/texture/%s.astc", name);
				if (FS_FileExists(texname, false))
				{
					gl_texturenum = xe::TexLru_LoadTextureExternal(texname, flags, filter);
					break;
				}
			}
#endif

			Q_snprintf(texname, sizeof(texname), "models/texture/%s.tga", name);
			if (FS_FileExists(texname, false))
			{
                gl_texturenum = xe::TexLru_LoadTextureExternal(texname, flags, filter);
				break;
			}

			Q_snprintf(texname, sizeof(texname), "models/texture/%s.bmp", name);
			if (FS_FileExists(texname, false))
			{
                gl_texturenum = xe::TexLru_LoadTextureExternal(texname, flags, filter);
				break;
			}
			break;
		}

#ifdef XASH_ASTC
        // Load internal from ASTC cooked
		if(GL_Support(GL_ASTC_EXT) > 0)
        {
            Q_snprintf(texname, sizeof(texname), "ddc/%s.mdl/%s.astc", mdlname, name);
            if (FS_FileExists(texname, false)) {
                gl_texturenum = xe::TexLru_LoadTextureExternal(texname, flags, filter);
                break;
            }
        }
#endif
	} while (0);

    if (gl_texturenum)
    {
        gltexture_t *gltex = R_GetTexture(gl_texturenum);
        if (gltex)
        {
            xe::TexLru_CreateAttr(ptexture, gl_texturenum);
            load_external = true; // sucessfully loaded
        }
    }

	if( !load_external )
	{
		// NOTE: replace index with pointer to start of imagebuffer, ImageLib expected it
		//ptexture->index = (int)((byte *)phdr) + ptexture->index;
		Image_SetMDLPointer((const byte *)phdr + ptexture->index);
		size = sizeof( mstudiotexture_t ) + ptexture->width * ptexture->height + 768;

		if( host.features & ENGINE_DISABLE_HDTEXTURES && ptexture->flags & STUDIO_NF_TRANSPARENT )
			flags |= TF_KEEP_8BIT; // Paranoia2 alpha-tracing

		// build the texname
		Q_snprintf( texname, sizeof( texname ), "#%s/%s.mdl", mdlname, name );
        gl_texturenum = xe::TexLru_LoadTextureInternal(texname, mod->name, phdr, ptexture, size, flags, filter);
        xe::TexLru_CreateAttr(ptexture, gl_texturenum);
	}
	else MsgDev( D_NOTE, "Loading HQ: %s\n", texname );

	if( !gl_texturenum )
	{
		MsgDev( D_WARN, "%s has null texture %s\n", mod->name, ptexture->name );
        gl_texturenum = tr.defaultTexture;
        xe::TexLru_CreateAttr(ptexture, gl_texturenum);
	}
	else
	{
		// duplicate texnum for easy acess 
		if( tx ) tx->gl_texturenum = gl_texturenum;
		GL_SetTextureType( gl_texturenum, TEX_STUDIO );
        // upload immediately
       // xe::TexLru_Upload(ptexture);
	}
}

/*
=================
R_StudioLoadHeader
=================
*/
const studiohdr_t *R_StudioLoadHeader( model_t *mod, const void *buffer )
{
    const byte		*pin;
    const studiohdr_t	*phdr;
	int		i;

	if( !buffer ) return NULL;

	pin = (const byte *)buffer;
	phdr = (const studiohdr_t *)pin;
	i = LittleLong(phdr->version);

	if( i != STUDIO_VERSION )
	{
		MsgDev( D_ERROR, "%s has wrong version number (%i should be %i)\n", mod->name, i, STUDIO_VERSION );
		return NULL;
	}	

#ifdef XASH_BIG_ENDIAN
	Mod_StudioBigEndian( mod, phdr );
#endif

	return (const studiohdr_t *)buffer;
}

/*
=================
Mod_LoadStudioModel
=================
*/
void Mod_LoadStudioModel( model_t *mod, const byte *buffer, size_t filesize, qboolean *loaded )
{
    const studiohdr_t	*phdr;

	if( loaded ) *loaded = false;
	loadmodel->mempool = Mem_AllocSubPool( mempool_mdl, va( "^2%s^7", loadmodel->name ));
	loadmodel->type = mod_studio;

	phdr = R_StudioLoadHeader( mod, buffer );
	if( !phdr ) return;	// bad model

	// just copy model into memory
	loadmodel->cache.data = (void *)buffer; // Mem_Alloc( loadmodel->mempool, phdr->length );
    loadmodel->buffer = buffer;
    loadmodel->buffer_size = filesize;
    phdr = (const studiohdr_t *)loadmodel->cache.data;

    // load model
    if( !Host_IsDedicated() )
    {
        const mstudiotexture_t	*ptexture = (const mstudiotexture_t *)(((const byte *)phdr) + phdr->textureindex);
        if( phdr->textureindex > 0 && phdr->numtextures <= MAXSTUDIOSKINS )
        {
            for( int i = 0; i < phdr->numtextures; i++ )
                R_StudioLoadTexture( mod, phdr, &ptexture[i] );
        }
    }

	// setup bounding box
	VectorCopy( phdr->bbmin, loadmodel->mins );
	VectorCopy( phdr->bbmax, loadmodel->maxs );

	loadmodel->numframes = R_StudioBodyVariations( loadmodel );
	loadmodel->radius = RadiusFromBounds( loadmodel->mins, loadmodel->maxs );
	loadmodel->flags = phdr->flags; // copy header flags

	if( loaded ) *loaded = true;
}

/*
=================
Mod_UnloadStudioModel
=================
*/
void Mod_UnloadStudioModel( model_t *mod )
{
	studiohdr_t	*pstudio;
	mstudiotexture_t	*ptexture;
	int		i;

	ASSERT( mod != NULL );

	if( mod->type != mod_studio )
		return; // not a studio

	pstudio = (studiohdr_t *)mod->cache.data;
	if( !pstudio ) return; // already freed

	ptexture = (mstudiotexture_t *)(((byte *)pstudio) + pstudio->textureindex);

	// release all textures
	for( i = 0; i < pstudio->numtextures; i++ )
	{
		if( ptexture[i].index == tr.defaultTexture )
			continue;
		xe::TexLru_FreeTexture( &ptexture[i] );
	}

	Mem_FreePool( &mod->mempool );
    FS_MapFree((const byte *)mod->buffer, mod->buffer_size);
	Q_memset( mod, 0, sizeof( *mod ));
}
		
bool GAME_EXPORT StudioSetupBones_Pre(struct cl_entity_s* ent, studiohdr_t* pstudiohdr)
{
#ifdef XASH_RAGDOLL
	if (ent->curstate.iuser4 == 114514)
	{
		if (physics::gPhysicsManager.SetupBones(pstudiohdr, ent->index))
			return false;
	}
	else if (ent->curstate.iuser4 == 1919810)
	{
		if (physics::gPhysicsManager.SetupBones(pstudiohdr, ent->curstate.number))
			return false;
	}
#endif
	return true;
}

void GAME_EXPORT StudioSetupBones_Post(struct cl_entity_s* ent, studiohdr_t* pstudiohdr)
{
#ifdef XASH_RAGDOLL
	/*if (IsEntityBarnacle(currententity))
	{
		auto player = gCorpseManager.FindPlayerForBarnacle(currententity->index);
		if (player)
		{
			gPhysicsManager.MergeBarnacleBones(m_pStudioHeader, player->index);
		}
	}*/

	if (ent->curstate.iuser4 == 114515)
	{
		if (physics::gPhysicsManager.SetupJiggleBones(pstudiohdr, ent->index))
			return;
	}
	else if (ent->curstate.iuser4 == 1919811)
	{
		if (physics::gPhysicsManager.SetupJiggleBones(pstudiohdr, ent->curstate.number))
			return;
	}
#endif
}

static engine_studio_api_t gStudioAPI =
{
	Mod_Calloc,
	Mod_CacheCheck,
	Mod_LoadCacheFile,
	Mod_ForName,
	Mod_Extradata,
	Mod_Handle,
	pfnGetCurrentEntity,
	pfnPlayerInfo,
	R_StudioGetPlayerState,
	pfnGetViewEntity,
	pfnGetEngineTimes,
	pfnCVarGetPointer,
	pfnGetViewInfo,
	R_GetChromeSprite,
	pfnGetModelCounters,
	pfnGetAliasScale,
	pfnStudioGetBoneTransform,
	pfnStudioGetLightTransform,
	pfnStudioGetAliasTransform,
	pfnStudioGetRotationMatrix,
	R_StudioSetupModel,
	R_StudioCheckBBox,
	R_StudioDynamicLight,
	R_StudioEntityLight,
	R_StudioSetupLighting,
	R_StudioDrawPoints,
	R_StudioDrawHulls,
	R_StudioDrawAbsBBox,
	R_StudioDrawBones,
	R_StudioSetupSkin,
	R_StudioSetRemapColors,
	R_StudioSetupPlayerModel,
	R_StudioClientEvents,
	R_StudioGetForceFaceFlags,
	R_StudioSetForceFaceFlags,
	R_StudioSetHeader,
	R_StudioSetRenderModel,
	R_StudioSetupRenderer,
	R_StudioRestoreRenderer,
	R_StudioSetChromeOrigin,
	pfnIsHardware,
	GL_StudioDrawShadow,
	GL_SetRenderMode,
	R_StudioSetRenderamt,
	R_StudioSetCullState,
	R_StudioRenderShadow,
	StudioSetupBones_Pre,
	StudioSetupBones_Post
};

static r_studio_interface_t gStudioDraw =
{
	STUDIO_INTERFACE_VERSION,
	nullptr,
	nullptr,
};

/*
===============
CL_InitStudioAPI

Initialize client studio
===============
*/
void CL_InitStudioAPI( void )
{
	pStudioDraw = &gStudioDraw;

	// trying to grab them from client.dll
	cl_righthand = Cvar_FindVar("cl_righthand");

	if (cl_righthand == NULL)
		cl_righthand = Cvar_Get("cl_righthand", "0", FCVAR_ARCHIVE, "flip viewmodel (left to right)");

	// Xash will be used internal StudioModelRenderer
	if( !clgame.dllFuncs.pfnGetStudioModelInterface )
	{
		g_bCustomStudioRenderer = false;
		return;
	}

	MsgDev( D_NOTE, "InitStudioAPI " );

	if( clgame.dllFuncs.pfnGetStudioModelInterface( STUDIO_INTERFACE_VERSION, &pStudioDraw, &gStudioAPI ))
	{
		g_bCustomStudioRenderer = true;
		MsgDev( D_NOTE, "- ok\n" );
		return;
	}

	g_bCustomStudioRenderer = false;
	MsgDev( D_NOTE, "- failed\n" );

	// NOTE: we always return true even if game interface was not correct
	// because we need Draw our StudioModels
	// just restore pointer to builtin function
	pStudioDraw = &gStudioDraw;
}
#endif // XASH_DEDICATED
