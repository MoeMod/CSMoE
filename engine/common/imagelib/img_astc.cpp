/*
img_astc.cpp - astc format load
Copyright (C) 2022 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "imagelib.h"
#include "astcenc.h"

#ifndef XASH_DEDICATED
#include "gl_local.h"
#endif

#include <vector>

static const unsigned int thread_count_r = 1;
static const unsigned int thread_count_w = 1;
static const astcenc_profile profile = ASTCENC_PRF_LDR;
static const float quality = ASTCENC_PRE_MEDIUM;
static const astcenc_swizzle swizzle {
		ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A
};

/* ============================================================================
	ASTC compressed file loading
============================================================================ */
struct astc_header
{
	uint8_t magic[4];
	uint8_t block_x;
	uint8_t block_y;
	uint8_t block_z;
	uint8_t dim_x[3];			// dims = dim[0] + (dim[1] << 8) + (dim[2] << 16)
	uint8_t dim_y[3];			// Sizes are given in texels;
	uint8_t dim_z[3];			// block count is inferred
};

static const uint32_t ASTC_MAGIC_ID = 0x5CA1AB13;

static unsigned int unpack_bytes(
		uint8_t a,
		uint8_t b,
		uint8_t c,
		uint8_t d
) {
	return ((unsigned int)(a))       +
	       ((unsigned int)(b) << 8)  +
	       ((unsigned int)(c) << 16) +
	       ((unsigned int)(d) << 24);
}

static astcenc_image *alloc_image(
		unsigned int bitness,
		unsigned int dim_x,
		unsigned int dim_y,
		unsigned int dim_z
) {
	astcenc_image *img = new astcenc_image;
	img->dim_x = dim_x;
	img->dim_y = dim_y;
	img->dim_z = dim_z;

	if (bitness == 8)
	{
		void** data = new void*[dim_z];
		img->data_type = ASTCENC_TYPE_U8;
		img->data = data;

		for (unsigned int z = 0; z < dim_z; z++)
		{
			data[z] = new uint8_t[dim_x * dim_y * 4];
		}
	}
	else if (bitness == 16)
	{
		void** data = new void*[dim_z];
		img->data_type = ASTCENC_TYPE_F16;
		img->data = data;

		for (unsigned int z = 0; z < dim_z; z++)
		{
			data[z] = new uint16_t[dim_x * dim_y * 4];
		}
	}
	else if (bitness == 32)
	{
		void** data = new void*[dim_z];
		img->data_type = ASTCENC_TYPE_F32;
		img->data = data;

		for (unsigned int z = 0; z < dim_z; z++)
		{
			data[z] = new float[dim_x * dim_y * 4];
		}
	}

	return img;
}

/* See header for documentation. */
static void free_image(astcenc_image * img)
{
	if (img == nullptr)
	{
		return;
	}

	for (unsigned int z = 0; z < img->dim_z; z++)
	{
		delete[] (char*)img->data[z];
	}

	delete[] img->data;
	delete img;
}

/*
=============
Image_LoadTGA
=============
*/
qboolean Image_LoadASTC_SW( const char *name, const byte *buffer, size_t filesize )
{
	if(filesize <= sizeof(astc_header))
	{
		MsgDev( D_WARN, "Image_LoadASTC: EmptyFile: %s\n", name);
		return false;
	}
	const astc_header &hdr = *reinterpret_cast<const astc_header *>(buffer);


	unsigned int magicval = unpack_bytes(hdr.magic[0], hdr.magic[1], hdr.magic[2], hdr.magic[3]);
	if (magicval != ASTC_MAGIC_ID)
	{
		MsgDev( D_WARN, "Image_LoadASTC: File not recognized '%s'\n", name);
		return false;
	}

	// Ensure these are not zero to avoid div by zero
	unsigned int block_x = max((unsigned int)hdr.block_x, 1u);
	unsigned int block_y = max((unsigned int)hdr.block_y, 1u);
	unsigned int block_z = max((unsigned int)hdr.block_z, 1u);

	unsigned int dim_x = unpack_bytes(hdr.dim_x[0], hdr.dim_x[1], hdr.dim_x[2], 0);
	unsigned int dim_y = unpack_bytes(hdr.dim_y[0], hdr.dim_y[1], hdr.dim_y[2], 0);
	unsigned int dim_z = unpack_bytes(hdr.dim_z[0], hdr.dim_z[1], hdr.dim_z[2], 0);

	if (dim_x == 0 || dim_y == 0 || dim_z == 0)
	{
		MsgDev( D_WARN, "File corrupt '%s'\n", name);
		return false;
	}

	unsigned int xblocks = (dim_x + block_x - 1) / block_x;
	unsigned int yblocks = (dim_y + block_y - 1) / block_y;
	unsigned int zblocks = (dim_z + block_z - 1) / block_z;

	size_t data_size = xblocks * yblocks * zblocks * 16;

	astcenc_error status;

	// Initialize the default configuration for the block size and quality
	astcenc_config config;
	config.block_x = block_x;
	config.block_y = block_y;
	config.profile = profile;

	status = astcenc_config_init(profile, block_x, block_y, block_z, quality, 0, &config);
	if (status != ASTCENC_SUCCESS)
	{
		MsgDev( D_WARN, "Image_LoadASTC: Codec config init failed: %s\n", astcenc_get_error_string(status));
		return false;
	}

	astcenc_context* context;
	status = astcenc_context_alloc(&config, thread_count_r, &context);
	if (status != ASTCENC_SUCCESS)
	{
		MsgDev( D_WARN, "Image_LoadASTC: Codec context alloc failed: %s\n", astcenc_get_error_string(status));
		return false;
	}

	astcenc_image *image_decomp_out = alloc_image(8, dim_x, dim_y, dim_z);

	status = astcenc_decompress_image(context, (uint8_t*)buffer, filesize, image_decomp_out, &swizzle, 0);
	if (status != ASTCENC_SUCCESS)
	{
		MsgDev( D_WARN, "Image_LoadASTC: Codec decompress failed: %s\n", astcenc_get_error_string(status));
		return false;
	}

	image.width = image_decomp_out->dim_x;
	image.height = image_decomp_out->dim_y;
	image.type = PF_RGBA_32;
	image.size = image.width * image.height * 4;
	image.rgba = (byte *)Mem_Alloc( host.imagepool, image.size );
	Q_memcpy(image.rgba, image_decomp_out->data[0], image.size);
	image.flags |= IMAGE_HAS_ALPHA;
	image.flags |= IMAGE_HAS_COLOR;

	free_image(image_decomp_out);
	astcenc_context_free(context);
	return true;
}

