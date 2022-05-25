//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUIFONT_H
#define VGUIFONT_H
#ifdef _WIN32
#pragma once
#endif

// Structure passed to CWin32Font::GetCharsRGBA
struct newChar_t
{
#ifdef _WIN32
	char32_t	wch;		// A new character to generate texture data for
#else
	wchar_t	wch;		// A new character to generate texture data for
#endif
	int		fontWide;	// Texel width of the character
	int		fontTall;	// Texel height of the character
	int     offset;		// Offset into the buffer given to GetCharsRGBA
};

#if defined(LINUX) || defined(OSX) || defined(_WIN32)
#include "linuxfont.h"
typedef CLinuxFont font_t;
#elif defined(WIN32)
#include "Win32Font.h"
typedef CWin32Font font_t;
#else
#error
#endif


#endif //VGUIFONT_H 