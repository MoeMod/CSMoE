/*
Bitmap.h - bitmap menu item
Copyright (C) 2010 Uncle Mike
Copyright (C) 2017 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once
#ifndef MENU_BITMAP_H
#define MENU_BITMAP_H

#include "BaseItem.h"
namespace ui {
class CMenuBitmap : public CMenuBaseItem
{
public:
	typedef CMenuBaseItem BaseClass;

	CMenuBitmap();

	void VidInit( void ) override;
	const char * Key( int key, int down ) override;
	void Draw( void ) override;
	void SetPicture( const char *pic, const char *focusPic = NULL, const char *pressPic = NULL)
	{
		szPic = pic;
		szFocusPic = focusPic;
		szPressPic = pressPic;
	}

	void SetRenderMode( ERenderMode renderMode, ERenderMode focusRenderMode = QM_DRAWNORMAL, ERenderMode pressRenderMode = QM_DRAWNORMAL )
	{
		eRenderMode = renderMode;
		eFocusRenderMode = focusRenderMode;
		ePressRenderMode = pressRenderMode;
	}

	const char *szPic;
	ERenderMode eRenderMode;

	const char *szFocusPic;
	ERenderMode eFocusRenderMode;

	const char *szPressPic;
	ERenderMode ePressRenderMode;
};

// menu banners used fiexed rectangle (virtual screenspace at 640x480)
#define UI_BANNER_POSX		72
#define UI_BANNER_POSY		72
#define UI_BANNER_WIDTH		736
#define UI_BANNER_HEIGHT		128

class CMenuBannerBitmap : public CMenuBitmap
{
public:
	typedef CMenuBitmap BaseClass;

	CMenuBannerBitmap() : BaseClass()
	{
		iFlags = QMF_INACTIVE;
		SetCharSize( QM_BIGFONT );
		SetRect( UI_BANNER_POSX, UI_BANNER_POSY, UI_BANNER_WIDTH, UI_BANNER_HEIGHT );
		SetRenderMode( QM_DRAWADDITIVE );
	}
	void Draw( void ) override;
	void VidInit( void ) override;
	void SetInactive(bool) override { }
	void ToggleInactive() override { }
};
}

#endif // MENU_BITMAP_H
