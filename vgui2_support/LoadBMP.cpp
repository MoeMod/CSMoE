
#include <basetypes.h>

#include "LoadBMP.h"
#include "vgui_controls/controls.h"
#include "filesystem.h"

int LoadBMP(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height)
{
	FileHandle_t file = vgui2::filesystem()->Open(szFilename, "rb");

	if (!file)
	{
		return FALSE;
	}

	BITMAPFILEHEADER bmfHeader;
	LPBITMAPINFO lpbmi;
	DWORD dwFileSize = vgui2::filesystem()->Size(file);

	if (!vgui2::filesystem()->Read(&bmfHeader, sizeof(bmfHeader), file))
	{
		*width = 0;
		*height = 0;
		vgui2::filesystem()->Close(file);
		return FALSE;
	}

	if (bmfHeader.bfType == DIB_HEADER_MARKER)
	{
		DWORD dwBitsSize = dwFileSize - sizeof(bmfHeader);

		HGLOBAL hDIB = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize);
		char *pDIB = (LPSTR)::GlobalLock((HGLOBAL)hDIB);

		if (!vgui2::filesystem()->Read(pDIB, dwBitsSize, file))
		{
			::GlobalUnlock(hDIB);
			::GlobalFree((HGLOBAL)hDIB);

			*width = 0;
			*height = 0;

			vgui2::filesystem()->Close(file);
			return FALSE;
		}

		lpbmi = (LPBITMAPINFO)pDIB;

		if (width)
			*width = lpbmi->bmiHeader.biWidth;

		if (height)
			*height = lpbmi->bmiHeader.biHeight;

		unsigned char *rgba = (unsigned char *)(pDIB + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

		for (int j = 0; j < lpbmi->bmiHeader.biHeight; j++)
		{
			for (int i = 0; i < lpbmi->bmiHeader.biWidth; i++)
			{
				int y = (lpbmi->bmiHeader.biHeight - j - 1);

				int offs = (y * lpbmi->bmiHeader.biWidth + i);
				int offsdest = (j * lpbmi->bmiHeader.biWidth + i) * 4;
				unsigned char *src = rgba + offs;
				unsigned char *dst = buffer + offsdest;

				dst[0] = lpbmi->bmiColors[*src].rgbRed;
				dst[1] = lpbmi->bmiColors[*src].rgbGreen;
				dst[2] = lpbmi->bmiColors[*src].rgbBlue;
				dst[3] = 255;
			}
		}

		::GlobalUnlock(hDIB);
		::GlobalFree((HGLOBAL)hDIB);
	}

	vgui2::filesystem()->Close(file);


	return TRUE;
}

int GetBMPSize(const char *szFilename, int *width, int *height)
{
	FileHandle_t file = vgui2::filesystem()->Open(szFilename, "rb");

	if (!file)
	{
		*width = 0;
		*height = 0;
		return FALSE;
	}

	BITMAPFILEHEADER bmfHeader;
	LPBITMAPINFO lpbmi;
	DWORD dwFileSize = vgui2::filesystem()->Size(file);

	if (!vgui2::filesystem()->Read(&bmfHeader, sizeof(bmfHeader), file))
	{
		*width = 0;
		*height = 0;

		vgui2::filesystem()->Close(file);
		return FALSE;
	}

	if (bmfHeader.bfType == DIB_HEADER_MARKER)
	{
		DWORD dwBitsSize = dwFileSize - sizeof(bmfHeader);

		HGLOBAL hDIB = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize);
		char *pDIB = (LPSTR)::GlobalLock((HGLOBAL)hDIB);

		if (!vgui2::filesystem()->Read(pDIB, dwBitsSize, file))
		{
			::GlobalUnlock(hDIB);
			::GlobalFree((HGLOBAL)hDIB);

			*width = 0;
			*height = 0;

			vgui2::filesystem()->Close(file);
			return FALSE;
		}

		lpbmi = (LPBITMAPINFO)pDIB;

		if (width)
			*width = lpbmi->bmiHeader.biWidth;

		if (height)
			*height = lpbmi->bmiHeader.biHeight;

		::GlobalUnlock(hDIB);
		::GlobalFree((HGLOBAL)hDIB);
	}

	vgui2::filesystem()->Close(file);
	return TRUE;
}