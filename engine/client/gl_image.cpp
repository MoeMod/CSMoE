/*
gl_image.c - texture uploading and processing
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
#include "studio.h"

#include <string>
#include <deque>
#include <list>
#include <unordered_map>
#include <variant>
#include <optional>
#include <memory>
#include "mod_decryptor.h"
#include "mod_extend_seq.h"
#include "gl_texlru.h"

#define TEXTURES_HASH_SIZE	64

static int	r_textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
static int	r_textureMagFilter = GL_LINEAR;

std::deque<gltexture_t> r_textures(1);
std::unordered_map<std::string, int> r_texturesHashTable;

static byte	*scaledImage = NULL;			// pointer to a scaled image
static image_ref	r_image;					// generic pixelbuffer used for internal textures

// internal tables
static vec3_t	r_luminanceTable[256];	// RGB to luminance
static byte	r_particleTexture[8][8] =
{
{0,0,0,0,0,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,1,1,1,1,1,1,0},
{0,1,1,1,1,1,1,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,0,0,0,0,0},
};

static byte	r_oldParticleTexture[8][8] =
{
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,1,1,1,1,0,0},
{0,0,1,1,1,1,0,0},
{0,0,1,1,1,1,0,0},
{0,0,1,1,1,1,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
};

const char *GL_Target( GLenum target )
{
	switch( target )
	{
	case GL_TEXTURE_1D:
		return "1D";
	case GL_TEXTURE_2D:
		return "2D";
	case GL_TEXTURE_3D:
		return "3D";
	case GL_TEXTURE_CUBE_MAP_ARB:
		return "Cube";
	case GL_TEXTURE_RECTANGLE_EXT:
		return "Rect";
	}
	return "??";
}

size_t R_TextureNum()
{
	return r_textures.size();
}

/*
=================
GL_Bind
=================
*/
void GL_Bind( GLint tmu, GLenum texnum )
{
	gltexture_t	*texture;

	// missed texture ?
	if( texnum <= 0 ) texnum = tr.defaultTexture;
	ASSERT( texnum > 0 && texnum < R_TextureNum() );

	if( tmu != GL_KEEP_UNIT )
		GL_SelectTexture( tmu );
	else tmu = glState.activeTMU;

	// wrong texture unit
	//if( tmu < 0 || tmu >= MAX_TEXTURE_UNITS )
		//return;

	texture = &r_textures[texnum];

	if( glState.currentTextureTargets[tmu] != texture->target )
	{
		if( glState.currentTextureTargets[tmu] != GL_NONE )
			pglDisable( glState.currentTextureTargets[tmu] );

		glState.currentTextureTargets[tmu] = texture->target;
		pglEnable( glState.currentTextureTargets[tmu] );
	}

	if( glState.currentTextures[tmu] == texture->texnum )
		return;

	pglBindTexture( texture->target, texture->texnum );
	glState.currentTextures[tmu] = texture->texnum;
}

/*
=================
R_GetTexture
=================
*/
gltexture_t *R_GetTexture( GLenum texnum )
{
	ASSERT( texnum < R_TextureNum() );
	return &r_textures[texnum];
}

/*
=================
GL_SetTextureType

Just for debug (r_showtextures uses it)
=================
*/
void GL_SetTextureType( GLenum texnum, GLenum type )
{
	ASSERT( texnum < R_TextureNum() );
	r_textures[texnum].texType = type;
}

/*
=================
GL_TexFilter
=================
*/
void GL_TexFilter( gltexture_t *tex, qboolean update )
{
	qboolean	allowNearest;
	vec4_t	zeroClampBorder = { 0.0f, 0.0f, 0.0f, 1.0f };
	vec4_t	alphaZeroClampBorder = { 0.0f, 0.0f, 0.0f, 0.0f };

	switch( tex->texType )
	{
	case TEX_NOMIP:
	case TEX_CUBEMAP:
	case TEX_LIGHTMAP:
		allowNearest = false;
		break;
	default:
		allowNearest = true;
		break;
	}

#ifdef __EMSCRIPTEN__
	// glGenerateMipmaps may delay,
	// but we need draw lightmaps after re-uploading
	if( r_vbo->integer )
	{
		if( tex->texType == TEX_LIGHTMAP )
			tex->flags |= TF_NOMIPMAP;
	}
#endif

	// set texture filter
	if( tex->flags & TF_DEPTHMAP )
	{
		pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		if( !( tex->flags & TF_NOCOMPARE ))
		{
			pglTexParameteri( tex->target, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL );
			pglTexParameteri( tex->target, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB );
		}

		if( tex->flags & TF_LUMINANCE )
			pglTexParameteri( tex->target, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE );
		else pglTexParameteri( tex->target, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY );

		if( GL_Support( GL_ANISOTROPY_EXT ))
			pglTexParameterf( tex->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
	}
	else if( tex->flags & TF_NOMIPMAP )
	{
		if( tex->flags & TF_NEAREST )
		{
			pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
		else
		{
			if( r_textureMagFilter == GL_NEAREST && allowNearest )
			{
				pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, r_textureMagFilter );
				pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, r_textureMagFilter );
			}
			else
			{
				pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			}
		}
	}
	else
	{
		if( tex->flags & TF_NEAREST )
		{
			pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
			pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
		else
		{
			pglTexParameteri( tex->target, GL_TEXTURE_MIN_FILTER, r_textureMinFilter );
			pglTexParameteri( tex->target, GL_TEXTURE_MAG_FILTER, r_textureMagFilter );
		}

		// set texture anisotropy if available
		if( GL_Support( GL_ANISOTROPY_EXT ) && !( tex->flags & TF_ALPHACONTRAST ))
			pglTexParameterf( tex->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_texture_anisotropy->value );

		// set texture LOD bias if available
		if( GL_Support( GL_TEXTURE_LODBIAS ))
			pglTexParameterf( tex->target, GL_TEXTURE_LOD_BIAS_EXT, gl_texture_lodbias->value );
	}

	if( update ) return;

	if( tex->flags & ( TF_BORDER|TF_ALPHA_BORDER ) && !GL_Support( GL_CLAMP_TEXBORDER_EXT ))
	{
		// border is not support, use clamp instead
		tex->flags &= ~(TF_BORDER|TF_ALPHA_BORDER);
		tex->flags |= TF_CLAMP;
	}

	// set texture wrap
	if( tex->flags & TF_CLAMP )
	{
		if( GL_Support( GL_CLAMPTOEDGE_EXT ))
		{
			pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );

			if( tex->target != GL_TEXTURE_1D )
				pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

			if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
				pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
		}
		else
		{
			pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP );

			if( tex->target != GL_TEXTURE_1D )
				pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP );

			if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
				pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_CLAMP );
		}
	}
	else if( tex->flags & ( TF_BORDER|TF_ALPHA_BORDER ))
	{
		pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );

		if( tex->target != GL_TEXTURE_1D )
			pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

		if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
			pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );

		if( tex->flags & TF_BORDER )
			pglTexParameterfv( tex->target, GL_TEXTURE_BORDER_COLOR, zeroClampBorder.data() );
		else if( tex->flags & TF_ALPHA_BORDER )
			pglTexParameterfv( tex->target, GL_TEXTURE_BORDER_COLOR, alphaZeroClampBorder.data() );
	}
	else
	{
		pglTexParameteri( tex->target, GL_TEXTURE_WRAP_S, GL_REPEAT );

		if( tex->target != GL_TEXTURE_1D )
			pglTexParameteri( tex->target, GL_TEXTURE_WRAP_T, GL_REPEAT );

		if( tex->target == GL_TEXTURE_3D || tex->target == GL_TEXTURE_CUBE_MAP_ARB )
			pglTexParameteri( tex->target, GL_TEXTURE_WRAP_R, GL_REPEAT );
	}
}

