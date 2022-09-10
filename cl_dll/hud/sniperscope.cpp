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

	R_InitTexture(m_iAimBG, "sprites/kronos_aim_bg.tga");
	R_InitTexture(m_iAimFrame, "sprites/kronos_aim_frame.tga");
	R_InitTexture(m_iAimGauge, "sprites/kronos_aim_gauge.tga");

	R_InitTexture(m_iCrossbowex21_AimBG, "sprites/crossbowex21_bg.tga");
	R_InitTexture(m_iCrossbowex21_AimImage, "sprites/crossbowex21_aimimage_01.tga");

	R_InitTexture(m_iBUFFAWP_BG, "sprites/buffawp_bg.tga");
	R_InitTexture(m_iBUFFAWP_Bar, "sprites/buffawp_bar.tga");
	R_InitTexture(m_iBUFFAWP_Light[0], "sprites/buffawp_light_1.tga");
	R_InitTexture(m_iBUFFAWP_Light[1], "sprites/buffawp_light_2.tga");
	R_InitTexture(m_iBUFFAWP_Light[2], "sprites/buffawp_light_3.tga");
	R_InitTexture(m_iBUFFAWP_Outline, "sprites/buffawp_outline.tga");

	m_flScopeTime = 0.0f;
	m_flScopeTimeEnd = 0.0f;
	m_flBUFFAWPStartTime = 0.0f;

	R_InitTexture(m_iLockOnGun_BG, "sprites/lockongun_bg.tga");
	R_InitTexture(m_iLockOnGun_Top, "sprites/lockongun_top.tga");
	R_InitTexture(m_iLockOnGun_Center, "sprites/lockongun_center.tga");
	R_InitTexture(m_iLockOnGun_Bottom_BG, "sprites/lockongun_bottom_bg.tga");
	R_InitTexture(m_iLockOnGun_Bottom[0], "sprites/lockongun_bottom_etc.tga");
	R_InitTexture(m_iLockOnGun_Bottom[1], "sprites/lockongun_bottom_head.tga");
	R_InitTexture(m_iLockOnGun_Bottom[2], "sprites/lockongun_bottom_chest.tga");
	R_InitTexture(m_iLockOnGun_Bottom[3], "sprites/lockongun_bottom_stomach.tga");
	R_InitTexture(m_iLockOnGun_Bottom[4], "sprites/lockongun_bottom_arms.tga");
	R_InitTexture(m_iLockOnGun_Bottom[5], "sprites/lockongun_bottom_leg.tga");

	ClearAllLockOnData();

	R_InitTexture(m_iPatrolDrone_Top_BG, "sprites/patroldrone_top_bg.tga");
	R_InitTexture(m_iPatrolDrone_Bottom_BG, "sprites/patroldrone_bottom_bg.tga");
	R_InitTexture(m_iPatrolDrone_Center_Default, "sprites/patroldrone_center_default.tga");
	R_InitTexture(m_iPatrolDrone_Icon_Disabled, "sprites/patroldrone_icon_disabled.tga");
	R_InitTexture(m_iPatrolDrone_Icon_Warning01, "sprites/patroldrone_icon_warning01.tga");
	R_InitTexture(m_iPatrolDrone_Icon_Warning02, "sprites/patroldrone_icon_warning02.tga");
	R_InitTexture(m_iPatrolDrone_Icon_Activation, "sprites/patroldrone_icon_activation.tga");
	R_InitTexture(m_iPatrolDrone_Icon_Blasting, "sprites/patroldrone_icon_blasting.tga");
	R_InitTexture(m_iPatrolDrone_Center_Red, "sprites/patroldrone_center_ani_red.tga");

	m_flPatrolDroneDeployTime = 0.0f;
	m_iPatrolDroneState = PATROLDRONE_OFF;

	//bunkerbuster

	R_InitTexture(m_iBunkerBusterTgaBG_Blue, "sprites/bunkerbuster_aim_bg_blue.tga");
	R_InitTexture(m_iBunkerBusterTgaBG_Red, "sprites/bunkerbuster_aim_bg_red.tga");
	R_InitTexture(m_iBunkerBusterTgaFrame_Blue, "sprites/bunkerbuster_aim_frame_blue.tga");
	R_InitTexture(m_iBunkerBusterTgaFrame_Red, "sprites/bunkerbuster_aim_frame_red.tga");
	R_InitTexture(m_iBunkerBusterTgaGauge, "sprites/bunkerbuster_aim_gauge.tga");
	R_InitTexture(m_iBunkerBusterTgaCoolTimeLeft, "sprites/bunkerbuster_cooltime_number_left_bg.tga");
	R_InitTexture(m_iBunkerBusterTgaCoolTimeRight, "sprites/bunkerbuster_cooltime_number_right_bg.tga");

	m_flBunkerBusterCoolDown = 0.0f;
	m_flBunkerBusterCoolDownGlobal = 0.0f;
	m_flLastFrameTime = 0.0f;

	m_flBunkerBusterShoot = 0.0f;
	m_flBunkerBusterShootEnd = 0.0f;
	m_iGauge = 0;

	m_iBunkerSprite = SPR_Load("sprites/bunkerbuster_scope.spr");
	m_pModelBunkerSprite = (struct model_s*)gEngfuncs.GetSpritePointer(m_iBunkerSprite);
	m_iBunkerSpriteFrame = 0;

	for (int i = 0; i < 10; i++)
	{
		char str[32];
		sprintf(str, "bunkerbuster_cooltime%d", i);
		iBunkerSpriteNumber[i] = gHUD.GetSpriteIndex(str);
	}

	R_InitTexture(m_iMGSM_Aim_BG, "sprites/mgsm_aim_bg.tga");
	R_InitTexture(m_iMGSM_Aim_Gauge, "sprites/mgsm_aim_gauge.tga");


	m_flMGSMTimeChargeStart = gHUD.m_flTime + 9999.0;

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
		FuncDraw2DQuadScaled(tex, width2, height2, width, height);
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

