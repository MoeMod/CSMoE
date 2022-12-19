/*
gl_sprite.c - sprite rendering
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
#include "pm_local.h"
#include "sprite.h"
#include "studio.h"
#include "entity_types.h"
#include "cl_tent.h"
#include "gl_texlru.h"

// it's a Valve default value for LoadMapSprite (probably must be power of two)
#define MAPSPRITE_SIZE	128
#define GLARE_FALLOFF	19000.0f

convar_t		*r_sprite_lerping;
convar_t		*r_sprite_lighting;
char		group_suffix[8];
static vec3_t	sprite_mins, sprite_maxs;
static float	sprite_radius;
static uint	r_texFlags = 0;

/*
====================
R_SpriteInit

====================
*/
void R_SpriteInit( void )
{
	r_sprite_lerping = Cvar_Get( "r_sprite_lerping", "1", CVAR_ARCHIVE, "enables sprite animation lerping" );
	r_sprite_lighting = Cvar_Get( "r_sprite_lighting", "1", CVAR_ARCHIVE, "enables sprite lighting (blood etc)" );
}

/*
====================
R_SpriteLoadFrame_DDS

upload a single frame
====================
*/
#define DDPF_ALPHAPIXELS    0x000001
#define DDPF_ALPHA            0x000002
#define DDPF_FOURCC            0x000004
#define DDPF_RGB            0x000040
#define DDPF_YUV            0x000200
#define DDPF_LUMINANCE        0x020000

#define D3DFMT_DXT1    (('D'<<0)|('X'<<8)|('T'<<16)|('1'<<24))
#define D3DFMT_DXT3    (('D'<<0)|('X'<<8)|('T'<<16)|('3'<<24))
#define D3DFMT_DXT5    (('D'<<0)|('X'<<8)|('T'<<16)|('5'<<24))

#define SIZE_OF_DXT1(width, height)    ( max(1, ( (width + 3) >> 2 ) ) * max(1, ( (height + 3) >> 2 ) ) * 8 )
#define SIZE_OF_DXT2(width, height)    ( max(1, ( (width + 3) >> 2 ) ) * max(1, ( (height + 3) >> 2 ) ) * 16 )


#define DDSD_CAPS            0x000001
#define DDSD_HEIGHT            0x000002
#define DDSD_WIDTH            0x000004
#define DDSD_PITCH            0x000008
#define DDSD_PIXELFORMAT    0x001000
#define DDSD_MIPMAPCOUNT    0x020000
#define DDSD_LINEARSIZE        0x080000
#define DDSD_DEPTH            0x800000

static const byte *R_SpriteLoadFrame_DDS( model_t *mod, const byte *pin, mspriteframe_t **ppframe, int num )
{
	typedef struct
	{
		DWORD    dwSize;
		DWORD    dwFlags;
		DWORD    dwFourCC;
		DWORD    dwRGBBitCount;
		DWORD    dwRBitMask;
		DWORD    dwGBitMask;
		DWORD    dwBBitMask;
		DWORD    dwABitMask;
	} DDS_PIXELFORMAT;

	typedef struct
	{
		DWORD            dwSize;
		DWORD            dwFlags;
		DWORD            dwHeight;
		DWORD            dwWidth;
		DWORD            dwPitchOrLinearSize;
		DWORD            dwDepth;
		DWORD            dwMipMapCount;
		DWORD            dwReserved1[11];
		DDS_PIXELFORMAT    ddspf;
		DWORD            dwCaps;
		DWORD            dwCaps2;
		DWORD            dwCaps3;
		DWORD            dwCaps4;
		DWORD            dwReserved2;
	} DDS_HEADER;

	typedef struct
	{
		DWORD        dwMagic;
		DDS_HEADER    Header;
	} DDS_FILEHEADER;

	DDS_FILEHEADER* header;
	DWORD            compressFormat;
	GLuint            texnum;
	GLvoid* data;
	GLsizei            imageSize;

	header = (DDS_FILEHEADER*)pin;

	if (header->dwMagic != 0x20534444) {
		printf("bad dds file\n");
		return 0;
	}

	if (header->Header.dwSize != 124) {
		printf("bad header size\n");
		return 0;
	}

	if (!(header->Header.dwFlags & DDSD_LINEARSIZE)) {
		printf("bad file type\n");
		return 0;
	}

	if (!(header->Header.ddspf.dwFlags & DDPF_FOURCC)) {
		printf("bad pixel format\n");
		return 0;
	}

	compressFormat = header->Header.ddspf.dwFourCC;

	if (compressFormat != D3DFMT_DXT1 &&
		compressFormat != D3DFMT_DXT3 &&
		compressFormat != D3DFMT_DXT5) {
		printf("bad compress format\n");
		return 0;
	}

	int type = 0;
	switch (compressFormat)
	{
	case D3DFMT_DXT1:
		type = 1;
		imageSize = SIZE_OF_DXT1(header->Header.dwWidth, header->Header.dwHeight);
		break;
	case D3DFMT_DXT3:
		type = 2;
		imageSize = SIZE_OF_DXT2(header->Header.dwWidth, header->Header.dwHeight);
		break;
	case D3DFMT_DXT5:
		type = 3;
		imageSize = SIZE_OF_DXT2(header->Header.dwWidth, header->Header.dwHeight);
		break;
	}


	data = (GLvoid*)(header + 1);    // header data skipped

	////////////////////////////

	mspriteframe_t	*pspriteframe;
	char		texname[128], sprname[128];
	qboolean		load_external = false;
	int		gl_texturenum = 0;

	// build unique frame name
	if( mod->flags & 256 ) // it's a HUD sprite
	{
		Q_snprintf( texname, sizeof( texname ), "#HUD/%s_%s_%i%i.spr", mod->name, group_suffix, num / 10, num % 10 );
		gl_texturenum = GL_LoadTexture( texname, pin, imageSize, r_texFlags, NULL );
	}
	else
	{
		if( !load_external )
		{
			Q_snprintf( texname, sizeof( texname ), "#%s_%s_%i%i.dds", mod->name, group_suffix, num / 10, num % 10 );
			gl_texturenum = GL_LoadTexture_DDS(texname, data, type, imageSize, header->Header.dwWidth, header->Header.dwHeight);
		}
		else MsgDev( D_NOTE, "loading HQ: %s\n", texname );
	}

	// setup frame description
	pspriteframe = (mspriteframe_t *)Mem_ZeroAlloc( mod->mempool, sizeof( mspriteframe_t ));
	pspriteframe->width = header->Header.dwWidth;
	pspriteframe->height = header->Header.dwHeight;
	pspriteframe->up = 0.5 * header->Header.dwHeight;
	pspriteframe->left = -0.5 * header->Header.dwWidth;
	pspriteframe->down = -0.5 * header->Header.dwHeight;
	pspriteframe->right = 0.5 * header->Header.dwWidth;
	pspriteframe->gl_texturenum = gl_texturenum;
	*ppframe = pspriteframe;

	GL_SetTextureType( pspriteframe->gl_texturenum, TEX_SPRITE );

	return ( pin + sizeof(DDS_FILEHEADER) + imageSize);
}

