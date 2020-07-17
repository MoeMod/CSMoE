/*
money.cpp -- Money HUD Widget
Copyright (C) 2015-2016 a1batross

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

In addition, as a special exception, the author gives permission to
link the code of this program with the Half-Life Game Engine ("HL
Engine") and Modified Game Libraries ("MODs") developed by Valve,
L.L.C ("Valve").  You must obey the GNU General Public License in all
respects for all of the code used other than the HL Engine and MODs
from Valve.  If you modify this file, you may extend this exception
to your version of the file, but you are not obligated to do so.  If
you do not wish to do so, delete this exception statement from your
version.
*/


#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "triangleapi.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "gamemode/mods_const.h"
#include <string.h>
#include "vgui_parser.h"
#include "draw_util.h"
DECLARE_MESSAGE( m_Money, Money )
DECLARE_MESSAGE( m_Money, BlinkAcct )

int CHudMoney::Init( )
{
	HOOK_MESSAGE( Money );
	HOOK_MESSAGE( BlinkAcct );
	gHUD.AddHudElem(this);
	m_fFade = 0;
	m_iFlags = 0;
	return 1;
}

int CHudMoney::VidInit()
{
	m_hDollar.SetSpriteByName("dollar");
	m_hMinus.SetSpriteByName("minus");
	m_hPlus.SetSpriteByName("plus");
	R_InitTexture(m_pTexture_Black, "resource/hud/csgo/blacka");

	return 1;
}

