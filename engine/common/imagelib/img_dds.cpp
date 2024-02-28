/*
img_dds.c - dds format load
Copyright (C) 2015 Uncle Mike

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
#ifndef XASH_DEDICATED
#include "gl_local.h"
#endif

qboolean Image_CheckDXT3Alpha( dds_t *hdr, byte *fin )
{
	auto width = hdr->dwWidth;
	auto height = hdr->dwHeight;
	unsigned int blockCountX = (width + 3) / 4;
	unsigned int blockCountY = (height + 3) / 4;

	for (unsigned int x = 0; x < blockCountY * 4; x+=4)
	{
		for (unsigned int y = 0; y < blockCountX * 4; y+=4)
		{
			auto blockStorage = fin + blockCountX * y*4 + x*4;

			for (int j=0; j < 4; j++)
			{
				for (int i=0; i < 4; i++)
				{
					word *alphaPtr = (word *)blockStorage;
					word alphaLine = alphaPtr[j];
					unsigned int alphaCode = (alphaLine >> i * 4) & 0x0F;
					unsigned char finalAlpha = alphaCode << 4;

					if (x + i < width)
					{
						// valid pixel
						if (finalAlpha != 255)
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

qboolean Image_CheckDXT5Alpha( dds_t *hdr, byte *fin )
{
	auto width = hdr->dwWidth;
	auto height = hdr->dwHeight;
	unsigned int blockCountX = (width + 3) / 4;
	unsigned int blockCountY = (height + 3) / 4;

	for (unsigned int x = 0; x < blockCountY * 4; x+=4)
	{
		for (unsigned int y = 0; y < blockCountX * 4; y+=4)
		{
			auto blockStorage = fin + blockCountX * y*4 + x*4;

			unsigned char alpha0 = *reinterpret_cast<const unsigned char *>(blockStorage);
			unsigned char alpha1 = *reinterpret_cast<const unsigned char *>(blockStorage + 1);

			const unsigned char *bits = fin + 2;
			unsigned int alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
			unsigned short alphaCode2 = bits[0] | (bits[1] << 8);

			for (int j=0; j < 4; j++)
			{
				for (int i=0; i < 4; i++)
				{
					int alphaCodeIndex = 3*(4*j+i);
					int alphaCode;

					if (alphaCodeIndex <= 12)
					{
						alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
					}
					else if (alphaCodeIndex == 15)
					{
						alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
					}
					else // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
					{
						alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
					}

					unsigned char finalAlpha;
					if (alphaCode == 0)
					{
						finalAlpha = alpha0;
					}
					else if (alphaCode == 1)
					{
						finalAlpha = alpha1;
					}
					else
					{
						if (alpha0 > alpha1)
						{
							finalAlpha = ((8-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/7;
						}
						else
						{
							if (alphaCode == 6)
								finalAlpha = 0;
							else if (alphaCode == 7)
								finalAlpha = 255;
							else
								finalAlpha = ((6-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/5;
						}
					}

					if (x + i < width)
					{
						// valid pixel
						if (finalAlpha != 255)
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}
		
void Image_DXTGetPixelFormat( dds_t *hdr )
{
	uint bits = hdr->dsPixelFormat.dwRGBBitCount;

	// all volume textures I've seem so far didn't have the DDS_COMPLEX flag set,
	// even though this is normally required. But because noone does set it,
	// also read images without it (TODO: check file size for 3d texture?)
	if( !( hdr->dsCaps.dwCaps2 & DDS_VOLUME ))
		hdr->dwDepth = 1;

	if( hdr->dsPixelFormat.dwFlags & DDS_FOURCC )
	{
		switch( hdr->dsPixelFormat.dwFourCC )
		{
		case TYPE_DXT1: 
			image.type = PF_DXT1;
			break;
		case TYPE_DXT2:
			image.flags &= ~IMAGE_HAS_ALPHA; // alpha is already premultiplied by color
		case TYPE_DXT3:
			image.type = PF_DXT3;
			break;
		case TYPE_DXT4:
			image.flags &= ~IMAGE_HAS_ALPHA; // alpha is already premultiplied by color
		case TYPE_DXT5:
			image.type = PF_DXT5;
			break;
		default:
			image.type = PF_UNKNOWN; // assume error
			break;
		}
	}
	else
	{
		// this dds texture isn't compressed so write out ARGB or luminance format
		if( hdr->dsPixelFormat.dwFlags & DDS_DUDV )
		{
			image.type = PF_UNKNOWN; // assume error
		}
		else if( hdr->dsPixelFormat.dwFlags & DDS_LUMINANCE )
		{
			image.type = PF_UNKNOWN; // assume error
		}
		else 
		{
			if( bits == 32 )
				image.type = PF_BGRA_32;
			else if( bits == 24 )
				image.type = PF_BGR_24;
			else image.type = PF_UNKNOWN; // assume error;
		}
	}

	// setup additional flags
	if( hdr->dsCaps.dwCaps1 & DDS_COMPLEX && hdr->dsCaps.dwCaps2 & DDS_CUBEMAP )
		image.flags |= IMAGE_CUBEMAP;

	if( hdr->dwFlags & DDS_MIPMAPCOUNT )
		image.num_mips = hdr->dwMipMapCount; // get actual mip count
}

size_t Image_DXTGetLinearSize( int type, int width, int height, int depth )
{
	switch( type )
	{
	case PF_DXT1: return ((( width + 3 ) / 4 ) * (( height + 3 ) / 4 ) * depth * 8 );
	case PF_DXT3:
	case PF_DXT5: return ((( width + 3 ) / 4 ) * (( height + 3 ) / 4 ) * depth * 16 );
	case PF_BGR_24:
	case PF_RGB_24: return (width * height * depth * 3);
	case PF_BGRA_32:
	case PF_RGBA_32: return (width * height * depth * 4);
	}	

	return 0;
}

size_t Image_DXTCalcMipmapSize( const dds_t *hdr )
{
	size_t	buffsize = 0;
	int	i, width, height;
		
	// now correct buffer size
	for( i = 0; i < hdr->dwMipMapCount; i++ )
	{
		width = max( 1, ( hdr->dwWidth >> i ));
		height = max( 1, ( hdr->dwHeight >> i ));
		buffsize += Image_DXTGetLinearSize( image.type, width, height, image.depth );
	}

	return buffsize;
}

uint Image_DXTCalcSize( const dds_t *hdr )
{
	size_t buffsize = 0;
	int w = image.width;
	int h = image.height;
	int d = image.depth;

    uint linearSize = hdr->dwLinearSize;
    uint flags = hdr->dwFlags;

    if( hdr->dwDepth > 1 )
    {
        linearSize = Image_DXTGetLinearSize( image.type, hdr->dwWidth, hdr->dwHeight, hdr->dwDepth );
        flags |= DDS_LINEARSIZE;
    }

	if( flags & ( DDS_LINEARSIZE ))
	{
		// just in case (no need, really)
		buffsize = linearSize;
	}
	else if( hdr->dsCaps.dwCaps2 & DDS_CUBEMAP )
	{
		// cubemap w*h always match for all sides
		buffsize = Image_DXTCalcMipmapSize( hdr ) * 6;
	}
	else if( flags & DDS_MIPMAPCOUNT )
	{
		// if mipcount > 1
		buffsize = Image_DXTCalcMipmapSize( hdr );
	}
	else 
	{
		// pretty solution for microsoft bug
		buffsize = Image_DXTCalcMipmapSize( hdr );
	}

	return buffsize;
}

/*
=============
Image_LoadDDS
=============
*/
qboolean Image_LoadDDS_HW( imglib_t& image, const char *name, const byte *buffer, size_t filesize )
{
	dds_t	header;
	byte	*fin;

	if( filesize < sizeof( dds_t ))
	{
		MsgDev( D_ERROR, "Image_LoadDDS: file (%s) have invalid size\n", name );
		return false;
	}

	Q_memcpy( &header, buffer, sizeof( dds_t ));

	if( header.dwIdent != DDSHEADER )
		return false; // it's not a dds file, just skip it

	if( header.dwSize != sizeof( dds_t ) - sizeof( uint )) // size of the structure (minus MagicNum)
	{
		MsgDev( D_ERROR, "Image_LoadDDS: (%s) have corrupted header\n", name );
		return false;
	}

	if( header.dsPixelFormat.dwSize != sizeof( dds_pixf_t )) // size of the structure
	{
		MsgDev( D_ERROR, "Image_LoadDDS: (%s) have corrupt pixelformat header\n", name );
		return false;
	}

	image.width = header.dwWidth;
	image.height = header.dwHeight;

	if( header.dwFlags & DDS_DEPTH )
		image.depth = header.dwDepth;
	else image.depth = 1;

	if( !Image_ValidSize( image, name )) return false;

	Image_DXTGetPixelFormat( &header ); // and image type too :)

	if( image.type == PF_UNKNOWN ) 
	{
		MsgDev( D_WARN, "Image_LoadDDS: (%s) has unrecognized type\n", name );
		return false;
	}

	auto buffsize = Image_DXTCalcSize( &header );
	if( filesize != buffsize + sizeof(dds_t) ) // main check
	{
		MsgDev( D_WARN, "Image_LoadDDS: (%s) probably corrupted(%i should be %i)\n", name, buffsize, filesize );
		return false;
	}
	if( buffsize == 0 ) return false; // just in case
	fin = (byte *)(buffer + sizeof( dds_t ));

	// copy an encode method
    image.size = buffsize;
	image.encode = (word)header.dwReserved1[0];

	switch( image.encode )
	{
	case DXT_ENCODE_COLOR_YCoCg:
		image.flags |= IMAGE_HAS_COLOR;
		break;
	case DXT_ENCODE_NORMAL_AG_ORTHO:
	case DXT_ENCODE_NORMAL_AG_STEREO:
	case DXT_ENCODE_NORMAL_AG_PARABOLOID:
	case DXT_ENCODE_NORMAL_AG_QUARTIC:
	case DXT_ENCODE_NORMAL_AG_AZIMUTHAL:
		image.flags |= IMAGE_HAS_COLOR;
		break;
	default:	// check for real alpha-pixels
		if( image.type == PF_DXT3 && Image_CheckDXT3Alpha( &header, fin ))
			image.flags |= IMAGE_HAS_ALPHA;
		else if( image.type == PF_DXT5 && Image_CheckDXT5Alpha( &header, fin ))
			image.flags |= IMAGE_HAS_ALPHA;
		image.flags |= IMAGE_HAS_COLOR;
		break;
	}

	// dds files will be uncompressed on a render. requires minimal of info for set this
	image.rgba = (byte*)Mem_Alloc( host.imagepool, image.size );
    Mem_VirtualCopy( image.rgba, fin, image.size );

	return true;
}

