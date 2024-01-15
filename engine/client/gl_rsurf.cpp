/*
gl_rsurf.c - surface-related refresh code
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
#include "gl_local.h"
#include "mod_local.h"
#include "mathlib.h"
#include <triangleapi.h>
extern triangleapi_t gTriApi;
			
typedef struct
{
	int		allocated[BLOCK_SIZE_MAX];
	int		current_lightmap_texture;
	msurface_t	*dynamic_surfaces;
	msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];
	byte		lightmap_buffer[BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*4];
	byte		deluxemap_buffer[BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*4];
} gllightmapstate_t;

static int		nColinElim; // stats
static vec2_t		world_orthocenter;
static vec2_t		world_orthohalf;
static byte		visbytes[MAX_MAP_LEAFS/8];
static uint		r_blocklights[BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*3];
static int		r_blockdeluxe[BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*3];
static glpoly_t		*fullbright_polys[MAX_TEXTURES];
static qboolean		draw_fullbrights = false;
static mextrasurf_t		*detail_surfaces[MAX_TEXTURES];
static qboolean		draw_details = false;
static msurface_t		*skychain = NULL;
static gllightmapstate_t	gl_lms;

static void LM_UploadBlock( qboolean dynamic );

static qboolean R_AddSurfToVBO( msurface_t *surf, qboolean buildlightmaps );
static void R_DrawVBO( qboolean drawlightmaps, qboolean drawtextures );

byte *Mod_GetCurrentVis( void )
{
	return Mod_LeafPVS( r_viewleaf, cl.worldmodel );
}

void Mod_SetOrthoBounds( const vec2_t mins, const vec2_t maxs )
{
	if( clgame.drawFuncs.GL_OrthoBounds )
	{
		clgame.drawFuncs.GL_OrthoBounds( mins, maxs );
	}

    world_orthocenter = midpoint( maxs, mins );
    world_orthohalf = maxs - world_orthocenter;
}

static void BoundPoly( int numverts, const vec3_t *verts, vec3_t_ref mins, vec3_t_ref maxs )
{
	int	i, j;

	mins[0] = mins[1] = mins[2] = 99999.0f;
	maxs[0] = maxs[1] = maxs[2] = -99999.0f;

	for( i = 0; i < numverts; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			if( verts[i][j] < mins[j] ) mins[j] = verts[i][j];
			if( verts[i][j] > maxs[j] ) maxs[j] = verts[i][j];
		}
	}
}

static void SubdividePolygon_r( msurface_t *warpface, int numverts, vec3_t *verts )
{
	int	i, j, f, b;
	vec3_t	mins, maxs;
	float	m, frac, s, t, vertsDiv;
    vertex_t *verts_p;
	vec3_t	front[SUBDIVIDE_SIZE], back[SUBDIVIDE_SIZE], total;
	float	dist[SUBDIVIDE_SIZE], total_s, total_t, total_ls, total_lt;
	glpoly_t	*poly;

	Q_memset( dist, 0, SUBDIVIDE_SIZE * sizeof( float ) );

	if( numverts > ( SUBDIVIDE_SIZE - 4 ))
		Host_Error( "Mod_SubdividePolygon: too many vertexes on face ( %i )\n", numverts );

	BoundPoly( numverts, verts, mins, maxs );

	for( i = 0; i < 3; i++ )
	{
		m = ( mins[i] + maxs[i] ) * 0.5f;
		m = SUBDIVIDE_SIZE * floor( m / SUBDIVIDE_SIZE + 0.5f );
		if( maxs[i] - m < 8 ) continue;
		if( m - mins[i] < 8 ) continue;

		// cut it
		//vec3_t *v = verts + i;
		for( j = 0; j < numverts; j++ )
			dist[j] = verts[j][i] - m;

		// wrap cases
		dist[j] = dist[0];
        verts[j] = verts[0];

		f = b = 0;
		for( j = 0; j < numverts; j++ )
		{
			if( dist[j] >= 0 )
			{
                front[f] = verts[j];
				f++;
			}

			if( dist[j] <= 0 )
			{
                back[b] = verts[j];
				b++;
			}

			if( dist[j] == 0 || dist[j+1] == 0 )
				continue;

			if(( dist[j] > 0 ) != ( dist[j+1] > 0 ))
			{
				// clip point
				frac = dist[j] / ( dist[j] - dist[j+1] );
                front[f] = back[b] = fma((verts[j+1] - verts[j]), frac, verts[j]);
				f++;
				b++;
			}
		}

		SubdividePolygon_r( warpface, f, front );
		SubdividePolygon_r( warpface, b, back );
		return;
	}

	// add a point in the center to help keep warp valid
	poly = (glpoly_t*)Mem_ZeroAlloc( loadmodel->mempool, sizeof( glpoly_t ) + ( ( numverts - 4 ) + 2) * VERTEXSIZE * sizeof( float ));
	poly->next = warpface->polys;
	poly->flags = warpface->flags;
	warpface->polys = poly;
	poly->numverts = numverts + 2;
	VectorClear( total );
	total_s = total_ls = 0.0f;
	total_t = total_lt = 0.0f;

	for( i = 0; i < numverts; i++ )
	{
        poly->verts[i + 1].xyz = verts[i];
		VectorAdd( total, verts[i], total );

		if( warpface->flags & SURF_DRAWTURB )
		{
			s = DotProduct( verts[i], warpface->texinfo->vecs[0] );
			t = DotProduct( verts[i], warpface->texinfo->vecs[1] );
		}
		else
		{
			s = DotProduct( verts[i], warpface->texinfo->vecs[0] ) + warpface->texinfo->vecs[0][3];
			t = DotProduct( verts[i], warpface->texinfo->vecs[1] ) + warpface->texinfo->vecs[1][3];
			s /= warpface->texinfo->texture->width; 
			t /= warpface->texinfo->texture->height; 
		}

        poly->verts[i + 1].s1t1 = { s, t };
 
		total_s += s;
		total_t += t;

		// for speed reasons
		if( !( warpface->flags & SURF_DRAWTURB ))
		{
			// lightmap texture coordinates
			s = DotProduct( verts[i], warpface->texinfo->vecs[0] ) + warpface->texinfo->vecs[0][3];
			s -= warpface->texturemins[0];
			s += warpface->light_s * LM_SAMPLE_SIZE;
			s += LM_SAMPLE_SIZE >> 1;
			s /= BLOCK_SIZE * LM_SAMPLE_SIZE; //fa->texinfo->texture->width;

			t = DotProduct( verts[i], warpface->texinfo->vecs[1] ) + warpface->texinfo->vecs[1][3];
			t -= warpface->texturemins[1];
			t += warpface->light_t * LM_SAMPLE_SIZE;
			t += LM_SAMPLE_SIZE >> 1;
			t /= BLOCK_SIZE * LM_SAMPLE_SIZE; //fa->texinfo->texture->height;

            poly->verts[i + 1].s2t2 = { s, t };

			total_ls += s;
			total_lt += t;
		}
	}

	vertsDiv = ( 1.0f / (float)numverts );

    poly->verts[0].xyz = total * vertsDiv;
    poly->verts[0].s1t1 = { total_s * vertsDiv, total_t * vertsDiv };

	if( !( warpface->flags & SURF_DRAWTURB ))
	{
        poly->verts[0].s2t2 = { total_ls * vertsDiv, total_lt * vertsDiv };
	}

	// copy first vertex to last
    poly->verts[i + 1] = poly->verts[1];
}

void GL_SetupFogColorForSurfaces( void )
{
	vec3_t	fogColor;
	float	factor, div;

    fogColor = {}; //Result pow( RI.fogColor[i] / div, ( 1.0f / factor )) may be nan.

	if(( !RI.fogEnabled && !RI.fogCustom ) || RI.refdef.onlyClientDraw || !RI.currententity )
		return;

	if( RI.currententity->curstate.rendermode == kRenderTransTexture )
	{
		pglFogfv( GL_FOG_COLOR, RI.fogColor.data() );
		return;
	}

	div = (r_detailtextures->integer) ? 2.0f : 1.0f;
	factor = (r_detailtextures->integer) ? 3.0f : 2.0f;
	fogColor[0] = pow( RI.fogColor[0] / div, ( 1.0f / factor ));
	fogColor[1] = pow( RI.fogColor[1] / div, ( 1.0f / factor ));
	fogColor[2] = pow( RI.fogColor[2] / div, ( 1.0f / factor ));
	pglFogfv( GL_FOG_COLOR, fogColor.data() );
}

void GL_ResetFogColor( void )
{
	// restore fog here
	if(( RI.fogEnabled || RI.fogCustom ) && !RI.refdef.onlyClientDraw )
		pglFogfv( GL_FOG_COLOR, RI.fogColor.data() );
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface( msurface_t *fa )
{
	vec3_t	verts[SUBDIVIDE_SIZE];
	int	numverts;
	int	i, lindex;
	vec3_t vec;

	// convert edges back to a normal polygon
	numverts = 0;
	for( i = 0; i < fa->numedges; i++ )
	{
		lindex = loadmodel->surfedges[fa->firstedge + i];

		if( lindex > 0 ) vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
		VectorCopy( vec, verts[numverts] );
		numverts++;
	}

	// do subdivide
	SubdividePolygon_r( fa, numverts, verts );
}

/*
================
GL_BuildPolygonFromSurface
================
*/
void GL_BuildPolygonFromSurface( model_t *mod, msurface_t *fa )
{
	int		i, lindex, lnumverts;
	medge_t		*pedges, *r_pedge;
	int		vertpage;
	texture_t		*tex;
	gltexture_t	*glt;
    vec3_t      vec;
	float		s, t;
	glpoly_t		*poly;

	// already created
	if( !mod || fa->polys ) return;

	if( !fa->texinfo || !fa->texinfo->texture )
		return; // bad polygon ?

	if( fa->flags & SURF_CONVEYOR && fa->texinfo->texture->gl_texturenum != 0 )
	{
		glt = R_GetTexture( fa->texinfo->texture->gl_texturenum );
		tex = fa->texinfo->texture;
		ASSERT( glt != NULL && tex != NULL );

		// update conveyor widths for keep properly speed of scrolling
		glt->srcWidth = tex->width;
		glt->srcHeight = tex->height;
	}

	// reconstruct the polygon
	pedges = mod->edges;
	lnumverts = fa->numedges;
	vertpage = 0;

	// draw texture
	poly = (glpoly_t*)Mem_ZeroAlloc( mod->mempool, sizeof( glpoly_t ) + ( lnumverts - 4 ) * VERTEXSIZE * sizeof( float ) );
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for( i = 0; i < lnumverts; i++ )
	{
		lindex = mod->surfedges[fa->firstedge + i];

		if( lindex > 0 )
		{
			r_pedge = &pedges[lindex];
			vec = mod->vertexes[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = mod->vertexes[r_pedge->v[1]].position;
		}

		s = DotProduct( vec, fa->texinfo->vecs[0] ) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->texture->width;

		t = DotProduct( vec, fa->texinfo->vecs[1] ) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->texture->height;

        poly->verts[i].xyz = vec;
        poly->verts[i].s1t1 = {s, t};

		// lightmap texture coordinates
		s = DotProduct( vec, fa->texinfo->vecs[0] ) + fa->texinfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s * LM_SAMPLE_SIZE;
		s += LM_SAMPLE_SIZE >> 1;
		s /= BLOCK_SIZE * LM_SAMPLE_SIZE; //fa->texinfo->texture->width;

		t = DotProduct( vec, fa->texinfo->vecs[1] ) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t * LM_SAMPLE_SIZE;
		t += LM_SAMPLE_SIZE >> 1;
		t /= BLOCK_SIZE * LM_SAMPLE_SIZE; //fa->texinfo->texture->height;

        poly->verts[i].s2t2 = {s, t};
	}

	// remove co-linear points - Ed
	if( !gl_keeptjunctions->integer && !( fa->flags & SURF_UNDERWATER ))
	{
		for( i = 0; i < lnumverts; i++ )
		{
			vec3_t	v1, v2;
			vertex_t	*prev, *pthis, *next;

			prev = &poly->verts[((i + lnumverts - 1) % lnumverts)];
			next = &poly->verts[((i + 1) % lnumverts)];
			pthis = &poly->verts[i];

            v1 = (pthis->xyz - prev->xyz).Normalize();
            v2 = (next->xyz - prev->xyz).Normalize();

			// skip co-linear points
			if(( fabs( v1[0] - v2[0] ) <= 0.001f) && (fabs( v1[1] - v2[1] ) <= 0.001f) && (fabs( v1[2] - v2[2] ) <= 0.001f))
			{
				for( int j = i + 1; j < lnumverts; j++ )
				{
                    poly->verts[( j - 1 )] = poly->verts[j];
				}

				// retry next vertex next time, which is now current vertex
				lnumverts--;
				nColinElim++;
				i--;
			}
		}
	}

	poly->numverts = lnumverts;
}

_inline texture_t *R_TextureRandomTiling( texture_t *base, int surfacenum )
{
	int reletive;
	int count = 0;

	if( base->anim_total >= 0 )
		return base;

	reletive = abs( surfacenum ) % abs( base->anim_total );

	while( base->anim_min > reletive || base->anim_max <= reletive )
	{
		base = base->anim_next;
		if( !base ) Host_Error( "R_TextureRandomTiling: broken loop\n" );
		if( ++count > 100 ) Host_Error( "R_TextureRandomTiling: infinite loop\n" );
	}
	return base;
}

_inline texture_t *R_TextureAnim( texture_t *base )
{
	int	reletive;
	int	count, speed;

	if( base->anim_total < 0 )
		return base;
	if( RI.currententity->curstate.frame )
	{
		if( base->alternate_anims )
			base = base->alternate_anims;
	}

	if( !base->anim_total )
		return base;

	// GoldSrc and Quake1 has different animating speed
	if( world.sky_sphere || world.version == Q1BSP_VERSION )
		speed = 10;
	else speed = 20;

	reletive = (int)(cl.time * speed) % base->anim_total;
	count = 0;

	while( base->anim_min > reletive || base->anim_max <= reletive )
	{
		base = base->anim_next;
		if( !base ) Host_Error( "R_TextureAnimation: broken loop\n" );
		if( ++count > 100 ) Host_Error( "R_TextureAnimation: infinite loop\n" );
	}
	return base;
}

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
texture_t *R_TextureAnimation( texture_t *base, int surfacenum )
{
	// random tiling textures
	if( base->anim_total < 0 )
		return R_TextureRandomTiling( base, surfacenum );

	return R_TextureAnim( base );
}

/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights( msurface_t *surf )
{
	float		dist, rad, minlight;
	int		lnum, s, t, sd, td, smax, tmax;
	float		sl, tl, sacc, tacc;
	vec3_t		impact, origin_l;
	mtexinfo_t	*tex;
	dlight_t		*dl;
	uint		*bl;

	// no dlighted surfaces here
	if( !R_CountSurfaceDlights( surf )) return;

	smax = (surf->extents[0] / LM_SAMPLE_SIZE) + 1;
	tmax = (surf->extents[1] / LM_SAMPLE_SIZE) + 1;
	tex = surf->texinfo;

	for( lnum = 0; lnum < MAX_DLIGHTS; lnum++ )
	{
		if(!( surf->dlightbits & BIT( lnum )))
			continue;	// not lit by this light

		dl = &cl_dlights[lnum];

		// transform light origin to local bmodel space
		if( !tr.modelviewIdentity )
			Matrix4x4_VectorITransform( RI.objectMatrix, dl->origin, origin_l );
		else VectorCopy( dl->origin, origin_l );

		rad = dl->radius;
		dist = PlaneDiff( origin_l, surf->plane );
		rad -= fabs( dist );

		// rad is now the highest intensity on the plane
		minlight = dl->minlight;
		if( rad < minlight )
			continue;

		minlight = rad - minlight;

		if( surf->plane->type < 3 )
		{
			VectorCopy( origin_l, impact );
			impact[surf->plane->type] -= dist;
		}
		else VectorMA( origin_l, -dist, surf->plane->normal, impact );

		sl = DotProduct( impact, tex->vecs[0] ) + tex->vecs[0][3] - surf->texturemins[0];
		tl = DotProduct( impact, tex->vecs[1] ) + tex->vecs[1][3] - surf->texturemins[1];

		bl = r_blocklights;
		for( t = 0, tacc = 0; t < tmax; t++, tacc += LM_SAMPLE_SIZE )
		{
			td = tl - tacc;
			if( td < 0 ) td = -td;

			for( s = 0, sacc = 0; s < smax; s++, sacc += LM_SAMPLE_SIZE, bl += 3 )
			{
				sd = sl - sacc;
				if( sd < 0 ) sd = -sd;

				if( sd > td ) dist = sd + (td >> 1);
				else dist = td + (sd >> 1);

				if( dist < minlight )
				{
					bl[0] += ( rad - dist ) * TextureToTexGamma( dl->color.r );
					bl[1] += ( rad - dist ) * TextureToTexGamma( dl->color.g );
					bl[2] += ( rad - dist ) * TextureToTexGamma( dl->color.b );
				}
			}
		}
	}
}

/*
================
R_SetCacheState
================
*/
void R_SetCacheState( msurface_t *surf )
{
	int	maps;

	for( maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++ )
	{
		surf->cached_light[maps] = RI.lightstylevalue[surf->styles[maps]];
	}
}

/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/
static void LM_InitBlock( void )
{
	Q_memset( gl_lms.allocated, 0, sizeof( gl_lms.allocated ));
}

static int LM_AllocBlock( int w, int h, int *x, int *y )
{
	int	i, j;
	int	best, best2;

	best = BLOCK_SIZE;

	for( i = 0; i < BLOCK_SIZE - w; i++ )
	{
		best2 = 0;

		for( j = 0; j < w; j++ )
		{
			if( gl_lms.allocated[i+j] >= best )
				break;
			if( gl_lms.allocated[i+j] > best2 )
				best2 = gl_lms.allocated[i+j];
		}

		if( j == w )
		{	
			// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if( best + h > BLOCK_SIZE )
		return false;

	for( i = 0; i < w; i++ )
		gl_lms.allocated[*x + i] = best + h;

	return true;
}

static void LM_UploadDynamicBlock()
{
	int	height = 0, i;

	for( i = 0; i < BLOCK_SIZE; i++ )
	{
		if( gl_lms.allocated[i] > height )
			height = gl_lms.allocated[i];
	}

	pglTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, BLOCK_SIZE, height, GL_RGBA, GL_UNSIGNED_BYTE, gl_lms.lightmap_buffer );
}

static void LM_UploadBlock( qboolean dynamic )
{
	int	i;

	if( dynamic )
	{
		int	height = 0;

		for( i = 0; i < BLOCK_SIZE; i++ )
		{
			if( gl_lms.allocated[i] > height )
				height = gl_lms.allocated[i];
		}

		if( host.features & ENGINE_LARGE_LIGHTMAPS )
			GL_Bind( XASH_TEXTURE0, tr.dlightTexture2 );
		else GL_Bind( XASH_TEXTURE0, tr.dlightTexture );

		pglTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, BLOCK_SIZE, height, GL_RGBA, GL_UNSIGNED_BYTE, gl_lms.lightmap_buffer );
	}
	else
	{
		image_ref r_lightmap = Image_NewTemp();
		char	lmName[16];

		i = gl_lms.current_lightmap_texture;

		// upload static lightmaps only during loading
		Q_snprintf( lmName, sizeof( lmName ), "*lightmap%i", i );

		r_lightmap->width = BLOCK_SIZE;
		r_lightmap->height = BLOCK_SIZE;
		r_lightmap->type = PF_RGBA_32;
		r_lightmap->size = r_lightmap->width * r_lightmap->height * 4;
		r_lightmap->flags = (( world.version == Q1BSP_VERSION ) ? 0 : IMAGE_HAS_COLOR) | IMAGE_TEMP;
		r_lightmap->buffer = gl_lms.lightmap_buffer;
		tr.lightmapTextures[i] = GL_LoadTextureInternal( lmName, r_lightmap, (texFlags_t)TF_FONT, false );
		GL_SetTextureType( tr.lightmapTextures[i], TEX_LIGHTMAP );

		if( tr.deluxemap )
		{
			image_ref r_deluxemap = Image_NewTemp();
			r_deluxemap->width = BLOCK_SIZE;
			r_deluxemap->height = BLOCK_SIZE;
			r_deluxemap->type = PF_RGBA_32;
			r_deluxemap->size = r_deluxemap->width * r_deluxemap->height * 4;
			r_deluxemap->flags = IMAGE_HAS_COLOR|IMAGE_TEMP;
			r_deluxemap->buffer = gl_lms.deluxemap_buffer;
			Q_snprintf( lmName, sizeof( lmName ), "*deluxemap%i", i );
			tr.deluxemapTextures[i] = GL_LoadTextureInternal( lmName, r_deluxemap, (texFlags_t)TF_FONT, false );
		}

		if( ++gl_lms.current_lightmap_texture == MAX_LIGHTMAPS )
			Host_Error( "AllocBlock: full\n" );
	}
}

/*
=================
R_BuildLightmap

Combine and scale multiple lightmaps into the floating
format in r_blocklights
=================
*/
static void R_BuildLightMap( msurface_t *surf, byte *dest, int stride, qboolean dynamic )
{
	int	smax, tmax;
	uint	*bl, scale;
	int	i, map, size, s, t;
	color24	*lm;

	smax = ( surf->extents[0] / LM_SAMPLE_SIZE ) + 1;
	tmax = ( surf->extents[1] / LM_SAMPLE_SIZE ) + 1;
	size = smax * tmax;

	lm = surf->samples;

	Q_memset( r_blocklights, 0, sizeof( uint ) * size * 3 );

	// add all the lightmaps
	for( map = 0; map < MAXLIGHTMAPS && surf->styles[map] != 255 && lm; map++ )
	{
		scale = RI.lightstylevalue[surf->styles[map]];

		for( i = 0, bl = r_blocklights; i < size; i++, bl += 3, lm++ )
		{
			bl[0] += TextureToTexGamma( lm->r ) * scale;
			bl[1] += TextureToTexGamma( lm->g ) * scale;
			bl[2] += TextureToTexGamma( lm->b ) * scale;
		}
	}

	// add all the dynamic lights
	if( surf->dlightframe == tr.framecount && dynamic )
		R_AddDynamicLights( surf );

	// Put into texture format
	stride -= (smax << 2);
	bl = r_blocklights;

	for( t = 0; t < tmax; t++, dest += stride )
	{
		for( s = 0; s < smax; s++ )
		{
			dest[0] = min((bl[0] >> 7), 255 );
			dest[1] = min((bl[1] >> 7), 255 );
			dest[2] = min((bl[2] >> 7), 255 );
			dest[3] = 255;

			bl += 3;
			dest += 4;
		}
	}
}

static void R_BuildDeluxeMap( msurface_t *surf, byte *dest, int stride )
{
	int	smax, tmax, *bl;
	unsigned int scale;
	int	i, map, size, s, t;
	color24	*lm, *dm;

	smax = ( surf->extents[0] / LM_SAMPLE_SIZE ) + 1;
	tmax = ( surf->extents[1] / LM_SAMPLE_SIZE ) + 1;
	size = smax * tmax;

	lm = surf->samples;
	dm = tr.deluxemap + ( surf->samples - cl.worldmodel->lightdata );

	Q_memset( r_blockdeluxe, 0, sizeof( int ) * size * 3 );

	// add all the lightmaps
	for( map = 0; map < MAXLIGHTMAPS && surf->styles[map] != 255 && lm; map++ )
	{
		scale = RI.lightstylevalue[surf->styles[map]];

		for( i = 0, bl = r_blockdeluxe; i < size; i++, bl += 3, lm++, dm++ )
		{
			int l;
			vec3_t v = { (float)lm->r, (float)lm->g, (float)lm->b };

			l = (int)(VectorLength(v) * scale);
			bl[0] += ((int)dm->r - 128) * l;
			bl[1] += ((int)dm->g - 128) * l;
			bl[2] += ((int)dm->b - 128) * l;
		}
	}

	// Put into texture format
	stride -= (smax << 2);
	bl = r_blockdeluxe;

	for( t = 0; t < tmax; t++, dest += stride )
	{
		for( s = 0; s < smax; s++ )
		{
			int l;
			vec3_t n( bl[0], bl[1], bl[2] ); // = *(Vector*)bl;

			VectorNormalize(n);

			l = (int)(n[0] * 128 + 128); dest[0] = bound(0, l, 255);
			l = (int)(n[1] * 128 + 128); dest[1] = bound(0, l, 255);
			l = (int)(n[2] * 128 + 128); dest[2] = bound(0, l, 255);
			dest[3] = 255;

			bl += 3;
			dest += 4;
		}
	}
}

/*
================
DrawGLPoly
================
*/
void DrawGLPoly( glpoly_t *p, float xScale, float yScale )
{
	float		sOffset, sy;
	float		tOffset, cy;
	cl_entity_t	*e = RI.currententity;
	int		i, hasScale = false;
	
	if( !p )
		return;

	// special hack for non-lightmapped surfaces
	if( p->flags & SURF_DRAWTILED )
		GL_ResetFogColor();

	if( p->flags & SURF_CONVEYOR )
	{
		gltexture_t	*texture;
		float		flConveyorSpeed;
		float		flRate, flAngle;

		flConveyorSpeed = (e->curstate.rendercolor.g<<8|e->curstate.rendercolor.b) / 16.0f;
		if( e->curstate.rendercolor.r ) flConveyorSpeed = -flConveyorSpeed;
		texture = R_GetTexture( glState.currentTextures[glState.activeTMU] );

		flRate = fabs( flConveyorSpeed ) / (float)texture->srcWidth;
		flAngle = ( flConveyorSpeed >= 0 ) ? 180 : 0;

		SinCos( flAngle * ( M_PI / 180.0f ), &sy, &cy );
		sOffset = RI.refdef.time * cy * flRate;
		tOffset = RI.refdef.time * sy * flRate;
	
		// make sure that we are positive
		if( sOffset < 0.0f ) sOffset += 1.0f + -(int)sOffset;
		if( tOffset < 0.0f ) tOffset += 1.0f + -(int)tOffset;

		// make sure that we are in a [0,1] range
		sOffset = sOffset - (int)sOffset;
		tOffset = tOffset - (int)tOffset;
	}
	else
	{
		sOffset = tOffset = 0.0f;
	}

	if( xScale != 0.0f && yScale != 0.0f )
		hasScale = true;

	gTriApi.Begin( TRI_POLYGON );

	for( i = 0; i < p->numverts; i++ )
	{
		if( hasScale )
			gTriApi.TexCoord2f(( p->verts[i].s1t1[0] + sOffset ) * xScale, ( p->verts[i].s1t1[1] + tOffset ) * yScale );
		else gTriApi.TexCoord2f( p->verts[i].s1t1[0] + sOffset, p->verts[i].s1t1[1] + tOffset );

		gTriApi.Vertex3fv( p->verts[i].xyz );
	}

	gTriApi.End();

	// special hack for non-lightmapped surfaces
	if( p->flags & SURF_DRAWTILED )
		GL_SetupFogColorForSurfaces();
}

/*
================
DrawGLPolyChain

Render lightmaps
================
*/
void DrawGLPolyChain( glpoly_t *p, float soffset, float toffset )
{
	qboolean	dynamic = true;

	if( soffset == 0.0f && toffset == 0.0f )
		dynamic = false;

	for( ; p != NULL; p = p->chain )
	{
		int	i;

		gTriApi.Begin( TRI_POLYGON );

		for( i = 0; i < p->numverts; i++ )
		{
			if( !dynamic ) gTriApi.TexCoord2f( p->verts[i].s2t2[0], p->verts[i].s2t2[1] );
			else gTriApi.TexCoord2f( p->verts[i].s2t2[0] - soffset, p->verts[i].s2t2[1] - toffset );
			gTriApi.Vertex3fv( p->verts[i].xyz );
		}
		gTriApi.End ();
	}
}

/*
================
R_BlendLightmaps
================
*/
void R_BlendLightmaps( void )
{
	msurface_t	*surf, *newsurf = NULL;
	mextrasurf_t	*info;
	int		i;

	if( r_fullbright->integer || !cl.worldmodel->lightdata )
		return;

	GL_SetupFogColorForSurfaces ();

	if( RI.currententity )
	{
		// check for rendermode
		switch( RI.currententity->curstate.rendermode )
		{
		case kRenderTransTexture:
		case kRenderTransColor:
		case kRenderTransAdd:
		case kRenderGlow:
			return; // no lightmaps
		}

		if( RI.currententity->curstate.effects & EF_FULLBRIGHT )
			return;	// disabled by user
	}

	if( !r_lightmap->integer )
	{
		pglEnable( GL_BLEND );

		if( !glState.drawTrans )
		{
			// lightmapped solid surfaces
			pglDepthMask( GL_FALSE );
			pglDepthFunc( GL_EQUAL );
		}

		pglDisable( GL_ALPHA_TEST );

		if( gl_overbright->integer == 1 )
			gTriApi.Color4f( 1 / 1.5, 1 / 1.5, 1 / 1.5, 1.0 );
		if( gl_overbright->integer )
			pglBlendFunc( GL_DST_COLOR, GL_SRC_COLOR );
		else
			pglBlendFunc( GL_ZERO, GL_SRC_COLOR );

		pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	}

	// render static lightmaps first
	for( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		if( gl_lms.lightmap_surfaces[i] )
		{
			GL_Bind( XASH_TEXTURE0, tr.lightmapTextures[i] );

			for( surf = gl_lms.lightmap_surfaces[i]; surf != NULL; surf = surf->lightmapchain )
			{
				if( surf->polys ) DrawGLPolyChain( surf->polys, 0.0f, 0.0f );
			}
		}
	}

	// render dynamic lightmaps
	if( r_dynamic->integer )
	{
		LM_InitBlock();

		if( host.features & ENGINE_LARGE_LIGHTMAPS )
			GL_Bind( XASH_TEXTURE0, tr.dlightTexture2 );
		else GL_Bind( XASH_TEXTURE0, tr.dlightTexture );

		newsurf = gl_lms.dynamic_surfaces;

		for( surf = gl_lms.dynamic_surfaces; surf != NULL; surf = surf->lightmapchain )
		{
			int	smax, tmax;
			byte	*base;

			smax = ( surf->extents[0] / LM_SAMPLE_SIZE ) + 1;
			tmax = ( surf->extents[1] / LM_SAMPLE_SIZE ) + 1;
			info = SURF_INFO( surf, RI.currentmodel );

			if( LM_AllocBlock( smax, tmax, &info->dlight_s, &info->dlight_t ))
			{
				base = gl_lms.lightmap_buffer;
				base += ( info->dlight_t * BLOCK_SIZE + info->dlight_s ) * 4;

				R_BuildLightMap( surf, base, BLOCK_SIZE * 4, true );
			}
			else
			{
				msurface_t	*drawsurf;

				// upload what we have so far
				LM_UploadBlock( true );

				// draw all surfaces that use this lightmap
				for( drawsurf = newsurf; drawsurf != surf; drawsurf = drawsurf->lightmapchain )
				{
					if( drawsurf->polys )
					{
						info = SURF_INFO( drawsurf, RI.currentmodel );

						DrawGLPolyChain( drawsurf->polys,
						( drawsurf->light_s - info->dlight_s ) * ( 1.0f / (float)BLOCK_SIZE ), 
						( drawsurf->light_t - info->dlight_t ) * ( 1.0f / (float)BLOCK_SIZE ));
					}
				}

				newsurf = drawsurf;

				// clear the block
				LM_InitBlock();

				info = SURF_INFO( surf, RI.currentmodel );

				// try uploading the block now
				if( !LM_AllocBlock( smax, tmax, &info->dlight_s, &info->dlight_t ))
					Host_Error( "AllocBlock: full\n" );

				base = gl_lms.lightmap_buffer;
				base += ( info->dlight_t * BLOCK_SIZE + info->dlight_s ) * 4;

				R_BuildLightMap( surf, base, BLOCK_SIZE * 4, true );
			}
		}

		// draw remainder of dynamic lightmaps that haven't been uploaded yet
		if( newsurf ) LM_UploadBlock( true );

		for( surf = newsurf; surf != NULL; surf = surf->lightmapchain )
		{
			if( surf->polys )
			{
				info = SURF_INFO( surf, RI.currentmodel );

				DrawGLPolyChain( surf->polys,
				( surf->light_s - info->dlight_s ) * ( 1.0f / (float)BLOCK_SIZE ),
				( surf->light_t - info->dlight_t ) * ( 1.0f / (float)BLOCK_SIZE ));
			}
		}
	}

	if( !r_lightmap->integer )
	{
		pglDisable( GL_BLEND );

		if( !glState.drawTrans )
		{
			// restore depth state
			pglDepthMask( GL_TRUE );
			pglDepthFunc( GL_LEQUAL );
		}

		pglDisable( GL_ALPHA_TEST );
		pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	}

	// restore fog here
	GL_ResetFogColor();
}

/*
================
R_RenderFullbrights
================
*/
void R_RenderFullbrights( void )
{
	glpoly_t	*p;
	int	i;

	if( !draw_fullbrights )
		return;

	if( RI.fogEnabled && !RI.refdef.onlyClientDraw )
		pglDisable( GL_FOG );

	pglEnable( GL_BLEND );
	pglDepthMask( GL_FALSE );
	pglDisable( GL_ALPHA_TEST );
	pglBlendFunc( GL_ONE, GL_ONE );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	for( i = 1; i < MAX_TEXTURES; i++ )
	{
		if( !fullbright_polys[i] )
			continue;
		GL_Bind( XASH_TEXTURE0, i );

		for( p = fullbright_polys[i]; p; p = p->next )
		{
			if( p->flags & SURF_DRAWTURB )
				EmitWaterPolys( p, ( p->flags & SURF_NOCULL ));
			else DrawGLPoly( p, 0.0f, 0.0f );
		}

		fullbright_polys[i] = NULL;		
	}

	pglDisable( GL_BLEND );
	pglDepthMask( GL_TRUE );
	pglDisable( GL_ALPHA_TEST );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

	draw_fullbrights = false;

	// restore for here
	if( RI.fogEnabled && !RI.refdef.onlyClientDraw )
		pglEnable( GL_FOG );
}

/*
================
R_RenderDetails
================
*/
void R_RenderDetails( void )
{
	gltexture_t	*glt;
	mextrasurf_t	*es, *p;
	msurface_t	*fa;
	int		i;

	if( !draw_details )
		return;

	GL_SetupFogColorForSurfaces();

	pglEnable( GL_BLEND );
	pglBlendFunc( GL_DST_COLOR, GL_SRC_COLOR );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

	if( RI.currententity->curstate.rendermode == kRenderTransAlpha )
		pglDepthFunc( GL_EQUAL );

	for( i = 1; i < MAX_TEXTURES; i++ )
	{
		es = detail_surfaces[i];
		if( !es ) continue;

		GL_Bind( XASH_TEXTURE0, i );

		for( p = es; p; p = p->detailchain )
		{
			fa = INFO_SURF( p, RI.currentmodel );
			glt = R_GetTexture( fa->texinfo->texture->gl_texturenum ); // get texture scale
			DrawGLPoly( fa->polys, glt->xscale, glt->yscale );
		}

		detail_surfaces[i] = NULL;
		es->detailchain = NULL;		
	}

	pglDisable( GL_BLEND );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

	if( RI.currententity->curstate.rendermode == kRenderTransAlpha )
		pglDepthFunc( GL_LEQUAL );

	draw_details = false;

	// restore fog here
	GL_ResetFogColor();
}

/*
================
R_RenderBrushPoly
================
*/
void R_RenderBrushPoly( msurface_t *fa )
{
	texture_t	*t;
	int	maps;
	qboolean	is_dynamic = false;
	qboolean	is_mirror = false;
	
	if( RI.currententity == clgame.entities )
		r_stats.c_world_polys++;
	else r_stats.c_brush_polys++; 

	if( fa->flags & SURF_DRAWSKY )
	{	
		if( world.sky_sphere )
		{
			// warp texture, no lightmaps
			EmitSkyLayers( fa );
		}
		return;
	}
		
	t = R_TextureAnimation( fa->texinfo->texture, fa - RI.currententity->model->surfaces );

	if( RP_NORMALPASS() && fa->flags & SURF_REFLECT )
	{
		if( SURF_INFO( fa, RI.currentmodel )->mirrortexturenum )
		{
			GL_Bind( XASH_TEXTURE0, SURF_INFO( fa, RI.currentmodel )->mirrortexturenum );
			is_mirror = true;

			// BEGIN WATER STUFF
			if( fa->flags & SURF_DRAWTURB )
			{
				R_BeginDrawMirror( fa );
				GL_Bind( XASH_TEXTURE1, t->gl_texturenum );
				pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			}
		}
		else GL_Bind( XASH_TEXTURE0, t->gl_texturenum ); // dummy

		// DEBUG: reset the mirror texture after drawing
		SURF_INFO( fa, RI.currentmodel )->mirrortexturenum = 0;
	}
	else GL_Bind( XASH_TEXTURE0, t->gl_texturenum );

	if( fa->flags & SURF_DRAWTURB )
	{	
		// warp texture, no lightmaps
		EmitWaterPolys( fa->polys, ( fa->flags & SURF_NOCULL ));
		if( is_mirror ) R_EndDrawMirror();
		// END WATER STUFF
		return;
	}

	if( t->fb_texturenum )
	{
		// HACKHACK: store fullbrights in poly->next (only for non-water surfaces)
		fa->polys->next = fullbright_polys[t->fb_texturenum];
		fullbright_polys[t->fb_texturenum] = fa->polys;
		draw_fullbrights = true;
	}

	if( r_detailtextures->integer )
	{
		mextrasurf_t *es = SURF_INFO( fa, RI.currentmodel );

		if( RI.fogEnabled || RI.fogCustom )
		{
			// don't apply detail textures for windows in the fog
			if( RI.currententity->curstate.rendermode != kRenderTransTexture )
			{
				if( t->dt_texturenum )
				{
					es->detailchain = detail_surfaces[t->dt_texturenum];
					detail_surfaces[t->dt_texturenum] = es;
				}
				else
				{
					// draw stub detail texture for underwater surfaces
					es->detailchain = detail_surfaces[tr.grayTexture];
					detail_surfaces[tr.grayTexture] = es;
				}
				draw_details = true;
			}
		}
		else if( t->dt_texturenum )
		{
			es->detailchain = detail_surfaces[t->dt_texturenum];
			detail_surfaces[t->dt_texturenum] = es;
			draw_details = true;
		}
	}

	if( is_mirror ) R_BeginDrawMirror( fa );
	DrawGLPoly( fa->polys, 0.0f, 0.0f );
	if( is_mirror ) R_EndDrawMirror();
	DrawSurfaceDecals( fa );

	// NOTE: draw mirror through in mirror show dummy lightmapped texture
	if( fa->flags & SURF_REFLECT && RP_NORMALPASS() && r_lighting_extended->integer )
		return; // no lightmaps for mirror

	if( fa->flags & SURF_DRAWTILED )
		return; // no lightmaps anyway

	// check for lightmap modification
	for( maps = 0; maps < MAXLIGHTMAPS && fa->styles[maps] != 255; maps++ )
	{
		if( RI.lightstylevalue[fa->styles[maps]] != fa->cached_light[maps] )
			goto dynamic;
	}

	if( maps == MAXLIGHTMAPS )
		maps--;

	// dynamic this frame or dynamic previously
	if( fa->dlightframe == tr.framecount )
	{
dynamic:
		// NOTE: at this point we have only valid textures
		if( r_dynamic->integer ) is_dynamic = true;
	}

	if( is_dynamic )
	{
		if(( fa->styles[maps] >= 32 || fa->styles[maps] == 0 ) && ( fa->dlightframe != tr.framecount ))
		{
			byte	temp[132*132*4];
			int	smax, tmax;

			smax = ( fa->extents[0] / LM_SAMPLE_SIZE ) + 1;
			tmax = ( fa->extents[1] / LM_SAMPLE_SIZE ) + 1;

			if( smax < 132 && tmax < 132 )
			{
				if( tr.deluxemap )
				{
					R_BuildDeluxeMap( fa, temp, smax * 4 );

					GL_Bind( XASH_TEXTURE0, tr.deluxemapTextures[fa->lightmaptexturenum] );

					pglTexSubImage2D( GL_TEXTURE_2D, 0, fa->light_s, fa->light_t, smax, tmax,
						GL_RGBA, GL_UNSIGNED_BYTE, temp );
				}

				R_BuildLightMap( fa, temp, smax * 4, true );
			}
			else
			{
				smax = min( smax, 132 );
				tmax = min( tmax, 132 );
				Host_MapDesignError( "R_RenderBrushPoly: bad surface extents: %d %d", fa->extents[0], fa->extents[1] );
				Q_memset( temp, 255, sizeof( temp ) );
			}
			R_SetCacheState( fa );

			GL_Bind( XASH_TEXTURE0, tr.lightmapTextures[fa->lightmaptexturenum] );

			pglTexSubImage2D( GL_TEXTURE_2D, 0, fa->light_s, fa->light_t, smax, tmax,
				GL_RGBA, GL_UNSIGNED_BYTE, temp );

			fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
			gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
		}
		else
		{
			fa->lightmapchain = gl_lms.dynamic_surfaces;
			gl_lms.dynamic_surfaces = fa;
		}
	}
	else
	{
		fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
		gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
	}
}

/*
================
R_DrawTextureChains
================
*/
void R_DrawTextureChains( void )
{
	int		i;
	msurface_t	*s;
	texture_t		*t;

	// make sure what color is reset
	gTriApi.Color4ub( 255, 255, 255, 255 );
	R_LoadIdentity();	// set identity matrix

	GL_SetupFogColorForSurfaces();

	// restore worldmodel
	RI.currententity = clgame.entities;
	RI.currentmodel = RI.currententity->model;

	// clip skybox surfaces
	for( s = skychain; s != NULL; s = s->texturechain )
		R_AddSkyBoxSurface( s );

	for( i = 0; i < cl.worldmodel->numtextures; i++ )
	{
		t = cl.worldmodel->textures[i];
		if( !t ) continue;

		s = t->texturechain;
		if( !s ) continue;

		if( i == tr.skytexturenum )
		{
			if( world.sky_sphere )
				R_DrawSkyChain( s );
		}
		else
		{
			if(( s->flags & SURF_DRAWTURB ) && cl.refdef.movevars->wateralpha < 1.0f )
				continue;	// draw translucent water later

			for( ; s != NULL; s = s->texturechain )
				R_RenderBrushPoly( s );
		}
		t->texturechain = NULL;
	}

	GL_ResetFogColor();
}

/*
================
R_DrawWaterSurfaces
================
*/
void R_DrawWaterSurfaces( void )
{
	int		i;
	msurface_t	*s;
	texture_t		*t;

	if( !RI.drawWorld || RI.refdef.onlyClientDraw )
		return;

	// non-transparent water is already drawed
	if( cl.refdef.movevars->wateralpha >= 1.0f )
		return;

	// go back to the world matrix
	R_LoadIdentity();

	pglEnable( GL_BLEND );
	pglDepthMask( GL_FALSE );
	pglDisable( GL_ALPHA_TEST );
	pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	gTriApi.Color4f( 1.0f, 1.0f, 1.0f, cl.refdef.movevars->wateralpha );

	for( i = 0; i < cl.worldmodel->numtextures; i++ )
	{
		t = cl.worldmodel->textures[i];
		if( !t ) continue;

		s = t->texturechain;
		if( !s ) continue;

		if(!( s->flags & SURF_DRAWTURB ))
			continue;

		// set modulate mode explicitly
		GL_Bind( XASH_TEXTURE0, t->gl_texturenum );

		for( ; s; s = s->texturechain )
			EmitWaterPolys( s->polys, ( s->flags & SURF_NOCULL ));
			
		t->texturechain = NULL;
	}

	pglDisable( GL_BLEND );
	pglDepthMask( GL_TRUE );
	pglDisable( GL_ALPHA_TEST );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	gTriApi.Color4ub( 255, 255, 255, 255 );
}

/*
=================
R_SurfaceCompare

compare translucent surfaces
=================
*/
static bool R_SurfaceCompare( msurface_t	*surf1, msurface_t	*surf2 )
{
	mextrasurf_t	*info1, *info2;
	vec3_t		org1, org2;
	float		len1, len2;

	info1 = SURF_INFO( surf1, RI.currentmodel );
	info2 = SURF_INFO( surf2, RI.currentmodel );

	VectorAdd( RI.currententity->origin, info1->origin, org1 );
	VectorAdd( RI.currententity->origin, info2->origin, org2 );

	// compare by plane dists
	len1 = DotProduct( org1, RI.vforward ) - RI.viewplanedist;
	len2 = DotProduct( org2, RI.vforward ) - RI.viewplanedist;

	return len1 > len2;
}

_inline qboolean R_HasLightmap( void )
{
	if( r_fullbright->integer || !cl.worldmodel->lightdata )
		return false;

	if( RI.currententity )
	{
		// check for rendermode
		switch( RI.currententity->curstate.rendermode )
		{
		case kRenderTransTexture:
		case kRenderTransColor:
		case kRenderTransAdd:
		case kRenderGlow:
			return false; // no lightmaps
		}

		if( RI.currententity->curstate.effects & EF_FULLBRIGHT )
			return false;	// disabled by user
	}

	return true;
}

/*
=================
R_DrawBrushModel
=================
*/
void R_DrawBrushModel( cl_entity_t *e )
{
	int		i, k, num_sorted;
	qboolean		need_sort = false;
	vec3_t		origin_l, oldorigin;
	vec3_t		mins, maxs;
	msurface_t	*psurf;
	model_t		*clmodel;
	qboolean		rotated;
	qboolean allow_vbo = r_vbo->integer;
	dlight_t		*l;

	if( !RI.drawWorld ) return;

	clmodel = e->model;

	// external models not loaded to VBO
	if( clmodel->surfaces != cl.worldmodel->surfaces )
		allow_vbo = false;

	RI.currentWaveHeight = RI.currententity->curstate.scale * 32.0f;

	// don't reflect this entity in mirrors
	if( e->curstate.effects & EF_NOREFLECT && RI.params & RP_MIRRORVIEW )
		return;

	// draw only in mirrors
	if( e->curstate.effects & EF_REFLECTONLY && !( RI.params & RP_MIRRORVIEW ))
		return;

	if( !VectorIsNull( e->angles ))
	{
        vec3_t delta = { clmodel->radius, clmodel->radius, clmodel->radius };
        mins = e->origin - delta;
        maxs = e->origin + delta;
		rotated = true;
	}
	else
	{
        mins = e->origin + clmodel->mins;
        maxs = e->origin + clmodel->maxs;
		rotated = false;
	}

	if( R_CullBox( mins, maxs, RI.clipFlags ))
		return;

	Q_memset( gl_lms.lightmap_surfaces, 0, sizeof( gl_lms.lightmap_surfaces ));
	gl_lms.dynamic_surfaces = NULL;

	if( rotated ) R_RotateForEntity( e );
	else R_TranslateForEntity( e );

	e->visframe = tr.framecount; // visible

	if( rotated ) Matrix4x4_VectorITransform( RI.objectMatrix, RI.cullorigin, tr.modelorg );
	else VectorSubtract( RI.cullorigin, e->origin, tr.modelorg );

	// calculate dynamic lighting for bmodel
	for( k = 0, l = cl_dlights; k < MAX_DLIGHTS; k++, l++ )
	{
		if( l->die < cl.time || !l->radius )
			continue;

		VectorCopy( l->origin, oldorigin ); // save lightorigin
		Matrix4x4_VectorITransform( RI.objectMatrix, l->origin, origin_l );
		VectorCopy( origin_l, l->origin ); // move light in bmodel space
		R_MarkLights( l, 1U << k, clmodel->nodes + clmodel->hulls[0].firstclipnode );
		VectorCopy( oldorigin, l->origin ); // restore lightorigin
	}

	// setup the rendermode
	GL_SetRenderMode( e->curstate.rendermode );
	GL_SetupFogColorForSurfaces ();

	if( e->curstate.rendermode == kRenderTransTexture && r_lighting_extended->value >= 2.0f )
		pglBlendFunc( GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA );

	// setup the color and alpha
	switch( e->curstate.rendermode )
	{
	case kRenderTransAdd:
		if( RI.fogCustom )
			pglDisable( GL_FOG );
	case kRenderTransTexture:
		need_sort = true;
		allow_vbo = false;
	case kRenderGlow:
		gTriApi.Color4ub( 255, 255, 255, e->curstate.renderamt );
		break;
	case kRenderTransColor:
		pglDisable( GL_TEXTURE_2D );
		gTriApi.Color4ub( e->curstate.rendercolor.r, e->curstate.rendercolor.g,
			e->curstate.rendercolor.b, e->curstate.renderamt );
		allow_vbo = false;
		break;
	case kRenderTransAlpha:
		// NOTE: brushes can't change renderamt for 'Solid' mode
		pglAlphaFunc( GL_GEQUAL, 0.5f );
	default:	
		gTriApi.Color4ub( 255, 255, 255, 255 );
		break;
	}

	num_sorted = 0;

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];
	for( i = 0; i < clmodel->nummodelsurfaces; i++, psurf++ )
	{
		if( R_CullSurface( psurf, 0 ))
			continue;

		if( need_sort && !gl_nosort->integer )
		{
			world.draw_surfaces[num_sorted] = psurf;
			num_sorted++;
			ASSERT( world.max_surfaces >= num_sorted );
		}
		else
		{
			// render unsorted (solid)
			if( !allow_vbo || !R_AddSurfToVBO( psurf, true ) )
				R_RenderBrushPoly( psurf );
		}
	}

	if (need_sort && !gl_nosort->integer)
		std::sort(world.draw_surfaces, world.draw_surfaces + num_sorted, R_SurfaceCompare);

	// draw sorted translucent surfaces
	for( i = 0; i < num_sorted; i++ )
		R_RenderBrushPoly( world.draw_surfaces[i] );

	GL_ResetFogColor();

	if( allow_vbo && !num_sorted )
		R_DrawVBO( R_HasLightmap(), true );

	if( e->curstate.rendermode == kRenderTransColor )
		pglEnable( GL_TEXTURE_2D );

	R_BlendLightmaps();
	R_RenderFullbrights();
	R_RenderDetails();

	// restore fog here
	if( e->curstate.rendermode == kRenderTransAdd )
	{
		if( RI.fogCustom )
			pglEnable( GL_FOG );
	}

	R_LoadIdentity();	// restore worldmatrix
}

