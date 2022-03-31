/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  ammohistory.cpp
//


#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "ammohistory.h"
#include "draw_util.h"
#include "legacy/hud_scoreboard_legacy.h"

namespace cl {

HistoryResource gHR;

#define AMMO_PICKUP_GAP (gHR.iHistoryGap+5)
#define AMMO_PICKUP_PICK_HEIGHT		(gHUD.m_iFontHeight * 3 + (gHR.iHistoryGap * 2))
#define AMMO_PICKUP_HEIGHT_MAX		(ScreenHeight - 100)

#define NEWHUD_AMMO_PICKUP_GAP			24
#define NEWHUD_AMMO_PICKUP_PICK_HEIGHT		47
#define NEWHUD_WEAPON_PICKUP_GAP		23		//weapon_get_bg_new  170w 23h
#define NEWHUD_WEAPON_PICKUP_PICK_HEIGHT		160	//y2 y1 should + 23
#define NEWHUD_AMMO_PICKUP_HEIGHT_MAX		(ScreenHeight - 100)
#define NEWHUD_WEAPON_PICKUP_HEIGHT_MAX		(ScreenHeight - 100)


#define MAX_ITEM_NAME	32
int HISTORY_DRAW_TIME = 5;

// keep a list of items
struct ITEM_INFO
{
	char szName[MAX_ITEM_NAME];
	HSPRITE spr;
	wrect_t rect;
};

void HistoryResource :: AddToHistory( int iType, int iId, int iCount )
{
	if ( iType == HISTSLOT_AMMO && !iCount )
		return;  // no amount, so don't add

	if (gHUD.m_hudstyle->value == 2)
	{
		if (iType == HISTSLOT_AMMO)
		{
			if ((((NEWHUD_AMMO_PICKUP_GAP * iCurrentHistorySlot) + NEWHUD_AMMO_PICKUP_GAP) > NEWHUD_AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= NEWHUD_MAX_HISTORY))
			{	// the pic would have to be drawn too high
				// so start from the bottom
				iCurrentHistorySlot = 0;
			}

			HIST_ITEM* freeslot = &rgAmmoNewHudHistory[iCurrentHistorySlot++];  // default to just writing to the first slot
			HISTORY_DRAW_TIME = gHUD.m_Ammo.m_pHud_DrawHistory_Time->value;

			freeslot->type = iType;
			freeslot->iId = iId;
			freeslot->iCount = iCount;
			freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;
		}
		else if (iType == HISTSLOT_WEAP)
		{
			if ((((NEWHUD_WEAPON_PICKUP_GAP * iNewHudWepaonCurrentHistorySlot) + NEWHUD_WEAPON_PICKUP_PICK_HEIGHT) > NEWHUD_AMMO_PICKUP_HEIGHT_MAX) || (iNewHudWepaonCurrentHistorySlot >= NEWHUD_MAX_HISTORY))
			{	// the pic would have to be drawn too high
				// so start from the bottom
				iNewHudWepaonCurrentHistorySlot = 0;
			}

			HIST_ITEM* freeslot = &WeaponNewHudHistory[iNewHudWepaonCurrentHistorySlot++];  // default to just writing to the first slot
			HISTORY_DRAW_TIME = gHUD.m_Ammo.m_pHud_DrawHistory_Time->value;

			freeslot->type = iType;
			freeslot->iId = iId;
			freeslot->iCount = iCount;
			freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;

		}
	}
	else
	{
		if ((((AMMO_PICKUP_GAP * iCurrentHistorySlot) + AMMO_PICKUP_PICK_HEIGHT) > AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= MAX_HISTORY))
		{	// the pic would have to be drawn too high
			// so start from the bottom
			iCurrentHistorySlot = 0;
		}

		HIST_ITEM* freeslot = &rgAmmoHistory[iCurrentHistorySlot++];  // default to just writing to the first slot
		HISTORY_DRAW_TIME = gHUD.m_Ammo.m_pHud_DrawHistory_Time->value;

		freeslot->type = iType;
		freeslot->iId = iId;
		freeslot->iCount = iCount;
		freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;

	}



}

void HistoryResource :: AddToHistory( int iType, const char *szName, int iCount )
{
	if ( iType != HISTSLOT_ITEM )
		return;

	if (gHUD.m_hudstyle->value == 2)
	{
		if ((((NEWHUD_AMMO_PICKUP_GAP * iCurrentHistorySlot) + AMMO_PICKUP_PICK_HEIGHT) > NEWHUD_AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= NEWHUD_MAX_HISTORY))
		{	// the pic would have to be drawn too high
			// so start from the bottom
			iCurrentHistorySlot = 0;
		}

		HIST_ITEM* freeslot = &rgAmmoNewHudHistory[iCurrentHistorySlot++];  // default to just writing to the first slot

		int i = gHUD.GetSpriteIndex(szName);
		if (i == -1)
			return;  // unknown sprite name, don't add it to history

		freeslot->iId = i;
		freeslot->type = iType;
		freeslot->iCount = iCount;

		HISTORY_DRAW_TIME = gHUD.m_Ammo.m_pHud_DrawHistory_Time->value;
		freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;
	}
	else
	{
		if ((((AMMO_PICKUP_GAP * iCurrentHistorySlot) + AMMO_PICKUP_PICK_HEIGHT) > AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= MAX_HISTORY))
		{	// the pic would have to be drawn too high
			// so start from the bottom
			iCurrentHistorySlot = 0;
		}

		HIST_ITEM* freeslot = &rgAmmoHistory[iCurrentHistorySlot++];  // default to just writing to the first slot

	// I am really unhappy with all the code in this file
	// I am too, -- a1batross

		int i = gHUD.GetSpriteIndex(szName);
		if (i == -1)
			return;  // unknown sprite name, don't add it to history

		freeslot->iId = i;
		freeslot->type = iType;
		freeslot->iCount = iCount;

		HISTORY_DRAW_TIME = gHUD.m_Ammo.m_pHud_DrawHistory_Time->value;
		freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;
	}
}


void HistoryResource::CheckClearWeaponHistory(void)
{
	for (int i = 0; i < NEWHUD_MAX_HISTORY; i++)
	{
		if (WeaponNewHudHistory[i].type)
			return;
	}

	iNewHudWepaonCurrentHistorySlot = 0;
}

void HistoryResource :: CheckClearHistory( void )
{
	if (gHUD.m_hudstyle->value == 2)
	{
		for (int i = 0; i < NEWHUD_MAX_HISTORY; i++)
		{
			if (rgAmmoNewHudHistory[i].type)
				return;
		}

		iCurrentHistorySlot = 0;
	}
	else
	{
		for (int i = 0; i < MAX_HISTORY; i++)
		{
			if (rgAmmoHistory[i].type)
				return;
		}

		iCurrentHistorySlot = 0;
	}

}

int HistoryResource::DrawNEWHudAmmoHistory(float flTime)
{
	//fix later
	for (int i = 0; i < NEWHUD_MAX_HISTORY; i++)
	{
		if (rgAmmoNewHudHistory[i].type)
		{
			rgAmmoNewHudHistory[i].DisplayTime = min(rgAmmoNewHudHistory[i].DisplayTime, gHUD.m_flTime + HISTORY_DRAW_TIME);

			if (rgAmmoNewHudHistory[i].DisplayTime <= flTime)
			{  // pic drawing time has expired
				memset(&rgAmmoNewHudHistory[i], 0, sizeof(HIST_ITEM));
				CheckClearHistory();
			}
			else if (rgAmmoNewHudHistory[i].type == HISTSLOT_AMMO)
			{
				wrect_t rcPic;
				HSPRITE* spr = gWR.GetAmmoPicFromWeapon(rgAmmoNewHudHistory[i].iId, rcPic);

				int r, g, b;
				DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
				float scale = (rgAmmoNewHudHistory[i].DisplayTime - flTime) * 80;
				DrawUtils::ScaleColors(r, g, b, min((int)scale, 255));

				// Draw the pic
				//dollar is 13w 18h ammmo is 24w 24h  puls is 13w 18h
				int iH = rcPic.bottom - rcPic.top;
				int ypos = ScreenHeight - (NEWHUD_AMMO_PICKUP_PICK_HEIGHT + (NEWHUD_AMMO_PICKUP_GAP * i) + iH);
				int xpos = ScreenWidth - 224;
				if (spr && *spr)    // weapon isn't loaded yet so just don't draw the pic
				{ // the dll has to make sure it has sent info the weapons you need
					SPR_Set(*spr, r, g, b);
					SPR_DrawAdditive(0, xpos, ypos, &rcPic);
				}

				// Draw the number
				xpos -= DrawUtils::GetNEWHudNumberWidth(1, rgAmmoNewHudHistory[i].iCount, FALSE, 4);
				DrawUtils::DrawNEWHudNumber(1, xpos, ypos + abs(iH - gHUD.m_NEWHUD_iFontHeight_Dollar) / 2, rgAmmoNewHudHistory[i].iCount, r, g, b, min((int)scale, 255), FALSE, 4);

				//Draw Plus
				wrect_t rcPlus = gHUD.GetSpriteRect(gHUD.m_NEWHUD_hPlus);
				int iW = rcPlus.right - rcPlus.left;
				SPR_Set(gHUD.GetSprite(gHUD.m_NEWHUD_hPlus), r, g, b);
				SPR_DrawAdditive(0, xpos - iW, ypos + abs(iH - gHUD.m_NEWHUD_iFontHeight_Dollar) / 2, &rcPlus);

			}
			else if (rgAmmoNewHudHistory[i].type == HISTSLOT_ITEM)
			{
				int r, g, b;

				if (!rgAmmoNewHudHistory[i].iId)
					continue;  // sprite not loaded

				wrect_t rect = gHUD.GetSpriteRect(rgAmmoNewHudHistory[i].iId);

				DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
				float scale = (rgAmmoNewHudHistory[i].DisplayTime - flTime) * 80;
				DrawUtils::ScaleColors(r, g, b, min((int)scale, 255));

				int iH = rect.bottom - rect.top;
				int ypos = ScreenHeight - (NEWHUD_AMMO_PICKUP_PICK_HEIGHT + (NEWHUD_AMMO_PICKUP_GAP * i) + iH);
				int xpos = ScreenWidth - 224;

				SPR_Set(gHUD.GetSprite(rgAmmoNewHudHistory[i].iId), r, g, b);
				SPR_DrawAdditive(0, xpos, ypos, &rect);

				// Draw the number
				xpos -= DrawUtils::GetNEWHudNumberWidth(1, 1, FALSE, 1);
				DrawUtils::DrawNEWHudNumber(1, xpos, ypos + abs(iH - gHUD.m_NEWHUD_iFontHeight_Dollar) / 2, 1, r, g, b, min((int)scale, 255), FALSE, 1);

				wrect_t rcPlus = gHUD.GetSpriteRect(gHUD.m_NEWHUD_hPlus);
				int iW = rcPlus.right - rcPlus.left;
				SPR_Set(gHUD.GetSprite(gHUD.m_NEWHUD_hPlus), r, g, b);
				SPR_DrawAdditive(0, xpos - iW, ypos + abs(iH - gHUD.m_NEWHUD_iFontHeight_Dollar) / 2, &rcPlus);
			}
		}
	}

	//draw wpn pickup
	for (int i = 0; i < NEWHUD_MAX_HISTORY; i++)
	{
		if (WeaponNewHudHistory[i].type)
		{
			WeaponNewHudHistory[i].DisplayTime = min(WeaponNewHudHistory[i].DisplayTime, gHUD.m_flTime + HISTORY_DRAW_TIME);

			if (WeaponNewHudHistory[i].DisplayTime <= flTime)
			{  // pic drawing time has expired
				memset(&WeaponNewHudHistory[i], 0, sizeof(HIST_ITEM));
				CheckClearWeaponHistory();
			}
			else if (WeaponNewHudHistory[i].type == HISTSLOT_WEAP)
			{
				WEAPON* weap = gWR.GetWeapon(WeaponNewHudHistory[i].iId);

				if (!weap)
					continue;  // we don't know about the weapon yet, so don't draw anything

				int r, g, b;
				DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);

				if (weap->iSlot != 2 && !gWR.HasAmmo(weap))
					DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);	// if the weapon doesn't have ammo, display it as red

				float scale = (WeaponNewHudHistory[i].DisplayTime - flTime) * 80;
				DrawUtils::ScaleColors(r, g, b, min((int)scale, 255));

				wrect_t rect = gHUD.GetSpriteRect(gHUD.m_iWeaponGet);
				int iH = rect.bottom - rect.top;
				int iW = rect.right - rect.left;
				int ypos = ScreenHeight - (NEWHUD_WEAPON_PICKUP_PICK_HEIGHT + (NEWHUD_WEAPON_PICKUP_GAP * i) + iH);
				int xpos = ScreenWidth - iW - 5;

				SPR_Set(gHUD.GetSprite(gHUD.m_iWeaponGet), r, g, b);
				SPR_DrawAdditive(0, xpos, ypos, &rect);

				int iLength, iHeight;

				gEngfuncs.pfnDrawSetTextColor(1.0f, 1.0f, 1.0f);
				gEngfuncs.pfnDrawConsoleStringLen(weap->szName, &iLength, &iHeight);
				gEngfuncs.pfnDrawConsoleString(xpos, ypos + abs(iH - iHeight) / 2, weap->szName);
			}
		}
	}

	return 1;
}