static const byte* R_SpriteLoadFrame(model_t* mod, const dsprite_t *psprite, const byte* pin, mspriteframe_t** ppframe, int num)
{
	const dspriteframe_t*	pinframe;
	mspriteframe_t* pspriteframe;
	char		texname[128], sprname[128];
	qboolean		load_external = false;
	int		gl_texturenum = 0;

    pinframe = reinterpret_cast<const dspriteframe_t *>(pin);

	// build unique frame name
#if XASH_ASTC
    Q_snprintf(texname, sizeof(texname), "ddc/%s/%s_%i%i.astc", mod->name, group_suffix, num / 10, num % 10);
    if (GL_Support(GL_ASTC_EXT) > 0 && FS_FileExists(texname, true)) // it's a HUD sprite
    {
        gl_texturenum = xe::TexLru_LoadTextureExternal(texname, r_texFlags, NULL);
        if(auto tex = R_GetTexture(gl_texturenum)) tex->texType = TEX_SPRITE;
    }
    else
#endif
	if (mod->flags & 256) // it's a HUD sprite
	{
		Q_snprintf(texname, sizeof(texname), "#HUD/%s_%s_%i%i.spr", mod->name, group_suffix, num / 10, num % 10);
		gl_texturenum = xe::TexLru_LoadTextureSPR(texname, psprite, pin, pinframe, r_texFlags, NULL);
	}
	else
	{
        Q_snprintf(texname, sizeof(texname), "#%s_%s_%i%i.spr", mod->name, group_suffix, num / 10, num % 10);
        gl_texturenum = xe::TexLru_LoadTextureSPR(texname, psprite, pin, pinframe, r_texFlags, NULL);
	}

	// setup frame description
	pspriteframe = (mspriteframe_t*)Mem_ZeroAlloc(mod->mempool, sizeof(mspriteframe_t));
	pspriteframe->width = pinframe->width;
	pspriteframe->height = pinframe->height;
	pspriteframe->up = pinframe->origin[1];
	pspriteframe->left = pinframe->origin[0];
	pspriteframe->down = pinframe->origin[1] - pinframe->height;
	pspriteframe->right = pinframe->width + pinframe->origin[0];
	pspriteframe->gl_texturenum = gl_texturenum;
	*ppframe = pspriteframe;

	GL_SetTextureType(pspriteframe->gl_texturenum, TEX_SPRITE);

	return (pin + sizeof(dspriteframe_t) + pinframe->width * pinframe->height);
}


/*
====================
R_SpriteLoadGroup

upload a group frames
====================
*/
static const byte *R_SpriteLoadGroup( model_t *mod, const dsprite_t *psprite, const byte *pin, mspriteframe_t **ppframe, int framenum )
{
	const dspritegroup_t	*pingroup;
	mspritegroup_t	*pspritegroup;
	dspriteinterval_t	pin_intervals;
	float		*poutintervals;
	int		i, groupsize, numframes;

    pingroup = reinterpret_cast<const dspritegroup_t *>(pin);

	groupsize = sizeof( mspritegroup_t ) + (numframes - 1) * sizeof( pspritegroup->frames[0] );
	pspritegroup = (mspritegroup_t * )Mem_ZeroAlloc( mod->mempool, groupsize );
	pspritegroup->numframes = numframes;

	Q_memcpy( ppframe, pspritegroup, sizeof(mspriteframe_t));
	pin += sizeof(dspritegroup_t);
	Q_memcpy( &pin_intervals, pin, sizeof(dspriteinterval_t));

	poutintervals = (float* )Mem_Alloc( mod->mempool, numframes * sizeof( float ));
	pspritegroup->intervals = poutintervals;

	for( i = 0; i < numframes; i++ )
	{
		*poutintervals = LittleFloat(pin_intervals.interval);
		if( *poutintervals <= 0.0f )
			*poutintervals = 1.0f; // set error value
		poutintervals++;
		pin += sizeof(dspriteinterval_t);
	}

	for( i = 0; i < numframes; i++ )
	{
		pin = R_SpriteLoadFrame( mod, psprite, pin, &pspritegroup->frames[i], framenum * 10 + i );
	}

	return pin;
}

