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
	WPN_DGAXE,
	WPN_HUNTBOW
};

enum
{
	DGAXE_AIM_OFF,
	DGAXE_AIM_ON,
	DGAXE_AIM_ON_2,
	AIM_NO,
	HUNTBOW_AIM_IDLE,
	HUNTBOW_AIM_HIT,
	HUNTBOW_HIT_LEFT,
	HUNTBOW_HIT_RIGHT,
	HUNTBOW_AIM_CHARGIN,
	HUNTBOW_AIM_HIT_LEFT,
	HUNTBOW_AIM_HIT_RIGHT
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

	iWeapon = reader.ReadByte();
	iType = reader.ReadByte();
	switch (iType)
	{
	case HUNTBOW_HIT_LEFT:
	{
		iStoredType = HUNTBOW_HIT_LEFT;
		DisplayTime = gHUD.m_flTime + 0.5;
		break;
	}
	case HUNTBOW_HIT_RIGHT:
	{
		iStoredType = HUNTBOW_HIT_RIGHT;
		DisplayTime = gHUD.m_flTime + 0.5;
		break;
	}
	case HUNTBOW_AIM_HIT_LEFT:
	{
		iStoredType = HUNTBOW_AIM_HIT_LEFT;
		DisplayTime = gHUD.m_flTime + 0.5;
		break;
	}
	case HUNTBOW_AIM_HIT_RIGHT:
	{
		iStoredType = HUNTBOW_AIM_HIT_RIGHT;
		DisplayTime = gHUD.m_flTime + 0.5;
		break;
	}
	}

	return 1;
}

int CHudDgaxe::VidInit()
{
	m_iFlags |= HUD_DRAW;
	R_InitTexture(m_pCurTexture[0], "sprites/dgaxe_aim_off.tga");
	R_InitTexture(m_pCurTexture[1], "sprites/dgaxe_aim_on.tga");
	R_InitTexture(m_pCurTexture[2], "sprites/dgaxe_aim_on_2.tga");
	R_InitTexture(m_pCurTexture[3], "sprites/huntbow_aim_default1.tga");	//center
	R_InitTexture(m_pCurTexture[4], "sprites/huntbow_aim_default2.tga");	//left
	R_InitTexture(m_pCurTexture[5], "sprites/huntbow_aim_default3.tga");	//right
	R_InitTexture(m_pCurTexture[6], "sprites/huntbow_aim_charging.tga");
	return 1;
}

void CHudDgaxe::Reset(void)
{
	DisplayTime = 0;
	std::fill(std::begin(m_pCurTexture), std::end(m_pCurTexture), nullptr);
	VidInit();
}

int CHudDgaxe::Draw(float flTime)
{
	if (!m_pCurTexture)
		return 1;

	switch (iWeapon)
	{
	case WPN_DGAXE:
	{
		switch (iType)
		{
		case DGAXE_AIM_OFF:
		case DGAXE_AIM_ON:
		case DGAXE_AIM_ON_2:
		{
			wide = 200;
			height = 200; break;
		}
		}

		float x = ScreenWidth / 2;
		float y = ScreenHeight / 2;
		const float flScale = 0.0f;

		float iX, iY;

		iX = x - wide / 2;
		iY = y - height / 2;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
		if (iType != 3)
			m_pCurTexture[iType]->Bind();

		switch (iType)
		{
		case DGAXE_AIM_OFF:DrawUtils::Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case DGAXE_AIM_ON:DrawUtils::Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case DGAXE_AIM_ON_2:DrawUtils::Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case AIM_NO: return 1; break;
		default:
			break;
		}

		break;
	}
	case WPN_HUNTBOW:
	{
		wide = 90;
		height = 90;

		float x = ScreenWidth / 2;
		float y = ScreenHeight / 2;
		const float flScale = 0.0f;

		float iX, iY;

		iX = x - wide / 2;
		iY = y - height / 2;

		if(iType == AIM_NO)
		{
			return 1;
		}
		if(DisplayTime > gHUD.m_flTime)
			DrawHuntbowCrossHair(iX, iY, wide, height, iStoredType);
		else
			DrawHuntbowCrossHair(iX, iY, wide, height, iType);
	}
	}

	

	return 1;
}

void CHudDgaxe::DrawHuntbowCrossHair(float x, float y, float wide, float height, int iType)
{
	switch (iType)
	{
	case HUNTBOW_AIM_IDLE:
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				m_pCurTexture[i]->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + wide, y + height);
			}

		}
		break;
	}
	case HUNTBOW_AIM_HIT:
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->Color4ub(0, 255, 0, 255);
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				m_pCurTexture[i]->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + wide, y + height);
			}

		}
		break;
	}
	case HUNTBOW_HIT_LEFT:
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if(i == 4)
					gEngfuncs.pTriAPI->Color4ub(255, 0, 0, 255);
				else
					gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
				m_pCurTexture[i]->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + wide, y + height);
			}
		}
		break;
	}
	case HUNTBOW_HIT_RIGHT:
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if (i == 5)
					gEngfuncs.pTriAPI->Color4ub(255, 0, 0, 255);
				else
					gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
				m_pCurTexture[i]->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + wide, y + height);
			}
		}
		break;
	}
	case HUNTBOW_AIM_CHARGIN:
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
		if (m_pCurTexture[6])
		{
			m_pCurTexture[6]->Bind();
			DrawUtils::Draw2DQuadScaled(x, y, x + wide, y + height);
		}
		break;
	}
	case HUNTBOW_AIM_HIT_LEFT:
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if (i == 3)
					gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
				else if (i == 4)
					gEngfuncs.pTriAPI->Color4ub(255, 0, 0, 255);
				else
					gEngfuncs.pTriAPI->Color4ub(0, 255, 0, 255);
				m_pCurTexture[i]->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + wide, y + height);
			}
		}
		break;
	}
	case HUNTBOW_AIM_HIT_RIGHT:
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if (i == 3)
					gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
				else if (i == 5)
					gEngfuncs.pTriAPI->Color4ub(255, 0, 0, 255);
				else
					gEngfuncs.pTriAPI->Color4ub(0, 255, 0, 255);
				m_pCurTexture[i]->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + wide, y + height);
			}
		}
		break;
	}
	}
}



void CHudDgaxe::Shutdown()
{
	std::fill(std::begin(m_pCurTexture), std::end(m_pCurTexture), nullptr);
}