/*
=================
R_SetTextureParameters
=================
*/
void R_SetTextureParameters( void )
{
	gltexture_t	*texture;
	int		i;

	if( !Q_stricmp( gl_texturemode->string, "GL_NEAREST" ))
	{
		r_textureMinFilter = GL_NEAREST;
		r_textureMagFilter = GL_NEAREST;
	}
	else if( !Q_stricmp( gl_texturemode->string, "GL_LINEAR" ))
	{
		r_textureMinFilter = GL_LINEAR;
		r_textureMagFilter = GL_LINEAR;
	}
	else if( !Q_stricmp( gl_texturemode->string, "GL_NEAREST_MIPMAP_NEAREST" ))
	{
		r_textureMinFilter = GL_NEAREST_MIPMAP_NEAREST;
		r_textureMagFilter = GL_NEAREST;
	}
	else if( !Q_stricmp( gl_texturemode->string, "GL_LINEAR_MIPMAP_NEAREST" ))
	{
		r_textureMinFilter = GL_LINEAR_MIPMAP_NEAREST;
		r_textureMagFilter = GL_LINEAR;
	}
	else if( !Q_stricmp( gl_texturemode->string, "GL_NEAREST_MIPMAP_LINEAR" ))
	{
		r_textureMinFilter = GL_NEAREST_MIPMAP_LINEAR;
		r_textureMagFilter = GL_NEAREST;
	}
	else if( !Q_stricmp( gl_texturemode->string, "GL_LINEAR_MIPMAP_LINEAR" ))
	{
		r_textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		r_textureMagFilter = GL_LINEAR;
	}
	else
	{
		MsgDev( D_ERROR, "gl_texturemode invalid mode %s, defaulting to GL_LINEAR_MIPMAP_LINEAR\n", gl_texturemode->string );
		Cvar_Set( "gl_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
		r_textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		r_textureMagFilter = GL_LINEAR;
	}

	gl_texturemode->modified = false;

	if( GL_Support( GL_ANISOTROPY_EXT ))
	{
		if( gl_texture_anisotropy->value > glConfig.max_texture_anisotropy )
			Cvar_SetFloat( "gl_anisotropy", glConfig.max_texture_anisotropy );
		else if( gl_texture_anisotropy->value < 1.0f )
			Cvar_SetFloat( "gl_anisotropy", 1.0f );
	}

	gl_texture_anisotropy->modified = false;

	if( GL_Support( GL_TEXTURE_LODBIAS ))
	{
		if( gl_texture_lodbias->value > glConfig.max_texture_lodbias )
			Cvar_SetFloat( "gl_texture_lodbias", glConfig.max_texture_lodbias );
		else if( gl_texture_lodbias->value < -glConfig.max_texture_lodbias )
			Cvar_SetFloat( "gl_texture_lodbias", -glConfig.max_texture_lodbias );
	}

	gl_texture_lodbias->modified = false;

	// change all the existing mipmapped texture objects
	for( auto &texture : r_textures )
	{
		if( !texture.texnum ) continue;	// free slot
		GL_Bind( XASH_TEXTURE0, texture.texnum );
		GL_TexFilter( &texture, true );
	}
}

/*
===============
R_TextureList_f
===============
*/
void R_TextureList_f( void )
{
	int		i, texCount, bytes = 0;

	Msg( "\n" );
	Msg("      -w-- -h-- -size- -fmt- type -data-- -encode-- -wrap-- -name--------\n" );

	for( auto image = r_textures.begin(); image != r_textures.end(); ++image )
	{
		if( !image->texnum ) continue;

		bytes += image->size;
		texCount++;

		Msg( "%4i: ", i );
		Msg( "%4i %4i ", image->width, image->height );
		Msg( "%5lik ", image->size >> 10 );

		switch( image->format )
		{
		case GL_COMPRESSED_RGBA_ARB:
			Msg( "CRGBA " );
			break;
		case GL_COMPRESSED_RGB_ARB:
			Msg( "CRGB  " );
			break;
		case GL_COMPRESSED_LUMINANCE_ALPHA_ARB:
			Msg( "CLA   " );
			break;
		case GL_COMPRESSED_LUMINANCE_ARB:
			Msg( "CL    " );
			break;
		case GL_COMPRESSED_ALPHA_ARB:
			Msg( "CA    " );
			break;
		case GL_COMPRESSED_INTENSITY_ARB:
			Msg( "CI    " );
			break;
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			Msg( "DXT1c " );
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			Msg( "DXT1a " );
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			Msg( "DXT3  " );
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			Msg( "DXT5  " );
			break;
		case GL_RGBA:
			Msg( "RGBA  " );
			break;
		case GL_RGBA8:
			Msg( "RGBA8 " );
			break;
		case GL_RGBA4:
			Msg( "RGBA4 " );
			break;
		case GL_RGB:
			Msg( "RGB   " );
			break;
		case GL_RGB8:
			Msg( "RGB8  " );
			break;
		case GL_RGB5:
			Msg( "RGB5  " );
			break;
		case GL_LUMINANCE4_ALPHA4:
			Msg( "L4A4  " );
			break;
		case GL_LUMINANCE_ALPHA:
		case GL_LUMINANCE8_ALPHA8:
			Msg( "L8A8  " );
			break;
		case GL_LUMINANCE4:
			Msg( "L4    " );
			break;
		case GL_LUMINANCE:
		case GL_LUMINANCE8:
			Msg( "L8    " );
			break;
		case GL_ALPHA8:
			Msg( "A8    " );
			break;
		case GL_INTENSITY8:
			Msg( "I8    " );
			break;
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT24:
			Msg( "DEPTH24" );
			break;
		case GL_DEPTH_COMPONENT32F:
			Msg( "DEPTH32" );
			break;
		case GL_LUMINANCE16F_ARB:
			Msg( "L16F  " );
			break;
		case GL_LUMINANCE32F_ARB:
			Msg( "L32F  " );
			break;
		case GL_LUMINANCE_ALPHA16F_ARB:
			Msg( "LA16F " );
			break;
		case GL_LUMINANCE_ALPHA32F_ARB:
			Msg( "LA32F " );
			break;
		case GL_RGB16F_ARB:
			Msg( "RGB16F" );
			break;
		case GL_RGB32F_ARB:
			Msg( "RGB32F" );
			break;
		case GL_RGBA16F_ARB:
			Msg( "RGBA16F" );
			break;
		case GL_RGBA32F_ARB:
			Msg( "RGBA32F" );
			break;
		default:
			Msg( "????? " );
			break;
		}

		switch( image->target )
		{
		case GL_TEXTURE_1D:
			Msg( " 1D  " );
			break;
		case GL_TEXTURE_2D:
			Msg( " 2D  " );
			break;
		case GL_TEXTURE_3D:
			Msg( " 3D  " );
			break;
		case GL_TEXTURE_CUBE_MAP_ARB:
			Msg( "CUBE " );
			break;
		case GL_TEXTURE_RECTANGLE_EXT:
			Msg( "RECT " );
			break;
		default:
			Msg( "???? " );
			break;
		}

		if( image->flags & TF_NORMALMAP )
			Msg( "normal  " );
		else Msg( "diffuse " );

		switch( image->encode )
		{
		case DXT_ENCODE_COLOR_YCoCg:
			Msg( "YCoCg     " );
			break;
		case DXT_ENCODE_NORMAL_AG_ORTHO:
			Msg( "ortho     " );
			break;
		case DXT_ENCODE_NORMAL_AG_STEREO:
			Msg( "stereo    " );
			break;
		case DXT_ENCODE_NORMAL_AG_PARABOLOID:
			Msg( "parabolic " );
			break;
		case DXT_ENCODE_NORMAL_AG_QUARTIC:
			Msg( "quartic   " );
			break;
		case DXT_ENCODE_NORMAL_AG_AZIMUTHAL:
			Msg( "azimuthal " );
			break;
		default:
			Msg( "default   " );
			break;
		}

		if( image->flags & TF_CLAMP )
			Msg( "clamp  " );
		else if( image->flags & TF_BORDER )
			Msg( "border " );
		else if( image->flags & TF_ALPHA_BORDER )
			Msg( "aborder" );
		else Msg( "repeat " );
		Msg( "  %s\n", image->name );
	}

	Msg( "---------------------------------------------------------\n" );
	Msg( "%i total textures\n", texCount );
	Msg( "%s total memory used\n", Q_memprint( bytes ));
	Msg( "\n" );
}

/*
================
GL_CalcTextureSamples
================
*/
int GL_CalcTextureSamples( int flags )
{
	if( flags & IMAGE_HAS_COLOR )
		return (flags & IMAGE_HAS_ALPHA) ? 4 : 3;
	return (flags & IMAGE_HAS_ALPHA) ? 2 : 1;
}

/*
================
GL_ImageFlagsFromSamples
================
*/
int GL_ImageFlagsFromSamples( int samples )
{
	switch( samples )
	{
	case 2: return IMAGE_HAS_ALPHA;
	case 3: return IMAGE_HAS_COLOR;
	case 4: return (IMAGE_HAS_COLOR|IMAGE_HAS_ALPHA);
	}

	return 0;
}

/*
================
GL_CalcImageSamples
================
*/
int GL_CalcImageSamples( int s1, int s2 )
{
	int	samples;

	if( s1 == 1 ) samples = s2;
	else if( s1 == 2 )
	{
		if( s2 == 3 || s2 == 4 )
			samples = 4;
		else samples = 2;
	}
	else if( s1 == 3 )
	{
		if( s2 == 2 || s2 == 4 )
			samples = 4;
		else samples = 3;
	}
	else samples = s1;

	return samples;
}

/*
================
GL_RoundImageDimensions
================
*/
void GL_RoundImageDimensions( word *width, word *height, texFlags_t flags, qboolean force )
{
	int	scaledWidth, scaledHeight;

	scaledWidth = *width;
	scaledHeight = *height;

	if( flags & ( TF_TEXTURE_1D|TF_TEXTURE_3D )) return;

	if( force || !GL_Support( GL_ARB_TEXTURE_NPOT_EXT ))
	{
		// find nearest power of two, rounding down if desired
		scaledWidth = NearestPOW( scaledWidth, gl_round_down->integer );
		scaledHeight = NearestPOW( scaledHeight, gl_round_down->integer );
	}

	if( flags & TF_SKYSIDE )
	{
		// let people sample down the sky textures for speed
		scaledWidth >>= gl_skymip->integer;
		scaledHeight >>= gl_skymip->integer;
	}
	else if(!( flags & TF_NOPICMIP ))
	{
		// let people sample down the world textures for speed
		scaledWidth >>= gl_picmip->integer;
		scaledHeight >>= gl_picmip->integer;
	}

	if( flags & TF_CUBEMAP )
	{
		while( scaledWidth > glConfig.max_cubemap_size || scaledHeight > glConfig.max_cubemap_size )
		{
			scaledWidth >>= 1;
			scaledHeight >>= 1;
		}
	}
	else
	{
		if( flags & TF_TEXTURE_RECTANGLE )
		{
			while( scaledWidth > glConfig.max_2d_rectangle_size || scaledHeight > glConfig.max_2d_rectangle_size )
			{
				scaledWidth >>= 1;
				scaledHeight >>= 1;
			}
		}
		else
		{
			while( scaledWidth > glConfig.max_2d_texture_size || scaledHeight > glConfig.max_2d_texture_size )
			{
				scaledWidth >>= 1;
				scaledHeight >>= 1;
			}
		}
	}

	if( scaledWidth < 1 ) scaledWidth = 1;
	if( scaledHeight < 1 ) scaledHeight = 1;

	*width = scaledWidth;
	*height = scaledHeight;
}

/*
===============
GL_TextureFormat
===============
*/
static GLenum GL_TextureFormat( gltexture_t *tex, int *samples )
{
	qboolean	compress;
	GLenum	format;

	// check if it should be compressed
	if( !gl_compress_textures->integer || ( tex->flags & TF_UNCOMPRESSED ))
		compress = false;
	else compress = GL_Support( GL_TEXTURE_COMPRESSION_EXT );

	// set texture format
	if( tex->flags & TF_DEPTHMAP )
	{
		if( tex->flags & TF_FLOAT && GL_Support( GL_ARB_DEPTH_FLOAT_EXT ))
			format = GL_DEPTH_COMPONENT32F;
		else format = GL_DEPTH_COMPONENT24;
		tex->flags &= ~TF_INTENSITY;
	}
	else if( tex->flags & TF_FLOAT && GL_Support( GL_ARB_TEXTURE_FLOAT_EXT ))
	{
		int	bits = glw_state.desktopBitsPixel;

		switch( *samples )
		{
		case 1:
			switch( bits )
			{
			case 16: format = GL_LUMINANCE16F_ARB; break;
			default: format = GL_LUMINANCE32F_ARB; break;
			}
			break;
		case 2:
			switch( bits )
			{
			case 16: format = GL_LUMINANCE_ALPHA16F_ARB; break;
			default: format = GL_LUMINANCE_ALPHA32F_ARB; break;
			}
			break;
		case 3:
			switch( bits )
			{
			case 16: format = GL_RGB16F_ARB; break;
			default: format = GL_RGB32F_ARB; break;
			}
			break;		
		case 4:
		default:
			switch( bits )
			{
			case 16: format = GL_RGBA16F_ARB; break;
			default: format = GL_RGBA32F_ARB; break;
			}
			break;
		}
	}
	else if( tex->flags & TF_FONT_ALPHA )
	{
		format = GL_ALPHA;
	}
	else if( compress )
	{
		switch( *samples )
		{
		case 1: format = GL_COMPRESSED_LUMINANCE_ARB; break;
		case 2: format = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
		case 3: format = GL_COMPRESSED_RGB_ARB; break;
		case 4:
		default: format = GL_COMPRESSED_RGBA_ARB; break;
		}

		if( tex->flags & TF_INTENSITY )
			format = GL_COMPRESSED_INTENSITY_ARB;
		tex->flags &= ~TF_INTENSITY;
	}
	else
	{
		int	bits = glw_state.desktopBitsPixel;

		switch( *samples )
		{
		case 1: format = GL_LUMINANCE8; break;
		case 2: format = GL_LUMINANCE8_ALPHA8; break;
		case 3:
			if( gl_luminance_textures->integer && !( tex->flags & TF_UNCOMPRESSED ) )
			{
				switch( bits )
				{
				case 16: format = GL_LUMINANCE4; break;
				case 32: format = GL_LUMINANCE8; break;
				default: format = GL_LUMINANCE; break;
				}
				*samples = 1;	// merge for right calc statistics
			}
			else
			{
				switch( bits )
				{
				case 16: format = GL_RGB5; break;
				case 32: format = GL_RGB8; break;
				default: format = GL_RGB; break;
				}
			}
			break;		
		case 4:
		default:
			if( gl_luminance_textures->integer && !( tex->flags & TF_UNCOMPRESSED ) )
			{
				switch( bits )
				{
				case 16: format = GL_LUMINANCE4_ALPHA4; break;
				case 32: format = GL_LUMINANCE8_ALPHA8; break;
				default: format = GL_LUMINANCE_ALPHA; break;
				}
				*samples = 2;	// merge for right calc statistics
			}
			else
			{
				switch( bits )
				{
				case 16: format = GL_RGBA4; break;
				case 32: format = GL_RGBA8; break;
				default: format = GL_RGBA; break;
				}
			}
			break;
		}

		if( tex->flags & TF_INTENSITY )
			format = GL_INTENSITY8;
		tex->flags &= ~TF_INTENSITY;
	}
	return format;
}

/*
=================
GL_ResampleTexture

Assume input buffer is RGBA
=================
*/
byte *GL_ResampleTexture( const byte *source, int inWidth, int inHeight, int outWidth, int outHeight, qboolean isNormalMap )
{
	uint		frac, fracStep;
	uint		*in = (uint *)source;
	uint		p1[0x1000], p2[0x1000];
	byte		*pix1, *pix2, *pix3, *pix4;
	uint		*out, *inRow1, *inRow2;
	vec3_t		normal;
	int		i, x, y;

	if( !source ) return NULL;

	scaledImage = (byte*)Mem_Realloc( r_temppool, scaledImage, outWidth * outHeight * 4 );
	fracStep = inWidth * 0x10000 / outWidth;
	out = (uint *)scaledImage;

	frac = fracStep >> 2;
	for( i = 0; i < outWidth; i++ )
	{
		p1[i] = 4 * (frac >> 16);
		frac += fracStep;
	}

	frac = (fracStep >> 2) * 3;
	for( i = 0; i < outWidth; i++ )
	{
		p2[i] = 4 * (frac >> 16);
		frac += fracStep;
	}

	if( isNormalMap )
	{
		for( y = 0; y < outHeight; y++, out += outWidth )
		{
			inRow1 = in + inWidth * (int)(((float)y + 0.25f) * inHeight/outHeight);
			inRow2 = in + inWidth * (int)(((float)y + 0.75f) * inHeight/outHeight);

			for( x = 0; x < outWidth; x++ )
			{
				pix1 = (byte *)inRow1 + p1[x];
				pix2 = (byte *)inRow1 + p2[x];
				pix3 = (byte *)inRow2 + p1[x];
				pix4 = (byte *)inRow2 + p2[x];

				normal[0] = (pix1[0] * (1.0f/127.0f) - 1.0f) + (pix2[0] * (1.0f/127.0f) - 1.0f) + (pix3[0] * (1.0f/127.0f) - 1.0f) + (pix4[0] * (1.0f/127.0f) - 1.0f);
				normal[1] = (pix1[1] * (1.0f/127.0f) - 1.0f) + (pix2[1] * (1.0f/127.0f) - 1.0f) + (pix3[1] * (1.0f/127.0f) - 1.0f) + (pix4[1] * (1.0f/127.0f) - 1.0f);
				normal[2] = (pix1[2] * (1.0f/127.0f) - 1.0f) + (pix2[2] * (1.0f/127.0f) - 1.0f) + (pix3[2] * (1.0f/127.0f) - 1.0f) + (pix4[2] * (1.0f/127.0f) - 1.0f);

				if( !normal.NormalizeInPlace())
					VectorSet( normal, 0.0f, 0.0f, 1.0f );

				((byte *)(out+x))[0] = (byte)(128 + 127 * normal[0]);
				((byte *)(out+x))[1] = (byte)(128 + 127 * normal[1]);
				((byte *)(out+x))[2] = (byte)(128 + 127 * normal[2]);
				((byte *)(out+x))[3] = 255;
			}
		}
	}
	else
	{
		for( y = 0; y < outHeight; y++, out += outWidth )
		{
			inRow1 = in + inWidth * (int)(((float)y + 0.25f) * inHeight/outHeight);
			inRow2 = in + inWidth * (int)(((float)y + 0.75f) * inHeight/outHeight);

			for( x = 0; x < outWidth; x++ )
			{
				pix1 = (byte *)inRow1 + p1[x];
				pix2 = (byte *)inRow1 + p2[x];
				pix3 = (byte *)inRow2 + p1[x];
				pix4 = (byte *)inRow2 + p2[x];

				((byte *)(out+x))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
				((byte *)(out+x))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
				((byte *)(out+x))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;
				((byte *)(out+x))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3]) >> 2;
			}
		}
	}
	return scaledImage;
}