uint32_t PackRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    return ((r << 24) | (g << 16) | (b << 8) | a);
}

void DecompressBlockDXT1(unsigned int x, unsigned int y, unsigned int width, const byte *blockStorage, byte *out_rgba)
{
    unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage);
    unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 2);

    unsigned int temp;

    temp = (color0 >> 11) * 255 + 16;
    unsigned char r0 = (unsigned char)((temp/32 + temp)/32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    unsigned char g0 = (unsigned char)((temp/64 + temp)/64);
    temp = (color0 & 0x001F) * 255 + 16;
    unsigned char b0 = (unsigned char)((temp/32 + temp)/32);

    temp = (color1 >> 11) * 255 + 16;
    unsigned char r1 = (unsigned char)((temp/32 + temp)/32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    unsigned char g1 = (unsigned char)((temp/64 + temp)/64);
    temp = (color1 & 0x001F) * 255 + 16;
    unsigned char b1 = (unsigned char)((temp/32 + temp)/32);

    unsigned int code = *reinterpret_cast<const unsigned int *>(blockStorage + 4);

    for (int j=0; j < 4; j++)
    {
        for (int i=0; i < 4; i++)
        {
            uint32_t finalColor = 0;
            unsigned char positionCode = (code >>  2*(4*j+i)) & 0x03;

            if (color0 > color1)
            {
                switch (positionCode)
                {
                    case 0:
                        finalColor = PackRGBA(r0, g0, b0, 255);
                        break;
                    case 1:
                        finalColor = PackRGBA(r1, g1, b1, 255);
                        break;
                    case 2:
                        finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, 255);
                        break;
                    case 3:
                        finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, 255);
                        break;
                }
            }
            else
            {
                switch (positionCode)
                {
                    case 0:
                        finalColor = PackRGBA(r0, g0, b0, 255);
                        break;
                    case 1:
                        finalColor = PackRGBA(r1, g1, b1, 255);
                        break;
                    case 2:
                        finalColor = PackRGBA((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, 255);
                        break;
                    case 3:
                        finalColor = PackRGBA(0, 0, 0, 255);
                        break;
                }
            }

            if (x + i < width)
            {
                auto image = (uint32_t *)out_rgba;
                image[(y + j)*width + (x + i)] = finalColor;
            }
        }
    }
}