/*
====================
Mod_LoadSpriteModel

load sprite model
====================
*/
void Mod_LoadSpriteModel( model_t *mod, const byte *buffer, size_t filesize, qboolean *loaded, uint texFlags )
{
	const dsprite_t *pin;  //spr 头文件
	short		numi;      //record colors
	msprite_t		*psprite;   //记录当前帧的数据块
	int		i, size;
	qboolean is_dds_mode = false;

#if defined(__ANDROID__) || ( TARGET_OS_IOS || TARGET_OS_IPHONE )
	// dont build mipmap on iOS to reduce memory usage
	texFlags |= TF_NOPICMIP|TF_NOMIPMAP;
#endif

	if( loaded ) *loaded = false;
    pin = reinterpret_cast<const dsprite_t *>(buffer);
	mod->type = mod_sprite;
	r_texFlags = texFlags;
	i = LittleLong(pin->version);

	if( LittleLong(pin->ident) != IDSPRITEHEADER )
	{
		MsgDev( D_ERROR, "%s has wrong id (%x should be %x)\n", mod->name, pin->ident, IDSPRITEHEADER );
        FS_MapFree( buffer, filesize );
		return;
	}

	if( i != SPRITE_VERSION  && i != SPRITE_VERSION_DDS)
	{
		MsgDev( D_ERROR, "%s has wrong version number (%i should be %i)\n", mod->name, i, SPRITE_VERSION );
        FS_MapFree( buffer, filesize );
		return;
	}

	mod->mempool = Mem_AllocSubPool( mempool_mdl, va( "^2%s^7", mod->name ));
	size = sizeof( msprite_t ) + ( LittleLong(pin->numframes) - 1 ) * sizeof( psprite->frames );
	psprite = (msprite_t *)Mem_ZeroAlloc( mod->mempool, size );
	mod->cache.data = psprite;	// make link to extradata

	psprite->type = LittleLong(pin->type);
	psprite->texFormat = LittleLong(pin->texFormat);
	psprite->numframes = mod->numframes = LittleLong(pin->numframes);
	psprite->facecull = LittleLong(pin->facetype);
	psprite->radius = LittleLong(pin->boundingradius);
	psprite->synctype = LittleLong(pin->synctype);

	mod->mins[0] = mod->mins[1] = -LittleLong(pin->bounds[0]) / 2;
	mod->maxs[0] = mod->maxs[1] = LittleLong(pin->bounds[0]) / 2;
	mod->mins[2] = -LittleLong(pin->bounds[1]) / 2;
	mod->maxs[2] = LittleLong(pin->bounds[1]) / 2;
	buffer += sizeof(dsprite_t);   //头读取完毕
	Q_memcpy(&numi, buffer, sizeof(short));
	LittleShortSW(numi);

	if( Host_IsDedicated() )
	{
		// skip frames loading
		if( loaded ) *loaded = true;	// done
		psprite->numframes = 0;
        FS_MapFree( buffer, filesize );
		return;
	}
	if (i == SPRITE_VERSION_DDS) {
		is_dds_mode = true;
		psprite->synctype = 233;
		psprite->facecull = SPR_CULL_NONE;
	}
	else {
		if (numi == 256)
		{
			image_ref pal;

			buffer += sizeof(short);

			// install palette
			switch (psprite->texFormat)
			{
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
		else
		{
			MsgDev(D_ERROR, "%s has wrong number of palette colors %i (should be 256)\n", mod->name, numi);
            FS_MapFree( buffer, filesize );
			return;
		}
	}

	if( psprite->numframes < 1 )
	{
		MsgDev( D_ERROR, "%s has invalid # of frames: %d\n", mod->name, pin->numframes );
        FS_MapFree( buffer, filesize );
		return;
	}

	for( i = 0; i < psprite->numframes; i++ ) //依次读取每一帧的数据
	{
		if (is_dds_mode) {
			Q_strncpy(group_suffix, "one", sizeof(group_suffix));
			buffer = R_SpriteLoadFrame_DDS(mod, buffer, &psprite->frames[i].frameptr, i);
		}
		else {
			frametype_t frametype = (frametype_t)*buffer;
			psprite->frames[i].type = (spriteframetype_t)LittleLong(frametype);

			switch (frametype)
			{
			case FRAME_SINGLE:
				Q_strncpy(group_suffix, "one", sizeof(group_suffix));
				buffer = R_SpriteLoadFrame(mod, pin, buffer + sizeof(int), &psprite->frames[i].frameptr, i);
				break;
			case FRAME_GROUP:
				Q_strncpy(group_suffix, "grp", sizeof(group_suffix));
				buffer = R_SpriteLoadGroup(mod, pin, buffer + sizeof(int), &psprite->frames[i].frameptr, i);
				break;
			case FRAME_ANGLED:
				Q_strncpy(group_suffix, "ang", sizeof(group_suffix));
				buffer = R_SpriteLoadGroup(mod, pin, buffer + sizeof(int), &psprite->frames[i].frameptr, i);
				break;
			}
		}
		if( !buffer ) break; // technically an error
	}

	if( loaded ) *loaded = true;	// done

    // keep map buffer in memory
    //FS_MapFree( buffer, filesize );
    mod->buffer = buffer;
    mod->buffer_size = filesize;
}

/*
====================
Mod_LoadMapSprite

Loading a bitmap image as sprite with multiple frames
as pieces of input image
====================
*/
void Mod_LoadMapSprite( model_t *mod, const byte *buffer, size_t size, qboolean *loaded )
{
	byte		*src, *dst;
	char		texname[128];
	int		i, j, x, y, w, h;
	int		xl, yl, xh, yh;
	int		linedelta, numframes;
	mspriteframe_t	*pspriteframe;
	msprite_t		*psprite;
	int texFlags = TF_IMAGE;

	if( cl_sprite_nearest->integer )
		texFlags |= TF_NEAREST;

	if( loaded ) *loaded = false;
	Q_snprintf( texname, sizeof( texname ), "#%s", mod->name );
	host.overview_loading = true;
	auto pix = FS_LoadImage( texname, (byte*)buffer, size );
	host.overview_loading = false;
	if( !pix )
    {
        FS_MapFree( buffer, size );
        return;	// bad image or something else
    }

	mod->type = mod_sprite;
	r_texFlags = 0; // no custom flags for map sprites

	if( pix->width % MAPSPRITE_SIZE )
		w = pix->width - ( pix->width % MAPSPRITE_SIZE );
	else w = pix->width;

	if( pix->height % MAPSPRITE_SIZE )
		h = pix->height - ( pix->height % MAPSPRITE_SIZE );
	else h = pix->height;

	if( w < MAPSPRITE_SIZE ) w = MAPSPRITE_SIZE;
	if( h < MAPSPRITE_SIZE ) h = MAPSPRITE_SIZE;

	// resample image if needed
	Image_Process( &pix, w, h, 0.0f, IMAGE_FORCE_RGBA|IMAGE_RESAMPLE, NULL );

	w = h = MAPSPRITE_SIZE;

	// check range
	if( w > pix->width ) w = pix->width;
	if( h > pix->height ) h = pix->height;

	// determine how many frames we needs
	numframes = (pix->width * pix->height) / (w * h);
	mod->mempool = Mem_AllocSubPool( mempool_mdl, va( "^2%s^7", mod->name ));
	psprite = (msprite_t *)Mem_ZeroAlloc( mod->mempool, sizeof( msprite_t ) + ( numframes - 1 ) * sizeof( psprite->frames ));
	mod->cache.data = psprite;	// make link to extradata

	psprite->type = SPR_FWD_PARALLEL_ORIENTED;
	psprite->texFormat = SPR_ALPHTEST;
	psprite->numframes = mod->numframes = numframes;
	psprite->radius = sqrt((( w >> 1) * (w >> 1)) + ((h >> 1) * (h >> 1)));

	mod->mins[0] = mod->mins[1] = -w / 2;
	mod->maxs[0] = mod->maxs[1] = w / 2;
	mod->mins[2] = -h / 2;
	mod->maxs[2] = h / 2;

	// create a temporary pic
	image_ref temp = Image_NewTemp();
	temp->width = w;
	temp->height = h;
	temp->type = pix->type;
	temp->flags = pix->flags;	
	temp->size = w * h * PFDesc[temp->type].bpp;
	temp->buffer = (byte*)Mem_Alloc( r_temppool, temp->size );
	temp->palette = NULL;

	// chop the image and upload into video memory
	for( i = xl = yl = 0; i < numframes; i++ )
	{
		xh = xl + w;
		yh = yl + h;

		src = pix->buffer + ( yl * pix->width + xl ) * 4;
		linedelta = ( pix->width - w ) * 4;
		dst = temp->buffer;

		// cut block from source
		for( y = yl; y < yh; y++ )
		{
			for( x = xl; x < xh; x++ )
				for( j = 0; j < 4; j++ )
					*dst++ = *src++;
			src += linedelta;
		}

		// build uinque frame name
		Q_snprintf( texname, sizeof( texname ), "#MAP/%s_%i%i.spr", mod->name, i / 10, i % 10 );

		psprite->frames[i].frameptr = (mspriteframe_t*)Mem_ZeroAlloc( mod->mempool, sizeof( mspriteframe_t ));
		pspriteframe = psprite->frames[i].frameptr;
		pspriteframe->width = w;
		pspriteframe->height = h;
		pspriteframe->up = ( h >> 1 );
		pspriteframe->left = -( w >> 1 );
		pspriteframe->down = ( h >> 1 ) - h;
		pspriteframe->right = w + -( w >> 1 );
		pspriteframe->gl_texturenum = GL_LoadTextureInternal( texname, temp, (texFlags_t)texFlags, false );
		GL_SetTextureType( pspriteframe->gl_texturenum, TEX_NOMIP );
			
		xl += w;
		if( xl >= pix->width )
		{
			xl = 0;
			yl += h;
		}
	}

	if( loaded ) *loaded = true;
    FS_MapFree( buffer, size );
}

/*
====================
Mod_UnloadSpriteModel

release sprite model and frames
====================
*/
void Mod_UnloadSpriteModel( model_t *mod )
{
	msprite_t		*psprite;
	mspritegroup_t	*pspritegroup;	
	mspriteframe_t	*pspriteframe;
	int		i, j;

	ASSERT( mod != NULL );

	if( mod->type != mod_sprite )
		return; // not a sprite

	psprite = (msprite_t *)mod->cache.data;
	if( !psprite ) return; // already freed

	// release all textures
	for( i = 0; i < psprite->numframes; i++ )
	{
		if( Host_IsDedicated() )
			break; // nothing to release

		if( psprite->frames[i].type == SPR_SINGLE )
		{
			pspriteframe = psprite->frames[i].frameptr;
			GL_FreeTexture( pspriteframe->gl_texturenum );
		}
		else
		{
			pspritegroup = (mspritegroup_t *)psprite->frames[i].frameptr;

			for( j = 0; j < pspritegroup->numframes; j++ )
			{
				pspriteframe = pspritegroup->frames[i];
				GL_FreeTexture( pspriteframe->gl_texturenum );
			}
		}
	}

	Mem_FreePool( &mod->mempool );
    FS_MapFree((const byte *)mod->buffer, mod->buffer_size);
	Q_memset( mod, 0, sizeof( *mod ));
}

/*
================
R_GetSpriteFrame

assume pModel is valid
================
*/
mspriteframe_t *R_GetSpriteFrame( const model_t *pModel, int frame, float yaw )
{
	msprite_t		*psprite;
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe = NULL;
	float		*pintervals, fullinterval;
	float		targettime, time;
	int		i, numframes;

	ASSERT( pModel );
	psprite = (msprite_t *)pModel->cache.data;

	if( frame < 0 ) frame = 0;
	else if( frame >= psprite->numframes )
	{
		MsgDev( D_WARN, "R_GetSpriteFrame: no such frame %d (%s)\n", frame, pModel->name );
		frame = psprite->numframes - 1;
	}

	if( psprite->frames[frame].type == SPR_SINGLE )
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else if( psprite->frames[frame].type == SPR_GROUP ) 
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];
		time = cl.time;

		// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
		// are positive, so we don't have to worry about division by zero
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for( i = 0; i < (numframes - 1); i++ )
		{
			if( pintervals[i] > targettime )
				break;
		}
		pspriteframe = pspritegroup->frames[i];
	}
	else if( psprite->frames[frame].type == FRAME_ANGLED ) //-V556
	{
		int	angleframe = (int)(Q_rint(( RI.refdef.viewangles[1] - yaw + 45.0f ) / 360 * 8) - 4) & 7;

		// e.g. doom-style sprite monsters
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pspriteframe = pspritegroup->frames[angleframe];
	}

	return pspriteframe;
}

