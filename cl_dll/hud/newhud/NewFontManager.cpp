/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "NewFontManager.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"

namespace cl {
	static CHudDrawFontText s_HudDrawFontText;

	CHudDrawFontText& DrawFontText(void)
	{
		return s_HudDrawFontText;
	};


	CHudDrawFontText::CHudDrawFontText()
	{
		m_pFont.clear();
	}

	int CHudDrawFontText::Init(void)
	{
		return 1;
	}

	int CHudDrawFontText::VidInit(void)
	{

		m_pFont.emplace_back(PushBackFont("Default22"));
		m_pFont.emplace_back(PushBackFont("Default20"));
		m_pFont.emplace_back(PushBackFont("Default16"));

		m_pFont.emplace_back(PushBackFont("DefaultUnderline"));
		m_pFont.emplace_back(PushBackFont("ChatDefault"));
		m_pFont.emplace_back(PushBackFont("DefaultSmall"));


		return 1;

	}

	FontText_t CHudDrawFontText::PushBackFont(const char* SzFontName, bool bProportional)
	{
		vgui2::IScheme* pClientScheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetScheme("ClientScheme"));


		static FontText_t TempFont{};
		sprintf(TempFont.SzFontName, "%s", SzFontName);
		TempFont.HFont = pClientScheme->GetFont(SzFontName, bProportional);

		return TempFont;
	}

	vgui2::HFont CHudDrawFontText::TakeOutFont(const char* SzFontName)
	{
		vgui2::HFont HFont = vgui2::INVALID_FONT;

		if (!m_pFont.size())
			return vgui2::INVALID_FONT;

		if (!SzFontName)
			return vgui2::INVALID_FONT;

		for (std::vector<FontText_t>::iterator iter = m_pFont.begin(); iter != m_pFont.end(); ++iter)
		{
			FontText_t FontText = *iter;
			if (FontText.SzFontName[0] && FontText.SzFontName)
			{
				if (!Q_strcmp(FontText.SzFontName, SzFontName))
					HFont = FontText.HFont;
			}
		}
		return HFont;
	}

	char* CHudDrawFontText::GetFontFullName(const char* SzFontName, int SzFontSize)
	{
		char Font[32]; strcpy(Font, SzFontName);
		char FontSize[32];
		if (SzFontSize != -1)
			strcpy(FontSize, std::to_string(SzFontSize).c_str());

		//初始化字符串为空
		static char FontFullName[32];
		memset(FontFullName, 0, sizeof(FontFullName));
		//重新赋值字符串
		strncpy(FontFullName, Font, strlen(Font));

		if (SzFontSize != -1)
			strncpy(FontFullName + strlen(Font), FontSize, strlen(FontSize));

		//使用static，避免局部栈变量提前被释放
		return FontFullName;
	}

	int CHudDrawFontText::GetFontTextWide(const char* SzFontName, int SzFontSize, const wchar_t* str)
	{

		if (!str)
			return 0;

		const char* SzFontFullName = GetFontFullName(SzFontName, SzFontSize);

		if (!TakeOutFont(SzFontFullName))
			return 0;

		int width = 0;
		for (unsigned int i = 0; i < wcslen(str); i++)
			width += vgui2::surface()->GetCharacterWidth(TakeOutFont(SzFontFullName), str[i]);

		return width;
	}
	int CHudDrawFontText::GetFontTextHeight(const char* SzFontName, int SzFontSize)
	{
		const char* SzFontFullName = GetFontFullName(SzFontName, SzFontSize);
		if (!TakeOutFont(SzFontFullName))
			return 0;

		return vgui2::surface()->GetFontTall(TakeOutFont(SzFontFullName));
	}

	void CHudDrawFontText::DrawFontText(const char* SzFontName, int SzFontSize, int x, int y, int r, int g, int b, int a, const wchar_t* str)
	{
		if (!str)
			return;

		const char* SzFontFullName = GetFontFullName(SzFontName, SzFontSize);

		if (!TakeOutFont(SzFontFullName))
			return;

		vgui2::surface()->DrawSetTextFont(TakeOutFont(SzFontFullName));

		for (unsigned int i = 0; i < wcslen(str); i++)
		{
			vgui2::surface()->DrawSetTextPos(x, y);
			vgui2::surface()->DrawSetTextColor(r, g, b, a);
			vgui2::surface()->DrawUnicodeChar(str[i]);
			x += vgui2::surface()->GetCharacterWidth(TakeOutFont(SzFontFullName), str[i]);
		}

		vgui2::surface()->DrawFlushText();
	}


}