int CHudMoney::Draw(float flTime)
{
	if(( gHUD.m_iHideHUDDisplay & ( HIDEHUD_HEALTH ) ))
		return 1;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT))))
		return 1;

	int r, g, b, alphaBalance;
	m_fFade -= gHUD.m_flTimeDelta;
	if( m_fFade < 0)
	{
		m_fFade = 0.0f;
		m_iDelta = 0;
	}

	float interpolate = ( 5 - m_fFade ) / 5;

	int iDollarWidth = m_hDollar.rect.right - m_hDollar.rect.left;

	int x = ScreenWidth - iDollarWidth * 7;
	int y = ScreenHeight - 3 * gHUD.m_iFontHeight;
	if (gHUD.m_csgohud->value)
	{
		x = 5;
		if (gHUD.m_iModRunning != MOD_NONE)
			y = 0.175 * ScreenWidth  + (m_hDollar.rect.bottom - m_hDollar.rect.top) * 2;
		else
			y = 200;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 100);
		m_pTexture_Black->Bind();
		DrawUtils::Draw2DQuadScaled(0, y- 0.5 * gHUD.m_iFontHeight, x + (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left) * 10, y + 1.5 * gHUD.m_iFontHeight);
	}
	// Does weapon have seconday ammo?
	if (!gHUD.m_csgohud->value)
	{
		if (gHUD.m_Ammo.FHasSecondaryAmmo())
		{
			y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight / 4;
		}
	}

	if( m_iBlinkAmt )
	{
		m_fBlinkTime += gHUD.m_flTimeDelta;
		DrawUtils::UnpackRGB( r, g, b, m_fBlinkTime > 0.5f? RGB_REDISH : RGB_YELLOWISH );

		if( m_fBlinkTime > 1.0f )
		{
			m_fBlinkTime = 0.0f;
			--m_iBlinkAmt;
		}
	}
	else
	{
		if( m_iDelta != 0 )
		{
			int iDeltaR, iDeltaG, iDeltaB;
			int iDollarHeight = m_hDollar.rect.bottom - m_hDollar.rect.top;
			int iDeltaAlpha = 255 - interpolate * (255);

			DrawUtils::UnpackRGB(iDeltaR, iDeltaG, iDeltaB, m_iDelta < 0 ? RGB_REDISH : RGB_GREENISH);
			DrawUtils::ScaleColors(iDeltaR, iDeltaG, iDeltaB, iDeltaAlpha);

			if( m_iDelta > 0 )//add money
			{
				r = interpolate * ((RGB_YELLOWISH & 0xFF0000) >> 16);
				g = (RGB_GREENISH & 0xFF00) >> 8;
				b = (RGB_GREENISH & 0xFF);

				// draw delta
				SPR_Set(m_hPlus.spr, iDeltaR, iDeltaG, iDeltaB );
				SPR_DrawAdditive(0, x, y - iDollarHeight * 1.5, &m_hPlus.rect );
			}
			else if( m_iDelta < 0)//buy enough money
			{
				r = (RGB_REDISH & 0xFF0000) >> 16;
				g = ((RGB_REDISH & 0xFF00) >> 8) + interpolate * (((RGB_YELLOWISH & 0xFF00) >> 8) - ((RGB_REDISH & 0xFF00) >> 8));
				b = (RGB_REDISH & 0xFF) - interpolate * (RGB_REDISH & 0xFF);

				SPR_Set(m_hMinus.spr, iDeltaR, iDeltaG, iDeltaB );
				if (gHUD.m_csgohud->value)
					SPR_DrawAdditive(0, x, y + iDollarHeight * 1.5, &m_hMinus.rect );
				else
					SPR_DrawAdditive(0, x, y - iDollarHeight * 1.5, &m_hMinus.rect);
			}
			if (gHUD.m_csgohud->value)
			{
				if (m_iDelta < 0)
				DrawUtils::DrawHudNumber2(x + iDollarWidth, y + iDollarHeight * 1.5, false, 5,
					m_iDelta < 0 ? -m_iDelta : m_iDelta,
					iDeltaR, iDeltaG, iDeltaB);
				else if (m_iDelta > 0)
					DrawUtils::DrawHudNumber2(x + iDollarWidth, y - iDollarHeight * 1.5, false, 5,
						m_iDelta < 0 ? -m_iDelta : m_iDelta,
						iDeltaR, iDeltaG, iDeltaB);
			}
			else
				DrawUtils::DrawHudNumber2(x + iDollarWidth, y - iDollarHeight * 1.5, false, 5,
					m_iDelta < 0 ? -m_iDelta : m_iDelta,
					iDeltaR, iDeltaG, iDeltaB);
			if (!gHUD.m_csgohud->value)
				FillRGBA(x + iDollarWidth / 4, y - iDollarHeight * 1.5 + gHUD.m_iFontHeight / 4, 2, 2, iDeltaR, iDeltaG, iDeltaB, iDeltaAlpha );
		}
		else DrawUtils::UnpackRGB(r, g, b, gHUD.m_csgohud->value? RGB_WHITE : RGB_YELLOWISH );
	}

	alphaBalance = 255 - interpolate * (255 - MIN_ALPHA);

	if (gHUD.m_csgohud->value)
		alphaBalance = 255;
	DrawUtils::ScaleColors( r, g, b, alphaBalance );

	if (gHUD.m_csgohud->value)
		SPR_Set(m_hDollar.spr, 255, 255, 255);
	else
		SPR_Set(m_hDollar.spr, r, g, b);
	SPR_DrawAdditive(0, x, y, &m_hDollar.rect);
	if (gHUD.m_csgohud->value)
	{
		DrawUtils::DrawHudNumber2(x + iDollarWidth, y, false, 5, m_iMoneyCount, 255, 255, 255);
		FillRGBA(x + iDollarWidth / 4, y + gHUD.m_iFontHeight / 4, 2, 2, r, g, b, 255);
	}
	else
	{
		DrawUtils::DrawHudNumber2(x + iDollarWidth, y, false, 5, m_iMoneyCount, r, g, b);
		FillRGBA(x + iDollarWidth / 4, y + gHUD.m_iFontHeight / 4, 2, 2, r, g, b, alphaBalance);
	}
	return 1;
}

int CHudMoney::MsgFunc_Money(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf( pszName, pbuf, iSize );
	int iOldCount = m_iMoneyCount;
	m_iMoneyCount = buf.ReadLong();
	m_iDelta = m_iMoneyCount - iOldCount;
	m_fFade = 5.0f; //fade for 5 seconds
	m_iFlags |= HUD_DRAW;
	return 1;
}

int CHudMoney::MsgFunc_BlinkAcct(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf( pszName, pbuf, iSize );

	m_iBlinkAmt = buf.ReadByte();
	m_fBlinkTime = 0;
	return 1;
}