/*
================
R_GetSpriteFrameInterpolant

NOTE: we using prevblending[0] and [1] for holds interval
between frames where are we lerping
================
*/
float R_GetSpriteFrameInterpolant( cl_entity_t *ent, mspriteframe_t **oldframe, mspriteframe_t **curframe )
{
	msprite_t		*psprite;
	mspritegroup_t	*pspritegroup;
	int		i, j, numframes, frame;
	float		lerpFrac, time, jtime, jinterval;
	float		*pintervals, fullinterval, targettime;
	int		m_fDoInterp;

	psprite = (msprite_t *)ent->model->cache.data;
	frame = (int)ent->curstate.frame;
	lerpFrac = 1.0f;

	// misc info
	if( r_sprite_lerping->integer )
		m_fDoInterp = (ent->curstate.effects & EF_NOINTERP) ? false : true;
	else m_fDoInterp = false;

	if( frame < 0 )
	{
		frame = 0;
	}
	else if( frame >= psprite->numframes )
	{
		MsgDev( D_WARN, "R_GetSpriteFrameInterpolant: no such frame %d (%s)\n", frame, ent->model->name );
		frame = psprite->numframes - 1;
	}

	if( psprite->frames[frame].type == FRAME_SINGLE ) //-V556
	{
		if( m_fDoInterp )
		{
			if( ent->latched.prevblending[0] >= psprite->numframes || psprite->frames[ent->latched.prevblending[0]].type != FRAME_SINGLE ) //-V556
			{
				// this can be happens when rendering switched between single and angled frames
				// or change model on replace delta-entity
				ent->latched.prevblending[0] = ent->latched.prevblending[1] = frame;
				ent->latched.prevanimtime = RI.refdef.time;
				lerpFrac = 1.0f;
			}

			if( ent->latched.prevanimtime < RI.refdef.time )
			{
				if( frame != ent->latched.prevblending[1] )
				{
					ent->latched.prevblending[0] = ent->latched.prevblending[1];
					ent->latched.prevblending[1] = frame;
					ent->latched.prevanimtime = RI.refdef.time;
					lerpFrac = 0.0f;
				}
				else lerpFrac = (RI.refdef.time - ent->latched.prevanimtime) * 10;
			}
			else
			{
				ent->latched.prevblending[0] = ent->latched.prevblending[1] = frame;
				ent->latched.prevanimtime = RI.refdef.time;
				lerpFrac = 0.0f;
			}
		}
		else
		{
			ent->latched.prevblending[0] = ent->latched.prevblending[1] = frame;
			lerpFrac = 1.0f;
		}

		if( ent->latched.prevblending[0] >= psprite->numframes )
		{
			// reset interpolation on change model
			ent->latched.prevblending[0] = ent->latched.prevblending[1] = frame;
			ent->latched.prevanimtime = RI.refdef.time;
			lerpFrac = 0.0f;
		}

		// get the interpolated frames
		if( oldframe ) *oldframe = psprite->frames[ent->latched.prevblending[0]].frameptr;
		if( curframe ) *curframe = psprite->frames[frame].frameptr;
	}
	else if( psprite->frames[frame].type == FRAME_GROUP )  //-V556
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];
		jinterval = pintervals[1] - pintervals[0];
		time = RI.refdef.time;
		jtime = 0.0f;

		// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
		// are positive, so we don't have to worry about division by zero
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		// LordHavoc: since I can't measure the time properly when it loops from numframes - 1 to 0,
		// i instead measure the time of the first frame, hoping it is consistent
		for( i = 0, j = numframes - 1; i < (numframes - 1); i++ )
		{
			if( pintervals[i] > targettime )
				break;
			j = i;
			jinterval = pintervals[i] - jtime;
			jtime = pintervals[i];
		}

		if( m_fDoInterp )
			lerpFrac = (targettime - jtime) / jinterval;
		else j = i; // no lerping

		// get the interpolated frames
		if( oldframe ) *oldframe = pspritegroup->frames[j];
		if( curframe ) *curframe = pspritegroup->frames[i];
	}
	else if( psprite->frames[frame].type == FRAME_ANGLED ) //-V556
	{
		// e.g. doom-style sprite monsters
		float	yaw = ent->angles[YAW];
		int	angleframe = (int)(Q_rint(( RI.refdef.viewangles[1] - yaw + 45.0f ) / 360 * 8) - 4) & 7;

		if( m_fDoInterp )
		{
			if( ent->latched.prevblending[0] >= psprite->numframes || psprite->frames[ent->latched.prevblending[0]].type != FRAME_ANGLED ) //-V556
			{
				// this can be happens when rendering switched between single and angled frames
				// or change model on replace delta-entity
				ent->latched.prevblending[0] = ent->latched.prevblending[1] = frame;
				ent->latched.prevanimtime = RI.refdef.time;
				lerpFrac = 1.0f;
			}

			if( ent->latched.prevanimtime < RI.refdef.time )
			{
				if( frame != ent->latched.prevblending[1] )
				{
					ent->latched.prevblending[0] = ent->latched.prevblending[1];
					ent->latched.prevblending[1] = frame;
					ent->latched.prevanimtime = RI.refdef.time;
					lerpFrac = 0.0f;
				}
				else lerpFrac = (RI.refdef.time - ent->latched.prevanimtime) * ent->curstate.framerate;
			}
			else
			{
				ent->latched.prevblending[0] = ent->latched.prevblending[1] = frame;
				ent->latched.prevanimtime = RI.refdef.time;
				lerpFrac = 0.0f;
			}
		}
		else
		{
			ent->latched.prevblending[0] = ent->latched.prevblending[1] = frame;
			lerpFrac = 1.0f;
		}

		pspritegroup = (mspritegroup_t *)psprite->frames[ent->latched.prevblending[0]].frameptr;
		if( oldframe ) *oldframe = pspritegroup->frames[angleframe];

		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		if( curframe ) *curframe = pspritegroup->frames[angleframe];
	}

	return lerpFrac;
}

