/*
moe_touch.cpp - CSMoE Client HUD
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "parsemsg.h"

#include "ammohistory.h"
#include "moe_touch.h"

#include <vector>
#include <algorithm>

class CHudMoeTouch::impl_t
{
public:
	int m_fingerID = 0;
	bool m_bActive = false;
	float m_flLastX = -1;
	float m_flLastY = -1;
	float m_flLastDx = -1;
	float m_flLastDy = -1;
	float m_flStartY = -1;

	float m_flX = -1;
	float m_flY = -1;
	float m_flDx = -1;
	float m_flDy = -1;

	int m_iActiveSlot = 0;
	int m_iLastSlot = -1;

	std::map<std::string, UniqueTexture> m_weaponTexture;

	void ResetValues()
	{
		m_flLastX = m_flLastY = m_flX = m_flY = m_flStartY = -1;
		m_flLastDx = m_flLastDy = m_flDx = m_flDy = 0;
	}

};

int CHudMoeTouch::Init(void)
{
	pimpl = new CHudMoeTouch::impl_t;

	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);


	return 1;
}

int CHudMoeTouch::VidInit(void)
{
	
	return 1;
}

// ammo.cpp
extern WEAPON *gpActiveSel;	// NULL means off, 1 means just the menu bar, otherwise
						// this points to the active weapon menu item
extern WEAPON *gpLastSel;		// Last weapon menu selection 

int CHudMoeTouch::Draw(float time)
{
	if (!pimpl->m_bActive)
		return 0;

	const int x = ScreenWidth * pimpl->m_flX;
	const int y = ScreenHeight * pimpl->m_flStartY;
	const float scale = ScreenHeight / 480.0f;
	//int w = ScreenWidth - x;
	//int h = 250;

	int r, g, b;
	//gEngfuncs.pfnFillRGBA(x, y, w, h, 255, 255, 255, 255);

	const int iActiveSlot = pimpl->m_iActiveSlot;

	// Draw all of the buckets
	int y2 = y - iActiveSlot * 45 * scale;

	for (size_t i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		WEAPON *p = gWR.GetFirstPos(i);
		int iWidth = 175 * scale;
		int iHeight = 45 * scale;
		if (p)
		{
			iWidth = (p->rcActive.right - p->rcActive.left) * scale;
			iHeight = (p->rcActive.bottom - p->rcActive.top) * scale;
		}
		
		int x2 = x;
		if (i == iActiveSlot)
			x2 -= 45;

		for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
		{
			p = gWR.GetWeaponSlot(i, iPos);

			if (!p || !p->iId)
				continue;

			// if active, then we must have ammo.
			if (gWR.HasAmmo(p))
			{
				DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
				DrawUtils::ScaleColors(r, g, b, 192);
			}
			else
			{
				DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
				DrawUtils::ScaleColors(r, g, b, 128);
			}

			rect_s rc = p->rcActive;

			if (iActiveSlot == i)
			{
				SPR_Set(gHUD.GetSprite(gHUD.m_Ammo.m_HUD_selection), r, g, b);
				SPR_DrawAdditive(0, x2, y2, &gHUD.GetSpriteRect(gHUD.m_Ammo.m_HUD_selection));
			}

			SPR_Set(p->hInactive, r, g, b);
			SPR_DrawAdditive(0, x2, y2, &p->rcInactive);

			x2 += iWidth + 5 * scale;
		}

		
		y2 += iHeight + 5 * scale;
	}

	return 1;
}

void CHudMoeTouch::Think(void)
{
	if (!pimpl->m_bActive)
		return;

	if (pimpl->m_fingerID)
	{
		pimpl->m_flX = pimpl->m_flLastX;
		pimpl->m_flY = pimpl->m_flLastY;
		pimpl->m_flDx = pimpl->m_flLastDx;
		pimpl->m_flDy = pimpl->m_flLastDy;

		const float scale = ScreenHeight / 480.0f;
		const float flHeight = 45 * scale * gHUD.m_flScale;
		//const int iCurrentSlot = gHUD.m_Ammo.m_pWeapon ? gHUD.m_Ammo.m_pWeapon->iSlot : 0;
		const int iCurrentSlot = pimpl->m_iLastSlot;
		const int Result = (pimpl->m_flY - pimpl->m_flStartY) * ScreenHeight / flHeight + iCurrentSlot;
		pimpl->m_iActiveSlot = std::min(std::max(Result, 0), MAX_WEAPON_SLOTS - 1);
	}
	else
	{
		pimpl->m_flX += pimpl->m_flDx;
		pimpl->m_flY += pimpl->m_flDy;

		// make reversed x motion ->
		pimpl->m_flDx += (2 - pimpl->m_flX) * 0.005;

		// slow down dy
		if (std::abs(pimpl->m_flDy) < 0.1)
			pimpl->m_flDy = 0.0f;
		else if (pimpl->m_flDy > 0)
			pimpl->m_flDy -= 0.05f;
		else
			pimpl->m_flDy += 0.05f;

		// end
		if (pimpl->m_flX > 1)
			pimpl->m_bActive = false;
	}

}

void CHudMoeTouch::Reset(void)
{
	
}

void CHudMoeTouch::InitHUDData(void)
{
	
}

void CHudMoeTouch::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}

int CHudMoeTouch::TouchEvent(touchEventType type, int fingerID, float x, float y, float dx, float dy)
{
	if (pimpl->m_fingerID)
	{
		if (fingerID == pimpl->m_fingerID)
		{
			// pressing
			if (type == event_motion)
			{
				pimpl->m_flLastX = x;
				pimpl->m_flLastY = y;
				pimpl->m_flLastDx = dx;
				pimpl->m_flLastDy = dy;
			}
			else if (type == event_up)
			{
				/*if (pimpl->m_flLastX < 0.8)
				{
					gEngfuncs.pfnClientCmd("lastinv");
				}*/

				const int iPrevSlot = gHUD.m_Ammo.m_pWeapon ? gHUD.m_Ammo.m_pWeapon->iSlot : pimpl->m_iActiveSlot;
				if(pimpl->m_iActiveSlot != iPrevSlot)
					pimpl->m_iLastSlot = iPrevSlot;

				WEAPON *p = gWR.GetFirstPos(pimpl->m_iActiveSlot);
				if(p)
					ServerCmd(p->szName);
				pimpl->m_fingerID = 0;
			}
			return 1;
		}
	}
	else
	{
		if (type == event_down)
		{
			if (x > 0.95f)
			{
				pimpl->m_fingerID = fingerID;
				pimpl->m_bActive = true;
				pimpl->ResetValues();
				pimpl->m_flStartY = y;

				return 1;
			}
		}
	}

	return 0;
}