/*
=================
GL_ApplyGamma

Assume input buffer is RGBA
=================
*/
byte *GL_ApplyGamma( const byte *source, int pixels, qboolean isNormalMap )
{
	byte	*in = (byte *)source;
	byte	*out = (byte *)source;
	int	i;

	if( !isNormalMap )
	{
		for( i = 0; i < pixels; i++, in += 4 )
		{
			in[0] = TextureToGamma( in[0] );
			in[1] = TextureToGamma( in[1] );
			in[2] = TextureToGamma( in[2] );
		}
	}
	return out;
}

/*
=================
GL_BuildMipMap

Operates in place, quartering the size of the texture
=================
*/
static void GL_BuildMipMap( byte *in, int width, int height, qboolean isNormalMap )
{
	byte	*out = in;
	vec3_t	normal;
	int	x, y;

	width <<= 2;
	height >>= 1;

	if( isNormalMap )
	{
		for( y = 0; y < height; y++, in += width )
		{
			for( x = 0; x < width; x += 8, in += 8, out += 4 )
			{
				normal[0] = (in[0] * (1.0f/127.0f) - 1.0f) + (in[4] * (1.0f/127.0f) - 1.0f) + (in[width+0] * (1.0f/127.0f) - 1.0f) + (in[width+4] * (1.0f/127.0f) - 1.0f);
				normal[1] = (in[1] * (1.0f/127.0f) - 1.0f) + (in[5] * (1.0f/127.0f) - 1.0f) + (in[width+1] * (1.0f/127.0f) - 1.0f) + (in[width+5] * (1.0f/127.0f) - 1.0f);
				normal[2] = (in[2] * (1.0f/127.0f) - 1.0f) + (in[6] * (1.0f/127.0f) - 1.0f) + (in[width+2] * (1.0f/127.0f) - 1.0f) + (in[width+6] * (1.0f/127.0f) - 1.0f);

				if( !normal.NormalizeInPlace())
					VectorSet( normal, 0.0f, 0.0f, 1.0f );

				out[0] = (byte)(128 + 127 * normal[0]);
				out[1] = (byte)(128 + 127 * normal[1]);
				out[2] = (byte)(128 + 127 * normal[2]);
				out[3] = 255;
			}
		}
	}
	else
	{
		for( y = 0; y < height; y++, in += width )
		{
			for( x = 0; x < width; x += 8, in += 8, out += 4 )
			{
				out[0] = (in[0] + in[4] + in[width+0] + in[width+4]) >> 2;
				out[1] = (in[1] + in[5] + in[width+1] + in[width+5]) >> 2;
				out[2] = (in[2] + in[6] + in[width+2] + in[width+6]) >> 2;
				out[3] = (in[3] + in[7] + in[width+3] + in[width+7]) >> 2;
			}
		}
	}
}

/*
===============
GL_GenerateMipmaps

sgis generate mipmap
===============
*/
static void GL_GenerateMipmaps( byte *buffer, image_ref pic, gltexture_t *tex, GLenum glTarget, GLenum inFormat, int side, qboolean subImage )
{
	int	mipLevel;
	int	dataType = GL_UNSIGNED_BYTE;
	int	w, h;

	// not needs
	if( tex->flags & TF_NOMIPMAP )
		return;

	if( GL_Support( GL_SGIS_MIPMAPS_EXT ) && !( tex->flags & ( TF_NORMALMAP|TF_ALPHACONTRAST )))
	{
		pglHint( GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST );
		pglTexParameteri( glTarget, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
		pglGetError(); // clear error queue on mips generate
		return; 
	}

	// screen texture?
	if( !buffer ) return;

	mipLevel = 0;
	w = tex->width;
	h = tex->height;

	// software mipmap generator
	while( w > 1 || h > 1 )
	{
		// build the mipmap
		if( tex->flags & TF_ALPHACONTRAST ) Q_memset( buffer, pic->width >> mipLevel, w * h * 4 );
		else GL_BuildMipMap( buffer, w, h, ( tex->flags & TF_NORMALMAP ));

		w = (w+1)>>1;
		h = (h+1)>>1;
		mipLevel++;

		if( subImage ) pglTexSubImage2D( tex->target + side, mipLevel, 0, 0, w, h, inFormat, dataType, buffer );
		else pglTexImage2D( tex->target + side, mipLevel, tex->format, w, h, 0, inFormat, dataType, buffer );
		if( pglGetError( )) break; // can't create mip levels
	}
}

/*
=================
GL_MakeLuminance

Converts the given image to luminance
=================
*/
static void GL_MakeLuminance( image_ref in )
{
	byte	luminance;
	float	r, g, b;
	int	x, y;

	for( y = 0; y < in->height; y++ )
	{
		for( x = 0; x < in->width; x++ )
		{
			r = r_luminanceTable[in->buffer[4*(y*in->width+x)+0]][0];
			g = r_luminanceTable[in->buffer[4*(y*in->width+x)+1]][1];
			b = r_luminanceTable[in->buffer[4*(y*in->width+x)+2]][2];

			luminance = (byte)(r + g + b);

			in->buffer[4*(y*in->width+x)+0] = luminance;
			in->buffer[4*(y*in->width+x)+1] = luminance;
			in->buffer[4*(y*in->width+x)+2] = luminance;
		}
	}
}

static void GL_TextureImage( GLenum inFormat, GLenum outFormat, GLenum glTarget, GLint side, GLint level, GLint width, GLint height, GLint depth, qboolean subImage, size_t size, const void *data )
{
	GLint	dataType = GL_UNSIGNED_BYTE;

	if( glTarget == GL_TEXTURE_1D )
	{
		if( subImage ) pglTexSubImage1D( glTarget, level, 0, width, inFormat, dataType, data );
		else pglTexImage1D( glTarget, level, outFormat, width, 0, inFormat, dataType, data );
	}
	else if( glTarget == GL_TEXTURE_CUBE_MAP_ARB )
	{
		if( subImage ) pglTexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, level, 0, 0, width, height, inFormat, dataType, data );
		else pglTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, level, outFormat, width, height, 0, inFormat, dataType, data );
	}
	else if( glTarget == GL_TEXTURE_3D )
	{
		if( subImage ) pglTexSubImage3D( glTarget, level, 0, 0, 0, width, height, depth, inFormat, dataType, data );
		else pglTexImage3D( glTarget, level, outFormat, width, height, depth, 0, inFormat, dataType, data );
	}
	else
	{
		if( subImage ) pglTexSubImage2D( glTarget, level, 0, 0, width, height, inFormat, dataType, data );
		else pglTexImage2D( glTarget, level, outFormat, width, height, 0, inFormat, dataType, data );
	}
}

static void GL_TextureImageDXT( GLenum format, GLenum glTarget, GLint side, GLint level, GLint width, GLint height, GLint depth, qboolean subImage, size_t size, const void *data )
{
#if !defined XASH_NANOGL && !defined XASH_WES
	if( glTarget == GL_TEXTURE_1D )
	{
		if( subImage ) pglCompressedTexSubImage1DARB( glTarget, level, 0, width, format, size, data );
		else pglCompressedTexImage1DARB( glTarget, level, format, width, 0, size, data );
	}
	else if( glTarget == GL_TEXTURE_CUBE_MAP_ARB )
	{
		if( subImage ) pglCompressedTexSubImage2DARB( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, level, 0, 0, width, height, format, size, data );
		else pglCompressedTexImage2DARB( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, level, format, width, height, 0, size, data );
	}
	else if( glTarget == GL_TEXTURE_3D )
	{
		if( subImage ) pglCompressedTexSubImage3DARB( glTarget, level, 0, 0, 0, width, height, depth, format, size, data );
		else pglCompressedTexImage3DARB( glTarget, level, format, width, height, depth, 0, size, data );
	}
	else // 2D or RECT
	{
		if( subImage ) pglCompressedTexSubImage2DARB( glTarget, level, 0, 0, width, height, format, size, data );
		else pglCompressedTexImage2DARB( glTarget, level, format, width, height, 0, size, data );
	}
#endif
}

