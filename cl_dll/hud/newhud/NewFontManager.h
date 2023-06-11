#pragma once
#ifndef NEWFONTMANAGER_H
#define NEWFONTMANAGER_H

#include "vgui_controls/controls.h"
#include "vgui/IScheme.h"


typedef struct FontText_s
{
	char SzFontName[32];
	vgui2::HFont HFont;
}FontText_t;

namespace cl {
	class CHudDrawFontText : public CHudBase
	{
	public:
		CHudDrawFontText();

		int Init(void);
		int VidInit(void);

		FontText_t PushBackFont(const char* SzFontName, bool bProportional = false);
		vgui2::HFont TakeOutFont(const char* SzFontName);

		char* GetFontFullName(const char* SzFontName, int SzFontSize);
	public:
		int GetFontTextWide(const char* SzFontName, int SzFontSize, const wchar_t* str);
		int GetFontTextHeight(const char* SzFontName, int SzFontSize);
		void DrawFontText(const char* SzFontName, int SzFontSize, int x, int y, int r, int g, int b, int a, const wchar_t* str);
	private:
		std::vector<FontText_t> m_pFont;
	};
}


#endif