void CHudSniperScope::FuncDrawCircle(int index, int width, int height, float gauge)
{
	if (!index)
		return;

	float flScale = gEngfuncs.pfnGetCvarFloat("hud_scale");

	const int xx = (float)ScreenWidth / 2 * flScale;
	const int yy = (float)ScreenHeight / 2 * flScale;

	const int ww = width / 2 * flScale;
	const int tt = height / 2 * flScale;

	float amount, tan;
	int pl;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gRenderAPI.GL_Bind(0, index);

	const float defCoords[8][2] = {
		{0.5f, 0},
		{1, 0},
		{1, 0.5f},
		{1, 1},
		{0.5f, 1},
		{0, 1},
		{0, 0.5f},
		{0, 0}
	};

	const int defVerts[8][2] = {
		{xx, yy - tt},
		{xx + ww, yy - tt},
		{xx + ww, yy},
		{xx + ww, yy + tt},
		{xx, yy + tt},
		{xx - ww, yy + tt},
		{xx - ww, yy},
		{xx - ww, yy - tt}
	};

	for (int i = 0; i < 8; i++)
	{
		amount = 0.125f * i;
		if (gauge <= amount)
			break;

		if (gauge >= 0.125f + amount)
			tan = 1;
		else
		{
			if (i & 1)
				tan = 1.0f - tanf(M_PI / 4 - M_PI * (gauge - amount) * 2);
			else
				tan = tanf(M_PI * (gauge - amount) * 2);
		}

		gEngfuncs.pTriAPI->Begin(TRI_TRIANGLES);

		gEngfuncs.pTriAPI->TexCoord2f(defCoords[i][0], defCoords[i][1]);
		gEngfuncs.pTriAPI->Vertex3f(defVerts[i][0], defVerts[i][1], 0);

		gEngfuncs.pTriAPI->TexCoord2f(0.5f, 0.5f);
		gEngfuncs.pTriAPI->Vertex3f(xx, yy, 0);

		if ((i & 1) == ((i >> 1) & 1)) // 0347
		{
			pl = (float)ww * tan;
			if (!i || i == 7) // 07
			{
				gEngfuncs.pTriAPI->TexCoord2f(defCoords[i][0] + tan / 2, defCoords[i][1]);
				gEngfuncs.pTriAPI->Vertex3f(defVerts[i][0] + pl, defVerts[i][1], 0);
			}
			else // 34
			{
				gEngfuncs.pTriAPI->TexCoord2f(defCoords[i][0] - tan / 2, defCoords[i][1]);
				gEngfuncs.pTriAPI->Vertex3f(defVerts[i][0] - pl, defVerts[i][1], 0);
			}
		}
		else // 1256
		{
			pl = (float)tt * tan;
			if (i & 4) // 56
			{
				gEngfuncs.pTriAPI->TexCoord2f(defCoords[i][0], defCoords[i][1] - tan / 2);
				gEngfuncs.pTriAPI->Vertex3f(defVerts[i][0], defVerts[i][1] - pl, 0);
			}
			else // 12
			{
				gEngfuncs.pTriAPI->TexCoord2f(defCoords[i][0], defCoords[i][1] + tan / 2);
				gEngfuncs.pTriAPI->Vertex3f(defVerts[i][0], defVerts[i][1] + pl, 0);
			}
		}
		gEngfuncs.pTriAPI->End();
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
		if (pActiveBTEWeapon->m_iId == WEAPON_SKULL5 || pActiveBTEWeapon->m_iId == WEAPON_SKULL6 || pActiveBTEWeapon->m_iId == WEAPON_CROSSBOW)
		{
			if (gHUD.m_iFOV == 90)
				return 1;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_SFSNIPER)
		{
			return 1;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_KRONOS12 || 
			pActiveBTEWeapon->m_iId == WEAPON_KRONOS5 ||
			pActiveBTEWeapon->m_iId == WEAPON_KRONOS7 || 
			pActiveBTEWeapon->m_iId == WEAPON_KRONOS3 || 
			pActiveBTEWeapon->m_iId == WEAPON_KRONOS1)
		{
			if (gHUD.m_iFOV > 89)
				return 1;

			DrawKronosScope(flTime);
			return 0;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_CROSSBOWEX21)
		{
			if (gHUD.m_iFOV > 89)
				return 1;

			DrawCrossbowEX21Scope(flTime);
			return 0;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_BUFFAWP)
		{
			if (gHUD.m_iFOV > 40)
			{
				BUFFAWPEnd();
				return 1;
			}

			BUFFAWPRun();
			DrawBUFFAWPScope(flTime);
			return 0;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_LOCKONGUN)
		{
			if (gHUD.m_iFOV > 89)
				return 1;

			DrawLockOnGunScope(flTime);
			return 0;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_PATROLDRONE)
		{
			DrawPatrolDroneScope(flTime);
			return 1;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_MGSM)
		{
			DrawMGSMScope(flTime);
			return 1;
		}
		else if (pActiveBTEWeapon->m_iId == WEAPON_BUNKERBUSTER)
		{

			if (gHUD.m_iFOV > 89)
				return 1;

			DrawBunkerBusterScope(flTime);
			return 0;

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

}

void CHudSniperScope::DrawKronosScope(float flTime)
{
	float LENGTH_MID = min(ScreenWidth, ScreenHeight) * 0.75f;
	float x, y, w, h;

	x = (ScreenWidth - LENGTH_MID) / 2;
	y = (ScreenHeight - LENGTH_MID) / 2;
	w = LENGTH_MID;
	h = LENGTH_MID;

	m_iAimBG->Draw2DQuadScaled(x, y, x + w, y + h);

	FuncDrawCorner(*m_iAimFrame, TYPE_CORNER_NW, ScreenWidth / 2, ScreenHeight / 2);

	if (m_flScopeTimeEnd > flTime && m_flScopeTime)
	{
		float percent = (m_flScopeTimeEnd - flTime) / m_flScopeTime;
		FuncDrawCircle(m_iAimGauge->texnum(), LENGTH_MID * 0.36f, LENGTH_MID * 0.36f, percent);
	}
}

void CHudSniperScope::SetKronosTime(float time)
{
	m_flScopeTime = time;
	m_flScopeTimeEnd = gHUD.m_flTime + time;
}

void CHudSniperScope::DrawCrossbowEX21Scope(float flTime)
{
	FuncDraw2DQuadScaled(*m_iCrossbowex21_AimBG, 0, 0, ScreenWidth, ScreenHeight / 2);
	FuncDraw2DQuadScaled(*m_iCrossbowex21_AimBG, 0, ScreenHeight, ScreenWidth, -ScreenHeight / 2);

	int x = ScreenWidth / 2;
	int y = ScreenHeight / 2;
	int w = ScreenWidth / 3;
	int t = ScreenHeight / 2;

	FuncDraw2DQuadScaled(*m_iCrossbowex21_AimImage, x - w, y - t / 2, w, t);
	FuncDraw2DQuadScaled(*m_iCrossbowex21_AimImage, x + w, y - t / 2, -w, t);
}

void CHudSniperScope::DrawBUFFAWPScope(float flTime)
{
	if (!m_flBUFFAWPStartTime)
		return;

	int iH = ScreenHeight / 592.0f * 256.0f;

	FillRGBABlend(0, 0, ScreenWidth / 2 - iH + 1, ScreenHeight, 0, 0, 0, 255);
	FillRGBABlend(ScreenWidth / 2 + iH - 1, 0, ScreenWidth / 2 - iH + 10, ScreenHeight, 0, 0, 0, 255);

	FillRGBABlend(0, 0, ScreenWidth, ScreenHeight / 2 - iH + 1, 0, 0, 0, 255);
	FillRGBABlend(0, ScreenHeight / 2 + iH, ScreenWidth, ScreenHeight / 2 - iH + 10, 0, 0, 0, 255);

	//cross
	FillRGBABlend(0, ScreenHeight / 2, ScreenWidth / 2 - 20, 1, 0, 0, 0, 255);
	FillRGBABlend(ScreenWidth / 2 + 20, ScreenHeight / 2, ScreenWidth, 1, 0, 0, 0, 255);

	FillRGBABlend(ScreenWidth / 2, 0, 1, ScreenHeight / 2 - 20, 0, 0, 0, 255);
	FillRGBABlend(ScreenWidth / 2, ScreenHeight / 2 + 20, 1, ScreenHeight, 0, 0, 0, 255);	

	FuncDraw2DQuadScaled(*m_iBUFFAWP_Outline, ScreenWidth / 2 - iH, ScreenHeight / 2 - iH, iH, iH);
	FuncDraw2DQuadScaled(*m_iBUFFAWP_Outline, ScreenWidth / 2 - iH, ScreenHeight / 2 + iH, iH, -iH);
	FuncDraw2DQuadScaled(*m_iBUFFAWP_Outline, ScreenWidth / 2 + iH, ScreenHeight / 2 - iH, -iH, iH);
	FuncDraw2DQuadScaled(*m_iBUFFAWP_Outline, ScreenWidth / 2 + iH, ScreenHeight / 2 + iH, -iH, -iH);

	FuncDraw2DQuadScaled(*m_iBUFFAWP_BG, ScreenWidth / 2 - m_iBUFFAWP_BG->w() / 2, ScreenHeight / 4 * 3, m_iBUFFAWP_BG->w(), m_iBUFFAWP_BG->h());

	int iCharged = (flTime - m_flBUFFAWPStartTime) / 0.8;
	float flChargingPercent = (flTime - m_flBUFFAWPStartTime - iCharged * 0.8) / 0.8;
	if (iCharged >= 3)
	{
		iCharged = 3;
		flChargingPercent = 0;
	}

	for (int i = 0; i < 3; i++)
	{
		if (iCharged > i)
			FuncDraw2DQuadScaled(*m_iBUFFAWP_Light[i], ScreenWidth / 2 - m_iBUFFAWP_Light[i]->w() / 2, ScreenHeight / 4 * 3, m_iBUFFAWP_Light[i]->w(), m_iBUFFAWP_Light[i]->h());
		else
			break;
	}

	FuncDraw2DQuadScaled(*m_iBUFFAWP_Bar, ScreenWidth / 2 - m_iBUFFAWP_Bar->w() / 2, ScreenHeight / 4 * 3 + 24, m_iBUFFAWP_Bar->w() * flChargingPercent, m_iBUFFAWP_Bar->h());
}

void CHudSniperScope::BUFFAWPRun(void)
{
	if (m_flBUFFAWPStartTime > 0.0)
		return;
	m_flBUFFAWPStartTime = gHUD.m_flTime;
}

void CHudSniperScope::BUFFAWPEnd(void)
{
	m_flBUFFAWPStartTime = 0.0;
}

void CHudSniperScope::DrawLockOnGunScope(float flTime)
{
	FuncDraw2DQuadScaled(*m_iLockOnGun_BG, 0, 0, ScreenWidth, ScreenHeight);

	int iW, iH;
	iW = ScreenWidth * 0.3;
	iH = iW * m_iLockOnGun_Top->h() / m_iLockOnGun_Top->w();
	FuncDraw2DQuadScaled(*m_iLockOnGun_Top, 0, 0, iW, iH);
	FuncDraw2DQuadScaled(*m_iLockOnGun_Top, ScreenWidth, 0, -iW, iH);

	iH = ScreenHeight / 5;
	iW = iH * m_iLockOnGun_Center->w() / m_iLockOnGun_Center->h();
	FuncDraw2DQuadScaled(*m_iLockOnGun_Center, (ScreenWidth - iW) / 2, (ScreenHeight - iH) / 2, iW, iH);

	iH = ScreenHeight / 8;
	iW = iH * m_iLockOnGun_Bottom_BG->w() / m_iLockOnGun_Bottom_BG->h();
	FuncDraw2DQuadScaled(*m_iLockOnGun_Bottom_BG, ScreenWidth / 2 - iW * 1.5, ScreenHeight - iH * 2, iW, iH);
	FuncDraw2DQuadScaled(*m_iLockOnGun_Bottom_BG, ScreenWidth / 2 + iW * 0.5, ScreenHeight - iH * 2, iW, iH);

	int iX, iY;
	int iW2 = iW / 3, iH2 = iH / 2, remain;
	for (int i = 0; i < 10; i++)
	{
		if (m_iLockOnData[i].hitgroup == -1)
			continue;

		if (m_iLockOnData[i].dietime <= flTime)
		{
			m_iLockOnData[i].hitgroup = -1;
			continue;
		}

		iX = ScreenWidth / 2 - iW * 1.5;
		iY = ScreenHeight - iH * 2;

		remain = (i + 1) % 5;
		//1234012340
		if (remain == 4 || !remain) // downstair position
		{
			iX += iW / 2 + (remain == 4 ? -iW2 : 0);
			iY += iH * 0.5;
		}
		else
		{
			iX += iW2 * (remain - 1);
		}

		if (i > 4)
			iX += iW * 2.0;

		int a = 255;
		if (m_iLockOnData[i].dietime - flTime < 4.5)
		{
			float flife = m_iLockOnData[i].dietime - flTime;
			float fround = round(flife);

			if (flife < fround) // more than 0.5
				a = 255.0 * (fround - flife) / 0.5f;
			else
				a = 255.0 * (flife - fround) / 0.5f;
		}

		FuncDraw2DQuadScaled(*m_iLockOnGun_Bottom[m_iLockOnData[i].hitgroup], iX, iY, iW2, iH2, 0, 0, 1, 1, 255, 255, 255, a);
	}
}

void CHudSniperScope::SetLockOnData(float dietime, int slot, int hitgroup)
{
	m_iLockOnData[slot].dietime = gHUD.m_flTime + dietime;
	m_iLockOnData[slot].hitgroup = hitgroup;
}

void CHudSniperScope::ClearAllLockOnData()
{
	for (int i = 0; i < 10; i++)
	{
		m_iLockOnData[i].dietime = gHUD.m_flTime;
		m_iLockOnData[i].hitgroup = -1;
	}
}


void CHudSniperScope::InsertBunkerBusterData(int iGauge, float iCoolDown, float gCoolDown)
{
	if (iCoolDown != -1.0 && gCoolDown != -1.0)
	{
		m_flBunkerBusterCoolDown = gHUD.m_flTime + iCoolDown;
		m_flBunkerBusterCoolDownGlobal = gHUD.m_flTime + gCoolDown;
	}


	m_iGauge = iGauge;

}

void CHudSniperScope::InsertBunkerBusterData2(float ShootTime)
{
	m_flBunkerBusterShoot = ShootTime;
	m_flBunkerBusterShootEnd = gHUD.m_flTime + ShootTime;
}

void CHudSniperScope::DrawBunkerBusterScope(float flTime)
{
	float scale = 1.0;
	float flScale = gEngfuncs.pfnGetCvarFloat("hud_scale");
	;

	float LENGTH_SCOPE_BB = ScreenHeight / 2 * 0.75f;
	bool IsRed = false;


	if (m_iGauge)
	{
		if (m_flBunkerBusterCoolDown <= flTime && m_flBunkerBusterCoolDownGlobal <= flTime)
			IsRed = true;
	}
	if (!IsRed)
	{
		if (m_pModelBunkerSprite)
		{
			int iWidth = 512;
			int iHeight = 413;

			//iWidth = (int)(MAX(gEngfuncs.pfnSPR_Width(m_iBunkerSprite, m_iBunkerSpriteFrame), 0) * 1.5);
			//iHeight = MAX(gEngfuncs.pfnSPR_Height(m_iBunkerSprite, 0), 0);

			int iX = (ScreenWidth - iWidth) / 2;
			int iY = (ScreenHeight - iHeight) / 2;

			gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);

			gEngfuncs.pTriAPI->SpriteTexture(m_pModelBunkerSprite, m_iBunkerSpriteFrame);
			gEngfuncs.pTriAPI->Begin(TRI_QUADS);

			gEngfuncs.pTriAPI->TexCoord2f(0, 1);
			gEngfuncs.pTriAPI->Vertex3f(iX * flScale, (iY + iHeight) * flScale, 0);

			gEngfuncs.pTriAPI->TexCoord2f(1, 1);
			gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * flScale, (iY + iHeight) * flScale, 0);

			gEngfuncs.pTriAPI->TexCoord2f(1, 0);
			gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * flScale, iY * flScale, 0);

			gEngfuncs.pTriAPI->TexCoord2f(0, 0);
			gEngfuncs.pTriAPI->Vertex3f(iX * flScale, iY * flScale, 0);

			gEngfuncs.pTriAPI->End();


			if (1.0 / (30.0 * 1.0) <= gHUD.m_flTime - m_flLastFrameTime)
			{
				m_iBunkerSpriteFrame++;
				m_flLastFrameTime = gHUD.m_flTime;
			}

			if (m_iBunkerSpriteFrame > 25)
			{
				m_iBunkerSpriteFrame = 0;
			}

		}
	}


	FuncDrawCorner(IsRed ? *m_iBunkerBusterTgaFrame_Red : *m_iBunkerBusterTgaFrame_Blue, TYPE_CORNER_NW, LENGTH_SCOPE_BB, LENGTH_SCOPE_BB, 10, 10);

	//mid
	float HEIGHT_MID = ScreenHeight / 2;
	float WIDTH_MID = HEIGHT_MID * 1.24f;

	FuncDraw2DQuadScaled(IsRed ? *m_iBunkerBusterTgaBG_Red : *m_iBunkerBusterTgaBG_Blue, ScreenWidth / 2 - WIDTH_MID / 2, ScreenHeight / 2 - HEIGHT_MID / 2, WIDTH_MID, HEIGHT_MID);

	//129 : 56 = 512 : 413
	float HEIGHT_TIMER = HEIGHT_MID / 7.0f;
	float WIDTH_TIMER = WIDTH_MID / 4.0f;

	FuncDraw2DQuadScaled(*m_iBunkerBusterTgaCoolTimeLeft, (ScreenWidth - WIDTH_MID - WIDTH_TIMER) / 2, (ScreenHeight - HEIGHT_TIMER) / 2, WIDTH_TIMER, HEIGHT_TIMER);
	FuncDraw2DQuadScaled(*m_iBunkerBusterTgaCoolTimeLeft, (ScreenWidth + WIDTH_MID - WIDTH_TIMER) / 2, (ScreenHeight - HEIGHT_TIMER) / 2, WIDTH_TIMER, HEIGHT_TIMER);

	char sTime[10];
	if (m_flBunkerBusterCoolDownGlobal < flTime)
		sprintf(sTime, "0");
	else
		sprintf(sTime, "%d", (int)(m_flBunkerBusterCoolDownGlobal - flTime));

	int number = strlen(sTime);
	while (number > 0)
	{
		char temp[2];
		sprintf(temp, "%c", sTime[number - 1]);
		int iNum = atoi(temp);
		if (0 <= iNum && iNum < 10)
		{
			gEngfuncs.pfnSPR_Set(gHUD.GetSprite(iBunkerSpriteNumber[iNum]), 170, 240, 0);
			gEngfuncs.pfnSPR_DrawAdditive(0, (ScreenWidth - WIDTH_MID) / 2 - (strlen(sTime) - number) * 23.0f, (ScreenHeight - HEIGHT_TIMER / 2) / 2, &gHUD.GetSpriteRect(iBunkerSpriteNumber[iNum]));
		}
		number--;
	}
	char sTime2[10];
	if (m_flBunkerBusterCoolDown < flTime)
		sprintf(sTime2, "0");
	else
		sprintf(sTime2, "%d", (int)(m_flBunkerBusterCoolDown - flTime));

	int number2 = strlen(sTime2);
	while (number2 > 0)
	{
		char temp[2];
		sprintf(temp, "%c", sTime2[number2 - 1]);
		int iNum = atoi(temp);
		if (0 <= iNum && iNum < 10)
		{
			gEngfuncs.pfnSPR_Set(gHUD.GetSprite(iBunkerSpriteNumber[iNum]), 170, 240, 0);
			gEngfuncs.pfnSPR_DrawAdditive(0, (ScreenWidth + WIDTH_MID) / 2 - (strlen(sTime2) - number2) * 23.0f, (ScreenHeight - HEIGHT_TIMER / 2) / 2, &gHUD.GetSpriteRect(iBunkerSpriteNumber[iNum]));
		}
		number2--;
	}


	if (IsRed)
	{
		float percent = 1.0f - (m_flBunkerBusterShootEnd - flTime) / m_flBunkerBusterShoot;
		FuncDrawCircle(m_iBunkerBusterTgaGauge->texnum(), WIDTH_MID, HEIGHT_MID, percent);
	}
}
void CHudSniperScope::DrawPatrolDroneScope(float flTime)
{
	float scale = 1.0;
	float flScale = gEngfuncs.pfnGetCvarFloat("hud_scale");

	if (m_flPatrolDroneDeployTime && m_flPatrolDroneDeployTime + 0.25 > flTime)
		scale = (flTime - m_flPatrolDroneDeployTime) / 0.25;

	int wide, tall;

	wide = ScreenWidth * 0.48 * scale;
	tall = ScreenHeight * 0.48 * scale;

	FuncDrawScope(*m_iPatrolDrone_Top_BG, TYPE_DOUBLE_SQUARE_NOTCENTER, ScreenWidth / 2, ScreenHeight / 2 - tall, wide, tall);
	FuncDrawScope(*m_iPatrolDrone_Bottom_BG, TYPE_DOUBLE_SQUARE_NOTCENTER, ScreenWidth / 2, ScreenHeight / 2, wide, tall);

	wide = tall = ScreenHeight * scale / 2;

	switch (m_iPatrolDroneState)
	{
	case PATROLDRONE_OFF:
	{
		FuncDraw2DQuadScaled(*m_iPatrolDrone_Center_Default, (ScreenWidth - wide) / 2, (ScreenHeight - tall) / 2, wide, tall);
		break;
	}
	case PATROLDRONE_ATTACK:
	{
		FuncDraw2DQuadScaled(*m_iPatrolDrone_Center_Default, (ScreenWidth - wide) / 2, (ScreenHeight - tall) / 2, wide, tall);
		//FuncDraw2DQuadScaled(*m_iPatrolDrone_Center_Red, (ScreenWidth - wide) / 2, (ScreenHeight - tall) / 2, wide, tall);
		break;
	}
	}

	if (const int num = m_vecPatrolDroneData.size())
	{
		wide = ScreenHeight / 15 * flScale;
		tall = wide * flScale;

		const int maxclip = 90;
		const int offset = 8;
		const float imageoffset = 0.073; // 4 / 55
		const float fmax = 1.0 - imageoffset * 2;

		int iX = ScreenWidth / 2, iY = ScreenHeight * (0.5 + (0.2 * scale));

		iX -= (wide * num + offset * (num - 1)) / 2;

		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);

		int wide2 = ScreenHeight / 8;

		for (auto iter = m_vecPatrolDroneData.begin(); iter != m_vecPatrolDroneData.end(); ++iter)
		{
			if (iter->second.ammo == -1)
			{
				FuncDraw2DQuadScaled(*m_iPatrolDrone_Icon_Blasting, iX, iY, wide, tall);
				iX += wide + offset;
				continue;
			}

			int texid = 0;
			float percent = (float)iter->second.ammo / (float)maxclip;

			if (iter->second.state == PATROLDRONE_OFF)
			{
				m_iPatrolDroneState = PATROLDRONE_OFF;

				if (percent < 0.3)
					texid = m_iPatrolDrone_Icon_Warning02->texnum();
				else
					texid = m_iPatrolDrone_Icon_Activation->texnum();
			}
			else
			{
				if (iter->second.state == PATROLDRONE_ATTACK)
					m_iPatrolDroneState = PATROLDRONE_ATTACK;
				else
					m_iPatrolDroneState = PATROLDRONE_OFF;

				if (percent < 0.3)
				{
					if (iter->second.state == PATROLDRONE_WANDER)
						texid = m_iPatrolDrone_Icon_Warning01->texnum();
					else
						texid = m_iPatrolDrone_Icon_Activation->texnum();
				}
				else
					texid = m_iPatrolDrone_Icon_Activation->texnum();
			}

			if (iter->second.ammo >= 0 && iter->second.ammo <= 90)
			{
				if (iter->second.nextcheckammo < gHUD.m_flTime)
				{
					if (iter->second.state == PATROLDRONE_OFF)
					{
						iter->second.ammo++;
						iter->second.nextcheckammo = gHUD.m_flTime + 0.3;
					}
					else
					{
						iter->second.ammo--;
						iter->second.nextcheckammo = gHUD.m_flTime + 0.3;
					}
				}
			}

			percent *= fmax;

			if (!texid)
				continue;

			if (percent)
			{
				float drawoffset = imageoffset + fmax - percent;

				gRenderAPI.GL_Bind(0, texid);

				gEngfuncs.pTriAPI->Begin(TRI_QUADS);

				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(iX, iY + tall, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f(iX + wide, iY + tall, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1, drawoffset);
				gEngfuncs.pTriAPI->Vertex3f(iX + wide, iY + tall * drawoffset, 0);

				gEngfuncs.pTriAPI->TexCoord2f(0, drawoffset);
				gEngfuncs.pTriAPI->Vertex3f(iX, iY + tall * drawoffset, 0);

				gEngfuncs.pTriAPI->End();
			}

			if (percent < 1.0)
			{
				float drawoffset = imageoffset + fmax - percent;

				gRenderAPI.GL_Bind(0, m_iPatrolDrone_Icon_Disabled->texnum());

				gEngfuncs.pTriAPI->Begin(TRI_QUADS);

				gEngfuncs.pTriAPI->TexCoord2f(0, drawoffset);
				gEngfuncs.pTriAPI->Vertex3f(iX, iY + tall * drawoffset, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1, drawoffset);
				gEngfuncs.pTriAPI->Vertex3f(iX + wide, iY + tall * drawoffset, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(iX + wide, iY, 0);

				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(iX, iY, 0);

				gEngfuncs.pTriAPI->End();
			}

			iX += wide + offset;
		}

	
	}
}


void CHudSniperScope::InsertPatrolDroneData(int iSlot, int iClip, int iState)
{
	m_vecPatrolDroneData[iSlot].ammo = iClip;
	m_vecPatrolDroneData[iSlot].state = iState;
	m_vecPatrolDroneData[iSlot].nextcheckammo = gHUD.m_flTime + 0.3;
}

void CHudSniperScope::SetPatrolDroneDeployTime()
{
	m_flPatrolDroneDeployTime = gHUD.m_flTime;
}

void CHudSniperScope::DrawMGSMScope(float flTime)
{
	int iX = ScreenWidth / 2;
	int iY = ScreenHeight / 2;

	int len = iY / 4 * 3;

	FuncDraw2DQuadScaled(*m_iMGSM_Aim_BG, iX - len / 2, iY - len / 2, len, len);
	float delta = gHUD.m_flTime - m_flMGSMTimeChargeStart;
	float timepassed = (delta / m_flMGSMFinishTime);

	if (delta >= m_flMGSMFinishTime)
	{
		m_flMGSMTimeChargeStart = gHUD.m_flTime + 9999;
	}

	if (timepassed > 1.0 || timepassed < 0)
		timepassed = 1;

	FuncDrawCircle(m_iMGSM_Aim_Gauge->texnum(), len, len, 0.625f + timepassed / 4);
	

}

void CHudSniperScope::SetMGSMAmmo(float flDelta, float flFinishTime)
{
	if(flDelta)
		m_flMGSMTimeChargeStart = gHUD.m_flTime - flDelta;
	else
		m_flMGSMTimeChargeStart = gHUD.m_flTime;
	m_flMGSMFinishTime = flFinishTime;

}

void CHudSniperScope::Shutdown()
{
	std::fill(std::begin(m_iScopeArc), std::end(m_iScopeArc), nullptr);
}

}