/*
===============
GL_UploadTextureDXT

upload compressed texture into video memory
===============
*/
static void GL_UploadTextureDXT( image_ref pic, gltexture_t *tex, qboolean subImage, imgfilter_t *filter )
{
	byte		*buf;
	const byte	*bufend;
	GLenum		inFormat, glTarget;
	uint		width, height, depth;
	int		texsize = 0, samples;
	uint		i, j, s, numSides;
	int		numMips, err;

	ASSERT( pic != NULL && tex != NULL );

	tex->srcWidth = tex->width = pic->width;
	tex->srcHeight = tex->height = pic->height;
	s = tex->srcWidth * tex->srcHeight;

	tex->fogParams[0] = pic->fogParams[0];
	tex->fogParams[1] = pic->fogParams[1];
	tex->fogParams[2] = pic->fogParams[2];
	tex->fogParams[3] = pic->fogParams[3];

	// NOTE: normalmaps must be power of two or software mip generator will stop working
	GL_RoundImageDimensions( &tex->width, &tex->height, tex->flags, ( tex->flags & TF_NORMALMAP ));

	if( s&3 )
	{
		// will be resample, just tell me for debug targets
		MsgDev( D_NOTE, "GL_Upload: %s s&3 [%d x %d]\n", tex->name, tex->srcWidth, tex->srcHeight );
	}

	// clear all the unsupported flags
	tex->flags &= ~TF_KEEP_8BIT;
	tex->flags &= ~TF_KEEP_RGBDATA;
	tex->flags |= TF_NOPICMIP;
	tex->encode = pic->encode; // share encode method

	samples = GL_CalcTextureSamples( pic->flags );

	if( pic->flags & IMAGE_HAS_ALPHA )
		tex->flags |= TF_HAS_ALPHA;

	if( !pic->numMips ) tex->flags |= TF_NOMIPMAP; // disable mipmapping by user request

	// determine format
	inFormat = PFDesc[pic->type].glFormat;

	if( ImageDXT( pic->type ))
		tex->format = inFormat;
	else tex->format = GL_TextureFormat( tex, &samples );

	if( !( tex->flags & TF_HAS_ALPHA ) && inFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT )
		tex->format = inFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // OpenGL hint

	// determine target
	tex->target = glTarget = GL_TEXTURE_2D;

	numMips = (pic->numMips > 0) ? pic->numMips : 1;
	numSides = 1;

	if( pic->flags & IMAGE_CUBEMAP )
	{
		if( GL_Support( GL_TEXTURECUBEMAP_EXT ))
		{
			numSides = 6;
			tex->target = glTarget = GL_TEXTURE_CUBE_MAP_ARB;
			tex->flags |= TF_CUBEMAP;

			if( !GL_Support( GL_ARB_SEAMLESS_CUBEMAP ) && ( tex->flags & ( TF_BORDER|TF_ALPHA_BORDER )))
			{
				// don't use border for cubemaps (but allow for seamless cubemaps)
				tex->flags &= ~(TF_BORDER|TF_ALPHA_BORDER);
				tex->flags |= TF_CLAMP;
			}
		}
		else
		{
			MsgDev( D_WARN, "GL_UploadTexture: cubemaps isn't supported, %s ignored\n", tex->name );
			tex->flags &= ~TF_CUBEMAP;
		}
	}
	else if( tex->flags & TF_TEXTURE_1D || pic->height <= 1 )
	{
		// determine target
		tex->target = glTarget = GL_TEXTURE_1D;
	}
	else if( tex->flags & TF_TEXTURE_RECTANGLE )
	{
		if( glConfig.max_2d_rectangle_size )
			tex->target = glTarget = glConfig.texRectangle;
		// or leave as GL_TEXTURE_2D
	}
	else if( tex->flags & TF_TEXTURE_3D )
	{
		// determine target
		tex->target = glTarget = GL_TEXTURE_3D;
	}

	pglBindTexture( tex->target, tex->texnum );

	buf = pic->buffer;
	bufend = pic->buffer + pic->size;
	tex->size = pic->size;

	// uploading texture into video memory
	for( i = 0; i < numSides; i++ )
	{
		if( buf != NULL && buf >= bufend )
			Host_Error( "GL_UploadTextureDXT: %s image buffer overflow\n", tex->name );

		width = pic->width;
		height = pic->height;
		depth = pic->depth;

		for( j = 0; j < numMips; j++ )
		{
			width = max( 1, ( pic->width >> j ));
			height = max( 1, ( pic->height >> j ));
			texsize = Image_DXTGetLinearSize( pic->type, width, height, depth );
			if( ImageDXT( pic->type ))
				GL_TextureImageDXT( inFormat, glTarget, i, j, width, height, depth, subImage, texsize, buf );
			else GL_TextureImage( inFormat, tex->format, glTarget, i, j, width, height, depth, subImage, texsize, buf );

			buf += texsize; // move pointer

			// catch possible errors
			if(( err = pglGetError()) != GL_NO_ERROR )
				MsgDev( D_ERROR, "GL_UploadTexture: error %x while uploading %s [%s]\n", err, tex->name, GL_Target( glTarget ));

		}
	}
}


/*
===============
GL_UploadTexture

upload texture into video memory
===============
*/
static void GL_UploadTexture( image_ref pic, gltexture_t *tex, qboolean subImage, imgfilter_t *filter )
{
	byte		*buf, *data;
	const byte	*bufend;
	GLenum		outFormat, inFormat, glTarget;
	uint		i, s, numSides, offset = 0, err;
	int		texsize = 0, img_flags = 0, samples;
	GLint		dataType = GL_UNSIGNED_BYTE;

	ASSERT( pic != NULL && tex != NULL );

	if( pic->flags & IMAGE_DDS_FORMAT )
	{
		// special case for DDS textures
		GL_UploadTextureDXT( pic, tex, subImage, filter );
		return;
	}

	tex->srcWidth = tex->width = pic->width;
	tex->srcHeight = tex->height = pic->height;
	s = tex->srcWidth * tex->srcHeight;

	tex->fogParams[0] = pic->fogParams[0];
	tex->fogParams[1] = pic->fogParams[1];
	tex->fogParams[2] = pic->fogParams[2];
	tex->fogParams[3] = pic->fogParams[3];

	// NOTE: normalmaps must be power of two or software mip generator will stop working
	GL_RoundImageDimensions( &tex->width, &tex->height, tex->flags, ( tex->flags & TF_NORMALMAP ));

	if( s&3 )
	{
		// will be resample, just tell me for debug targets
		MsgDev( D_NOTE, "GL_Upload: %s s&3 [%d x %d]\n", tex->name, tex->srcWidth, tex->srcHeight );
	}
			
	// copy flag about luma pixels
	if( pic->flags & IMAGE_HAS_LUMA )
		tex->flags |= TF_HAS_LUMA;

	// create luma texture from quake texture
	if( tex->flags & TF_MAKELUMA )
	{
		img_flags |= IMAGE_MAKE_LUMA;
		tex->flags &= ~TF_MAKELUMA;
	}

	if( !subImage && tex->flags & TF_KEEP_8BIT )
		tex->original = FS_CopyImage( pic ); // because current pic will be expanded to rgba

	if( !subImage && tex->flags & TF_KEEP_RGBDATA )
		tex->original = pic; // no need to copy

	// we need to expand image into RGBA buffer
	if( pic->type == PF_INDEXED_24 || pic->type == PF_INDEXED_32 )
		img_flags |= IMAGE_FORCE_RGBA;

	// processing image before uploading (force to rgba, make luma etc)
	if( pic->buffer ) Image_Process( &pic, 0, 0, 0.0f, img_flags, filter );

	if( tex->flags & TF_LUMINANCE )
	{
		if( !( tex->flags & TF_DEPTHMAP ))
		{
			GL_MakeLuminance( pic );
			tex->flags &= ~TF_LUMINANCE;
		}
		pic->flags &= ~IMAGE_HAS_COLOR;
	}

	samples = GL_CalcTextureSamples( pic->flags );

	if( pic->flags & IMAGE_HAS_ALPHA )
		tex->flags |= TF_HAS_ALPHA;

	// determine format
	inFormat = PFDesc[pic->type].glFormat;
	outFormat = GL_TextureFormat( tex, &samples );
	tex->format = outFormat;

	// determine target
	tex->target = glTarget = GL_TEXTURE_2D;
	numSides = 1;

	if( tex->flags & TF_FLOATDATA )
		dataType = GL_FLOAT;

	if( tex->flags & TF_DEPTHMAP )
		inFormat = GL_DEPTH_COMPONENT;

	if( pic->flags & IMAGE_CUBEMAP )
	{
		if( GL_Support( GL_TEXTURECUBEMAP_EXT ))
		{		
			numSides = 6;
			tex->target = glTarget = GL_TEXTURE_CUBE_MAP_ARB;
			tex->flags |= TF_CUBEMAP;

			if( !GL_Support( GL_ARB_SEAMLESS_CUBEMAP ) && ( tex->flags & ( TF_BORDER|TF_ALPHA_BORDER )))
			{
				// don't use border for cubemaps
				tex->flags &= ~(TF_BORDER|TF_ALPHA_BORDER);
				tex->flags |= TF_CLAMP;
			}
		}
		else
		{
			MsgDev( D_WARN, "GL_UploadTexture: cubemaps isn't supported, %s ignored\n", tex->name );
			tex->flags &= ~TF_CUBEMAP;
		}
	}
	else if( tex->flags & TF_TEXTURE_1D )
	{
		// determine target
		tex->target = glTarget = GL_TEXTURE_1D;
	}
	else if( tex->flags & TF_TEXTURE_RECTANGLE )
	{
		if( glConfig.max_2d_rectangle_size )
			tex->target = glTarget = glConfig.texRectangle;
		// or leave as GL_TEXTURE_2D
	}
	else if( tex->flags & TF_TEXTURE_3D )
	{
		// determine target
		tex->target = glTarget = GL_TEXTURE_3D;
	}

	pglBindTexture( tex->target, tex->texnum );

	buf = pic->buffer;
	bufend = pic->buffer + pic->size;
	offset = pic->width * pic->height * PFDesc[pic->type].bpp;

	// NOTE: probably this code relies when gl_compressed_textures is enabled
	texsize = tex->width * tex->height * samples;

	// determine some texTypes
	if( tex->flags & TF_NOPICMIP )
		tex->texType = TEX_NOMIP;
	else if( tex->flags & TF_CUBEMAP )
		tex->texType = TEX_CUBEMAP;
	else if(( tex->flags & TF_DECAL ) == TF_DECAL )
		tex->texType = TEX_DECAL;

	// uploading texture into video memory
	for( i = 0; i < numSides; i++ )
	{
		if( buf != NULL && buf >= bufend )
			Host_Error( "GL_UploadTexture: %s image buffer overflow\n", tex->name );

		// copy or resample the texture
		if(( tex->width == tex->srcWidth && tex->height == tex->srcHeight ) || ( tex->flags & ( TF_TEXTURE_1D|TF_TEXTURE_3D )) || (pic->type != PF_RGBA_32 && pic->type != PF_BGRA_32))
		{
			data = buf;
		}
		else
		{
			data = GL_ResampleTexture( buf, tex->srcWidth, tex->srcHeight, tex->width, tex->height, ( tex->flags & TF_NORMALMAP ));
		}

		if(!( tex->flags & TF_NOMIPMAP ) && !( tex->flags & TF_SKYSIDE ) && !( tex->flags & TF_TEXTURE_3D ))
		{
			if(!PFDesc[pic->type].compressed)
				data = GL_ApplyGamma( data, tex->width * tex->height, ( tex->flags & TF_NORMALMAP ));
		}

		if( glTarget == GL_TEXTURE_1D )
		{
			if( subImage ) pglTexSubImage1D( tex->target, 0, 0, tex->width, inFormat, dataType, data );
			else pglTexImage1D( tex->target, 0, outFormat, tex->width, 0, inFormat, dataType, data );
		}
		else if( glTarget == GL_TEXTURE_CUBE_MAP_ARB )
		{
            if(PFDesc[pic->type].compressed)
            {
                if( subImage ) pglCompressedTexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, 0, 0, tex->width, tex->height, inFormat, pic->size, data );
                else pglCompressedTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, inFormat, tex->width, tex->height, 0, pic->size, data );
            }
            else
            {
                if( GL_Support( GL_SGIS_MIPMAPS_EXT ) && !( tex->flags & TF_NORMALMAP ))
                    GL_GenerateMipmaps( data, pic, tex, glTarget, inFormat, i, subImage );
                if( subImage ) pglTexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, 0, 0, tex->width, tex->height, inFormat, dataType, data );
                else pglTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, outFormat, tex->width, tex->height, 0, inFormat, dataType, data );
                if( !GL_Support( GL_SGIS_MIPMAPS_EXT ) || ( tex->flags & TF_NORMALMAP ))
                    GL_GenerateMipmaps( data, pic, tex, glTarget, inFormat, i, subImage );
            }
		}
		else if( glTarget == GL_TEXTURE_3D )
		{
			if( subImage ) pglTexSubImage3D( tex->target, 0, 0, 0, 0, tex->width, tex->height, pic->depth, inFormat, dataType, data );
			else pglTexImage3D( tex->target, 0, inFormat, tex->width, tex->height, pic->depth, 0, inFormat, dataType, data );
		}
		else
		{
            if(PFDesc[pic->type].compressed)
            {
                if( subImage ) pglCompressedTexSubImage2D( tex->target, 0, 0, 0, tex->width, tex->height, inFormat, pic->size, data );
                else pglCompressedTexImage2D( tex->target, 0, inFormat, tex->width, tex->height, 0, pic->size, data );
            }
            else
            {
                if( GL_Support( GL_SGIS_MIPMAPS_EXT ) && !( tex->flags & ( TF_NORMALMAP|TF_ALPHACONTRAST )))
                    GL_GenerateMipmaps( data, pic, tex, glTarget, inFormat, i, subImage );
                if( subImage ) pglTexSubImage2D( tex->target, 0, 0, 0, tex->width, tex->height, inFormat, dataType, data );
                else pglTexImage2D( tex->target, 0, outFormat, tex->width, tex->height, 0, inFormat, dataType, data );
                if( !GL_Support( GL_SGIS_MIPMAPS_EXT ) || ( tex->flags & ( TF_NORMALMAP|TF_ALPHACONTRAST )))
                    GL_GenerateMipmaps( data, pic, tex, glTarget, inFormat, i, subImage );
            }
		}

		if( numSides > 1 && buf != NULL )
			buf += offset;
		tex->size += texsize;

		// catch possible errors
		err = pglGetError();

		if( err != GL_NO_ERROR )
			MsgDev( D_ERROR, "GL_UploadTexture: error %x while uploading %s [%s]\n", err, tex->name, GL_Target( glTarget ));
	}
}