/*
================
R_StudioComputeBBox

Compute a full bounding box for current sequence
================
*/
qboolean R_SpriteComputeBBox( cl_entity_t *e, vec3_t bbox[8] )
{
	float	scale = 1.0f;
	vec3_t	p1;
	int	i;

	// copy original bbox (no rotation for sprites)
	VectorCopy( e->model->mins, sprite_mins );
	VectorCopy( e->model->maxs, sprite_maxs );

	// compute a full bounding box
	for( i = 0; bbox && i < 8; i++ )
	{
  		p1[0] = ( i & 1 ) ? sprite_mins[0] : sprite_maxs[0];
  		p1[1] = ( i & 2 ) ? sprite_mins[1] : sprite_maxs[1];
  		p1[2] = ( i & 4 ) ? sprite_mins[2] : sprite_maxs[2];

		VectorCopy( p1, bbox[i] );
	}

	if( e->curstate.scale > 0.0f )
		scale = e->curstate.scale;

	sprite_radius = RadiusFromBounds( sprite_mins, sprite_maxs ) * scale;

	return true;
}

/*
================
R_CullSpriteModel

Cull sprite model by bbox
================
*/
qboolean R_CullSpriteModel( cl_entity_t *e, vec3_t origin )
{
	if( !e->model->cache.data )
		return true;

	if( tr.fFlipViewModel )
		return true;

	if( !R_SpriteComputeBBox( e, NULL ))
		return true; // invalid frame

	return R_CullModel( e, origin, sprite_mins, sprite_maxs, sprite_radius );
}

