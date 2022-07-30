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
DECLARE_MESSAGE(m_SpecialCrossHair, SpecialCrossHair)

enum
{
	WPN_DGAXE,
	WPN_HUNTBOW,
	WPN_SKULLT9,
	WPN_WHIPSWORD,
	WPN_Y22S2SFSWORD,
	WPN_PIANOGUNEX,
	WPN_DIVINETITAN,
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
	HUNTBOW_AIM_HIT_RIGHT,
	SKULLT9_AIM_OFF,
	SKULLT9_AIM_ON,
	WHIPSWORD_AIM_OFF,
	WHIPSWORD_AIM_ON,
	Y22S2SFSWORD_AIM_ON,
	PIANOGUNEX_CRITICAL,
	PIANOGUNEX_HEAL,
	PIANOGUNEX_NULL,
	DIVINETITAN_ON,
	DIVINETITAN_BMODE,
	DIVINETITAN_FULL,
};

int CHudSpecialCrossHair::Init()
{
	gHUD.AddHudElem(this);
	m_iFlags |= HUD_DRAW;
	HOOK_MESSAGE(SpecialCrossHair);
	iType = 3;
	return 1;
}

int CHudSpecialCrossHair::MsgFunc_SpecialCrossHair(const char* pszName, int iSize, void* pbuf)
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

int CHudSpecialCrossHair::VidInit()
{
	m_iFlags |= HUD_DRAW;
	R_InitTexture(m_pCurTexture[0], "sprites/dgaxe_aim_off.tga");
	R_InitTexture(m_pCurTexture[1], "sprites/dgaxe_aim_on.tga");
	R_InitTexture(m_pCurTexture[2], "sprites/dgaxe_aim_on_2.tga");
	R_InitTexture(m_pCurTexture[3], "sprites/huntbow_aim_default1.tga");	//center
	R_InitTexture(m_pCurTexture[4], "sprites/huntbow_aim_default2.tga");	//left
	R_InitTexture(m_pCurTexture[5], "sprites/huntbow_aim_default3.tga");	//right
	R_InitTexture(m_pCurTexture[6], "sprites/huntbow_aim_charging.tga");
	R_InitTexture(m_pCurTexture[7], "sprites/skullt9_aim_off.tga");
	R_InitTexture(m_pCurTexture[8], "sprites/skullt9_aim_on.tga");
	R_InitTexture(m_pCurTexture[9], "sprites/whipsword_aim_off.tga");
	R_InitTexture(m_pCurTexture[10], "sprites/whipsword_aim_on.tga");
	R_InitTexture(m_pCurTexture[11], "sprites/y22s2sfsword_aim_on.tga");

	R_InitTexture(m_pCurTexture[12], "resource/hud/buffclassskill_heal.tga");
	R_InitTexture(m_pCurTexture[13], "resource/hud/buff_critical.tga");

	R_InitTexture(m_pCurTexture[14], "sprites/divinetitan_aim_on.tga");
	//R_InitTexture(m_pCurTexture[15], "sprites/ef_divinetitan_bmode.tga");
	return 1;
}

void CHudSpecialCrossHair::Reset(void)
{
	iWeapon = DisplayTime = 0;
}

