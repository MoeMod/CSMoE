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


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "draw_util.h"
#include "cs_wpn/bte_weapons.h"

namespace cl {

enum drawscope_type
{
	TYPE_QUADRD_SQUARE = 0,
	TYPE_DOUBLE_SQUARE,
	TYPE_DOUBLE_SQUARE_NOTCENTER
};

enum drawcorner_type
{
	TYPE_CORNER_SW = 0,
	TYPE_CORNER_SE,
	TYPE_CORNER_NW,
	TYPE_CORNER_NE
};

int CHudSniperScope::Init()
{
	gHUD.AddHudElem(this);
	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudSniperScope::VidInit()
{
	R_InitTexture(m_iScopeArc[0], "sprites/scope_arc_nw.tga");
	R_InitTexture(m_iScopeArc[1], "sprites/scope_arc_ne.tga");
	R_InitTexture(m_iScopeArc[2], "sprites/scope_arc.tga");
	R_InitTexture(m_iScopeArc[3], "sprites/scope_arc_sw.tga");

	R_InitTexture(m_iM95TigerScope[0], "sprites/m95tiger_scope_red.tga");
	R_InitTexture(m_iM95TigerScope[1], "sprites/m95tiger_scope_yellow.tga");
	R_InitTexture(m_iM95TigerScopeCorner, "sprites/m95tiger_scope_corner.tga");
	R_InitTexture(m_iCheytaclrrsScope, "sprites/scope_m200.tga");
	R_InitTexture(m_iFreedomScope, "sprites/scope_freedom.astc");
	//R_InitTexture(m_iSprifleScope, "sprites/scope_circle.tga");
	
	left = (TrueWidth - TrueHeight)/2;
	right = left + TrueHeight;
	centerx = TrueWidth/2;
	centery = TrueHeight/2;
	return 1;
}

void CHudSniperScope::FuncDraw2DQuadScaled(const CTextureRef& tex, int x, int y, int width, int height, float s1, float t1, float s2, float t2, byte r, byte g, byte b, byte a)
{
	tex.Draw2DQuadScaled(x, y, x + width, y + height, s1, t1, s2, t2, r, g, b, a);
}

void CHudSniperScope::FuncDrawCorner(const CTextureRef& tex, int type, int width, int height, int width2, int height2)
{
	switch (type)
	{
	case 0:
	{
		FuncDraw2DQuadScaled(tex, width2, ScreenHeight - height2 - height, width, height);
		FuncDraw2DQuadScaled(tex, width2, height + height2, width, -height);
		FuncDraw2DQuadScaled(tex, ScreenWidth - width2, height + height2, -width, -height);
		FuncDraw2DQuadScaled(tex, ScreenWidth - width2, ScreenHeight - height - height2, -width, height);
		break;
	}
	case 1:
	{
		FuncDraw2DQuadScaled(tex, ScreenWidth - width2 - width, ScreenHeight - height2 - height, width, height);
		FuncDraw2DQuadScaled(tex, ScreenWidth - width2 - width, height2, width, -height);
		FuncDraw2DQuadScaled(tex, width + width2, ScreenHeight - height2 - height, -width, height);
		FuncDraw2DQuadScaled(tex, width + width2, height2, -width, -height);
		break;
	}
	case 2:
	{
		FuncDraw2DQuadScaled(tex, width2, height2, width, height, height);
		FuncDraw2DQuadScaled(tex, width2, ScreenHeight - height2, width, -height);
		FuncDraw2DQuadScaled(tex, ScreenWidth - width2, height2, -width, height);
		FuncDraw2DQuadScaled(tex, ScreenWidth - width2, ScreenHeight - height2, -width, -height);
		break;
	}
	case 3:
	{
		FuncDraw2DQuadScaled(tex, ScreenWidth - width - width2, height2, width, height);
		FuncDraw2DQuadScaled(tex, ScreenWidth - width - width2, ScreenHeight - height2, width, -height);
		FuncDraw2DQuadScaled(tex, width + width2, height2, -width, height);
		FuncDraw2DQuadScaled(tex, width + width2, ScreenHeight - height2, -width, -height);
		break;
	}
	}
}

void CHudSniperScope::FuncDrawScope(const CTextureRef& tex, int type, int x, int y, int width, int height)
{
	switch (type)
	{
	case 0:
	{
		int iX, iY;
		iX = x - width;
		iY = y - height;
		tex.Draw2DQuadScaled(iX, iY, iX + iX + width, iY + iY + height);
		iY = ScreenHeight / 2 + height;
		tex.Draw2DQuadScaled(iX, iY, iX + iX + width, iY + iY - height);
		iX = ScreenWidth / 2 + width;
		iY = ScreenHeight / 2 - height;
		tex.Draw2DQuadScaled(iX, iY, iX + iX - width, iY + iY + height);
		iY = ScreenHeight / 2 + height;
		tex.Draw2DQuadScaled(iX, iY, iX + iX - width, iY + iY - height);
		break;
	}
	case 1:
	{
		tex.Draw2DQuadScaled(x - width, y - height / 2, x - width + width, y - height / 2 + y + height / 2);
		tex.Draw2DQuadScaled(x + width, y - height / 2, x + width - width, y - height / 2 + y + height / 2);
		break;
	}
	case 2:
	{
		tex.Draw2DQuadScaled(x - width, y, x - width + width,  y + height);
		tex.Draw2DQuadScaled(x + width, y, x + width - width, y + height);
		break;
	}
	}
}

int CHudSniperScope::Draw(float flTime)
{
	CBasePlayerWeapon* pActiveBTEWeapon = BTEClientWeapons().GetActiveWeaponEntity();
	if (pActiveBTEWeapon)
	{
		if (pActiveBTEWeapon->m_iId == WEAPON_SKULL5 || pActiveBTEWeapon->m_iId == WEAPON_SKULL6)
		{
			if (gHUD.m_iFOV == 90)
				return 1;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_SFSNIPER)
		{
			return 1;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_M95TIGER || pActiveBTEWeapon->m_iId == WEAPON_CHEYTACLRRS || pActiveBTEWeapon->m_iId == WEAPON_M400 || pActiveBTEWeapon->m_iId == WEAPON_Z4B_FREEDOM)
		{
			if (gHUD.m_iFOV > 40)
				return 1;

			DrawSpecialScope(pActiveBTEWeapon->m_iId, pActiveBTEWeapon->pev->iuser1);
			return 0;
		}
		else
		{
			if (gHUD.m_iFOV > 40)
				return 1;
		}
	}
	else
	{
		if (gHUD.m_iFOV > 40)
			return 1;
	}

	m_iScopeArc[0]->Draw2DQuad(left, 0, centerx, centery, 0, 0, 1, 1, 0, 0, 0, 255);

	m_iScopeArc[1]->Draw2DQuad(centerx, 0, right, centery, 0, 0, 1, 1, 0, 0, 0, 255);

	m_iScopeArc[2]->Draw2DQuad(centerx, centery, right, TrueHeight, 0, 0, 1, 1, 0, 0, 0, 255);

	m_iScopeArc[3]->Draw2DQuad(left, centery, centerx, TrueHeight, 0, 0, 1, 1, 0, 0, 0, 255);

	FillRGBABlend( 0, 0, (ScreenWidth - ScreenHeight) / 2 + 2, ScreenHeight, 0, 0, 0, 255 );
	FillRGBABlend( (ScreenWidth - ScreenHeight) / 2 - 2 + ScreenHeight, 0, (ScreenWidth - ScreenHeight) / 2 + 2, ScreenHeight, 0, 0, 0, 255 );

	FillRGBABlend(0,                  ScreenHeight/2, ScreenWidth/2 - 20, 1,  0, 0, 0, 255);
	FillRGBABlend(ScreenWidth/2 + 20, ScreenHeight/2, ScreenWidth       , 1,  0, 0, 0, 255);

	FillRGBABlend(ScreenWidth/2, 0                  , 1, ScreenHeight/2 - 20, 0, 0, 0, 255);
	FillRGBABlend(ScreenWidth/2, ScreenHeight/2 + 20, 1, ScreenHeight       , 0, 0, 0, 255);

	return 0;
}

void CHudSniperScope::DrawSpecialScope(int iId, int iType)
{
	if (iId == WEAPON_M95TIGER)
	{
		float Height = min(ScreenWidth, ScreenHeight);
		float Width = (Height * m_iM95TigerScope[iType]->w() / m_iM95TigerScope[iType]->h());
		int iX = ScreenWidth / 2;
		int iY = ScreenHeight / 2;

		FuncDrawScope(*m_iM95TigerScope[iType], TYPE_DOUBLE_SQUARE, iX, iY, Width, Height);

		FillRGBABlend(0, 0, iX - Width, ScreenHeight, 0, 0, 0, 93);
		FillRGBABlend(iX + Width, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 93);

		FillRGBABlend(0, iY + Height / 2, ScreenWidth, ScreenHeight, 0, 0, 0, 93);
		FillRGBABlend(0, 0, ScreenWidth, iY - Height / 2, 0, 0, 0, 93);

		FuncDrawCorner(*m_iM95TigerScopeCorner, TYPE_CORNER_SW, Width, Height / 2);
	}
	if (iId == WEAPON_CHEYTACLRRS)
	{
		float x, y, w, h;

		float scale = ScreenHeight / 0.85 / m_iCheytaclrrsScope->h();

		w = m_iCheytaclrrsScope->w() * scale;
		h = m_iCheytaclrrsScope->h() * scale;

		x = ScreenWidth / 2 - w / 2;
		y = ScreenHeight / 2 - h / 2;

		m_iCheytaclrrsScope->Draw2DQuadScaled(x, y, x + w,  y + h, 0, 0, 1, 1, 255, 255, 255, 255);

		int LENGTH_SCOPE = w / 2;

		FillRGBABlend(0, 0, ScreenWidth / 2 - LENGTH_SCOPE, ScreenHeight, 0, 0, 0, 255);
		FillRGBABlend(ScreenWidth / 2 + LENGTH_SCOPE, 0, ScreenWidth / 2 - LENGTH_SCOPE, ScreenHeight, 0, 0, 0, 255);

		FillRGBABlend(0, 0, ScreenWidth, ScreenHeight / 2 - LENGTH_SCOPE, 0, 0, 0, 255);
		FillRGBABlend(0, ScreenHeight / 2 + LENGTH_SCOPE, ScreenWidth, ScreenHeight / 2 - LENGTH_SCOPE, 0, 0, 0, 255);
	}
	if (iId == WEAPON_M400)
	{
		m_iScopeArc[0]->Draw2DQuad(left, 0, centerx, centery, 0, 0, 1, 1, 0, 0, 0, 255);

		m_iScopeArc[1]->Draw2DQuad(centerx, 0, right, centery, 0, 0, 1, 1, 0, 0, 0, 255);

		m_iScopeArc[2]->Draw2DQuad(centerx, centery, right, TrueHeight, 0, 0, 1, 1, 0, 0, 0, 255);

		m_iScopeArc[3]->Draw2DQuad(left, centery, centerx, TrueHeight, 0, 0, 1, 1, 0, 0, 0, 255);

		FillRGBABlend(0, 0, (ScreenWidth - ScreenHeight) / 2 + 2, ScreenHeight, 0, 0, 0, 255);
		FillRGBABlend((ScreenWidth - ScreenHeight) / 2 - 2 + ScreenHeight, 0, (ScreenWidth - ScreenHeight) / 2 + 2, ScreenHeight, 0, 0, 0, 255);
	}
	if (iId == WEAPON_SPRIFLE)
	{

	}
	if (iId == WEAPON_Z4B_FREEDOM)
	{
		float x, y, w, h;

		float scale = ScreenHeight / 0.85 / m_iFreedomScope->h();

		w = m_iFreedomScope->w() * scale;
		h = m_iFreedomScope->h() * scale;

		x = ScreenWidth / 2 - w / 2;
		y = ScreenHeight / 2 - h / 2;

		m_iFreedomScope->Draw2DQuadScaled(x, y, x + w, y + h, 0, 0, 1, 1, 255, 255, 255, 255);

		int LENGTH_SCOPE = w / 2;

		FillRGBABlend(0, 0, ScreenWidth / 2 - LENGTH_SCOPE, ScreenHeight, 164, 246, 255, 100);
		FillRGBABlend(ScreenWidth / 2 + LENGTH_SCOPE, 0, ScreenWidth / 2 - LENGTH_SCOPE, ScreenHeight, 164, 246, 255, 100);

		FillRGBABlend(0, 0, ScreenWidth, ScreenHeight / 2 - LENGTH_SCOPE, 164, 246, 255, 100);
		FillRGBABlend(0, ScreenHeight / 2 + LENGTH_SCOPE, ScreenWidth, ScreenHeight / 2 - LENGTH_SCOPE, 164, 246, 255, 100);
	}

}

void CHudSniperScope::Shutdown()
{
	std::fill(std::begin(m_iScopeArc), std::end(m_iScopeArc), nullptr);
}

}