/*
================
R_GlowSightDistance

Calc sight distance for glow-sprites
================
*/
static float R_GlowSightDistance( vec3_t glowOrigin )
{
	float	dist;
	vec3_t	glowDist;
	pmtrace_t	tr;

	VectorSubtract( glowOrigin, RI.vieworg, glowDist );
	dist = VectorLength( glowDist );

	if( RP_NORMALPASS( ))
	{
		tr = CL_TraceLine( RI.vieworg, glowOrigin, PM_GLASS_IGNORE|PM_STUDIO_IGNORE );

		if(( 1.0f - tr.fraction ) * dist > 8.0f )
			return -1;
	}
	return dist;
}

/*
================
R_GlowSightDistance

Set sprite brightness factor
================
*/
static float R_SpriteGlowBlend( vec3_t origin, int rendermode, int renderfx, int alpha, float *pscale )
{
	float	dist = R_GlowSightDistance( origin );
	float	brightness;

	if( dist <= 0.0f ) return 0.0f; // occluded

	if( renderfx == kRenderFxNoDissipation )
		return (float)alpha * (1.0f / 255.0f);

	*pscale = 0.0f; // variable sized glow

	brightness = GLARE_FALLOFF / ( dist * dist );
	brightness = bound( 0.01f, brightness, 1.0f );

	if( rendermode != kRenderWorldGlow )
	{
		// make the glow fixed size in screen space, taking into consideration the scale setting.
		if( *pscale == 0.0f ) *pscale = 1.0f;
		*pscale *= dist * ( 1.0f / bound( 100.0f, r_flaresize->value, 300.0f ));
	}

	return brightness;
}

/*
================
R_SpriteOccluded

Do occlusion test for glow-sprites
================
*/
qboolean R_SpriteOccluded( cl_entity_t *e, vec3_t origin, int *alpha, float *pscale )
{
	if( e->curstate.rendermode == kRenderGlow || e->curstate.rendermode == kRenderWorldGlow )
	{
		float	blend = 1.0f;
		vec3_t	v;

		// don't reflect this entity in mirrors
		if( e->curstate.effects & EF_NOREFLECT && RI.params & RP_MIRRORVIEW )
			return true;

		// draw only in mirrors
		if( e->curstate.effects & EF_REFLECTONLY && !( RI.params & RP_MIRRORVIEW ))
			return true;

		TriWorldToScreen( origin, v );

		if( v[0] < RI.refdef.viewport[0] || v[0] > RI.refdef.viewport[0] + RI.refdef.viewport[2] )
			return true; // do scissor
		if( v[1] < RI.refdef.viewport[1] || v[1] > RI.refdef.viewport[1] + RI.refdef.viewport[3] )
			return true; // do scissor

		blend *= R_SpriteGlowBlend( origin, e->curstate.rendermode, e->curstate.renderfx, *alpha, pscale );
		*alpha *= blend;

		if( blend <= 0.01f )
			return true; // faded
	}
	else
	{
		if( R_CullSpriteModel( e, origin ))
			return true;
	}
	return false;	
}