int CHudSpecialCrossHair::Draw(float flTime)
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

		switch (iType)
		{
		case DGAXE_AIM_OFF:m_pCurTexture[iType]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case DGAXE_AIM_ON:m_pCurTexture[iType]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case DGAXE_AIM_ON_2:m_pCurTexture[iType]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
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
		break;
	}
	case WPN_SKULLT9:
	{
		switch (iType)
		{
		case SKULLT9_AIM_OFF:
		case SKULLT9_AIM_ON:
		{
			wide = 92;
			height = 92;
			break;
		}
		case AIM_NO:return 1;
		}

		float x = ScreenWidth / 2;
		float y = ScreenHeight / 2;
		const float flScale = 0.0f;

		float iX, iY;

		iX = x - wide / 2;
		iY = y - height / 2;

		switch (iType)
		{
		case SKULLT9_AIM_OFF:m_pCurTexture[iType-4]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case SKULLT9_AIM_ON:m_pCurTexture[iType-4]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		default:
			break;
		}

		break;
	}
	case WPN_WHIPSWORD:
	{
		switch (iType)
		{
		case WHIPSWORD_AIM_OFF:
		case WHIPSWORD_AIM_ON:
		{
			wide = 220;
			height = 220;
			break;
		}
		case AIM_NO:return 1;
		}

		float x = ScreenWidth / 2;
		float y = ScreenHeight / 2;
		const float flScale = 0.0f;

		float iX, iY;

		iX = x - wide / 2;
		iY = y - height / 2;

		switch (iType)
		{
		case WHIPSWORD_AIM_OFF:m_pCurTexture[iType - 4]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case WHIPSWORD_AIM_ON:m_pCurTexture[iType - 4]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		default:
			break;
		}

		break;
	}
	case WPN_Y22S2SFSWORD:
	{
		wide = 220;
		height = 220;

		float x = ScreenWidth / 2;
		float y = ScreenHeight / 2;
		const float flScale = 0.0f;
		float iX, iY;
		iX = x - wide / 2;
		iY = y - height / 2;

		switch (iType)
		{
		case Y22S2SFSWORD_AIM_ON:m_pCurTexture[iType - 4]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		case AIM_NO:return 1;
		default:
			break;
		}


		break;
	}
	case WPN_PIANOGUNEX:
	{		
		wide = 96 * 3;
		height = 256 * 3;

		float x1 = 0;
		float x2 = ScreenWidth;
		float y = ScreenHeight;
		const float flScale = 0.0f;

		float iX1, iY, iX2;
		//Left UI
		iX1 = x1;
		iY = y - height;

		//Right UI
		iX2 = x2;

		//Fade
		float flAlpha;
		float flFadeTime = flTime;
		flFadeTime -= (int)flFadeTime;
		flFadeTime -= 0.5;
		flFadeTime = fabsf(flFadeTime);

		//How long to fade?
		flAlpha = flFadeTime + 0.5;
		flAlpha *= 255;

		float flMaxOffset;
		flMaxOffset = ScreenHeight * 0.15;

		float flMoved;
		float flMoving = flTime;
		flMoving -= (int)flMoving;
		flMoving -= 0.005;
		flMoving = fabsf(flMoving);
		flMoved = flMoving + 0.005;
		flMoved *= ScreenHeight * 0.015;

		if (flMoved >= flMaxOffset)
		{
			flMoved = 0;
		}

		float iY4,iY3;
		if(flMoved)
			iY3 = iY4 = iY - flMoved;
		else iY3 = iY4 = iY;

		switch (iType)
		{
		case PIANOGUNEX_CRITICAL:
		{
			m_pCurTexture[13]->Draw2DQuadScaled(iX1, iY3, iX1 + wide, iY3 + height, 0, 0, 1, 1, 255, 255, 255, flAlpha);
			m_pCurTexture[13]->Draw2DQuadScaled(iX2, iY4, iX2 - wide, iY4 + height, 0, 0, 1, 1, 255, 255, 255, flAlpha);

			break;
		}
		case PIANOGUNEX_HEAL:
		{
			m_pCurTexture[12]->Draw2DQuadScaled(iX1, iY3, iX1 + wide, iY3 + height, 0, 0, 1, 1, 255, 255, 255, flAlpha);
			m_pCurTexture[12]->Draw2DQuadScaled(iX2, iY4, iX2 - wide, iY4 + height, 0, 0, 1, 1, 255, 255, 255, flAlpha);

			break;
		}
		case PIANOGUNEX_NULL:
		{
			return 1;
		}
		}
		break;
	}
	case WPN_DIVINETITAN:
	{
		wide = 200;
		height = 200;

		float x = ScreenWidth / 2;
		float y = ScreenHeight / 2;
		const float flScale = 0.0f;
		float iX, iY;
		float iX2, iY2;
		iX = x - wide / 2;
		iY = y - height / 2;

		float wide2, height2;
		wide2 = 752;
		height2 = 752;
		iX2 = x - wide2 / 2;
		iY2 = y - height2 / 2;

		switch (iType)
		{
		case DIVINETITAN_ON:m_pCurTexture[14]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
		//case DIVINETITAN_BMODE:m_pCurTexture[15]->Draw2DQuadScaled(iX2, iY2, iX + wide2, iY + height2); break;
		case DIVINETITAN_FULL :
		{
			m_pCurTexture[14]->Draw2DQuadScaled(iX, iY, iX + wide, iY + height); break;
			//m_pCurTexture[15]->Draw2DQuadScaled(iX2, iY2, iX + wide2, iY + height2); break;
		}
		case AIM_NO:return 1;
		default:
			break;
		}



	}
	}



	return 1;
}

void CHudSpecialCrossHair::DrawHuntbowCrossHair(float x, float y, float wide, float height, int iType)
{
	switch (iType)
	{
	case HUNTBOW_AIM_IDLE:
	{
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height);
			}

		}
		break;
	}
	case HUNTBOW_AIM_HIT:
	{
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height);
			}

		}
		break;
	}
	case HUNTBOW_HIT_LEFT:
	{
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if(i == 4)
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 0, 1, 1, 255, 0, 0, 255);
				else
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 0, 1, 1, 255, 255, 255, 255);
			}
		}
		break;
	}
	case HUNTBOW_HIT_RIGHT:
	{
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if (i == 5)
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 0, 1, 1, 255, 0, 0, 255);
				else
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 0, 1, 1, 255, 255, 255, 255);
			}
		}
		break;
	}
	case HUNTBOW_AIM_CHARGIN:
	{
		if (m_pCurTexture[6])
		{
			m_pCurTexture[6]->Draw2DQuadScaled(x, y, x + wide, y + height);
		}
		break;
	}
	case HUNTBOW_AIM_HIT_LEFT:
	{
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if (i == 3)
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 0, 1, 1, 255, 255, 255, 255);
				else if (i == 4)
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 0, 1, 1, 255, 0, 0, 255);
				else
				m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 0, 1, 1, 0, 255, 0, 255);
			}
		}
		break;
	}
	case HUNTBOW_AIM_HIT_RIGHT:
	{
		for (int i = 3; i <= 5; i++)
		{
			if (m_pCurTexture[i])
			{
				if (i == 3)
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 255, 255, 255, 255);
				else if (i == 5)
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 255, 0, 0, 255);
				else
					m_pCurTexture[i]->Draw2DQuadScaled(x, y, x + wide, y + height, 0, 255, 0, 255);
			}
		}
		break;
	}
	}
}



void CHudSpecialCrossHair::Shutdown()
{
	std::fill(std::begin(m_pCurTexture), std::end(m_pCurTexture), nullptr);
}

}
