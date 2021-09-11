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
#include "triangleapi.h"
#include "r_efx.h"
#include "cl_util.h"

#include "parsemsg.h"
#include "draw_util.h"

#include <string.h>
#include <stdio.h>



DECLARE_MESSAGE(m_Dgaxe, Dgaxe)

enum
{
	DGAXE_AIM_OFF,
	DGAXE_AIM_ON,
	DGAXE_AIM_ON_2,
	DGAXE_AIM_NO
};

int CHudDgaxe::Init()
{
	gHUD.AddHudElem(this);
	m_iFlags |= HUD_DRAW;
	HOOK_MESSAGE(Dgaxe);
	iType = 3;
	return 1;
}

int CHudDgaxe::MsgFunc_Dgaxe(const char* pszName, int iSize, void* pbuf)
{
	m_iFlags |= HUD_DRAW;

	BufferReader reader(pszName, pbuf, iSize);

	iType = reader.ReadByte();


	return 1;
}

int CHudDgaxe::VidInit()
{
	m_iFlags |= HUD_DRAW;
	R_InitTexture(m_pCurTexture[0], "sprites/dgaxe_aim_off.tga");
	R_InitTexture(m_pCurTexture[1], "sprites/dgaxe_aim_on.tga");
	R_InitTexture(m_pCurTexture[2], "sprites/dgaxe_aim_on_2.tga");
	return 1;
}

void CHudDgaxe::Reset(void)
{
	std::fill(std::begin(m_pCurTexture), std::end(m_pCurTexture), nullptr);
	VidInit();
}

int CHudDgaxe::Draw(float flTime)
{
	if (!m_pCurTexture)
		return 1;

	wide = 200;
	height = 200;
	int x = ScreenWidth / 2;
	int y = ScreenHeight / 2;
	const float flScale = 0.0f;

	int iX, iY;

	iX = x - wide / 2;
	iY = y - height / 2;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
	if(iType != 3)
		m_pCurTexture[iType]->Bind();

	switch (iType)
	{
	case DGAXE_AIM_OFF:DrawUtils::Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
	case DGAXE_AIM_ON:DrawUtils::Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
	case DGAXE_AIM_ON_2:DrawUtils::Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
	case DGAXE_AIM_NO: return 1; break;
	default:
		break;
	}
	

	return 1;
}




void CHudDgaxe::Shutdown()
{
	std::fill(std::begin(m_pCurTexture), std::end(m_pCurTexture), nullptr);
}

