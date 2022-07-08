
#include <Windows.h>
#include <stdio.h>
#include "UtlBuffer.h"
#include <vgui/VGUI.h>
#include <vgui_controls/Controls.h>
#include "FileSystem.h"

#define TYP_LUMPY 64

typedef struct
{
	char identification[4];
	int numlumps;
	int infotableofs;
}
wadinfo_t;

typedef struct
{
	int filepos;
	int disksize;
	int size;
	char type;
	char compression;
	char pad1, pad2;
	char name[16];
}
lumpinfo_t;

typedef struct
{
	char name[16];
	unsigned width, height;
	unsigned offsets[4];
}
miptex_t;

unsigned char pixdata[256];

float linearpalette[256][3];
float d_red, d_green, d_blue;
int colors_used;
int color_used[256];
float maxdistortion;
unsigned char palLogo[768];

unsigned char AveragePixels(int count)
{
	return pixdata[0];
}

int GrabMip(HANDLE hdib, unsigned char *lump_p, char *lumpname, COLORREF crf, int *width, int *height)
{
	int x, y, xl, yl, xh, yh, w, h;
	unsigned char *screen_p, *source;
	miptex_t *qtex;
	int miplevel, mipstep;
	int xx, yy;
	int count;
	int byteimagewidth, byteimageheight;
	unsigned char *byteimage;
	LPBITMAPINFO lpbmi;

	lpbmi = (LPBITMAPINFO)::GlobalLock((HGLOBAL)hdib);
	unsigned char *lump_start = lump_p;

	xl = yl = 0;
	w = lpbmi->bmiHeader.biWidth;
	h = lpbmi->bmiHeader.biHeight;

	*width = w;
	*height = h;

	byteimage = (unsigned char *)((LPSTR)lpbmi + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

	if ((w & 15) || (h & 15))
		return 0;

	xh = xl + w;
	yh = yl + h;

	qtex = (miptex_t *)lump_p;
	qtex->width = (unsigned)(w);
	qtex->height = (unsigned)(h);
	strcpy(qtex->name, lumpname);
	lump_p = (unsigned char *)&qtex->offsets[4];

	byteimagewidth = w;
	byteimageheight = h;

	source = (unsigned char *)lump_p;
	qtex->offsets[0] = (unsigned)((unsigned char *)lump_p - (unsigned char *)qtex);
	screen_p = byteimage + (h - 1) * w;

	for (y = yl; y < yh; y++)
	{
		for (x = xl; x < xh; x++)
			*lump_p++ = *screen_p++;

		screen_p -= 2 * w;
	}

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			float f = (float)(palLogo[i * 3 + j] / 255.0);
			linearpalette[i][j] = f;
		}
	}

	maxdistortion = 0;
	colors_used = 256;

	for (int i = 0; i < 256; i++)
		color_used[i] = 1;

	for (miplevel = 1; miplevel < 4; miplevel++)
	{
		d_red = d_green = d_blue = 0;
		qtex->offsets[miplevel] = (unsigned)(lump_p - (unsigned char *)qtex);
		mipstep = 1 << miplevel;

		for (y = 0; y < h; y += mipstep)
		{
			for (x = 0; x < w; x += mipstep)
			{
				count = 0;

				for (yy = 0; yy < mipstep; yy++)
				{
					for (xx = 0; xx < mipstep; xx++)
						pixdata[count++] = source[(y + yy) * w + x + xx];
				}

				*lump_p++ = AveragePixels(count);
			}
		}
	}

	::GlobalUnlock(lpbmi);

	*(unsigned short *)lump_p = 256;
	lump_p += sizeof(short);

	memcpy(lump_p, &palLogo[0], 765);
	lump_p += 765;

	*lump_p++ = (unsigned char)(crf & 0xFF);
	*lump_p++ = (unsigned char)((crf >> 8) & 0xFF);
	*lump_p++ = (unsigned char)((crf >> 16) & 0xFF);

	return lump_p - lump_start;
}

void UpdateLogoWAD(void *phdib, int r, int g, int b)
{
	char logoname[32];
	sprintf(logoname, "LOGO");
	char *pszName = &logoname[0];

	HANDLE hdib = (HANDLE)phdib;
	COLORREF crf = RGB(r, g, b);

	if ((!pszName) || (pszName[0] == 0) || (hdib == NULL))
		return;

	unsigned char *buf = (unsigned char *)_alloca(16384);
	memset(buf, 0, 16384);
	CUtlBuffer buffer(0, 16384);

	int width, height;
	int length = GrabMip(hdib, buf, pszName, crf, &width, &height);

	if (length == 0)
		return;

	bool sizevalid = false;

	if (width == height)
	{
		if (width == 16 || width == 32 || width == 64)
			sizevalid = true;
	}

	if (!sizevalid)
		return;

	while (length & 3)
		length++;

	wadinfo_t header;
	header.identification[0] = 'W';
	header.identification[1] = 'A';
	header.identification[2] = 'D';
	header.identification[3] = '3';
	header.numlumps = 1;
	header.infotableofs = 0;
	buffer.Put(&header, sizeof(wadinfo_t));

	lumpinfo_t info;
	memset(&info, 0, sizeof(info));
	strcpy(&info.name[0], pszName);
	info.filepos = (int)sizeof(wadinfo_t);
	info.size = info.disksize = length;
	info.type = TYP_LUMPY;
	info.compression = 0;

	buffer.Put(buf, length);
	buffer.Put(&info, sizeof(lumpinfo_t));

	int savepos = buffer.TellPut();
	buffer.SeekPut(CUtlBuffer::SEEK_HEAD, 0);
	header.infotableofs = length + sizeof(wadinfo_t);
	buffer.Put(&header, sizeof(wadinfo_t));
	buffer.SeekPut(CUtlBuffer::SEEK_HEAD, savepos);

	FileHandle_t file = vgui2::filesystem()->Open("tempdecal.wad", "wb");

	if (file != FILESYSTEM_INVALID_HANDLE)
	{
		vgui2::filesystem()->Write(buffer.Base(), buffer.TellPut(), file);
		vgui2::filesystem()->Close(file);
	}
}