void BlockDecompressImageDXT1(unsigned int width, unsigned int height, const byte *blockStorage, byte *out_rgba)
{
    unsigned int blockCountX = (width + 3) / 4;
    unsigned int blockCountY = (height + 3) / 4;

    for (unsigned int j = 0; j < blockCountY; j++)
    {
        for (unsigned int i = 0; i < blockCountX; i++) DecompressBlockDXT1(i*4, j*4, width, blockStorage + i * 8, out_rgba);
        blockStorage += blockCountX * 8;
    }
}

void DecompressBlockDXT3(unsigned int x, unsigned int y, unsigned int width, const byte *blockStorage, byte *out_rgba)
{
	unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage + 8);
	unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 10);

	unsigned int temp;

	temp = (color0 >> 11) * 255 + 16;
	unsigned char r0 = (unsigned char)((temp/32 + temp)/32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g0 = (unsigned char)((temp/64 + temp)/64);
	temp = (color0 & 0x001F) * 255 + 16;
	unsigned char b0 = (unsigned char)((temp/32 + temp)/32);

	temp = (color1 >> 11) * 255 + 16;
	unsigned char r1 = (unsigned char)((temp/32 + temp)/32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g1 = (unsigned char)((temp/64 + temp)/64);
	temp = (color1 & 0x001F) * 255 + 16;
	unsigned char b1 = (unsigned char)((temp/32 + temp)/32);

	unsigned int code = *reinterpret_cast<const unsigned int *>(blockStorage + 12);

	for (int j=0; j < 4; j++)
	{
		for (int i=0; i < 4; i++)
		{
			word *alphaPtr = (word *)blockStorage;
			word alphaLine = alphaPtr[j];
			unsigned int alphaCode = (alphaLine >> i * 4) & 0x0F;
			unsigned char finalAlpha = alphaCode << 4;

			unsigned char colorCode = (code >> 2*(4*j+i)) & 0x03;

			uint32_t finalColor;
			switch (colorCode)
			{
				case 0:
					finalColor = PackRGBA(r0, g0, b0, finalAlpha);
					break;
				case 1:
					finalColor = PackRGBA(r1, g1, b1, finalAlpha);
					break;
				case 2:
					finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, finalAlpha);
					break;
				case 3:
					finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, finalAlpha);
					break;
			}

			if (x + i < width)
			{
				auto image = (uint32_t *)out_rgba;
				image[(y + j)*width + (x + i)] = finalColor;
			}
		}
	}
}