/*
=================
R_DrawSpriteQuad
=================
*/
static void R_DrawSpriteQuad( mspriteframe_t *frame, vec3_t org, vec3_t v_right, vec3_t v_up, float scale )
{
	vec3_t	point;

	r_stats.c_sprite_polys++;

	pglBegin( GL_QUADS );
		pglTexCoord2f( 0.0f, 1.0f );
		VectorMA( org, frame->down * scale, v_up, point );
		VectorMA( point, frame->left * scale, v_right, point );
		pglVertex3fv( point.data() );
		pglTexCoord2f( 0.0f, 0.0f );
		VectorMA( org, frame->up * scale, v_up, point );
		VectorMA( point, frame->left * scale, v_right, point );
		pglVertex3fv( point.data() );
		pglTexCoord2f( 1.0f, 0.0f );
		VectorMA( org, frame->up * scale, v_up, point );
		VectorMA( point, frame->right * scale, v_right, point );
		pglVertex3fv( point.data() );
		pglTexCoord2f( 1.0f, 1.0f );
		VectorMA( org, frame->down * scale, v_up, point );
		VectorMA( point, frame->right * scale, v_right, point );
		pglVertex3fv( point.data() );
	pglEnd();
}

static qboolean R_SpriteHasLightmap( cl_entity_t *e, int texFormat )
{
	if( !r_sprite_lighting->integer )
		return false;
	
	if( texFormat != SPR_ALPHTEST )
		return false;

	if( e->curstate.effects & EF_FULLBRIGHT )
		return false;

	if( e->curstate.renderamt <= 127 )
		return false;

	switch( e->curstate.rendermode )
	{
	case kRenderNormal:
	case kRenderTransAlpha:
	case kRenderTransTexture:
		break;
	default:
		return false;
	}

	return true;
}