//
// Draw Ammo pickup history
//
int HistoryResource :: DrawAmmoHistory( float flTime )
{
	for ( int i = 0; i < MAX_HISTORY; i++ )
	{
		if ( rgAmmoHistory[i].type )
		{
			rgAmmoHistory[i].DisplayTime = min( rgAmmoHistory[i].DisplayTime, gHUD.m_flTime + HISTORY_DRAW_TIME );

			if ( rgAmmoHistory[i].DisplayTime <= flTime )
			{  // pic drawing time has expired
				memset( &rgAmmoHistory[i], 0, sizeof(HIST_ITEM) );
				CheckClearHistory();
			}
			else if ( rgAmmoHistory[i].type == HISTSLOT_AMMO )
			{
				wrect_t rcPic;
				HSPRITE *spr = gWR.GetAmmoPicFromWeapon( rgAmmoHistory[i].iId, rcPic );

				int r, g, b;
				DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);
				float scale = (rgAmmoHistory[i].DisplayTime - flTime) * 80;
				DrawUtils::ScaleColors(r, g, b, min((int)scale, 255) );

				// Draw the pic
				int ypos = ScreenHeight - (AMMO_PICKUP_PICK_HEIGHT + (AMMO_PICKUP_GAP * i));
				int xpos = ScreenWidth - 24;
				if ( spr && *spr )    // weapon isn't loaded yet so just don't draw the pic
				{ // the dll has to make sure it has sent info the weapons you need
					SPR_Set( *spr, r, g, b );
					SPR_DrawAdditive( 0, xpos, ypos, &rcPic );
				}

				// Draw the number
				DrawUtils::DrawHudNumberString( xpos - 10, ypos, xpos - 100, rgAmmoHistory[i].iCount, r, g, b );
			}
			else if ( rgAmmoHistory[i].type == HISTSLOT_WEAP )
			{
				WEAPON *weap = gWR.GetWeapon( rgAmmoHistory[i].iId );

				if ( !weap )
					continue;  // we don't know about the weapon yet, so don't draw anything

				int r, g, b;
				DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);

				if (weap->iSlot != 2 && !gWR.HasAmmo(weap))
					DrawUtils::UnpackRGB(r,g,b, RGB_REDISH);	// if the weapon doesn't have ammo, display it as red

				float scale = (rgAmmoHistory[i].DisplayTime - flTime) * 80;
				DrawUtils::ScaleColors(r, g, b, min((int)scale, 255) );

				int ypos = ScreenHeight - (AMMO_PICKUP_PICK_HEIGHT + (AMMO_PICKUP_GAP * i));
				int xpos = ScreenWidth - (weap->rcInactive.right - weap->rcInactive.left);
				SPR_Set( weap->hInactive, r, g, b );
				SPR_DrawAdditive( 0, xpos, ypos, &weap->rcInactive );
			}
			else if ( rgAmmoHistory[i].type == HISTSLOT_ITEM )
			{
				int r, g, b;

				if ( !rgAmmoHistory[i].iId )
					continue;  // sprite not loaded

				wrect_t rect = gHUD.GetSpriteRect( rgAmmoHistory[i].iId );

				DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);
				float scale = (rgAmmoHistory[i].DisplayTime - flTime) * 80;
				DrawUtils::ScaleColors(r, g, b, min((int)scale, 255) );

				int ypos = ScreenHeight - (AMMO_PICKUP_PICK_HEIGHT + (AMMO_PICKUP_GAP * i));
				int xpos = ScreenWidth - (rect.right - rect.left) - 10;

				SPR_Set( gHUD.GetSprite( rgAmmoHistory[i].iId ), r, g, b );
				SPR_DrawAdditive( 0, xpos, ypos, &rect );
			}
		}
	}


	return 1;
}

}