/*
=================
R_DrawStaticModel

Merge static model brushes with world surfaces
=================
*/
void R_DrawStaticModel( cl_entity_t *e )
{
	int		i, k;
	model_t		*clmodel;
	msurface_t	*psurf;
	dlight_t		*l;
	
	clmodel = e->model;
	if( R_CullBox( clmodel->mins, clmodel->maxs, RI.clipFlags ))
		return;

	// calculate dynamic lighting for bmodel
	for( k = 0, l = cl_dlights; k < MAX_DLIGHTS; k++, l++ )
	{
		if( l->die < cl.time || !l->radius )
			continue;
		R_MarkLights( l, 1U << k, clmodel->nodes + clmodel->hulls[0].firstclipnode );
	}

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];
	for( i = 0; i < clmodel->nummodelsurfaces; i++, psurf++ )
	{
		if( R_CullSurface( psurf, RI.clipFlags ))
			continue;

		if( psurf->flags & SURF_DRAWSKY && !world.sky_sphere )
		{
			// make sky chain to right clip the skybox
			psurf->texturechain = skychain;
			skychain = psurf;
		}
		else if( !R_AddSurfToVBO( psurf, true ) )
		{ 
			psurf->texturechain = psurf->texinfo->texture->texturechain;
			psurf->texinfo->texture->texturechain = psurf;
		}
	}
}

