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

static const unsigned int thread_count = 1;
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
qboolean Image_LoadASTC( const char *name, const byte *buffer, size_t filesize )
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
	status = astcenc_context_alloc(&config, thread_count, &context);
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