int GL_AllocateTextureSlot(const char *name)
{
	int		i;

	if (!name || !name[0] || !glw_state.initialized)
		return 0;

	if (Q_strlen(name) >= sizeof(r_textures[0].name))
	{
		MsgDev(D_ERROR, "GL_LoadTexture: too long name %s\n", name);
		return 0;
	}

	// see if already loaded
	i = GL_FindTexture(name);

	if(i > 0)
		return i;

	r_textures.push_back({});
	i = r_textures.size() - 1;
	gltexture_t	*tex = &r_textures.back();
	Q_strncpy(tex->name, name, sizeof(tex->name));

	// add to hash table
	tex->target = GL_TEXTURE_2D;
	tex->texnum = i;
	tex->refCount = 0;
	r_texturesHashTable.emplace(name, i);

	// NOTE: always return texnum as index in array or engine will stop work !!!
	return i;
}

/*
================
GL_LoadTexture
================
*/
int GL_LoadTexture_DDS(const char *name, GLvoid* data, int type, int size, int w, int h)
{
	int i = GL_FindTexture(name);
	if(i > 0)
		return i;

	i = GL_AllocateTextureSlot(name);
	if(!i)
		return 0;

	pglBindTexture(GL_TEXTURE_2D, i);
	switch (type)
	{
	case 1:
		pglCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, w, h, 0, size, data);
		break;
	case 2:
		pglCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, w, h, 0, size, data);
		break;
	case 3:
		pglCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, w, h, 0, size, data);
		break;
	}

	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	pglBindTexture(GL_TEXTURE_2D, 0);

	return i;
}


int GL_LoadTexture( const char *name, const byte *buf, size_t size, int flags, imgfilter_t *filter )
{
	gltexture_t	*tex;
	uint		picFlags = 0;

	int i = GL_FindTexture(name);
	if(i > 0)
	{
		tex = R_GetTexture(i);
		if (tex->name[0] != '*')
			tex->refCount++;
		return i;
	}

	i = GL_AllocateTextureSlot(name);
	if(!i)
		return 0;

	if( flags & TF_NOFLIP_TGA )
		picFlags |= IL_DONTFLIP_TGA;

	if( flags & TF_KEEP_8BIT )
		picFlags |= IL_KEEP_8BIT;	

	// set some image flags
	Image_SetForceFlags( picFlags );

	// HACKHACK: get rid of black vertical line on a 'BlackMesa map'
	if( !Q_strcmp( name, "#lab1_map1.mip" ) || !Q_strcmp( name, "#lab1_map2.mip" ))
		flags |= TF_NEAREST;

	image_ref pic = FS_LoadImage( name, buf, size );
	if( !pic ) return i; // couldn't loading image

	// force upload texture as RGB or RGBA (detail textures requires this)
	if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

	tex = R_GetTexture(i);
	Q_strncpy( tex->name, name, sizeof( tex->name ));
	tex->flags = (texFlags_t)flags;

	// init reference exclude brush model's texture
	if (tex->name[0] != '*')
		tex->refCount = 1;

	if (flags & TF_SKYSIDE)
		tex->texnum = i;//tr.skyboxbasenum++;
	else tex->texnum = i; // texnum is used for fast acess into r_textures array too

	GL_UploadTexture( pic, tex, false, filter );
	GL_TexFilter( tex, false ); // update texture filter, wrap etc

	if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
		pic = nullptr; // release source texture

	return i;
}

/*
================
GL_LoadTextureInternal
================
*/
int GL_LoadTextureInternal( const char *name, image_ref pic, texFlags_t flags, qboolean update )
{
	int i = 0;

	if( !name || !name[0] || !glw_state.initialized )
		return 0;

	i = GL_FindTexture(name);
	if(i > 0 && !update)
	{
		// prolonge registration
		gltexture_t	*tex = &r_textures[i];
		tex->cacheframe = world.load_sequence;
		return i;
	}

	if( !pic ) return 0; // couldn't loading image
	if( update && i == 0 )
	{
		Host_Error( "Couldn't find texture %s for update\n", name );
	}

	// force upload texture as RGB or RGBA (detail textures requires this)
	if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

	gltexture_t *tex = nullptr;
	if( !update )
	{
		// find a free texture_t slot
		i = GL_AllocateTextureSlot(name);

		tex = &r_textures[i];
		tex->texnum = i;	// texnum is used for fast acess into r_textures array too
		tex->flags = flags;
	}
	else
	{
		tex = &r_textures[i];
		tex->flags |= flags;
	}

	GL_UploadTexture( pic, tex, update, NULL );
	GL_TexFilter( tex, update ); // update texture filter, wrap etc

	return i;
}

namespace xe {
	struct texlru_src_external_s {
		std::string path;
		const mstudiotexture_t *texture;
	};
	struct texlru_src_internal_s {
		std::string texture_name;
		std::string model_name;
        const studiohdr_t *phdr;
        const mstudiotexture_t *texture;
		size_t size;
	};
    struct texlru_src_spr_s {
        std::string path;
        const dsprite_t *psprite;
        const dspriteframe_t *texture;
    };
	struct texlru_extdata_s {
		int texnum;
		imgfilter_t *filter;
		std::variant<texlru_src_external_s, texlru_src_internal_s, texlru_src_spr_s> src;
	};

    std::unordered_map<const mstudiotexture_t *, TextureAttr> g_textureAttrMap;

    void UpdateAttr(const mstudiotexture_t *ptexture, TextureAttr &attr)
    {
        if ( ptexture->name[0] == '$' )
        {
            if ( ptexture->name[1] == '0' && ptexture->name[2] == 'b' )
            {
                attr.flags |= STUDIO_NF_ADDITIVE;
                attr.flags |= STUDIO_NF_FULLBRIGHT;
            }
        }

        gltexture_t *gltex = R_GetTexture(attr.index);
        if (gltex)
        {
            attr.width = gltex->width;
            attr.height = gltex->height;
        }
    }

    void TexLru_CreateAttr(const mstudiotexture_t *ptexture, int gl_texturenum)
    {
        TextureAttr attr = { ptexture->flags, ptexture->width, ptexture->height, gl_texturenum };

        if ( ptexture->name[0] == '$' )
        {
            if ( ptexture->name[1] == '0' && ptexture->name[2] == 'b' )
            {
                attr.flags |= STUDIO_NF_ADDITIVE;
                attr.flags |= STUDIO_NF_FULLBRIGHT;
            }
        }

        gltexture_t *gltex = R_GetTexture(gl_texturenum);
        if (gltex)
        {
            attr.index = gl_texturenum;
            attr.width = gltex->width;
            attr.height = gltex->height;
        }

        auto [iter, succ] = g_textureAttrMap.emplace(ptexture, TextureAttr{  ptexture->flags, ptexture->width, ptexture->height, gl_texturenum });
        return UpdateAttr(ptexture, iter->second);
    }

	int TexLru_LoadTextureExternal(const char *name, const mstudiotexture_t *ptexture, int flags, imgfilter_t *filter)
	{
		uint		picFlags = 0;

		if( !name || !name[0] || !glw_state.initialized )
			return 0;

		// see if already loaded
		int i = GL_FindTexture(name);
		if(i > 0)
		{
			// prolonge registration
			gltexture_t	*tex = &r_textures[i];
			tex->cacheframe = world.load_sequence;
			if (tex->name[0] != '*')
				tex->refCount++;

			return i;
		}

		if( flags & TF_NOFLIP_TGA )
			picFlags |= IL_DONTFLIP_TGA;

		if( flags & TF_KEEP_8BIT )
			picFlags |= IL_KEEP_8BIT;

		// set some image flags
		Image_SetForceFlags( picFlags );

		// HACKHACK: get rid of black vertical line on a 'BlackMesa map'
		if( !Q_strcmp( name, "#lab1_map1.mip" ) || !Q_strcmp( name, "#lab1_map2.mip" ))
			flags |= TF_NEAREST;

		i = GL_AllocateTextureSlot(name);

		auto tex = &r_textures[i];
		Q_strncpy( tex->name, name, sizeof( tex->name ));
		tex->flags = (texFlags_t)flags;

		// init reference exclude brush model's texture
		if (tex->name[0] != '*')
			tex->refCount = 1;

		if( flags & TF_SKYSIDE )
			tex->texnum = i;//tr.skyboxbasenum++;
		else tex->texnum = i; // texnum is used for fast acess into r_textures array too

		// add to lru
		tex->texlru_extdata = std::make_unique<texlru_extdata_s>(texlru_extdata_s{ i, filter, texlru_src_external_s{ name, ptexture } });

		// NOTE: always return texnum as index in array or engine will stop work !!!
		return i;
	}

	int TexLru_LoadTextureInternal(const char *name, const char *model_name, const studiohdr_t *phdr, const mstudiotexture_t *ptexture, size_t size, int flags, imgfilter_t *filter)
	{
		uint		picFlags = 0;

		if( !name || !name[0] || !glw_state.initialized )
			return 0;

		// see if already loaded
		int i = GL_FindTexture(name);
		if(i > 0)
		{
			// prolonge registration
			gltexture_t	*tex = &r_textures[i];
			tex->cacheframe = world.load_sequence;
			if (tex->name[0] != '*')
				tex->refCount++;

			return i;
		}

		if( flags & TF_NOFLIP_TGA )
			picFlags |= IL_DONTFLIP_TGA;

		if( flags & TF_KEEP_8BIT )
			picFlags |= IL_KEEP_8BIT;

		// set some image flags
		Image_SetForceFlags( picFlags );

		// HACKHACK: get rid of black vertical line on a 'BlackMesa map'
		if( !Q_strcmp( name, "#lab1_map1.mip" ) || !Q_strcmp( name, "#lab1_map2.mip" ))
			flags |= TF_NEAREST;

		i = GL_AllocateTextureSlot(name);
		gltexture_t	*tex = &r_textures[i];
		Q_strncpy( tex->name, name, sizeof( tex->name ));
		tex->flags = (texFlags_t)flags;

		// init reference exclude brush model's texture
		if (tex->name[0] != '*')
			tex->refCount = 1;

		if( flags & TF_SKYSIDE )
			tex->texnum = i;//tr.skyboxbasenum++;
		else tex->texnum = i; // texnum is used for fast acess into r_textures array too

		// add to lru
		tex->texlru_extdata = std::make_unique<texlru_extdata_s>(texlru_extdata_s{ i, filter, texlru_src_internal_s{ name, model_name, phdr, ptexture, size } });

		// NOTE: always return texnum as index in array or engine will stop work !!!
		return i;
	}

    int TexLru_LoadTextureSPR(const char *name, const dsprite_t *psprite, const byte *buf, const dspriteframe_t *frame, int flags, imgfilter_t *filter)
    {
        uint		picFlags = 0;

        if( !name || !name[0] || !glw_state.initialized )
            return 0;

        // see if already loaded
        int i = GL_FindTexture(name);
        if(i > 0)
        {
            // prolonge registration
            gltexture_t	*tex = &r_textures[i];
            tex->cacheframe = world.load_sequence;
            if (tex->name[0] != '*')
                tex->refCount++;

            return i;
        }

        if( flags & TF_NOFLIP_TGA )
            picFlags |= IL_DONTFLIP_TGA;

        if( flags & TF_KEEP_8BIT )
            picFlags |= IL_KEEP_8BIT;

        // set some image flags
        Image_SetForceFlags( picFlags );

        // HACKHACK: get rid of black vertical line on a 'BlackMesa map'
        if( !Q_strcmp( name, "#lab1_map1.mip" ) || !Q_strcmp( name, "#lab1_map2.mip" ))
            flags |= TF_NEAREST;

        i = GL_AllocateTextureSlot(name);

        auto tex = &r_textures[i];
        Q_strncpy( tex->name, name, sizeof( tex->name ));
        tex->flags = (texFlags_t)flags;

        // init reference exclude brush model's texture
        if (tex->name[0] != '*')
            tex->refCount = 1;

        if( flags & TF_SKYSIDE )
            tex->texnum = i;//tr.skyboxbasenum++;
        else tex->texnum = i; // texnum is used for fast acess into r_textures array too

        // add to lru
        tex->texlru_extdata = std::make_unique<texlru_extdata_s>(texlru_extdata_s{ i, filter, texlru_src_spr_s{ name, psprite, frame } });

        // NOTE: always return texnum as index in array or engine will stop work !!!
        return i;
    }