/*
=================
R_DrawStaticBrushes

Insert static brushes into world texture chains
=================
*/
void R_DrawStaticBrushes( void )
{
	uint	i;

	// draw static entities
	for( i = 0; i < tr.num_static_entities; i++ )
	{
		RI.currententity = tr.static_entities[i];
		RI.currentmodel = RI.currententity->model;
	
		ASSERT( RI.currententity != NULL );
		ASSERT( RI.currententity->model != NULL );

		switch( RI.currententity->model->type )
		{
		case mod_brush:
			R_DrawStaticModel( RI.currententity );
			break;
		default:
			Host_Error( "R_DrawStatics: non bsp model in static list!\n" );
			break;
		}
	}
}

qboolean R_AddSurfToVBO(msurface_t* surf, qboolean buildlightmap)
{
	return false;
}

void R_DrawVBO(qboolean drawlightmaps, qboolean drawtextures)
{
	
}

void R_ClearVBO()
{
	
}

void R_AddDecalVBO(decal_t* pdecal, msurface_t* surf)
{
	
}

void R_GenerateVBO()
{
	
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/
/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode( mnode_t *node, uint clipflags )
{
	const mplane_t	*clipplane;
	int		i, clipped;
	msurface_t	*surf, **mark;
	mleaf_t		*pleaf;
	int		c, side;
	float		dot;

	if( node->contents == CONTENTS_SOLID )
		return; // hit a solid leaf

	if( node->visframe != tr.visframecount )
		return;

	if( clipflags )
	{
		for( i = 0, clipplane = RI.frustum; i < 6; i++, clipplane++ )
		{
			if(!( clipflags & ( 1U << i )))
				continue;

			vec3_t mins = { node->minmaxs[0], node->minmaxs[1], node->minmaxs[2] };
			vec3_t maxs = { node->minmaxs[3], node->minmaxs[4], node->minmaxs[5] };
			clipped = BoxOnPlaneSide( mins, maxs, clipplane );
			if( clipped == 2 ) return;
			if( clipped == 1 ) clipflags &= ~(1U << i);
		}
	}

	// if a leaf node, draw stuff
	if( node->contents < 0 )
	{
		pleaf = (mleaf_t *)node;

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if( c )
		{
			do
			{
				(*mark)->visframe = tr.framecount;
				mark++;
			} while( --c );
		}

		// deal with model fragments in this leaf
		if( pleaf->efrags )
			R_StoreEfrags( &pleaf->efrags, tr.framecount );

		r_stats.c_world_leafs++;
		return;
	}

	// node is just a decision point, so go down the apropriate sides

	// find which side of the node we are on
	dot = PlaneDiff( tr.modelorg, node->plane );
	side = (dot >= 0.0f) ? 0 : 1;

	// recurse down the children, front side first
	R_RecursiveWorldNode( node->children[side], clipflags );

	// draw stuff
	for( c = node->numsurfaces, surf = cl.worldmodel->surfaces + node->firstsurface; c; c--, surf++ )
	{
		if( R_CullSurface( surf, clipflags ))
			continue;

		if( surf->flags & SURF_DRAWSKY && !world.sky_sphere )
		{
			// make sky chain to right clip the skybox
			surf->texturechain = skychain;
			skychain = surf;
		}
		else if( !R_AddSurfToVBO( surf, true ) )
		{
			surf->texturechain = surf->texinfo->texture->texturechain;
			surf->texinfo->texture->texturechain = surf;
		}
	}

	// recurse down the back side
	R_RecursiveWorldNode( node->children[!side], clipflags );
}

/*
================
R_CullNodeTopView

cull node by user rectangle (simple scissor)
================
*/
qboolean R_CullNodeTopView( mnode_t *node )
{
	vec2_t	delta, size;
	vec3_t	center, half;

	// build the node center and half-diagonal
	VectorAverage( node->minmaxs, node->minmaxs + 3, center );
	VectorSubtract( node->minmaxs + 3, center, half );

	// cull against the screen frustum or the appropriate area's frustum.
	Vector2Subtract( center, world_orthocenter, delta );
	Vector2Add( half, world_orthohalf, size );

	return ( fabs( delta[0] ) > size[0] ) || ( fabs( delta[1] ) > size[1] );
}

/*
================
R_DrawTopViewLeaf
================
*/
static void R_DrawTopViewLeaf( mleaf_t *pleaf, uint clipflags )
{
	msurface_t	**mark, *surf;
	int		i;

	for( i = 0, mark = pleaf->firstmarksurface; i < pleaf->nummarksurfaces; i++, mark++ )
	{
		surf = *mark;

		// don't process the same surface twice
		if( surf->visframe == tr.framecount )
			continue;

		surf->visframe = tr.framecount;

		if( R_CullSurface( surf, clipflags ))
			continue;

		if(!( surf->flags & SURF_DRAWSKY ))
		{ 
			surf->texturechain = surf->texinfo->texture->texturechain;
			surf->texinfo->texture->texturechain = surf;
		}
	}

	// deal with model fragments in this leaf
	if( pleaf->efrags )
		R_StoreEfrags( &pleaf->efrags, tr.framecount );

	r_stats.c_world_leafs++;
}

/*
================
R_DrawWorldTopView
================
*/
void R_DrawWorldTopView( mnode_t *node, uint clipflags )
{
	const mplane_t	*clipplane;
	int		c, clipped;
	msurface_t	*surf;

	do
	{
		if( node->contents == CONTENTS_SOLID )
			return;	// hit a solid leaf

		if( node->visframe != tr.visframecount )
			return;

		if( clipflags )
		{
			for( c = 0, clipplane = RI.frustum; c < 6; c++, clipplane++ )
			{
				if(!( clipflags & ( 1U << c )))
					continue;

				vec3_t mins = { node->minmaxs[0], node->minmaxs[1], node->minmaxs[2] };
				vec3_t maxs = { node->minmaxs[3], node->minmaxs[4], node->minmaxs[5] };
				clipped = BoxOnPlaneSide( mins, maxs, clipplane );
				if( clipped == 2 ) return;
				if( clipped == 1 ) clipflags &= ~(1U << c);
			}
		}

		// cull against the screen frustum or the appropriate area's frustum.
		if( R_CullNodeTopView( node ))
			return;

		// if a leaf node, draw stuff
		if( node->contents < 0 )
		{
			R_DrawTopViewLeaf( (mleaf_t *)node, clipflags );
			return;
		}

		// draw stuff
		for( c = node->numsurfaces, surf = cl.worldmodel->surfaces + node->firstsurface; c; c--, surf++ )
		{
			// don't process the same surface twice
			if( surf->visframe == tr.framecount )
				continue;

			surf->visframe = tr.framecount;

			if( R_CullSurface( surf, clipflags ))
				continue;

			if(!( surf->flags & SURF_DRAWSKY ))
			{ 
				surf->texturechain = surf->texinfo->texture->texturechain;
				surf->texinfo->texture->texturechain = surf;
			}
		}

		// recurse down both children, we don't care the order...
		R_DrawWorldTopView( node->children[0], clipflags );
		node = node->children[1];

	} while( node );
}

/*
=============
R_DrawTriangleOutlines
=============
*/
void R_DrawTriangleOutlines( void )
{
	int		i, j;
	msurface_t	*surf;
	glpoly_t		*p;
		
	if( !gl_wireframe->integer )
		return;

	pglDisable( GL_TEXTURE_2D );
	pglDisable( GL_DEPTH_TEST );
	gTriApi.Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
	pglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	// render static surfaces first
	for( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		for( surf = gl_lms.lightmap_surfaces[i]; surf != NULL; surf = surf->lightmapchain )
		{
			p = surf->polys;
			for( ; p != NULL; p = p->chain )
			{
				gTriApi.Begin( TRI_POLYGON );
				for( j = 0; j < p->numverts; j++ )
					gTriApi.Vertex3fv( p->verts[i].xyz );
				gTriApi.End ();
			}
		}
	}

	// render surfaces with dynamic lightmaps
	for( surf = gl_lms.dynamic_surfaces; surf != NULL; surf = surf->lightmapchain )
	{
		p = surf->polys;

		for( ; p != NULL; p = p->chain )
		{
			gTriApi.Begin( TRI_POLYGON );
			for( j = 0; j < p->numverts; j++ )
				gTriApi.Vertex3fv( p->verts[i].xyz );
			gTriApi.End();
		}
	}

	pglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	pglEnable( GL_DEPTH_TEST );
	pglEnable( GL_TEXTURE_2D );
}

/*
=============
R_DrawWorld
=============
*/
void R_DrawWorld( void )
{
	// paranoia issues: when gl_renderer is "0" we need have something valid for currententity
	// to prevent crashing until HeadShield drawing.
	RI.currententity = clgame.entities;
	RI.currentmodel = RI.currententity->model;

	if( !RI.drawWorld || RI.refdef.onlyClientDraw )
		return;

	VectorCopy( RI.cullorigin, tr.modelorg );
	Q_memset( gl_lms.lightmap_surfaces, 0, sizeof( gl_lms.lightmap_surfaces ));
	Q_memset( fullbright_polys, 0, sizeof( fullbright_polys ));
	Q_memset( detail_surfaces, 0, sizeof( detail_surfaces ));

	RI.currentWaveHeight = RI.waveHeight;
	GL_SetRenderMode( kRenderNormal );
	gl_lms.dynamic_surfaces = NULL;

	R_ClearSkyBox ();

	// draw the world fog
	R_DrawFog ();

	if( RI.drawOrtho )
	{
		R_DrawWorldTopView( cl.worldmodel->nodes, RI.clipFlags );
	}
	else
	{
		R_RecursiveWorldNode( cl.worldmodel->nodes, RI.clipFlags );
	}

	R_DrawStaticBrushes();

#ifndef XASH_QINDIEGL
	R_DrawVBO( !r_fullbright->integer && !!cl.worldmodel->lightdata, true );
#endif

	R_DrawTextureChains();


	R_BlendLightmaps();

	R_RenderFullbrights();
	R_RenderDetails();

	if( skychain )
		R_DrawSkyBox();
	skychain = NULL;

	R_DrawTriangleOutlines ();
}

/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current leaf
===============
*/
void R_MarkLeaves( void )
{
	byte	*vis;
	mnode_t	*node;
	int	i;

	if( !RI.drawWorld ) return;

	if( r_novis->modified || tr.fResetVis )
	{
		// force recalc viewleaf
		r_novis->modified = false;
		tr.fResetVis = false;
		r_viewleaf = NULL;
	}

	if( r_viewleaf == r_oldviewleaf && r_viewleaf2 == r_oldviewleaf2 && !r_novis->integer && r_viewleaf != NULL )
		return;

	// development aid to let you run around
	// and see exactly where the pvs ends
	if( r_lockpvs->integer ) return;

	tr.visframecount++;
	r_oldviewleaf = r_viewleaf;
	r_oldviewleaf2 = r_viewleaf2;
		
	if( r_novis->integer || RI.drawOrtho || !r_viewleaf || !cl.worldmodel->visdata )
	{
		// mark everything
		for( i = 0; i < cl.worldmodel->numleafs; i++ )
			cl.worldmodel->leafs[i+1].visframe = tr.visframecount;
		for( i = 0; i < cl.worldmodel->numnodes; i++ )
			cl.worldmodel->nodes[i].visframe = tr.visframecount;
		return;
	}

	// may have to combine two clusters
	// because of solid water boundaries
	vis = Mod_LeafPVS( r_viewleaf, cl.worldmodel );

	if( r_viewleaf != r_viewleaf2 )
	{
		int	longs = ( cl.worldmodel->numleafs + 31 ) >> 5;

		Q_memcpy( visbytes, vis, longs << 2 );
		vis = Mod_LeafPVS( r_viewleaf2, cl.worldmodel );

		for( i = 0; i < longs; i++ )
			((int *)visbytes)[i] |= ((int *)vis)[i];

		vis = visbytes;
	}

	for( i = 0; i < cl.worldmodel->numleafs; i++ )
	{
		if( vis[i>>3] & ( 1U << ( i & 7 )))
		{
			node = (mnode_t *)&cl.worldmodel->leafs[i+1];
			do
			{
				if( node->visframe == tr.visframecount )
					break;
				node->visframe = tr.visframecount;
				node = node->parent;
			} while( node );
		}
	}
}

/*
========================
GL_CreateSurfaceLightmap
========================
*/
void GL_CreateSurfaceLightmap( msurface_t *surf )
{
	int	smax, tmax;
	byte	*base;

	if( !cl.worldmodel->lightdata ) return;
	if( surf->flags & SURF_DRAWTILED )
		return;

	smax = ( surf->extents[0] / LM_SAMPLE_SIZE ) + 1;
	tmax = ( surf->extents[1] / LM_SAMPLE_SIZE ) + 1;

	if( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ))
	{
		LM_UploadBlock( false );
		LM_InitBlock();

		if( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ))
			Host_Error( "AllocBlock: full\n" );
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	if( tr.deluxemap )
	{
		base = gl_lms.deluxemap_buffer;
		base += ( surf->light_t * BLOCK_SIZE + surf->light_s ) * 4;

		R_BuildDeluxeMap( surf, base, BLOCK_SIZE * 4 );
	}


	base = gl_lms.lightmap_buffer;
	base += ( surf->light_t * BLOCK_SIZE + surf->light_s ) * 4;

	R_SetCacheState( surf );
	R_BuildLightMap( surf, base, BLOCK_SIZE * 4, false );

	// moved here in case we need valid lightmap coords
	if( host.features & ENGINE_BUILD_SURFMESHES )
		Mod_BuildSurfacePolygons( surf, SURF_INFO( surf, loadmodel ));
}


