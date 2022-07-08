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