	int TexLru_ReleaseTexture(gltexture_t *image)
	{
		// release source
		if( image->flags & (TF_KEEP_RGBDATA|TF_KEEP_8BIT) && image->original )
		{
			image->original = nullptr;
		}

		//pglDeleteTextures( 1, &image->texnum );

		byte data2D[4] = {  };
		data2D[0] = data2D[1] = data2D[2] = 80;
		data2D[3] = 255;

        pglBindTexture( image->target, image->texnum );
		pglTexImage2D( image->target, 0, image->format, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2D);
        pglBindTexture( image->target, 0 );
		MsgDev(D_INFO, "TexLru : swap out texture %s\n", image->name);

		return 0;
	}

    void SPR_InstallPal(const dsprite_t *psprite)
    {
        image_ref pal;

        auto buffer = (const byte *)psprite + sizeof(dsprite_t) + sizeof(short);

        // install palette
        switch (psprite->texFormat) {
            case SPR_ADDITIVE:
                pal = FS_LoadImage("#normal.pal", buffer, 768);
                break;
            case SPR_INDEXALPHA:
                pal = FS_LoadImage("#decal.pal", buffer, 768);
                break;
            case SPR_ALPHTEST:
                pal = FS_LoadImage("#transparent.pal", buffer, 768);
                break;
            case SPR_NORMAL:
            default:
                pal = FS_LoadImage("#normal.pal", buffer, 768);
                break;
        }

        buffer += 768; //读下一个数据块
        pal = nullptr; // palette installed, no reason to keep this data
    }

	struct TexLru_Uploader{
		gltexture_t	*tex;
		imgfilter_t *filter;
		std::shared_ptr<gltexture_t> operator()(texlru_src_external_s &src) const
		{
			int flags = tex->flags;
			auto pic = FS_LoadImage( src.path.c_str(), (byte*)src.texture, 0 );
			if( !pic )
			{
				MsgDev(D_ERROR, "TexLru : fail to load load external texture %s\n", src.path.c_str());
				return {}; // couldn't loading image
			}

			// force upload texture as RGB or RGBA (detail textures requires this)
			if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

			GL_UploadTexture( pic, tex, false, filter );
			GL_TexFilter( tex, false ); // update texture filter, wrap etc

			if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
				pic = nullptr; // release source texture

			MsgDev(D_INFO, "TexLru : load external texture %s\n", src.path.c_str());
			return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture };
		}
		std::shared_ptr<gltexture_t> operator()(texlru_src_internal_s &src) const
		{
            const studiohdr_t *phdr = src.phdr;
			Image_SetMDLPointer((const byte *)phdr + src.texture->index);

			int flags = tex->flags;
			// hack : expected mstudiotexture_t* as buffer
			auto pic = FS_LoadImage( src.texture_name.c_str(), (const byte *)src.texture, src.size );
			if( !pic )
			{
				MsgDev(D_ERROR, "TexLru : fail to load internal texture %s from %s\n", src.texture->name, src.model_name.c_str());
				return {}; // couldn't loading image
			}

			// force upload texture as RGB or RGBA (detail textures requires this)
			if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

			GL_UploadTexture( pic, tex, false, filter );
			GL_TexFilter( tex, false ); // update texture filter, wrap etc

			if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
				pic = nullptr; // release source texture

			MsgDev(D_INFO, "TexLru : load internal texture %s from %s\n", src.texture->name, src.model_name.c_str());
			return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture };
		}
        std::shared_ptr<gltexture_t> operator()(texlru_src_spr_s &src) const
        {
            SPR_InstallPal(src.psprite);
            int flags = tex->flags;
            auto pic = FS_LoadImage( src.path.c_str(), (const byte*)src.texture, src.texture->width * src.texture->height );
            if( !pic )
            {
                MsgDev(D_ERROR, "TexLru : fail to load load sprite texture %s\n", src.path.c_str());
                return {}; // couldn't loading image
            }

            // force upload texture as RGB or RGBA (detail textures requires this)
            if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

            GL_UploadTexture( pic, tex, false, filter );
            GL_TexFilter( tex, false ); // update texture filter, wrap etc

            if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
                pic = nullptr; // release source texture

            // set spr type
            tex->texType = TEX_SPRITE;

            MsgDev(D_INFO, "TexLru : load SPR texture %s\n", src.path.c_str());
            return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture };
        }
	};

	template<class Key, class Value>
	class lru_cache
	{
	public:
		typedef Key key_type;
		typedef Value value_type;
		typedef std::list<key_type> list_type;
		typedef std::unordered_map<
				key_type,
				std::pair<value_type, typename list_type::iterator>
		> map_type;

		lru_cache(size_t capacity)
				: m_capacity(capacity)
		{
		}

		~lru_cache()
		{
		}

		size_t size() const
		{
			return m_map.size();
		}

		size_t capacity() const
		{
			return m_capacity;
		}

		bool empty() const
		{
			return m_map.empty();
		}

		bool contains(const key_type &key)
		{
			return m_map.find(key) != m_map.end();
		}

		void insert(const key_type &key, const value_type &value)
		{
			auto i = m_map.find(key);
			if(i == m_map.end()){
				// insert item into the cache, but first check if it is full
				while(m_capacity && size() >= m_capacity){
					// cache is full, evict the least recently used item
					evict();
				}

				// insert the new item
				m_list.push_front(key);
				m_map[key] = std::make_pair(value, m_list.begin());
			}
		}

		void remove(const key_type &key)
		{
			auto i = m_map.find(key);
			auto i2 = i->second.second;
			m_list.erase(i2);
			m_map.erase(i);
		}

		std::optional<value_type> get(const key_type &key)
		{
			// lookup value in the cache
			auto i = m_map.find(key);
			if(i == m_map.end()){
				// value not in cache
				return std::nullopt;
			}

			// return the value, but first update its place in the most
			// recently used list
			auto j = i->second.second;
			if(j != m_list.begin()){
				// move item to the front of the most recently used list
				m_list.erase(j);
				m_list.push_front(key);

				// update iterator in map
				j = m_list.begin();
				const value_type &value = i->second.first;
				m_map[key] = std::make_pair(value, j);

				// return the value
				return value;
			}
			else {
				// the item is already at the front of the most recently
				// used list so just return it
				return i->second.first;
			}
		}

		void clear()
		{
			m_map.clear();
			m_list.clear();
		}

		void set_capacity(size_t value)
		{
			m_capacity = value;
		}

		void evict()
		{
			// evict item from the end of most recently used list
			typename list_type::iterator i = --m_list.end();
			m_map.erase(*i);
			m_list.erase(i);
		}

	private:
		map_type m_map;
		list_type m_list;
		size_t m_capacity;
	};

	static lru_cache<int, std::shared_ptr<gltexture_t>> textureCache(0);

	bool TexLru_Upload( int texnum )
	{
		gltexture_t	*tex = &r_textures[texnum];
		if(!tex->texlru_extdata)
			return false; // unknown texture

		if (textureCache.contains((GLuint)texnum)) {
			return false; // already present
		}

		textureCache.set_capacity(r_texlru->integer);

		auto &lrudata = *tex->texlru_extdata;
		auto deleter = std::visit(TexLru_Uploader{ tex, lrudata.filter }, lrudata.src);
		textureCache.insert((GLuint)texnum, std::move(deleter));
        return true;
	}

    const TextureAttr &TexLru_GetAttr(const mstudiotexture_t *ptexture)
    {
        return g_textureAttrMap.at(ptexture);
    }

    void TexLru_FreeTexture(const mstudiotexture_t *ptexture)
    {
        // TODO : prevent crash ?
        auto iter = g_textureAttrMap.find(ptexture);
        if(iter != g_textureAttrMap.end())
        {
            GL_FreeTexture(iter->second.index);
            g_textureAttrMap.erase(iter);
        }
    }

	void TexLru_Free( int texnum )
	{
		gltexture_t	*tex = &r_textures[texnum];
		tex->texlru_extdata = nullptr;
		if(textureCache.contains(texnum))
			textureCache.remove(texnum);
	}

	void TexLru_Shrink()
	{
		while( textureCache.size() >= 64 ){
			// cache is full, evict the least recently used item
			textureCache.evict();
		}
	}

	void TexLru_Clear()
	{
        textureCache.clear();
	}

    void TexLru_Bind(const mstudiotexture_t *ptexture)
    {
        auto &attr = g_textureAttrMap.at(ptexture);

        // set size for external texture
        TexLru_Upload(attr.index);
        UpdateAttr(ptexture, attr);
        return GL_Bind( XASH_TEXTURE0, attr.index);
    }

    void TexLru_Bind(const mspriteframe_t *pspriteframe)
    {
        TexLru_Upload(pspriteframe->gl_texturenum);
        return GL_Bind(XASH_TEXTURE0, pspriteframe->gl_texturenum);
    }
}

/*
================
GL_CreateTexture

creates an empty 32-bit texture (just reserve slot)
================
*/
int GL_CreateTexture( const char *name, int width, int height, const void *buffer, texFlags_t flags )
{
	int	texture;
	image_ref r_empty = Image_NewTemp();
	r_empty->width = width;
	r_empty->height = height;
	r_empty->type = PF_RGBA_32;
	r_empty->size = r_empty->width * r_empty->height * 4;
	r_empty->flags = IMAGE_HAS_COLOR | (( flags & TF_HAS_ALPHA ) ? IMAGE_HAS_ALPHA : 0 ) | IMAGE_TEMP;
	r_empty->buffer = (byte *)buffer;

	if( flags & TF_TEXTURE_1D )
	{
		r_empty->height = 1;
		r_empty->size = r_empty->width * 4;
	}
	else if( flags & TF_TEXTURE_3D )
	{
		if( !GL_Support( GL_TEXTURE_3D_EXT ))
			return 0;

		r_empty->depth = r_empty->width;
		r_empty->size = r_empty->width * r_empty->height * r_empty->depth * 4;
	}
	else if( flags & TF_CUBEMAP )
	{
		flags &= ~TF_CUBEMAP; // will be set later
		r_empty->flags |= IMAGE_CUBEMAP;
		r_empty->size *= 6;
	}

	texture = GL_LoadTextureInternal( name, r_empty, flags, false );

	if( flags & TF_DEPTHMAP )
		GL_SetTextureType( texture, TEX_DEPTHMAP );
	else GL_SetTextureType( texture, TEX_CUSTOM );

	return texture;
}

/*
================
GL_ProcessTexture
================
*/
void GL_ProcessTexture( int texnum, float gamma, int topColor, int bottomColor )
{
	gltexture_t	*image;
	int		flags = 0;

	if( texnum <= 0 ) return; // missed image
	ASSERT( texnum > 0 && texnum < R_TextureNum() );
	image = &r_textures[texnum];

	// select mode
	if( gamma != -1.0f )
	{
		flags = IMAGE_LIGHTGAMMA;
	}
	else if( topColor != -1 && bottomColor != -1 )
	{
		flags = IMAGE_REMAP;
	}
	else
	{
		MsgDev( D_ERROR, "GL_ProcessTexture: bad operation for %s\n", image->name );
		return;
	}

	if(!( image->flags & (TF_KEEP_RGBDATA|TF_KEEP_8BIT)) || !image->original )
	{
		MsgDev( D_ERROR, "GL_ProcessTexture: no input data for %s\n", image->name );
		return;
	}

	if( ImageDXT( image->original->type ))
	{
		MsgDev( D_ERROR, "GL_ProcessTexture: can't process compressed texture %s\n", image->name );
		return;
	}

	// all the operations makes over the image copy not an original
	auto pic = FS_CopyImage( image->original );
	Image_Process( &pic, topColor, bottomColor, gamma, flags, NULL );

	GL_UploadTexture( pic, image, true, NULL );
	GL_TexFilter( image, true ); // update texture filter, wrap etc
}

/*
================
GL_LoadTexture
================
*/
int GL_FindTexture( const char *name )
{
	gltexture_t	*tex;
	uint		hash;

	if( !name || !name[0] || !glw_state.initialized )
		return 0;

	// see if already loaded
	auto iter = r_texturesHashTable.find(name);
	if(iter != r_texturesHashTable.end())
	{
		return iter->second;
	}

	return 0;
}

/*
================
GL_FreeImage

Frees image by name
================
*/
void GL_FreeImage( const char *name )
{
	if( !name || !name[0] || !glw_state.initialized )
		return;

	// see if already loaded
	auto iter = r_texturesHashTable.find(name);
	if(iter != r_texturesHashTable.end())
	{
		int i = iter->second;
		R_FreeImage( &r_textures[i] );
	}
}