/*
=================
R_DrawSpriteModel
=================
*/
void R_DrawSpriteModel( cl_entity_t *e )
{
	mspriteframe_t	*frame, *oldframe;
	msprite_t		*psprite;
	model_t		*model;
	int		i, alpha, type;
	float		angle, dot, sr, cr, flAlpha;
	vec3_t		v_forward, v_right, v_up;
	vec3_t		origin, color, color2 = {};

	if( RI.params & RP_ENVVIEW )
		return;

	model = e->model;
	psprite = (msprite_t * )model->cache.data;
	VectorCopy( e->origin, origin );	// set render origin

	// do movewith
	if( e->curstate.aiment > 0 && (e->curstate.movetype == MOVETYPE_FOLLOW || e->curstate.movetype == MOVETYPE_FOLLOWMOVE))
	{
		cl_entity_t	*parent;
	
		parent = CL_GetEntityByIndex( e->curstate.aiment );

		if( parent && parent->model )
		{
			if( parent->model->type == mod_studio && e->curstate.body > 0 )
			{
				if (e->curstate.movetype == MOVETYPE_FOLLOWMOVE)
				{
					int num = bound(1, e->curstate.body, MAXSTUDIOATTACHMENTS);
					VectorLerp(parent->attachment[num - 1], e->curstate.frame, parent->attachment[num], origin);
				}
				else
				{
					int num = bound(1, e->curstate.body, MAXSTUDIOATTACHMENTS);  //gr3
					VectorCopy(parent->attachment[num - 1], origin);
				}
			}
			else VectorCopy( parent->origin, origin );
		}
	}

    float scale;

	alpha = e->curstate.renderamt;
	scale = e->curstate.scale;

	if( R_SpriteOccluded( e, origin, &alpha, &scale ))
		return; // sprite culled

	r_stats.c_sprite_models_drawn++;

	if( psprite->texFormat == SPR_ALPHTEST && e->curstate.rendermode != kRenderTransAdd )
	{
		pglEnable( GL_ALPHA_TEST );
		pglAlphaFunc( GL_GREATER, 0.0f );
	}

	if( e->curstate.rendermode == kRenderGlow || e->curstate.rendermode == kRenderWorldGlow )
		pglDisable( GL_DEPTH_TEST );

	// select properly rendermode
	switch( e->curstate.rendermode )
	{
	case kRenderTransAlpha:
	case kRenderTransColor:
	case kRenderTransTexture:
		pglEnable( GL_BLEND );
		pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		break;
	case kRenderGlow:
	case kRenderTransAdd:
	case kRenderWorldGlow:
		pglDisable( GL_FOG );
		pglEnable( GL_BLEND );
		pglBlendFunc( GL_SRC_ALPHA, GL_ONE );
		break;
	case kRenderNormal:
	default:
		if( psprite->texFormat == SPR_INDEXALPHA )
		{
			pglEnable( GL_BLEND );
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
		else pglDisable( GL_BLEND );
		break;
	}

	// all sprites can have color
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// add basecolor (any rendermode can colored sprite)
	color[0] = (float)e->curstate.rendercolor.r * ( 1.0f / 255.0f );
	color[1] = (float)e->curstate.rendercolor.g * ( 1.0f / 255.0f );
	color[2] = (float)e->curstate.rendercolor.b * ( 1.0f / 255.0f );

	if( R_SpriteHasLightmap( e, psprite->texFormat ))
	{
		color24	lightColor;
		qboolean	invLight;

		invLight = (e->curstate.effects & EF_INVLIGHT) ? true : false;
		R_LightForPoint( origin, &lightColor, invLight, true, sprite_radius );
		color2[0] = (float)lightColor.r * ( 1.0f / 255.0f );
		color2[1] = (float)lightColor.g * ( 1.0f / 255.0f );
		color2[2] = (float)lightColor.b * ( 1.0f / 255.0f );

		if( glState.drawTrans )
			pglDepthMask( GL_TRUE );

		// NOTE: sprites with 'lightmap' looks ugly when alpha func is GL_GREATER 0.0
		pglAlphaFunc( GL_GEQUAL, 0.5f );
	}

    float lerp = 1.0f, ilerp;

	if( e->curstate.rendermode == kRenderNormal || e->curstate.rendermode == kRenderTransAlpha || e->curstate.rendermode == kRenderTransAdd)
		frame = oldframe = R_GetSpriteFrame( model, e->curstate.frame, e->angles[YAW] );
	else lerp = R_GetSpriteFrameInterpolant( e, &oldframe, &frame );

	type = psprite->type;

	// automatically roll parallel sprites if requested
	if( e->angles[ROLL] != 0.0f && type == SPR_FWD_PARALLEL )
		type = SPR_FWD_PARALLEL_ORIENTED;

	switch( type )
	{
	case SPR_ORIENTED:
		AngleVectors( e->angles, v_forward, v_right, v_up );
		VectorScale( v_forward, 0.01f, v_forward );	// to avoid z-fighting
		VectorSubtract( origin, v_forward, origin );
		break;
	case SPR_FACING_UPRIGHT:
		VectorSet( v_right, origin[1] - RI.vieworg[1], -(origin[0] - RI.vieworg[0]), 0.0f );
		VectorSet( v_up, 0.0f, 0.0f, 1.0f );
		VectorNormalize( v_right );
		break;
	case SPR_FWD_PARALLEL_UPRIGHT:
		dot = RI.vforward[2];
		if(( dot > 0.999848f ) || ( dot < -0.999848f ))	// cos(1 degree) = 0.999848
			return; // invisible
		VectorSet( v_up, 0.0f, 0.0f, 1.0f );
		VectorSet( v_right, RI.vforward[1], -RI.vforward[0], 0.0f );
		VectorNormalize( v_right );
		break;
	case SPR_FWD_PARALLEL_ORIENTED:
		angle = e->angles[ROLL] * (M_PI2 / 360.0f);
		SinCos( angle, &sr, &cr );
        v_right = RI.vright * cr + RI.vup * sr;
        v_up = RI.vright * -sr + RI.vup * cr;
		break;
	case SPR_FWD_PARALLEL: // normal sprite
	default:
		VectorCopy( RI.vright, v_right ); 
		VectorCopy( RI.vup, v_up );
		break;
	}

	flAlpha = (float)alpha * ( 1.0f / 255.0f );

	if( psprite->facecull == SPR_CULL_NONE )
		GL_Cull( GL_NONE );
		
	if (psprite->synctype == 233)
	{
		pglEnable(GL_TEXTURE_2D);
		xe::TexLru_Bind(frame);

		pglEnable(GL_BLEND);
		pglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//pglBlendFunc(GL_ONE, GL_ONE);
		pglDepthMask(GL_FALSE);
	}

	if( oldframe == frame )
	{
		// draw the single non-lerped frame
		pglColor4f(color[0], color[1], color[2], flAlpha);
		if(psprite->synctype != 233) xe::TexLru_Bind(frame);
		R_DrawSpriteQuad(frame, origin, v_right, v_up, scale);
	}
	else
	{
		// draw two combined lerped frames
		lerp = bound( 0.0f, lerp, 1.0f );
		ilerp = 1.0f - lerp;

		if (ilerp != 0.0f)
		{
			pglColor4f(color[0], color[1], color[2], flAlpha * ilerp);
			if (psprite->synctype != 233) xe::TexLru_Bind(oldframe);
			R_DrawSpriteQuad(oldframe, origin, v_right, v_up, scale);
		}
		if (lerp != 0.0f)
		{
			pglColor4f(color[0], color[1], color[2], flAlpha * lerp);
			if (psprite->synctype != 233) xe::TexLru_Bind(frame);
			R_DrawSpriteQuad(frame, origin, v_right, v_up, scale);
		}
	}

	// draw the sprite 'lightmap' :-)
	if( R_SpriteHasLightmap( e, psprite->texFormat ))
	{
		pglEnable( GL_BLEND );
		pglDepthFunc( GL_EQUAL );
		pglDisable( GL_ALPHA_TEST );
		pglBlendFunc( GL_ZERO, GL_SRC_COLOR );
		pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		pglColor4f( color2[0], color2[1], color2[2], flAlpha );
		GL_Bind( XASH_TEXTURE0, tr.whiteTexture );
		R_DrawSpriteQuad( frame, origin, v_right, v_up, scale );

		if( glState.drawTrans ) 
			pglDepthMask( GL_FALSE );
	}

	if( psprite->facecull == SPR_CULL_NONE )
		GL_Cull( GL_FRONT );

	if( e->curstate.rendermode == kRenderGlow || e->curstate.rendermode == kRenderWorldGlow )
		pglEnable( GL_DEPTH_TEST );

	if( psprite->texFormat == SPR_ALPHTEST && e->curstate.rendermode != kRenderTransAdd )
		pglDisable( GL_ALPHA_TEST );

	pglDisable( GL_BLEND );
	pglDepthFunc( GL_LEQUAL );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	pglColor4ub( 255, 255, 255, 255 );

	if( RI.fogCustom || ( RI.fogEnabled && !glState.drawTrans ))
		pglEnable( GL_FOG );
}
#endif // XASH_DEDICATED