qboolean Image_LoadASTC_HW( const char *name, const byte *buffer, size_t filesize )
{
    if(filesize <= sizeof(astc_header))
    {
        MsgDev( D_WARN, "Image_LoadASTC: EmptyFile: %s\n", name);
        return false;
    }
    const astc_header &hdr = *reinterpret_cast<const astc_header *>(buffer);

    unsigned int magicval = unpack_bytes(hdr.magic[0], hdr.magic[1], hdr.magic[2], hdr.magic[3]);
    if (magicval != ASTC_MAGIC_ID)
    {
        MsgDev( D_WARN, "Image_LoadASTC_HW: File not recognized '%s'\n", name);
        return false;
    }

    // Ensure these are not zero to avoid div by zero
    unsigned int block_x = max((unsigned int)hdr.block_x, 1u);
    unsigned int block_y = max((unsigned int)hdr.block_y, 1u);
    unsigned int block_z = max((unsigned int)hdr.block_z, 1u);

    unsigned int dim_x = unpack_bytes(hdr.dim_x[0], hdr.dim_x[1], hdr.dim_x[2], 0);
    unsigned int dim_y = unpack_bytes(hdr.dim_y[0], hdr.dim_y[1], hdr.dim_y[2], 0);
    unsigned int dim_z = unpack_bytes(hdr.dim_z[0], hdr.dim_z[1], hdr.dim_z[2], 0);

    if (dim_x == 0 || dim_y == 0 || dim_z == 0)
    {
        MsgDev( D_WARN, "Image_LoadASTC_HW: File corrupt '%s'\n", name);
        return false;
    }

    unsigned int xblocks = (dim_x + block_x - 1) / block_x;
    unsigned int yblocks = (dim_y + block_y - 1) / block_y;
    unsigned int zblocks = (dim_z + block_z - 1) / block_z;

    size_t data_size = xblocks * yblocks * zblocks * 16;

    if(filesize < sizeof(astc_header) + data_size)
    {
        MsgDev( D_WARN, "Image_LoadASTC_HW: Incomplete file '%s'\n", name);
        return false;
    }

    pixformat_t format = PF_UNKNOWN;
    if(block_x == 4 && block_y == 4)
        format = PF_ASTC_4x4;
    if(block_x == 6 && block_y == 6)
        format = PF_ASTC_6x6;
    if(block_x == 8 && block_y == 8)
        format = PF_ASTC_8x8;
    if(block_x == 10 && block_y == 10)
        format = PF_ASTC_10x10;
    if(block_x == 12 && block_y == 12)
        format = PF_ASTC_12x12;

    if (format == PF_UNKNOWN)
    {
        MsgDev( D_WARN, "Image_LoadASTC_HW: Invalid format '%s'\n", name);
        return false;
    }

    image.width = dim_x;
    image.height = dim_y;
    image.type = format;
    image.size = data_size;
    image.rgba = (byte *)Mem_Alloc( host.imagepool, image.size );
    Q_memcpy(image.rgba, buffer + sizeof(astc_header), image.size);
    image.flags |= IMAGE_HAS_ALPHA;
    image.flags |= IMAGE_HAS_COLOR;

    return true;
}

qboolean Image_LoadASTC( const char *name, const byte *buffer, size_t filesize )
{
#ifndef XASH_DEDICATED
    if(GL_Support(GL_ASTC_EXT) > 0)
        return Image_LoadASTC_HW(name, buffer, filesize);
    else
#endif
        return Image_LoadASTC_SW(name, buffer, filesize);
}

