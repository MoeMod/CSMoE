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

constexpr float POSITION_SWITCH_ANIM = 0.25f;

class CHudMoeTouch::impl_t
{
public:
	int m_fingerID = -1;
	bool m_bActive = false;
	float m_flLastX = -1;
	float m_flLastY = -1;
	float m_flLastDx = -1;
	float m_flLastDy = -1;
	float m_flStartY = -1;
	float m_flMinX = -1;

	float m_flX = -1;
	float m_flY = -1;
	float m_flDx = -1;
	float m_flDy = -1;

	int m_iActiveSlot = 0;
	int m_iLastSlot = -1;

	void ResetValues()
	{
		m_flLastY = m_flY = m_flStartY = -1;
		m_flLastX = m_flX = m_flMinX = 1;
		m_flLastDx = m_flLastDy = m_flDx = m_flDy = 0;
		std::fill(std::begin(m_flSwitchPositionJudgeStartTime), std::end(m_flSwitchPositionJudgeStartTime), 0.0f);
		std::fill(std::begin(m_flSwitchPositionJudgeAction), std::end(m_flSwitchPositionJudgeAction), false);

		//std::fill(std::begin(m_flLastSwitchPosition), std::end(m_flLastSwitchPosition), 0.0f);
	}

	static constexpr size_t SLOT_COUNT_WEAPON = MAX_WEAPON_SLOTS;
	static constexpr size_t SLOT_COUNT_SKILL = 4;

	int m_iWeaponSlotSelectedPosition[MAX_WEAPON_SLOTS] = {};
	float m_flSwitchPositionJudgeStartTime[MAX_WEAPON_SLOTS] = {};
	bool m_flSwitchPositionJudgeAction[MAX_WEAPON_SLOTS] = {};
	//float m_flLastSwitchPosition[MAX_WEAPON_SLOTS] = {};

