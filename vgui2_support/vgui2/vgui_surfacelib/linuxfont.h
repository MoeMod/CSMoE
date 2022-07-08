//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef LINUXFONT_H
#define LINUXFONT_H

#include <map>
#include <set>
#include <tier0/memdbgoff.h>
#include <tier0/memdbgon.h>
#include "tier1/strtools.h"
#include "tier1/utlstring.h"

#ifdef _WIN32
#undef DLL_IMPORT
#endif
#include <ft2build.h>
#include FT_FREETYPE_H


//-----------------------------------------------------------------------------
// Purpose: encapsulates a OSX font
//-----------------------------------------------------------------------------
class CLinuxFont
{
public:
	CLinuxFont();
	~CLinuxFont();

	// creates the font from windows.  returns false if font does not exist in the OS.
	virtual bool CreateFromMemory(const char *windowsFontName, void *data, int size, int tall, int weight, int blur, int scanlines, int flags);

	// writes the char into the specified 32bpp texture
	virtual void GetCharRGBA( uchar32 ch, int rgbaWide, int rgbaTall, unsigned char *rgba);

	// returns true if the font is equivalent to that specified
	virtual bool IsEqualTo(const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags);

	// returns true only if this font is valid for use
	virtual bool IsValid();

	// gets the abc widths for a character
	//  A spacing is the distance to add to the current position before drawing the character glyph.
	//  B spacing is the width of the drawn portion of the glyph.
	//  C spacing is the distance to add to the current position to provide white space to the right of the glyph.
	virtual void GetCharABCWidths(int ch, int &a, int &b, int &c);

	// set the font to be the one to currently draw with in the gdi
	void *SetAsActiveFont( void *glContext );

	// returns the height of the font, in pixels
	virtual int GetHeight();

	// returns the requested height of the font.
	virtual int GetHeightRequested();

	// returns the ascent of the font, in pixels (ascent=units above the base line)
	virtual int GetAscent();

	// returns the maximum width of a character, in pixels
	virtual int GetMaxCharWidth();

	// returns the flags used to make this font
	virtual int GetFlags();

	// returns true if this font is underlined
	virtual bool GetUnderlined() { return m_bUnderlined; }
	
	// gets the name of this font
	const char *GetName() { return m_szName.String(); }
	const char *GetFamilyName() { return m_face ? m_face->family_name : NULL; }

	// gets the weight of the font
	virtual int GetWeight() { return m_iWeight; }
	
	bool HasChar(uchar32 wch);

	// gets the width of ch given its position around before and after chars
	virtual void GetKernedCharWidth( uchar32 ch, uchar32 chBefore, uchar32 chAfter, float &wide, float &abcA, float &abcC );

#ifdef DBGFLAG_VALIDATE
	void Validate( CValidator &validator, char *pchName );
#endif

	// Given a font name from windows, match it to the filename and return that.
	static char *GetFontFileName(const char *windowsFontName, int flags);


protected:
	CUtlString m_szName;
	int m_iTall;
	int m_iWeight;
	int m_iFlags;
	bool m_bAntiAliased;
	bool m_bRotary;
	bool m_bAdditive;
	int m_iDropShadowOffset;
	bool m_bUnderlined;
	int m_iOutlineSize;

	int m_iHeight;
	int m_iHeightRequested;
	int m_iMaxCharWidth;
	int m_iAscent;

	int m_iScanLines;
	int m_iBlur;

private:
	static void CreateFontList();
	// abc widths
	struct abc_t
	{
		short b;
		char a;
		char c;
	};

	// cache for storing asian abc widths (since it's too big too just store them all)
	struct abc_cache_t
	{
        uchar32 wch;
		abc_t abc;
	};

	
	std::map<int, abc_t> m_ExtendedABCWidthsCache;

	// cache for storing asian abc widths (since it's too big too just store them all)
	struct kernedSize
	{
		float wide;
	};

    std::map<std::tuple<uchar32, uchar32, uchar32>, kernedSize> m_ExtendedKernedABCWidthsCache;

	bool m_faceValid;
	FT_Face m_face;

	struct font_name_entry
	{
		font_name_entry()
		{
			m_pchFile = NULL;
			m_pchFriendlyName = NULL;
		}
		
		char *m_pchFile;
		char *m_pchFriendlyName;
		bool operator<( const font_name_entry &rhs ) const
		{
			return V_stricmp( rhs.m_pchFriendlyName, m_pchFriendlyName ) > 0;
		}
	};
	static std::set< font_name_entry > m_FriendlyNameCache;
};

#endif // LINUXFONT_H