/*
==================
R_LoadDeluxeMap
==================
*/
void R_LoadDeluxeMap( void )
{
	int compare;
	fs_offset_t filesize;
	char file[80];
	byte *deluxemap;

	Q_strncpy( file, cl.worldmodel->name, 80 );
	FS_StripExtension( file );
	FS_DefaultExtension( file, ".dlit" );

	tr.deluxemap = NULL;

	if( !COM_CompareFileTime( file, cl.worldmodel->name, &compare ) )
		return;

	if( compare < 0 )
	{
		MsgDev( D_NOTE, "R_LoadDeluxeMap(): %s  is out of date!\n", file );
		return;
	}

	deluxemap = FS_LoadFile( file, &filesize, true );

	if( !deluxemap )
	{
		MsgDev( D_NOTE, "R_LoadDeluxeMap(): %s not found.\n", file );
		return;
	}

	if( deluxemap[0] != 'Q' || deluxemap[1] != 'L' ||
		deluxemap[2] != 'I' || deluxemap[3] != 'T' ||
		*(int *)( deluxemap + 4 ) != 1 )
	{
		MsgDev( D_NOTE, "R_LoadDeluxeMap(): %s is invalid file.\n", file );
		COM_FreeFile( deluxemap );
		return;
	}

	tr.deluxemap = (color24 *)Mem_Alloc( cl.worldmodel->mempool, filesize - 8 );
    Mem_VirtualCopy( tr.deluxemap, deluxemap + 8, filesize - 8 );
	Mem_Free( deluxemap );

	MsgDev( D_NOTE, "R_LoadDeluxeMap(): %s loaded succesfully.\n", file );
}