	int WeaponCountInSlot(int slot) const
	{
		int result = 0;
		for (int i = 0; i < MAX_WEAPON_POSITIONS; ++i)
			if (gWR.GetWeaponSlot(slot, i))
				++result;
		return result;
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

int CHudMoeTouch::Draw(float time)
{
	if (!pimpl->m_bActive)
		return 0;
	
	const int x = ScreenWidth * pimpl->m_flX;
	const int y = ScreenHeight * pimpl->m_flStartY;
	const float scale = 1;

	int r, g, b;

	const int iActiveSlot = pimpl->m_iActiveSlot;

	// Draw all of the buckets
	int y2 = y - iActiveSlot * 45 * scale;

	for (int iSlot = 0; iSlot < MAX_WEAPON_SLOTS; iSlot++)
	{
		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		WEAPON *p = gWR.GetFirstPos(iSlot);
		int iWidth = 175 * scale;
		int iHeight = 45 * scale;
		if (p)
		{
			iWidth = (p->rcActive.right - p->rcActive.left) * scale;
			iHeight = (p->rcActive.bottom - p->rcActive.top) * scale;
		}
		
		int x2 = x;

		bool bFirstPosition = true;
		const int giBucketWeight = 20;
		const int giBucketHeight = 20;

		const int iBoundaryX = ScreenWidth - iWidth;
		if (x < iBoundaryX)
		{
			const float extraDistance = iBoundaryX - x;
			x2 = iBoundaryX - log10(1 + extraDistance / ScreenWidth) * ScreenWidth;
		}
		if (iActiveSlot == iSlot)
			x2 -= giBucketWeight;

		for (int j = 0; j < MAX_WEAPON_POSITIONS; ++j)
		{
			const int iPos = (pimpl->m_iWeaponSlotSelectedPosition[iSlot] + j) % MAX_WEAPON_POSITIONS;

			p = gWR.GetWeaponSlot(iSlot, iPos);

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

			int x3 = x2;
			int a = 255;
			if (iActiveSlot == iSlot && pimpl->m_flSwitchPositionJudgeStartTime[iSlot] > 0.0f)
			{
				const float percent = std::min((gHUD.m_flTime - pimpl->m_flSwitchPositionJudgeStartTime[iSlot]) / POSITION_SWITCH_ANIM, 1.0f);
				x3 -= percent * iWidth;
				if (bFirstPosition)
					a = 255 * (1 - percent) * (1 - percent);
			}

			if (iActiveSlot == iSlot && bFirstPosition)
			{
				bFirstPosition = false;
				DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
				SPR_Set(gHUD.GetSprite(gHUD.m_Ammo.m_HUD_bucket0 + iSlot), r, g, b);
				SPR_DrawAdditive(0, x2, y2 - giBucketHeight, &gHUD.GetSpriteRect(gHUD.m_Ammo.m_HUD_bucket0 + iSlot));

				DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
				SPR_Set(gHUD.GetSprite(gHUD.m_Ammo.m_HUD_selection), r, g, b);
				SPR_DrawAdditive(0, x2, y2, &gHUD.GetSpriteRect(gHUD.m_Ammo.m_HUD_selection));

				DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
				DrawUtils::ScaleColors(r, g, b, a);
				SPR_Set(p->hInactive, r, g, b);
				SPR_DrawAdditive(0, x3, y2, &p->rcInactive);
			}
			else
			{
				SPR_Set(p->hInactive, r, g, b);
				SPR_DrawAdditive(0, x3, y2, &p->rcInactive);
			}

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

	if (pimpl->m_fingerID >= 0)
	{
		pimpl->m_flX = pimpl->m_flLastX;
		pimpl->m_flDx = pimpl->m_flLastDx;
		pimpl->m_flY = pimpl->m_flLastY;
		pimpl->m_flDy = pimpl->m_flLastDy;

		const float scale = 1;
		const float flWidth = 170 * scale;
		const float flHeight = 45 * scale;

		pimpl->m_flMinX = pimpl->m_flX;

		//const int iCurrentSlot = gHUD.m_Ammo.m_pWeapon ? gHUD.m_Ammo.m_pWeapon->iSlot : 0;
		const int iCurrentSlot = pimpl->m_iLastSlot;

		// switching slot ?
		if(pimpl->m_flX < 0.95 && abs(pimpl->m_flDy) >= abs(pimpl->m_flDx) / 4)
		{
			const int Result = static_cast<int>((pimpl->m_flY - pimpl->m_flStartY) * ScreenHeight / flHeight + iCurrentSlot);
			const int NewActiveSlot = std::min(std::max(Result, 0), MAX_WEAPON_SLOTS - 1);

			if (pimpl->m_iActiveSlot != NewActiveSlot)
			{
				pimpl->m_flSwitchPositionJudgeStartTime[NewActiveSlot] = -1.0f;
				pimpl->m_iActiveSlot = NewActiveSlot;
			}
		}
		
		if (pimpl->m_flX < (ScreenWidth - flWidth * 2) / ScreenWidth)
		{
			const int iSlot = pimpl->m_iActiveSlot;
			const int iWeaponCount = pimpl->WeaponCountInSlot(iSlot);
			if (iWeaponCount > 1 && !pimpl->m_flSwitchPositionJudgeAction[iSlot])
			{
				// pull and hold to start switching position
				pimpl->m_flSwitchPositionJudgeAction[iSlot] = true;
				pimpl->m_flSwitchPositionJudgeStartTime[iSlot] = gHUD.m_flTime;

			}

			// find next slot
			if (pimpl->m_flSwitchPositionJudgeStartTime[iSlot] > 0.0f && gHUD.m_flTime > pimpl->m_flSwitchPositionJudgeStartTime[iSlot] + POSITION_SWITCH_ANIM)
			{
				pimpl->m_flSwitchPositionJudgeStartTime[iSlot] = -1.0f;
				for (int j = 0; j < MAX_WEAPON_POSITIONS; ++j)
				{
					const int iPos = (pimpl->m_iWeaponSlotSelectedPosition[iSlot] + j + 1) % MAX_WEAPON_POSITIONS;
					if (gWR.GetWeaponSlot(iSlot, iPos))
					{
						pimpl->m_iWeaponSlotSelectedPosition[iSlot] = iPos;
						break;
					}
				}
			}
		}
		else
		{
			for (int iSlot = 0; iSlot < MAX_WEAPON_SLOTS; ++iSlot)
			{
				pimpl->m_flSwitchPositionJudgeAction[iSlot] = false;
			}
		}
	}
	else
	{
		pimpl->m_flX += pimpl->m_flDx;
		pimpl->m_flY += pimpl->m_flDy;

		// make reversed x motion ->
		pimpl->m_flDx += (2 - pimpl->m_flX) * 0.005;

		pimpl->m_flMinX = std::min(pimpl->m_flMinX, pimpl->m_flX);

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

		if (pimpl->m_flMinX * ScreenWidth < (ScreenWidth - 170) && pimpl->m_flDx > 0.0f)
		{
			const int iPrevSlot = gHUD.m_Ammo.m_pWeapon ? gHUD.m_Ammo.m_pWeapon->iSlot : pimpl->m_iActiveSlot;

			//WEAPON *p = gWR.GetFirstPos(pimpl->m_iActiveSlot);
			const int iSlot = pimpl->m_iActiveSlot;
			int iPos = pimpl->m_iWeaponSlotSelectedPosition[iSlot] - 1;
			if (pimpl->m_flSwitchPositionJudgeStartTime[iSlot] > 0.0f)
				iPos += 1;
			WEAPON *p = gWR.GetNextActivePos(iSlot, iPos);
			if (!p)
			{
				p = gWR.GetFirstPos(iSlot);
				pimpl->m_iWeaponSlotSelectedPosition[iSlot] = 0;
			}
				
			if (p)
			{
				ServerCmd(p->szName);
				if (pimpl->m_iActiveSlot != iPrevSlot)
					pimpl->m_iLastSlot = iPrevSlot;
			}
		}
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
	if (pimpl->m_fingerID >= 0)
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
				pimpl->m_fingerID = -1;
			}
			return 1;
		}
	}
	else
	{
		if (type == event_down)
		{
			if (x > 0.95f && abs(dy) <= -dx / 2.0f)
			{
				pimpl->m_fingerID = fingerID;
				pimpl->m_bActive = true;
				pimpl->ResetValues();
				pimpl->m_flMinX = 1;
				pimpl->m_flStartY = y;

				return 1;
			}
		}
	}

	return 0;
}