static const unsigned int save_block_x = 8;
static const unsigned int save_block_y = 8;
static const unsigned int save_block_z = 1;

astcenc_context *Image_SaveASTC_CreateContext(int thread_num)
{
    astcenc_error status;
    // Initialize the default configuration for the block size and quality
    astcenc_config config;
    config.block_x = save_block_x;
    config.block_y = save_block_y;
    config.profile = profile;

    status = astcenc_config_init(profile, save_block_x, save_block_y, save_block_z, quality, 0, &config);
    if (status != ASTCENC_SUCCESS)
    {
        MsgDev( D_WARN, "Image_SaveASTC: Codec config init failed: %s\n", astcenc_get_error_string(status));
        return nullptr;
    }

    astcenc_context* context;
    status = astcenc_context_alloc(&config, thread_count_w, &context);
    if (status != ASTCENC_SUCCESS)
    {
        MsgDev( D_WARN, "Image_SaveASTC: Codec context alloc failed: %s\n", astcenc_get_error_string(status));
        return nullptr;
    }
    return context;
}

void Image_SaveASTC_DestroyContext(astcenc_context *context)
{
    astcenc_context_free(context);
}

std::vector<byte> Image_SaveASTC_Worker(astcenc_context* context, byte *rgba, int width, int height, int size, int thread_id)
{
    // Compute the number of ASTC blocks in each dimension
    unsigned int block_count_x = (width + save_block_x - 1) / save_block_x;
    unsigned int block_count_y = (height + save_block_y - 1) / save_block_y;

    astcenc_error status;
    // ------------------------------------------------------------------------
    // Compress the image
    astcenc_image image;
    image.dim_x = width;
    image.dim_y = height;
    image.dim_z = 1;
    image.data_type = ASTCENC_TYPE_U8;
    uint8_t* slices = rgba;
    image.data = reinterpret_cast<void**>(&slices);

    // Space needed for 16 bytes of output per compressed block
    size_t comp_len = block_count_x * block_count_y * 16;
    std::vector<byte> result(sizeof(astc_header) + comp_len);
    uint8_t* comp_data = result.data() + sizeof(astc_header);

    status = astcenc_compress_image(context, &image, &swizzle, comp_data, comp_len, thread_id);
    if (status != ASTCENC_SUCCESS)
    {
        MsgDev( D_WARN, "Image_SaveASTC: Codec compress failed: %s\n", astcenc_get_error_string(status));
        return {};
    }

    // Stage : write file

    astc_header &hdr = *reinterpret_cast<astc_header *>(result.data());
    hdr.magic[0] =  ASTC_MAGIC_ID        & 0xFF;
    hdr.magic[1] = (ASTC_MAGIC_ID >>  8) & 0xFF;
    hdr.magic[2] = (ASTC_MAGIC_ID >> 16) & 0xFF;
    hdr.magic[3] = (ASTC_MAGIC_ID >> 24) & 0xFF;

    hdr.block_x = static_cast<uint8_t>(save_block_x);
    hdr.block_y = static_cast<uint8_t>(save_block_y);
    hdr.block_z = static_cast<uint8_t>(save_block_z);

    hdr.dim_x[0] =  image.dim_x        & 0xFF;
    hdr.dim_x[1] = (image.dim_x >>  8) & 0xFF;
    hdr.dim_x[2] = (image.dim_x >> 16) & 0xFF;

    hdr.dim_y[0] =  image.dim_y       & 0xFF;
    hdr.dim_y[1] = (image.dim_y >>  8) & 0xFF;
    hdr.dim_y[2] = (image.dim_y >> 16) & 0xFF;

    hdr.dim_z[0] =  image.dim_z        & 0xFF;
    hdr.dim_z[1] = (image.dim_z >>  8) & 0xFF;
    hdr.dim_z[2] = (image.dim_z >> 16) & 0xFF;

    return result;
}

qboolean Image_SaveASTC( const char *name, image_ref pix )
{
    if( FS_FileExists( name, false ) && !Image_CheckFlag( IL_ALLOW_OVERWRITE ) && !host.write_to_clipboard )
        return false; // already existed

    // bogus parameter check
    if( !pix->buffer )
        return false;

    // get image description
    switch( pix->type )
    {
        case PF_RGBA_32:
            break;
        default:
            MsgDev( D_ERROR, "Image_SaveASTC: unsupported image type %s\n", PFDesc[pix->type].name );
            return false;
    }

    auto context = Image_SaveASTC_CreateContext(1);
    auto astc_tex = Image_SaveASTC_Worker(context, pix->buffer, pix->width, pix->height, pix->size, 0);
    Image_SaveASTC_DestroyContext(context);
    if(astc_tex.empty())
        return false;

    file_t *pfile = FS_Open( name, "wb", false );
    if( !pfile ) return false;
    FS_Write(pfile, astc_tex.data(), astc_tex.size());
    FS_Close(pfile);
    return true;
}