/*
================
GL_FreeTexture
================
*/
void GL_FreeTexture( GLenum texnum )
{
	// number 0 it's already freed
	if( texnum <= 0 || !glw_state.initialized )
		return;

	ASSERT( texnum > 0 && texnum < R_TextureNum() );
	if (texnum >= R_TextureNum())
		return;
	R_FreeImage( &r_textures[texnum] );
}

/*
================
R_FreeImage
================
*/
void R_FreeImage( gltexture_t *image )
{
	uint		hash;
	gltexture_t	*cur;
	gltexture_t	**prev;

	ASSERT( image != NULL );

	if( !image->name[0] )
	{
		if( image->texnum != 0 )
			MsgDev( D_ERROR, "trying to free unnamed texture with texnum %i\n", image->texnum );
		return;
	}

	if (image->refCount && --image->refCount > 0)
	{
		return;
	}

	auto iter = r_texturesHashTable.find(image->name);
	if (iter == r_texturesHashTable.end())
	{
		MsgDev(D_ERROR, "couldn't find %i %s\n", image->texnum, image->name);
		return;
	}

	xe::TexLru_Free( image->texnum );

	// remove from hash table
	iter = r_texturesHashTable.find(image->name);
	if(iter != r_texturesHashTable.end())
	{
		r_texturesHashTable.erase(iter);
	}

	// release source
	if( image->flags & (TF_KEEP_RGBDATA|TF_KEEP_8BIT) && image->original )
	{
		image->original.reset();
	}

	pglDeleteTextures( 1, &image->texnum );
	*image = {};
}

/*
==============================================================================

INTERNAL TEXTURES

==============================================================================
*/
/*
==================
R_InitDefaultTexture
==================
*/
static image_ref R_InitDefaultTexture( texFlags_t *flags )
{
	int	x, y;

	// also use this for bad textures, but without alpha
	r_image->width = r_image->height = 16;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	*flags = (texFlags_t)0;

	// emo-texture from quake1
	for( y = 0; y < 16; y++ )
	{
		for( x = 0; x < 16; x++ )
		{
			if(( y < 8 ) ^ ( x < 8 ))
				data2D[y*16+x] = 0xFFFF00FF;
			else data2D[y*16+x] = 0xFF000000;
		}
	}
	return r_image;
}

/*
==================
R_InitParticleTexture
==================
*/
static image_ref R_InitParticleTexture( texFlags_t *flags )
{
	int	x, y;
	int	dx2, dy, d;

	// particle texture
	r_image->width = r_image->height = 16;
	r_image->flags = (IMAGE_HAS_COLOR|IMAGE_HAS_ALPHA);
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	*flags = TF_NOPICMIP|TF_NOMIPMAP;

	for( x = 0; x < 16; x++ )
	{
		dx2 = x - 8;
		dx2 = dx2 * dx2;

		for( y = 0; y < 16; y++ )
		{
			dy = y - 8;
			d = 255 - 35 * sqrt( dx2 + dy * dy );
			data2D[( y*16 + x ) * 4 + 3] = bound( 0, d, 255 );
		}
	}
	return r_image;
}

/*
==================
R_InitOldParticleTexture
==================
*/
static image_ref R_InitOldParticleTexture(texFlags_t *flags)
{
	int	x, y;

	// particle texture
	r_image->width = r_image->height = 8;
	r_image->flags = (IMAGE_HAS_COLOR | IMAGE_HAS_ALPHA);
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	*flags = TF_NOPICMIP | TF_NOMIPMAP;

	for (x = 0; x < 8; x++)
	{
		for (y = 0; y < 8; y++)
		{
			data2D[(y * 8 + x) * 4 + 0] = 255;
			data2D[(y * 8 + x) * 4 + 1] = 255;
			data2D[(y * 8 + x) * 4 + 2] = 255;
			data2D[(y * 8 + x) * 4 + 3] = r_oldParticleTexture[x][y] * 255;
		}
	}
	return r_image;
}

/*
==================
R_InitParticleTexture2
==================
*/
static image_ref R_InitParticleTexture2( texFlags_t *flags )
{
	int	x, y;

	// particle texture
	r_image->width = r_image->height = 8;
	r_image->flags = (IMAGE_HAS_COLOR|IMAGE_HAS_ALPHA);
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	*flags = TF_NOPICMIP|TF_NOMIPMAP;

	for( x = 0; x < 8; x++ )
	{
		for( y = 0; y < 8; y++ )
		{
			data2D[(y * 8 + x) * 4 + 0] = 255;
			data2D[(y * 8 + x) * 4 + 1] = 255;
			data2D[(y * 8 + x) * 4 + 2] = 255;
			data2D[(y * 8 + x) * 4 + 3] = r_particleTexture[x][y] * 255;
		}
	}
	return r_image;
}

/*
==================
R_InitSkyTexture
==================
*/
static image_ref R_InitSkyTexture( texFlags_t *flags )
{
	int	i;

	r_image->width = r_image->height = 16;
	r_image->size = r_image->width * r_image->height * 4;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	// skybox texture
	for( i = 0; i < 256; i++ )
		((uint *)data2D)[i] = 0xFFFFDEB5;

	*flags = TF_NOPICMIP|TF_UNCOMPRESSED;

	return r_image;
}

/*
==================
R_InitCinematicTexture
==================
*/
static image_ref R_InitCinematicTexture( texFlags_t *flags )
{
	r_image->type = PF_RGBA_32;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->width = r_image->height = 256;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	*flags = TF_NOMIPMAP|TF_NOPICMIP|TF_UNCOMPRESSED|TF_CLAMP;

	return r_image;
}

/*
==================
R_InitSolidColorTexture
==================
*/
static image_ref R_InitSolidColorTexture( texFlags_t *flags, int color )
{
	// solid color texture
	r_image->width = r_image->height = 1;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGB_24;
	r_image->size = r_image->width * r_image->height * 3;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	*flags = TF_NOMIPMAP|TF_NOPICMIP|TF_UNCOMPRESSED;

	data2D[0] = data2D[1] = data2D[2] = color;
	return r_image;
}

/*
==================
R_InitWhiteTexture
==================
*/
static image_ref R_InitWhiteTexture( texFlags_t *flags )
{
	return R_InitSolidColorTexture( flags, 255 );
}

/*
==================
R_InitGrayTexture
==================
*/
static image_ref R_InitGrayTexture( texFlags_t *flags )
{
	return R_InitSolidColorTexture( flags, 127 );
}

/*
==================
R_InitBlackTexture
==================
*/
static image_ref R_InitBlackTexture( texFlags_t *flags )
{
	return R_InitSolidColorTexture( flags, 0 );
}

/*
==================
R_InitBlankBumpTexture
==================
*/
static image_ref R_InitBlankBumpTexture( texFlags_t *flags )
{
	int	i;
	r_image->width = r_image->height = 16;
	r_image->size = r_image->width * r_image->height * 4;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	// default normalmap texture
	for( i = 0; i < 256; i++ )
	{
		data2D[i*4+0] = 127;
		data2D[i*4+1] = 127;
		data2D[i*4+2] = 255;
	}

	*flags = TF_NORMALMAP|TF_UNCOMPRESSED;

	return r_image;
}

/*
==================
R_InitBlankDeluxeTexture
==================
*/
static image_ref R_InitBlankDeluxeTexture( texFlags_t *flags )
{
	int	i;

	r_image->width = r_image->height = 16;
	r_image->size = r_image->width * r_image->height * 4;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	// default normalmap texture
	for( i = 0; i < 256; i++ )
	{
		data2D[i*4+0] = 127;
		data2D[i*4+1] = 127;
		data2D[i*4+2] = 0;	// light from ceiling
	}

	*flags = TF_NORMALMAP|TF_UNCOMPRESSED;

	return r_image;
}

/*
==================
R_InitAttenuationTexture
==================
*/
static image_ref R_InitAttenTextureGamma( texFlags_t *flags, float gamma )
{
	int	i;

	// 1d attenuation texture
	r_image->width = 256;
	r_image->height = 1;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	for( i = 0; i < r_image->width; i++ )
	{
		float atten = 255 - bound( 0, 255 * pow((i + 0.5f) / r_image->width, gamma ) + 0.5f, 255 );

		// clear attenuation at ends to prevent light go outside
		if( i == (r_image->width - 1) || i == 0 )
			atten = 0.0f;

		data2D[(i * 4) + 0] = (byte)atten;
		data2D[(i * 4) + 1] = (byte)atten;
		data2D[(i * 4) + 2] = (byte)atten;
		data2D[(i * 4) + 3] = (byte)atten;
	}

	*flags = TF_UNCOMPRESSED|TF_NOMIPMAP|TF_CLAMP|TF_TEXTURE_1D;

	return r_image;
}

static image_ref R_InitAttenuationTexture( texFlags_t *flags )
{
	return R_InitAttenTextureGamma( flags, 1.5f );
}

static image_ref R_InitAttenuationTexture2( texFlags_t *flags )
{
	return R_InitAttenTextureGamma( flags, 0.5f );
}

static image_ref R_InitAttenuationTexture3( texFlags_t *flags )
{
	return R_InitAttenTextureGamma( flags, 3.5f );
}

static image_ref R_InitAttenuationTextureNoAtten( texFlags_t *flags )
{
	// 1d attenuation texture
	r_image->width = 256;
	r_image->height = 1;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	Q_memset( data2D, 0xFF, r_image->size );
	*flags = TF_UNCOMPRESSED|TF_NOMIPMAP|TF_CLAMP|TF_TEXTURE_1D;

	return r_image;
}

/*
==================
R_InitAttenuationTexture3D
==================
*/
static image_ref R_InitAttenTexture3D( texFlags_t *flags )
{
	vec3_t	v = { 0, 0, 0 };
	int	x, y, z, d, size, size2, halfsize;
	float	intensity;

	if( !GL_Support( GL_TEXTURE_3D_EXT ))
		return NULL;

	// 3d attenuation texture
	r_image->width = 32;
	r_image->height = 32;
	r_image->depth = 32;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * r_image->depth * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	size = 32;
	halfsize = size / 2;
	intensity = halfsize * halfsize;
	size2 = size * size;

	for( x = 0; x < r_image->width; x++ )
	{
		for( y = 0; y < r_image->height; y++ )
		{
			for( z = 0; z < r_image->depth; z++ )
			{
				v[0] = (( x + 0.5f ) * ( 2.0f / (float)size ) - 1.0f );
				v[1] = (( y + 0.5f ) * ( 2.0f / (float)size ) - 1.0f );
				if( r_image->depth > 1 ) v[2] = (( z + 0.5f ) * ( 2.0f / (float)size ) - 1.0f );

				intensity = 1.0f - sqrt( DotProduct( v, v ) );
				if( intensity > 0 ) intensity = intensity * intensity * 215.5f;
				d = bound( 0, intensity, 255 );

				data2D[((z * size + y) * size + x) * 4 + 0] = d;
				data2D[((z * size + y) * size + x) * 4 + 1] = d;
				data2D[((z * size + y) * size + x) * 4 + 2] = d;
			}
		}
	}

	*flags = TF_UNCOMPRESSED|TF_NOMIPMAP|TF_CLAMP|TF_TEXTURE_3D;

	return r_image;
}

static image_ref R_InitDlightTexture( texFlags_t *flags )
{
	// solid color texture
	r_image->width = BLOCK_SIZE_DEFAULT;
	r_image->height = BLOCK_SIZE_DEFAULT;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	Q_memset( data2D, 0x00, r_image->size );

	*flags = TF_NOPICMIP|TF_UNCOMPRESSED|TF_NOMIPMAP;

	return r_image;
}

static image_ref R_InitDlightTexture2( texFlags_t *flags )
{
	// solid color texture
	r_image->width = BLOCK_SIZE_MAX;
	r_image->height = BLOCK_SIZE_MAX;
	r_image->flags = IMAGE_HAS_COLOR;
	r_image->type = PF_RGBA_32;
	r_image->size = r_image->width * r_image->height * 4;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	Q_memset( data2D, 0x00, r_image->size );

	*flags = TF_NOPICMIP|TF_UNCOMPRESSED|TF_NOMIPMAP;

	return r_image;
}