void BlockDecompressImageDXT3(unsigned int width, unsigned int height, const byte *blockStorage, byte *out_rgba)
{
	unsigned int blockCountX = (width + 3) / 4;
	unsigned int blockCountY = (height + 3) / 4;

	for (unsigned int j = 0; j < blockCountY; j++)
	{
		for (unsigned int i = 0; i < blockCountX; i++) DecompressBlockDXT3(i*4, j*4, width, blockStorage + i * 16, out_rgba);
		blockStorage += blockCountX * 16;
	}
}

void DecompressBlockDXT5(unsigned int x, unsigned int y, unsigned int width, const byte *blockStorage, byte *out_rgba)
{
    unsigned char alpha0 = *reinterpret_cast<const unsigned char *>(blockStorage);
    unsigned char alpha1 = *reinterpret_cast<const unsigned char *>(blockStorage + 1);

    const unsigned char *bits = blockStorage + 2;
    unsigned int alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
    unsigned short alphaCode2 = bits[0] | (bits[1] << 8);

    unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage + 8);
    unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 10);

    unsigned int temp;

    temp = (color0 >> 11) * 255 + 16;
    unsigned char r0 = (unsigned char)((temp/32 + temp)/32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    unsigned char g0 = (unsigned char)((temp/64 + temp)/64);
    temp = (color0 & 0x001F) * 255 + 16;
    unsigned char b0 = (unsigned char)((temp/32 + temp)/32);

    temp = (color1 >> 11) * 255 + 16;
    unsigned char r1 = (unsigned char)((temp/32 + temp)/32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    unsigned char g1 = (unsigned char)((temp/64 + temp)/64);
    temp = (color1 & 0x001F) * 255 + 16;
    unsigned char b1 = (unsigned char)((temp/32 + temp)/32);

    unsigned int code = *reinterpret_cast<const unsigned int *>(blockStorage + 12);

    for (int j=0; j < 4; j++)
    {
        for (int i=0; i < 4; i++)
        {
            int alphaCodeIndex = 3*(4*j+i);
            int alphaCode;

            if (alphaCodeIndex <= 12)
            {
                alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
            }
            else if (alphaCodeIndex == 15)
            {
                alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
            }
            else // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
            {
                alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
            }

            unsigned char finalAlpha;
            if (alphaCode == 0)
            {
                finalAlpha = alpha0;
            }
            else if (alphaCode == 1)
            {
                finalAlpha = alpha1;
            }
            else
            {
                if (alpha0 > alpha1)
                {
                    finalAlpha = ((8-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/7;
                }
                else
                {
                    if (alphaCode == 6)
                        finalAlpha = 0;
                    else if (alphaCode == 7)
                        finalAlpha = 255;
                    else
                        finalAlpha = ((6-alphaCode)*alpha0 + (alphaCode-1)*alpha1)/5;
                }
            }

            unsigned char colorCode = (code >> 2*(4*j+i)) & 0x03;

            uint32_t finalColor;
            switch (colorCode)
            {
                case 0:
                    finalColor = PackRGBA(r0, g0, b0, finalAlpha);
                    break;
                case 1:
                    finalColor = PackRGBA(r1, g1, b1, finalAlpha);
                    break;
                case 2:
                    finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, finalAlpha);
                    break;
                case 3:
                    finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, finalAlpha);
                    break;
            }

            if (x + i < width)
            {
                auto image = (uint32_t *)out_rgba;
                image[(y + j)*width + (x + i)] = finalColor;
            }
        }
    }
}

void BlockDecompressImageDXT5(unsigned int width, unsigned int height, const byte *blockStorage, byte *out_rgba)
{
    unsigned int blockCountX = (width + 3) / 4;
    unsigned int blockCountY = (height + 3) / 4;

    for (unsigned int j = 0; j < blockCountY; j++)
    {
        for (unsigned int i = 0; i < blockCountX; i++) DecompressBlockDXT5(i*4, j*4, width, blockStorage + i * 16, out_rgba);
        blockStorage += blockCountX * 16;
    }
}

qboolean Image_LoadDDS_SW( imglib_t& image, const char *name, const byte *buffer, size_t filesize )
{
    dds_t	header;
    byte	*fin;

    if( filesize < sizeof( dds_t ))
    {
        MsgDev( D_ERROR, "Image_LoadDDS: file (%s) have invalid size\n", name );
        return false;
    }

    Q_memcpy( &header, buffer, sizeof( dds_t ));

    if( header.dwIdent != DDSHEADER )
        return false; // it's not a dds file, just skip it

    if( header.dwSize != sizeof( dds_t ) - sizeof( uint )) // size of the structure (minus MagicNum)
    {
        MsgDev( D_ERROR, "Image_LoadDDS: (%s) have corrupted header\n", name );
        return false;
    }

    if( header.dsPixelFormat.dwSize != sizeof( dds_pixf_t )) // size of the structure
    {
        MsgDev( D_ERROR, "Image_LoadDDS: (%s) have corrupt pixelformat header\n", name );
        return false;
    }

    image.width = header.dwWidth;
    image.height = header.dwHeight;

    if( header.dwFlags & DDS_DEPTH )
        image.depth = header.dwDepth;
    else image.depth = 1;

    if( !Image_ValidSize( image, name )) return false;

    Image_DXTGetPixelFormat( &header ); // and image type too :)

    if( image.type == PF_UNKNOWN )
    {
        MsgDev( D_WARN, "Image_LoadDDS: (%s) has unrecognized type\n", name );
        return false;
    }

    auto buffsize = Image_DXTCalcSize( &header );
    if( filesize != buffsize + sizeof(dds_t) ) // main check
    {
        MsgDev( D_WARN, "Image_LoadDDS: (%s) probably corrupted(%i should be %i)\n", name, buffsize, filesize );
        return false;
    }
    if( buffsize == 0 ) return false; // just in case
    fin = (byte *)(buffer + sizeof( dds_t ));

    // copy an encode method
    image.size = buffsize;
    image.encode = (word)header.dwReserved1[0];

    switch( image.encode )
    {
        case DXT_ENCODE_COLOR_YCoCg:
            image.flags |= IMAGE_HAS_COLOR;
            break;
        case DXT_ENCODE_NORMAL_AG_ORTHO:
        case DXT_ENCODE_NORMAL_AG_STEREO:
        case DXT_ENCODE_NORMAL_AG_PARABOLOID:
        case DXT_ENCODE_NORMAL_AG_QUARTIC:
        case DXT_ENCODE_NORMAL_AG_AZIMUTHAL:
            image.flags |= IMAGE_HAS_COLOR;
            break;
        default:	// check for real alpha-pixels
            if( image.type == PF_DXT3 && Image_CheckDXT3Alpha( &header, fin ))
                image.flags |= IMAGE_HAS_ALPHA;
            else if( image.type == PF_DXT5 && Image_CheckDXT5Alpha( &header, fin ))
                image.flags |= IMAGE_HAS_ALPHA;
            image.flags |= IMAGE_HAS_COLOR;
            break;
    }

    // software decoding
    if( image.type == PF_DXT1 )
    {
        image.size = image.width * image.height * 4;
        image.type = PF_RGBA_32;
        image.rgba = (byte*)Mem_Alloc( host.imagepool, image.size );
        BlockDecompressImageDXT1(image.width, image.height, fin, image.rgba);
        return true;
    }
    if( image.type == PF_DXT3 )
    {
        image.size = image.width * image.height * 4;
        image.type = PF_RGBA_32;
        image.rgba = (byte*)Mem_Alloc( host.imagepool, image.size );
	    BlockDecompressImageDXT3(image.width, image.height, fin, image.rgba);
        return true;
    }
    if( image.type == PF_DXT5 )
    {
        image.size = image.width * image.height * 4;
        image.type = PF_RGBA_32;
        image.rgba = (byte*)Mem_Alloc( host.imagepool, image.size );
        BlockDecompressImageDXT5(image.width, image.height, fin, image.rgba);
        return true;
    }

    // unsupported
    return false;
}

qboolean Image_LoadDDS( imglib_t& image, const char *name, const byte *buffer, size_t filesize )
{
#ifndef XASH_DEDICATED
	if(GL_Support(GL_S3TC_EXT) > 0)
		return Image_LoadDDS_HW(image, name, buffer, filesize);
	else
#endif
		return Image_LoadDDS_SW(image, name, buffer, filesize);
}