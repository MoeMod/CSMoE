/*
timer.cpp -- HUD timer, progress bars, etc
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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include "draw_util.h"
#include "gamemode/mods_const.h"
#include "legacy/hud_scoreboard_legacy.h"

#include "vgui_controls/controls.h"
#include "vgui/ILocalize.h"

namespace cl {

DECLARE_MESSAGE( m_Timer, RoundTime )
DECLARE_MESSAGE( m_Timer, ShowTimer )

CHudTimer::CHudTimer()
{
	CHudScoreBoardLegacy::BuildNumberRC(m_iNum_BottomC, 8, 12);
}

int CHudTimer::Init()
{
	HOOK_MESSAGE( RoundTime );
	HOOK_MESSAGE( ShowTimer );
	m_iFlags = 0;
	m_bPanicColorChange = false;
	gHUD.AddHudElem(this);
	return 1;
}

void CHudTimer::Reset(void)
{
	m_closestRight = ScreenWidth;
}

int CHudTimer::VidInit()
{
	m_HUD_timer = gHUD.GetSpriteIndex( "stopwatch" );
	R_InitTexture(m_pTexture_Black, "resource/hud/csgo/black");

	if (!m_iNum_Bottom)
		m_iNum_Bottom = R_LoadTextureShared("resource/hud/hud_sb_num_bottom");

	if (!m_iColon_Bottom)
		m_iColon_Bottom = R_LoadTextureUnique("resource/hud/hud_sb_num_bottom_colon");

	return 1;
}

int CHudTimer::DrawNEWHudTimer(float fTime)
{
	int r, g, b;
	// time must be positive
	int minutes = max(0, (int)(m_iTime + m_fStartTime - gHUD.m_flTime) / 60);
	int seconds = max(0, (int)(m_iTime + m_fStartTime - gHUD.m_flTime) - (minutes * 60));

	if (minutes * 60 + seconds > 20)
	{
		DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
	}
	else
	{
		m_flPanicTime += gHUD.m_flTimeDelta;
		// add 0.1 sec, so it's not flicker fast
		if (m_flPanicTime > ((float)seconds / 40.0f) + 0.1f)
		{
			m_flPanicTime = 0;
			m_bPanicColorChange = !m_bPanicColorChange;
		}
		DrawUtils::UnpackRGB(r, g, b, m_bPanicColorChange ? RGB_WHITE : RGB_REDISH);
	}

	int iX, iY;
	iX = ScreenWidth / 2 - 20;
	iY = 60;
	DrawUtils::ScaleColors(r, g, b, 255);

	CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Bottom, m_iNum_BottomC, minutes, iX, iY, DHN_2DIGITS, 1, 1.0, r, g, b);

	int iH = m_iColon_Bottom->h();	//10
	int iW = m_iColon_Bottom->w();	//3

	iX = ScreenWidth / 2 - 1;
	m_iColon_Bottom->Draw2DQuadScaled(iX, iY, iX + iW, iY + iH, 0.0f, 0.0f, 1.0f, 1.0f, r, g, b);

	CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Bottom, m_iNum_BottomC, seconds, iX + iW + 1, iY, DHN_2DIGITS, 1, 1.0, r, g, b);

	return 1;
}

int CHudTimer::Draw( float fTime )
{
	if ( ( gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH ) )
        return 1;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;
	if (gHUD.m_hudstyle->value == 2 && (gHUD.m_iModRunning == MOD_NONE || gHUD.m_iModRunning == MOD_DM || gHUD.m_iModRunning == MOD_TDM || gHUD.m_iModRunning == MOD_ZB1 || gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZBZ))
	{
		DrawNEWHudTimer(fTime);
		return 1;
	}

	int r, g, b;
	// time must be positive
	int minutes = max( 0, (int)( m_iTime + m_fStartTime - gHUD.m_flTime ) / 60);
	int seconds = max( 0, (int)( m_iTime + m_fStartTime - gHUD.m_flTime ) - (minutes * 60));

	if( minutes * 60 + seconds > 20 )
	{
		DrawUtils::UnpackRGB(r,g,b, gHUD.m_hudstyle->value == 1 ? RGB_WHITE : RGB_YELLOWISH );
	}
	else
	{
		m_flPanicTime += gHUD.m_flTimeDelta;
		// add 0.1 sec, so it's not flicker fast
		if (gHUD.m_hudstyle->value == 1)
			DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
		else
		{
			{
				if (m_flPanicTime > ((float)seconds / 40.0f) + 0.1f)
				{
					m_flPanicTime = 0;
					m_bPanicColorChange = !m_bPanicColorChange;
				}
			}
			DrawUtils::UnpackRGB(r, g, b, m_bPanicColorChange ? RGB_YELLOWISH : RGB_REDISH);
		}
	}

	DrawUtils::ScaleColors( r, g, b,  MIN_ALPHA );


    int iWatchWidth = gHUD.GetSpriteRect(m_HUD_timer).right - gHUD.GetSpriteRect(m_HUD_timer).left;
	int iNumberWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

	int y = ScreenHeight - 1.5 * gHUD.m_iFontHeight ;

	if (gHUD.m_hudstyle->value == 1 && gHUD.m_iModRunning == MOD_NONE)
	{
		int x = ScreenWidth / 2;

		y = 5;
		m_pTexture_Black->Draw2DQuadScaled(x - 120 / 2, y, x + 120 / 2, y + 70);

		y = 17;
		x = DrawUtils::DrawHudNumber2(x - 46, y, true, 2, minutes, r * 255, g * 255, b * 255);
		// draw :
		FillRGBA(x + iWatchWidth / 4 , y + gHUD.m_iFontHeight / 4, 2, 2, r, g, b, 255);
		FillRGBA(x + iWatchWidth / 4, y + gHUD.m_iFontHeight - gHUD.m_iFontHeight / 4, 2, 2, r, g, b, 255);


		x = DrawUtils::DrawHudNumber2(x + iWatchWidth / 2, y, true, 2, seconds, r * 255, g * 255, b * 255);

		m_closestRight = x + (iNumberWidth) * 1.5;
	}
	else
	{
		int space = iWatchWidth / 4;
		int x = ScreenWidth / 2 - 2 * space - iNumberWidth * 2;
		SPR_Set(gHUD.GetSprite(m_HUD_timer), r, g, b);
		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_timer));

		x += iWatchWidth;
		x = DrawUtils::DrawHudNumber2(x, y, false, 2, minutes, r, g, b);

		// draw :
		x += space;
		FillRGBA(x, y + gHUD.m_iFontHeight / 4, 2, 2, r, g, b, 100);
		FillRGBA(x, y + gHUD.m_iFontHeight - gHUD.m_iFontHeight / 4, 2, 2, r, g, b, 100);

		x += space;
		x = DrawUtils::DrawHudNumber2(x, y, true, 2, seconds, r, g, b);

		m_closestRight = x + (iNumberWidth) * 1.5;
	}

	return 1;
}

int CHudTimer::MsgFunc_RoundTime(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );
	m_iTime = reader.ReadShort();
	m_fStartTime = gHUD.m_flTime;
	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudTimer::MsgFunc_ShowTimer(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags = HUD_DRAW;
	return 1;
}

#define UPDATE_BOTPROGRESS 0
#define CREATE_BOTPROGRESS 1
#define REMOVE_BOTPROGRESS 2

DECLARE_MESSAGE( m_ProgressBar, BarTime )
DECLARE_MESSAGE( m_ProgressBar, BarTime2 )
DECLARE_MESSAGE( m_ProgressBar, BotProgress )

int CHudProgressBar::Init()
{
	HOOK_MESSAGE( BarTime );
	HOOK_MESSAGE( BarTime2 );
	HOOK_MESSAGE( BotProgress );
	m_iFlags = 0;
	m_szLocalizedHeader = NULL;
	m_szHeader[0] = '\0';
	m_fStartTime = m_fPercent = 0.0f;

	gHUD.AddHudElem(this);
	return 1;
}

int CHudProgressBar::VidInit()
{
	return 1;
}

int CHudProgressBar::Draw( float flTime )
{
	// allow only 0.0..1.0
	if( (m_fPercent < 0.0f) || (m_fPercent > 1.0f) )
	{
		m_iFlags = 0;
		m_fPercent = 0.0f;
		return 1;
	}

	if( m_szLocalizedHeader && m_szLocalizedHeader[0] )
	{
		int r, g, b;
		DrawUtils::UnpackRGB( r, g, b, RGB_YELLOWISH );
		DrawUtils::DrawHudString( ScreenWidth / 4, ScreenHeight / 2, ScreenWidth, (char*)m_szLocalizedHeader, r, g, b );

		DrawUtils::DrawRectangle( ScreenWidth/ 4, ScreenHeight / 2 + gHUD.GetCharHeight(), ScreenWidth/2, ScreenHeight/30 );
		FillRGBA( ScreenWidth/4+2, ScreenHeight/2 + gHUD.GetCharHeight() + 2, m_fPercent * (ScreenWidth/2-4), ScreenHeight/30-4, 255, 140, 0, 255 );
		return 1;
	}

	// prevent SIGFPE
	if( m_iDuration != 0.0f )
	{
		m_fPercent = ((flTime - m_fStartTime) / m_iDuration);
	}
	else
	{
		m_fPercent = 0.0f;
		m_iFlags = 0;
		return 1;
	}

	DrawUtils::DrawRectangle( ScreenWidth/4, ScreenHeight*2/3, ScreenWidth/2, 10 );
	FillRGBA( ScreenWidth/4+2, ScreenHeight*2/3+2, m_fPercent * (ScreenWidth/2-4), 6, 255, 140, 0, 255 );

	return 1;
}

int CHudProgressBar::MsgFunc_BarTime(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	m_iDuration = reader.ReadShort();
	m_fPercent = 0.0f;

	m_fStartTime = gHUD.m_flTime;

	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudProgressBar::MsgFunc_BarTime2(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	m_iDuration = reader.ReadShort();
	m_fPercent = m_iDuration * (float)reader.ReadShort() / 100.0f;

	m_fStartTime = gHUD.m_flTime;

	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudProgressBar::MsgFunc_BotProgress(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );
	m_iDuration = 0.0f; // don't update our progress bar
	m_iFlags = HUD_DRAW;

	float fNewPercent;
	int flag = reader.ReadByte();
	switch( flag )
	{
	case UPDATE_BOTPROGRESS:
	case CREATE_BOTPROGRESS:
		fNewPercent = (float)reader.ReadByte() / 100.0f;
		// cs behavior:
		// just don't decrease percent values
		if( m_fPercent < fNewPercent )
		{
			m_fPercent = fNewPercent;
		}
		strncpy(m_szHeader, reader.ReadString(), sizeof(m_szHeader));
		if( m_szHeader[0] == '#' )
        {
            vgui2::localize()->ConvertUnicodeToANSI(vgui2::localize()->Find(m_szHeader), m_szHeader, sizeof(m_szHeader));
        }
            m_szLocalizedHeader = m_szHeader;
		break;
	case REMOVE_BOTPROGRESS:
	default:
		m_fPercent = 0.0f;
		m_szHeader[0] = '\0';
		m_iFlags = 0;
		m_szLocalizedHeader = NULL;
		break;
	}

	return 1;
}

}