/*
==================
R_InitNormalizeCubemap
==================
*/
static image_ref R_InitNormalizeCubemap( texFlags_t *flags )
{
	int	i, x, y, size = 32;

	r_image->width = r_image->height = size;
	r_image->size = r_image->width * r_image->height * 4 * 6;
	r_image->flags |= (IMAGE_CUBEMAP|IMAGE_HAS_COLOR); // yes it's cubemap
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;
	byte	*dataCM = data2D;
	float	s, t;
	vec3_t	normal;

	if( !GL_Support( GL_TEXTURECUBEMAP_EXT ))
		return NULL;

	// normal cube map texture
	for( i = 0; i < 6; i++ )
	{
		for( y = 0; y < size; y++ )
		{
			for( x = 0; x < size; x++ )
			{
				s = (((float)x + 0.5f) * (2.0f / size )) - 1.0f;
				t = (((float)y + 0.5f) * (2.0f / size )) - 1.0f;

				switch( i )
				{
				case 0: VectorSet( normal, 1.0f, -t, -s ); break;
				case 1: VectorSet( normal, -1.0f, -t, s ); break;
				case 2: VectorSet( normal, s,  1.0f,  t ); break;
				case 3: VectorSet( normal, s, -1.0f, -t ); break;
				case 4: VectorSet( normal, s, -t, 1.0f  ); break;
				case 5: VectorSet( normal, -s, -t, -1.0f); break;
				}

				VectorNormalize( normal );

				dataCM[4*(y*size+x)+0] = (byte)(128 + 127 * normal[0]);
				dataCM[4*(y*size+x)+1] = (byte)(128 + 127 * normal[1]);
				dataCM[4*(y*size+x)+2] = (byte)(128 + 127 * normal[2]);
				dataCM[4*(y*size+x)+3] = 255;
			}
		}
		dataCM += (size*size*4); // move pointer
	}

	*flags = (TF_NOPICMIP|TF_NOMIPMAP|TF_UNCOMPRESSED|TF_CUBEMAP|TF_CLAMP);

	return r_image;
}

/*
==================
R_InitDlightCubemap
==================
*/
static image_ref R_InitDlightCubemap( texFlags_t *flags )
{
	int	i, x, y, size = 4;

	r_image->width = r_image->height = size;
	r_image->size = r_image->width * r_image->height * 4 * 6;
	r_image->flags |= (IMAGE_CUBEMAP|IMAGE_HAS_COLOR); // yes it's cubemap
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;
	byte	*dataCM = data2D;
	int	dx2, dy, d;

	if( !GL_Support( GL_TEXTURECUBEMAP_EXT ))
		return NULL;

	// normal cube map texture
	for( i = 0; i < 6; i++ )
	{
		for( x = 0; x < size; x++ )
		{
			dx2 = x - size / 2;
			dx2 = dx2 * dx2;

			for( y = 0; y < size; y++ )
			{
				dy = y - size / 2;
				d = 255 - 35 * sqrt( dx2 + dy * dy );
				dataCM[( y * size + x ) * 4 + 0] = bound( 0, d, 255 );
				dataCM[( y * size + x ) * 4 + 1] = bound( 0, d, 255 );
				dataCM[( y * size + x ) * 4 + 2] = bound( 0, d, 255 );
			}
		}
		dataCM += (size * size * 4); // move pointer
	}

	*flags = (TF_NOPICMIP|TF_NOMIPMAP|TF_UNCOMPRESSED|TF_CUBEMAP|TF_CLAMP);

	return r_image;
}

/*
==================
R_InitGrayCubemap
==================
*/
static image_ref R_InitGrayCubemap( texFlags_t *flags )
{
	int	size = 4;

	r_image->width = r_image->height = size;
	r_image->size = r_image->width * r_image->height * 4 * 6;
	r_image->flags |= (IMAGE_CUBEMAP|IMAGE_HAS_COLOR); // yes it's cubemap
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*4);
	r_image->buffer = data2D;
	byte	*dataCM = data2D;

	if( !GL_Support( GL_TEXTURECUBEMAP_EXT ))
		return NULL;

	// gray cubemap - just stub for pointlights
	Q_memset( dataCM, 0x7F, size * size * 6 * 4 );

	*flags = (TF_NOPICMIP|TF_NOMIPMAP|TF_UNCOMPRESSED|TF_CUBEMAP|TF_CLAMP);

	return r_image;
}

/*
==================
R_InitWhiteCubemap
==================
*/
static image_ref R_InitWhiteCubemap( texFlags_t *flags )
{
	int	size = 4;

	r_image->width = r_image->height = size;
	r_image->size = r_image->width * r_image->height * 4 * 6;
	r_image->flags |= (IMAGE_CUBEMAP|IMAGE_HAS_COLOR); // yes it's cubemap
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*4);
	r_image->buffer = data2D;
	byte	*dataCM = data2D;

	if( !GL_Support( GL_TEXTURECUBEMAP_EXT ))
		return NULL;

	// white cubemap - just stub for pointlights
	Q_memset( dataCM, 0xFF, size * size * 6 * 4 );

	*flags = (TF_NOPICMIP|TF_NOMIPMAP|TF_UNCOMPRESSED|TF_CUBEMAP|TF_CLAMP);

	return r_image;
}

/*
==================
R_InitAlphaContrast
==================
*/
static image_ref R_InitAlphaContrast( texFlags_t *flags )
{
	int	size = 64;

	r_image->width = r_image->height = 64;
	r_image->size = r_image->width * r_image->height * 4;
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, BLOCK_SIZE_MAX*BLOCK_SIZE_MAX*4);
	r_image->buffer = data2D;
	byte	*data = data2D;

	*flags = (TF_NOPICMIP|TF_UNCOMPRESSED|TF_ALPHACONTRAST|TF_INTENSITY);

	Q_memset( data, size, r_image->size );

	return r_image;
}

/*
==================
R_InitVSDCTCubemap
==================
*/
static image_ref R_InitVSDCTCubemap( texFlags_t *flags )
{
	// maps to a 2x3 texture rectangle with normalized coordinates
	// +-
	// XX
	// YY
	// ZZ
	// stores abs(dir.xy), offset.xy/2.5

	r_image->width = r_image->height = 1;
	r_image->size = r_image->width * r_image->height * 4 * 6;
	r_image->flags |= (IMAGE_CUBEMAP|IMAGE_HAS_COLOR|IMAGE_HAS_ALPHA); // yes it's cubemap
	r_image->type = PF_RGBA_32;
	byte *data2D = (byte *)Mem_Alloc(r_temppool, r_image->size);
	r_image->buffer = data2D;

	static byte data[4*6] =
	{
		0xFF, 0x00, 0x33, 0x33, // +X: <1, 0>, <0.5, 0.5>
		0xFF, 0x00, 0x99, 0x33, // -X: <1, 0>, <1.5, 0.5>
		0x00, 0xFF, 0x33, 0x99, // +Y: <0, 1>, <0.5, 1.5>
		0x00, 0xFF, 0x99, 0x99, // -Y: <0, 1>, <1.5, 1.5>
		0x00, 0x00, 0x33, 0xFF, // +Z: <0, 0>, <0.5, 2.5>
		0x00, 0x00, 0x99, 0xFF, // -Z: <0, 0>, <1.5, 2.5>
	};
	Q_memcpy(data2D, data, sizeof(data));

	*flags = (TF_NOPICMIP|TF_UNCOMPRESSED|TF_NEAREST|TF_CUBEMAP|TF_CLAMP|TF_NOMIPMAP);

	return r_image;
}


/*
==================
R_InitBuiltinTextures
==================
*/
static void R_InitBuiltinTextures( void )
{
	texFlags_t	flags;

	const struct
	{
		const char	*name;
		int	*texnum;
		image_ref (*init)( texFlags_t *flags );
		int	texType;
	}
	textures[] =
	{
	{ "*default", &tr.defaultTexture, R_InitDefaultTexture, TEX_SYSTEM },
	{ "*white", &tr.whiteTexture, R_InitWhiteTexture, TEX_SYSTEM },
	{ "*gray", &tr.grayTexture, R_InitGrayTexture, TEX_SYSTEM },
	{ "*black", &tr.blackTexture, R_InitBlackTexture, TEX_SYSTEM },
	{ "*particle", &tr.particleTexture, R_InitParticleTexture, TEX_SYSTEM },
	{ "*oldparticle", &tr.oldParticleTexture, R_InitOldParticleTexture, TEX_SYSTEM },
	{ "*particle2", &tr.particleTexture2, R_InitParticleTexture2, TEX_SYSTEM },
	{ "*cintexture", &tr.cinTexture, R_InitCinematicTexture, TEX_NOMIP },	// force linear filter
	{ "*dlight", &tr.dlightTexture, R_InitDlightTexture, TEX_LIGHTMAP },
	{ "*dlight2", &tr.dlightTexture2, R_InitDlightTexture2, TEX_LIGHTMAP },
	{ "*atten", &tr.attenuationTexture, R_InitAttenuationTexture, TEX_SYSTEM },
	{ "*atten2", &tr.attenuationTexture2, R_InitAttenuationTexture2, TEX_SYSTEM },
	{ "*atten3", &tr.attenuationTexture3, R_InitAttenuationTexture3, TEX_SYSTEM },
	{ "*attnno", &tr.attenuationStubTexture, R_InitAttenuationTextureNoAtten, TEX_SYSTEM },
	{ "*normalize", &tr.normalizeTexture, R_InitNormalizeCubemap, TEX_CUBEMAP },
	{ "*blankbump", &tr.blankbumpTexture, R_InitBlankBumpTexture, TEX_SYSTEM },
	{ "*blankdeluxe", &tr.blankdeluxeTexture, R_InitBlankDeluxeTexture, TEX_SYSTEM },
	{ "*lightCube", &tr.dlightCubeTexture, R_InitDlightCubemap, TEX_CUBEMAP },
	{ "*grayCube", &tr.grayCubeTexture, R_InitGrayCubemap, TEX_CUBEMAP },
	{ "*whiteCube", &tr.whiteCubeTexture, R_InitWhiteCubemap, TEX_CUBEMAP },
	{ "*atten3D", &tr.attenuationTexture3D, R_InitAttenTexture3D, TEX_SYSTEM },
	{ "*sky", &tr.skyTexture, R_InitSkyTexture, TEX_SYSTEM },
	{ "*alphaContrast", &tr.acontTexture, R_InitAlphaContrast, TEX_SYSTEM },
	{ "*vsdct", &tr.vsdctCubeTexture, R_InitVSDCTCubemap, TEX_SYSTEM },
	{ NULL, NULL, NULL }
	};
	size_t	i, num_builtin_textures = sizeof( textures ) / sizeof( textures[0] ) - 1;

	for( i = 0; i < num_builtin_textures; i++ )
	{
		r_image = Image_NewTemp();
		auto pic = textures[i].init( &flags );
		if( pic == NULL ) continue;
		*textures[i].texnum = GL_LoadTextureInternal( textures[i].name, pic, flags, false );

		GL_SetTextureType( *textures[i].texnum, textures[i].texType );
		r_image = nullptr;
	}
}

/*
===============
R_InitImages
===============
*/
void R_InitImages( void )
{
	uint	i, hash;
	float	f;

	scaledImage = NULL;
	r_textures.clear();
	r_texturesHashTable.clear();

	// create unused 0-entry
	r_textures.resize(1);
	Q_strcpy( r_textures[0].name, "*unused*");

	// build luminance table
	for( i = 0; i < 256; i++ )
	{
		f = (float)i;
		r_luminanceTable[i][0] = f * 0.299f;
		r_luminanceTable[i][1] = f * 0.587f;
		r_luminanceTable[i][2] = f * 0.114f;
	}

	// set texture parameters
	R_SetTextureParameters();
	R_InitBuiltinTextures();

	R_ParseTexFilters( "scripts/texfilter.txt" );
}

/*
===============
R_ShutdownImages
===============
*/
void R_ShutdownImages( void )
{
	gltexture_t	*image;
	int		i;

	if( !glw_state.initialized ) return;

	for( i = ( MAX_TEXTURE_UNITS - 1); i >= 0; i-- )
	{
		if( i >= GL_MaxTextureUnits( ))
			continue;

		GL_SelectTexture( i );
		pglBindTexture( GL_TEXTURE_2D, 0 );

		if( GL_Support( GL_TEXTURECUBEMAP_EXT ))
			pglBindTexture( GL_TEXTURE_CUBE_MAP_ARB, 0 );
	}

	for( auto &image : r_textures )
	{
		if( !image.texnum ) continue;
		R_FreeImage( &image );
	}

	Q_memset( tr.lightmapTextures, 0, sizeof( tr.lightmapTextures ));
	r_texturesHashTable.clear();
	r_textures.clear();
}
#endif // XASH_DEDICATED