/*
==================
GL_RebuildLightmaps

Rebuilds the lightmap texture
when gamma is changed
==================
*/
void GL_RebuildLightmaps( void )
{
	int	i, j;
	model_t	*m;

	if( !cl.world ) return;	// wait for worldmodel
	vid_gamma->modified = false;

	// release old lightmaps
	for( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		if( !tr.lightmapTextures[i] ) break;
		GL_FreeTexture( tr.lightmapTextures[i] );
	}
	for( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		if( !tr.deluxemapTextures[i] ) break;
		GL_FreeTexture( tr.deluxemapTextures[i] );
	}

	Q_memset( tr.lightmapTextures, 0, sizeof( tr.lightmapTextures ));
	Q_memset( tr.deluxemapTextures, 0, sizeof( tr.deluxemapTextures ));
	gl_lms.current_lightmap_texture = 0;

	// setup all the lightstyles
	R_AnimateLight();

	LM_InitBlock();	

	for( i = 1; i < MAX_MODELS; i++ )
	{
		if(( m = Mod_Handle( i )) == NULL )
			continue;

		if( m->name[0] == '*' || m->type != mod_brush )
			continue;

		loadmodel = m;

		for( j = 0; j < m->numsurfaces; j++ )
			GL_CreateSurfaceLightmap( m->surfaces + j );
	}
	LM_UploadBlock( false );

	if( clgame.drawFuncs.GL_BuildLightmaps )
	{
		// build lightmaps on the client-side
		clgame.drawFuncs.GL_BuildLightmaps( );
	}
}

