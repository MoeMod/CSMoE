/*
hud_overlays.cpp - HUD Overlays
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

#include "hud.h"
#include "r_efx.h"
#include "cl_util.h"

#include "parsemsg.h"
#include "draw_util.h"

#include <string.h>
#include <stdio.h>

namespace cl {

DECLARE_MESSAGE(m_ShowWin, ShowWin)

enum
{
	CT_WIN,
	T_WIN,
	HUMAN_WIN,
	ZOMBIE_WIN
};

int CHudShowWin::Init()
{
	gHUD.AddHudElem(this);
	m_iFlags |= HUD_DRAW;
	HOOK_MESSAGE(ShowWin);

	return 1;
}

int CHudShowWin::MsgFunc_ShowWin(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);

	iType = reader.ReadByte();

	m_flDisplayTime = gHUD.m_flTime + reader.ReadByte();

	return 1;
}

int CHudShowWin::VidInit()
{
	m_iFlags |= HUD_DRAW;
	R_InitTexture(m_pCurTexture[0], "resource/basic/ctwin.tga");
	R_InitTexture(m_pCurTexture[1], "resource/basic/trwin.tga");
	R_InitTexture(m_pCurTexture[2], "resource/zombi/humanwin.tga");
	R_InitTexture(m_pCurTexture[3], "resource/zombi/zombiewin.tga");

	return 1;
}

void CHudShowWin::Reset(void)
{
	iType = 0;
	m_flDisplayTime = 0.0f;
}

int CHudShowWin::Draw(float flTime)
{
	if (!m_pCurTexture)
		return 1;

	if (gHUD.m_flTime > m_flDisplayTime)
		return 1;


	int x = ScreenWidth / 2;
	int y = ScreenHeight / 2;
	const float flScale = 0.0f;

	int iX, iY;


	switch (iType)
	{
		case CT_WIN:
		{
			wide = 330;
			height = 181;
			break;
		}
		case T_WIN:
		{
			wide = 330;
			height = 181;
			break;
		}
		case HUMAN_WIN:
		{
			wide = 450;
			height = 183;
			break;
		}
		case ZOMBIE_WIN:
		{
			wide = 450;
			height = 183;
			break;
		}
		default:
			break;
	}

	iX = x - wide / 2;
	iY = y - height / 2;

	switch (iType)
	{
	case CT_WIN:m_pCurTexture[iType]->Draw2DQuadScaled(iX, iY * 0.35, iX + wide, iY * 0.35 + height); break;
	case T_WIN:m_pCurTexture[iType]->Draw2DQuadScaled(iX, iY * 0.35, iX + wide, iY * 0.35 + height); break;
	case HUMAN_WIN:m_pCurTexture[iType]->Draw2DQuadScaled(iX, iY * 0.35, iX + wide, iY * 0.35 + height); break;
	case ZOMBIE_WIN:m_pCurTexture[iType]->Draw2DQuadScaled(iX, iY * 0.35, iX + wide, iY * 0.35 + height); break;
	default:
		break;
	}


	return 1;
}




void CHudShowWin::Shutdown()
{
	m_pCurTexture[0] = nullptr;
	m_pCurTexture[1] = nullptr;
	m_pCurTexture[2] = nullptr;
	m_pCurTexture[3] = nullptr;
}

}