/*
==================
GL_BuildLightmaps

Builds the lightmap texture
with all the surfaces from all brush models
==================
*/
void GL_BuildLightmaps( void )
{
	int	i, j;
	model_t	*m;

	// release old lightmaps
	for( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		if( !tr.lightmapTextures[i] ) break;
		GL_FreeTexture( tr.lightmapTextures[i] );
	}

	// release old mirror textures
	for( i = 0; i < MAX_MIRRORS; i++ )
	{
		if( !tr.mirrorTextures[i] ) break;
		GL_FreeTexture( tr.mirrorTextures[i] );
	}

	// release old deluxemap textures
	for( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		if( !tr.deluxemapTextures[i] ) break;
		GL_FreeTexture( tr.deluxemapTextures[i] );
	}

	Q_memset( tr.lightmapTextures, 0, sizeof( tr.lightmapTextures ));
	Q_memset( tr.deluxemapTextures, 0, sizeof( tr.deluxemapTextures ));
	Q_memset( tr.mirror_entities, 0, sizeof( tr.mirror_entities ));
	Q_memset( tr.mirrorTextures, 0, sizeof( tr.mirrorTextures ));
	Q_memset( visbytes, 0x00, sizeof( visbytes ));
	
	skychain = NULL;

	tr.framecount = tr.visframecount = 1;	// no dlight cache
	gl_lms.current_lightmap_texture = 0;
	tr.num_mirror_entities = 0;
	tr.num_mirrors_used = 0;
	nColinElim = 0;
	R_LoadDeluxeMap();

	// setup all the lightstyles
	R_AnimateLight();

	LM_InitBlock();


	for( i = 1; i < MAX_MODELS; i++ )
	{
		if(( m = Mod_Handle( i )) == NULL )
			continue;

		if( m->name[0] == '*' || m->type != mod_brush )
			continue;

		for( j = 0; j < m->numsurfaces; j++ )
		{
			// clearing all decal chains
			m->surfaces[j].pdecals = NULL;
			m->surfaces[j].visframe = 0;
			loadmodel = m;

			GL_CreateSurfaceLightmap( m->surfaces + j );

			if( m->surfaces[j].flags & SURF_DRAWTURB )
				continue;

			if( m->surfaces[j].flags & SURF_DRAWSKY && world.sky_sphere )
				continue;

			GL_BuildPolygonFromSurface( m, m->surfaces + j );
		}

		// clearing visframe
		for( j = 0; j < m->numleafs; j++ )
			m->leafs[j+1].visframe = 0;
		for( j = 0; j < m->numnodes; j++ )
			m->nodes[j].visframe = 0;
	}

	LM_UploadBlock( false );

	if( clgame.drawFuncs.GL_BuildLightmaps )
	{
		// build lightmaps on the client-side
		clgame.drawFuncs.GL_BuildLightmaps( );
	}

	if( !gl_keeptjunctions->integer )
		MsgDev( D_INFO, "Eliminated %i vertices\n", nColinElim );
}
#endif // XASH_DEDICATED
