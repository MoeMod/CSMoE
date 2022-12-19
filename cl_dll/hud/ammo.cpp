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
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cvardef.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>



#include "ammohistory.h"
#include "eventscripts.h"
#include "com_weapons.h"
#include "draw_util.h"
#include "triangleapi.h"
#include "weapons_const.h"

namespace cl {

//#include "vgui_TeamFortressViewport.h"

#ifndef __linux__

void sincosf (float rad, float *sine, float *cosine)
{
   *sine = sinf (rad);
   *cosine = cosf (rad);
}

#endif

WEAPON *gpActiveSel;	// NULL means off, 1 means just the menu bar, otherwise
						// this points to the active weapon menu item
WEAPON *gpLastSel;		// Last weapon menu selection

client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

WeaponsResource gWR;

int g_weaponselect = 0;
int g_iShotsFired;


void WeaponsResource :: LoadAllWeaponSprites( void )
{
	for (auto it = rgWeapons.begin(); it != rgWeapons.end(); ++it)
	{
		LoadWeaponSprites(&it->second);
	}
}

int WeaponsResource :: CountAmmo( int iId )
{
	if ( iId < 0 || iId >= MAX_AMMO_TYPES )
		return 0;

	return riAmmo[iId];
}

int WeaponsResource :: HasAmmo( WEAPON *p )
{
	if ( !p )
		return FALSE;

	// weapons with no max ammo can always be selected
	if ( p->iMax1 == -1 )
		return TRUE;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType)
		|| CountAmmo(p->iAmmo2Type) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
}


void WeaponsResource :: LoadWeaponSprites( WEAPON *pWeapon )
{
	int i, iRes = 640;

	char sz[128];

	if ( !pWeapon )
		return;

	memset( &pWeapon->rcActive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcInactive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo2, 0, sizeof(wrect_t) );
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	sprintf(sz, "sprites/%s.txt", pWeapon->szName);
	if (strncmp(pWeapon->szName, "knife_dgaxe", 11) && 
		strncmp(pWeapon->szName, "knife_skullt9", 13) && 
		strncmp(pWeapon->szName, "knife_summonknife", 17) && 
		strncmp(pWeapon->szName, "knife_y22s1holyswordmb", 22) && 
		strncmp(pWeapon->szName, "knife_y22s2sfsword", 18) && 
		strncmp(pWeapon->szName, "knife_vulcanus9", 15))
	{
		if (!strncmp(pWeapon->szName, "knife_", 6))
		{

			client_sprite_t* pList = SPR_GetList(sz, &i);

			if (!pList)
				return;

			client_sprite_t* p;

			char* buffer = (char*)gEngfuncs.COM_LoadFile((char*)sz, 5, nullptr);
			char szSpriteName[32]{};
			int x, y, w, h;
			if (buffer && sscanf(buffer, "%s %d %d %d %d", szSpriteName, &x, &y, &w, &h) == 5)
			{
				sprintf(sz, "sprites/%s.spr", szSpriteName);
				pWeapon->hInactive = pWeapon->hActive = SPR_Load(sz);
				pWeapon->rcInactive = pWeapon->rcActive = { x, x + w, y, y + h };

				gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
			}

			gEngfuncs.COM_FreeFile(buffer);
			return;
		}
	}

	client_sprite_t *pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t *p;

	p = GetSpriteList( pList, "crosshair", iRes, i );
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hCrosshair = SPR_Load(sz);
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = 0;

	p = GetSpriteList(pList, "autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAutoaim = SPR_Load(sz);
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList( pList, "zoom", iRes, i );
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedCrosshair = SPR_Load(sz);
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList(pList, "zoom_autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedAutoaim = SPR_Load(sz);
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;  //default to zoomed crosshair
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}

	p = GetSpriteList(pList, "weapon", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hInactive = SPR_Load(sz);
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList(pList, "weapon_s", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hActive = SPR_Load(sz);
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	p = GetSpriteList(pList, "ammo", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo = SPR_Load(sz);
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo2 = 0;

}

// Returns the first weapon for a given slot.
WEAPON *WeaponsResource :: GetFirstPos( int iSlot )
{
	WEAPON *pret = NULL;

	for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
	{
		if ( rgSlots[iSlot][i] /*&& HasAmmo( rgSlots[iSlot][i] )*/ )
		{
			pret = rgSlots[iSlot][i];
			break;
		}
	}

	return pret;
}


WEAPON* WeaponsResource :: GetNextActivePos( int iSlot, int iSlotPos )
{
	if ( iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS )
		return NULL;

	WEAPON *p = gWR.rgSlots[ iSlot ][ iSlotPos+1 ];

	if ( !p || !gWR.HasAmmo(p) )
		return GetNextActivePos( iSlot, iSlotPos + 1 );

	return p;
}


int giBucketHeight, giBucketWidth, giABHeight, giABWidth, g_iSelectionWidth, g_iSelectionHeight; // Ammo Bar width and height

HSPRITE ghsprBuckets;					// Sprite for top row of weapons menu

DECLARE_MESSAGE(m_Ammo, CurWeapon ) // Current weapon and clip
DECLARE_MESSAGE(m_Ammo, WeaponList) // new weapon type
DECLARE_MESSAGE(m_Ammo, WeaponList2) // new weapon type
DECLARE_MESSAGE(m_Ammo, AmmoX)      // update known ammo type's count
DECLARE_MESSAGE(m_Ammo, AmmoPickup) // flashes an ammo pickup record
DECLARE_MESSAGE(m_Ammo, WeapPickup) // flashes a weapon pickup record
DECLARE_MESSAGE(m_Ammo, HideWeapon) // hides the weapon, ammo, and crosshair displays temporarily
DECLARE_MESSAGE(m_Ammo, ItemPickup)
DECLARE_MESSAGE(m_Ammo, Crosshair)
DECLARE_MESSAGE(m_Ammo, Brass)
DECLARE_MESSAGE(m_Ammo, WeapDrop) // weapon drop

DECLARE_COMMAND(m_Ammo, Slot1)
DECLARE_COMMAND(m_Ammo, Slot2)
DECLARE_COMMAND(m_Ammo, Slot3)
DECLARE_COMMAND(m_Ammo, Slot4)
DECLARE_COMMAND(m_Ammo, Slot5)
DECLARE_COMMAND(m_Ammo, Slot6)
DECLARE_COMMAND(m_Ammo, Slot7)
DECLARE_COMMAND(m_Ammo, Slot8)
DECLARE_COMMAND(m_Ammo, Slot9)
DECLARE_COMMAND(m_Ammo, Slot10)
DECLARE_COMMAND(m_Ammo, Close)
DECLARE_COMMAND(m_Ammo, NextWeapon)
DECLARE_COMMAND(m_Ammo, PrevWeapon)
DECLARE_COMMAND(m_Ammo, Adjust_Crosshair)
DECLARE_COMMAND(m_Ammo, Rebuy)
DECLARE_COMMAND(m_Ammo, Autobuy)

// width of ammo fonts
#define AMMO_SMALL_WIDTH 10

int CHudAmmo::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(CurWeapon);
	HOOK_MESSAGE(WeaponList);
	HOOK_MESSAGE(WeaponList2);
	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(AmmoX);
	HOOK_MESSAGE(Crosshair);
	HOOK_MESSAGE(Brass);
	HOOK_MESSAGE(WeapDrop);

	HOOK_COMMAND("slot1", Slot1);
	HOOK_COMMAND("slot2", Slot2);
	HOOK_COMMAND("slot3", Slot3);
	HOOK_COMMAND("slot4", Slot4);
	HOOK_COMMAND("slot5", Slot5);
	HOOK_COMMAND("slot6", Slot6);
	HOOK_COMMAND("slot7", Slot7);
	HOOK_COMMAND("slot8", Slot8);
	HOOK_COMMAND("slot9", Slot9);
	HOOK_COMMAND("slot10", Slot10);
	HOOK_COMMAND("cancelselect", Close);
	HOOK_COMMAND("invnext", NextWeapon);
	HOOK_COMMAND("invprev", PrevWeapon);
	HOOK_COMMAND("adjust_crosshair", Adjust_Crosshair);
	HOOK_COMMAND("rebuy", Rebuy);
	HOOK_COMMAND("autobuy", Autobuy);

	Reset();

	m_pHud_DrawHistory_Time = CVAR_CREATE( "hud_drawhistory_time", "5", 0 );
	m_pHud_FastSwitch = CVAR_CREATE( "hud_fastswitch", "1", FCVAR_ARCHIVE );		// controls whether or not weapons can be selected in one keypress
	CVAR_CREATE( "cl_observercrosshair", "1", 0 );
	m_pClCrosshairColor = CVAR_CREATE( "cl_crosshair_color", "50 250 50", FCVAR_ARCHIVE );
	m_pClCrosshairTranslucent = CVAR_CREATE( "cl_crosshair_translucent", "1", FCVAR_ARCHIVE );
	m_pClCrosshairSize = CVAR_CREATE( "cl_crosshair_size", "auto", FCVAR_ARCHIVE );
	m_pClDynamicCrosshair = CVAR_CREATE("cl_dynamiccrosshair", "1", FCVAR_ARCHIVE);
	cl_crosshair_type = CVAR_CREATE("cl_crosshair_type", "1", FCVAR_ARCHIVE);

	m_iFlags = HUD_DRAW | HUD_THINK; //!!!
	m_R = 50;
	m_G = 250;
	m_B = 50;
	m_iAlpha = 200;

	m_cvarB = m_cvarR = m_cvarG = -1;
	m_iCurrentCrosshair = 0;
	m_bAdditive = true;
	m_iCrosshairScaleBase = -1;
	m_bDrawCrosshair = true;

	gWR.Init();
	gHR.Init();

	return 1;
}

void CHudAmmo::Reset(void)
{
	m_fFade = 0;

	gpActiveSel = NULL;
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();

	//	VidInit();

}

int CHudAmmo::VidInit(void)
{
	// Load sprites for buckets (top row of weapon menu)
	m_HUD_bucket0 = gHUD.GetSpriteIndex( "bucket1" );
	m_HUD_selection = gHUD.GetSpriteIndex( "selection" );

	m_iWeaponSelect = gHUD.GetSpriteIndex("weapon_selection_new");

	g_iSelectionWidth = gHUD.GetSpriteRect(m_HUD_selection).right - gHUD.GetSpriteRect(m_HUD_selection).left;
	g_iSelectionHeight = gHUD.GetSpriteRect(m_HUD_selection).bottom - gHUD.GetSpriteRect(m_HUD_selection).top;
	ghsprBuckets = gHUD.GetSprite(m_HUD_bucket0);
	giBucketWidth = gHUD.GetSpriteRect(m_HUD_bucket0).right - gHUD.GetSpriteRect(m_HUD_bucket0).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top;
	gHR.iHistoryGap = max(gHR.iHistoryGap, gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top);

	if(!m_iWeaponList)
		m_iWeaponList = R_LoadTextureUnique("resource/hud/weapon_list_new");

	if (!m_iWeapon_OffBG)
		m_iWeapon_OffBG = R_LoadTextureUnique("resource/hud/hud_weapon_off_bg");

	R_InitTexture(m_pTexture_Black, "resource/hud/csgo/blacka");
	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

	if (ScreenWidth >= 640)
	{
		giABWidth = 20;
		giABHeight = 4;
	}
	else
	{
		giABWidth = 10;
		giABHeight = 2;
	}

	m_flLastBuffHit = 0.0;
	m_hBuffHit = SPR_Load("sprites/ishot.spr");
	m_iInfinite = gHUD.GetSpriteIndex("infinite");
	/*m_hBuffHit = (struct model_s*)gEngfuncs.GetSpritePointer(gEngfuncs.pfnSPR_Load("sprites/ishot.spr"));
	m_iBuffHitHeight = SPR_Height(m_hBuffHit, 0);
	m_iBuffHitWidth = SPR_Width(m_hBuffHit, 0);*/
	return 1;
}

//
// Think:
//  Used for selection of weapon menu item.
//
void CHudAmmo::Think(void)
{
	if ( gHUD.m_fPlayerDead )
		return;

	/*if ( gHUD.m_iWeaponBits != gWR.iOldWeaponBits )
	{
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = 0; i < MAX_WEAPONS-1; i++ )
		{
			WEAPON *p = gWR.GetWeapon(i);

			if ( p )
			{
				if ( gHUD.m_iWeaponBits & ( 1 << p->iId ) )
				{
					gWR.PickupWeapon( p );
				}
				else
				{
					WEAPON* p2 = gWR.GetWeaponSlot(p->iSlot, p->iSlotPos);
					if (p2 && gHUD.m_iWeaponBits & (1 << p2->iId))
						continue;

					gWR.DropWeapon( p );
				}
			}
		}
	}*/

	if (!gpActiveSel)
		return;

	// has the player selected one?
	if (gHUD.m_iKeyBits & IN_ATTACK)
	{
		if (gpActiveSel != (WEAPON *)1)
		{
			ServerCmd(gpActiveSel->szName);
			g_weaponselect = gpActiveSel->iId;
		}

		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;

		PlaySound("common/wpn_select.wav", 1);
	}

}

//
// Helper function to return a Ammo pointer from id
//

HSPRITE* WeaponsResource :: GetAmmoPicFromWeapon( int iAmmoId, wrect_t& rect )
{
	for (auto it = rgWeapons.begin(); it != rgWeapons.end(); ++it)
	{
		if (it->second.iAmmoType == iAmmoId)
		{
			rect = it->second.rcAmmo;
			return &it->second.hAmmo;
		}
		else if (it->second.iAmmo2Type == iAmmoId)
		{
			rect = it->second.rcAmmo2;
			return &it->second.hAmmo2;
		}
	}

	/*for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if ( rgWeapons[i].iAmmoType == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if ( rgWeapons[i].iAmmo2Type == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}*/

	return NULL;
}


// Menu Selection Code

void WeaponsResource :: SelectSlot( int iSlot, int fAdvance, int iDirection )
{
	if ( gHUD.m_Menu.m_fMenuDisplayed && (fAdvance == FALSE) && (iDirection == 1) )
	{ // menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem( iSlot + 1 );  // slots are one off the key numbers
		return;
	}

	if ((gHUD.m_ZB2.m_iFlags & HUD_ACTIVE) && (fAdvance == FALSE) && (iDirection == 1))
	{
		if (gHUD.m_ZB2.SelectorCanDraw())
		{
			if (gHUD.m_ZB2.Selector((iSlot + 1)) == 10 ? 0 : (iSlot + 1))
				return;
		}
		if (gHUD.m_ZB2.ActivateSkill(iSlot + 1))
			return;
	}

	if ((gHUD.m_ZB4.m_iFlags & HUD_ACTIVE) && (fAdvance == FALSE) && (iDirection == 1))
	{
		if(gHUD.m_ZB4.ActivateSkill(iSlot + 1))
			return;
	}

	if ( iSlot > MAX_WEAPON_SLOTS )
		return;

	if ( gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return;


	WEAPON *p = NULL;
	bool fastSwitch = gHUD.m_Ammo.m_pHud_FastSwitch->value != 0.0f;

	if ( (gpActiveSel == NULL) || (gpActiveSel == (WEAPON *)1) || (iSlot != gpActiveSel->iSlot) )
	{
		PlaySound( "common/wpn_hudon.wav", 1 );
		p = GetFirstPos( iSlot );

		if ( p && fastSwitch ) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON *p2 = GetNextActivePos( p->iSlot, p->iSlotPos );
			if ( !p2 )
			{	// only one active item in bucket, so change directly to weapon
				ServerCmd( p->szName );
				g_weaponselect = p->iId;
				return;
			}
		}
	}
	else
	{
		PlaySound("common/wpn_moveselect.wav", 1);
		if ( gpActiveSel )
			p = GetNextActivePos( gpActiveSel->iSlot, gpActiveSel->iSlotPos );
		if ( !p )
			p = GetFirstPos( iSlot );
	}


	if ( !p )  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it
		if ( !fastSwitch )
			gpActiveSel = (WEAPON *)1;
		else
			gpActiveSel = NULL;
	}
	else
		gpActiveSel = p;
}

//------------------------------------------------------------------------
// Message Handlers
//------------------------------------------------------------------------

//
// AmmoX  -- Update the count of a known type of ammo
// 
int CHudAmmo::MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	int iIndex = reader.ReadByte();
	// fuck 255 limit
	int iCount;
	if (iSize == 3)
	{
		iCount = reader.ReadShort();
	}
	else
	{
		iCount = reader.ReadByte();
	}

	gWR.SetAmmo( iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	int iIndex = reader.ReadByte();
	int iCount = reader.ReadByte();

	// Add ammo to the history
	gHR.AddToHistory( HISTSLOT_AMMO, iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	int iIndex = reader.ReadShort();

	// Add the weapon to the history
	gHR.AddToHistory(HISTSLOT_WEAP, iIndex);
	gWR.PickupWeapon(gWR.GetWeapon(iIndex));

	return 1;
}

int CHudAmmo::MsgFunc_WeapDrop(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);
	int iIndex = reader.ReadShort();
	if (!iIndex) {
		gWR.DropAllWeapons();
	} else {
		gWR.DropWeapon(gWR.GetWeapon(iIndex));
	}

	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	const char *szName = reader.ReadString();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_ITEM, szName );

	return 1;
}


int CHudAmmo::MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	gHUD.m_iHideHUDDisplay = reader.ReadByte();

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_FLASHLIGHT | HIDEHUD_ALL ) )
	{
		gpActiveSel = NULL;
		SetCrosshair( 0, nullrc, 0, 0, 0 );
	}
	else
	{
		if ( m_pWeapon )
			SetCrosshair( m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255 );
	}

	return 1;
}

// 
//  CurWeapon: Update hud state with the current weapon and clip count. Ammo
//  counts are updated with AmmoX. Server assures that the Weapon ammo type 
//  numbers match a real ammo type.
//
int CHudAmmo::MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	int iState = reader.ReadByte();
	int iId = 0;
	int iClip = 0;
	if (iSize == 3)
	{
		iId = reader.ReadByte();
		iClip = reader.ReadByte();
	}
	else if (iSize == 4)
	{
		iId = reader.ReadShort();
		iClip = reader.ReadByte();
		if (iClip == 255)
			iClip = -1;
	}
	else
	{
		iId = reader.ReadShort();
		iClip = reader.ReadShort();
	}

	if ( iId < 1 )
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 0;
	}

	if ( g_iUser1 != OBS_IN_EYE )
	{
		// Is player dead???
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = TRUE;
			gpActiveSel = NULL;
			return 1;
		}
		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon( iId );

	if ( !pWeapon )
		return 0;

	if ( iClip < -1 )
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;


	if ( iState == 0 )	// we're not the current weapon, so update no more
		return 1;

	m_pWeapon = pWeapon;

	/*if( gHUD.m_iFOV <= 40 )
	{ // zoomed crosshairs
		if (fOnTarget && m_pWeapon->hZoomedAutoaim)
			SetCrosshair(m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);

	}
	else
	{
		SetCrosshair( 0, nullrc, 0, 0, 0);
	}*/


	m_fFade = 200.0f; //!!!

	return 1;
}

//
// WeaponList -- Tells the hud about a new weapon type.
//
int CHudAmmo::MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	WEAPON Weapon;

	strncpy( Weapon.szName, reader.ReadString(), MAX_WEAPON_NAME );
	Weapon.iAmmoType = reader.ReadChar();

	Weapon.iMax1 = reader.ReadByte();
	if (Weapon.iMax1 == 255)
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = reader.ReadChar();
	Weapon.iMax2 = reader.ReadByte();
	if (Weapon.iMax2 == 255)
		Weapon.iMax2 = -1;

	Weapon.iSlot = reader.ReadChar();
	Weapon.iSlotPos = reader.ReadChar();
	Weapon.iId = reader.ReadShort();
	Weapon.iFlags = reader.ReadByte();
	Weapon.iClip = 0;

	gWR.AddWeapon( &Weapon );

	return 1;

}

int CHudAmmo::MsgFunc_WeaponList2(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);

	WEAPON Weapon;

	strncpy(Weapon.szName, reader.ReadString(), MAX_WEAPON_NAME);
	Weapon.iAmmoType = reader.ReadShort();

	Weapon.iMax1 = reader.ReadByte();
	if (Weapon.iMax1 == 255)
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = reader.ReadShort();
	Weapon.iMax2 = reader.ReadByte();
	if (Weapon.iMax2 == 255)
		Weapon.iMax2 = -1;

	Weapon.iSlot = reader.ReadChar();
	Weapon.iSlotPos = reader.ReadChar();
	Weapon.iId = reader.ReadShort();
	Weapon.iFlags = reader.ReadByte();
	Weapon.iClip = 0;

	gWR.AddWeapon(&Weapon);

	return 1;

}

int CHudAmmo::MsgFunc_Crosshair(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	if( reader.ReadByte() > 0)
	{
		m_bDrawCrosshair = true;
	}
	else
	{
		m_bDrawCrosshair = false;
	}
   return 0;
}

int CHudAmmo::MsgFunc_Brass( const char *pszName, int iSize, void *pbuf )
{
	bool useNewBehavior = true;

	BufferReader reader( pszName, pbuf, iSize );
	if (!useNewBehavior)
	{
		reader.ReadByte(); // unused!
	}
	Vector origin, velocity;
	origin.x = reader.ReadCoord();
	origin.y = reader.ReadCoord();
	origin.z = reader.ReadCoord();
	if (!useNewBehavior)
	{
		reader.ReadCoord(); // unused!
		reader.ReadCoord(); // unused!
		reader.ReadCoord(); // unused!
	}	
	velocity.x = reader.ReadCoord();
	velocity.y = reader.ReadCoord();
	velocity.z = reader.ReadCoord();

	float Rotation = M_PI * reader.ReadAngle() / 180.0f;
	int ModelIndex = reader.ReadShort();
	int BounceSoundType = reader.ReadByte();
	int Life = 2.5;
	if (!useNewBehavior)
	{
		Life = reader.ReadByte();
	}
	int Client = reader.ReadByte();

	float sin, cos, x, y;
	sincosf( fabs(Rotation), &sin, &cos );

	if( gHUD.cl_righthand->value != 0.0f && EV_IsLocal( Client ) )
	{
		velocity.x += sin * -120.0;
		velocity.y += cos * 120.0;
		x = 9.0 * sin;
		y = -9.0 * cos;
	}
	else
	{
		x = -9.0 * sin;
		y = 9.0 * cos;
	}

	origin.x += x;
	origin.y += y;
	EV_EjectBrass( origin, velocity, Rotation, ModelIndex, BounceSoundType, Life );
	return 1;
}

//------------------------------------------------------------------------
// Command Handlers
//------------------------------------------------------------------------
// Slot button pressed
void CHudAmmo::SlotInput( int iSlot )
{
	// Let the Viewport use it first, for menus
	//if ( gViewPort && gViewPort->SlotInput( iSlot ) )
		//return;

	gWR.SelectSlot(iSlot, FALSE, 1);
}

void CHudAmmo::UserCmd_Slot1(void)
{
	SlotInput( 0 );
}

void CHudAmmo::UserCmd_Slot2(void)
{
	SlotInput( 1 );
}

void CHudAmmo::UserCmd_Slot3(void)
{
	SlotInput( 2 );
}

void CHudAmmo::UserCmd_Slot4(void)
{
	SlotInput( 3 );
}

void CHudAmmo::UserCmd_Slot5(void)
{
	SlotInput( 4 );
}

void CHudAmmo::UserCmd_Slot6(void)
{
	SlotInput( 5 );
}

void CHudAmmo::UserCmd_Slot7(void)
{
	SlotInput( 6 );
}

void CHudAmmo::UserCmd_Slot8(void)
{
	SlotInput( 7 );
}

void CHudAmmo::UserCmd_Slot9(void)
{
	SlotInput( 8 );
}

void CHudAmmo::UserCmd_Slot10(void)
{
	SlotInput( 9 );
}

void CHudAmmo::UserCmd_Adjust_Crosshair()
{
	int newCrosshair;
	int oldCrosshair = m_iCurrentCrosshair;

	if ( gEngfuncs.Cmd_Argc() <= 1 )
	{
		newCrosshair = (oldCrosshair + 1) % 5;
	}
	else
	{
		const char *arg = gEngfuncs.Cmd_Argv(1);
		newCrosshair = atoi(arg) % 10;
	}

	m_iCurrentCrosshair = newCrosshair;
	if ( newCrosshair <= 9 )
	{
		switch ( newCrosshair )
		{
		case 0:
		case 5:
			m_R = 50;
			m_G = 250;
			m_B = 50;
			break;
		case 1:
		case 6:
			m_R = 250;
			m_G = 50;
			m_B = 50;
			break;
		case 2:
		case 7:
			m_R = 50;
			m_G = 50;
			m_B = 250;
			break;
		case 3:
		case 8:
			m_R = 250;
			m_G = 250;
			m_B = 50;
			break;
		case 4:
		case 9:
			m_R = 50;
			m_G = 250;
			m_B = 250;
			break;
		}
		m_bAdditive = newCrosshair < 5 ? true: false;
	}
	else
	{
		m_R = 50;
		m_G = 250;
		m_B = 50;
		m_bAdditive = 1;
	}

	char s[16];
	sprintf(s, "%d %d %d", m_R, m_G, m_B);
	gEngfuncs.Cvar_Set("cl_crosshair_color", s);
	gEngfuncs.Cvar_Set("cl_crosshair_translucent", (char*)(m_bAdditive ? "1" : "0"));
}


void CHudAmmo::UserCmd_Close(void)
{
	if (gpActiveSel)
	{
		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		PlaySound("common/wpn_hudoff.wav", 1);
	}
	else
		ClientCmd("escape");
}


// Selects the next item in the weapon menu
void CHudAmmo::UserCmd_NextWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = 0;
	int slot = 0;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos + 1;
		slot = gpActiveSel->iSlot;
	}

	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot < MAX_WEAPON_SLOTS; slot++ )
		{
			for ( ; pos < MAX_WEAPON_POSITIONS; pos++ )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp /*&& gWR.HasAmmo(wsp)*/ )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = 0;
		}

		slot = 0;  // start looking from the first slot again
	}

	gpActiveSel = NULL;
}

// Selects the previous item in the menu
void CHudAmmo::UserCmd_PrevWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = MAX_WEAPON_POSITIONS-1;
	int slot = MAX_WEAPON_SLOTS-1;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos - 1;
		slot = gpActiveSel->iSlot;
	}

	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot >= 0; slot-- )
		{
			for ( ; pos >= 0; pos-- )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp /*&& gWR.HasAmmo(wsp)*/ )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS-1;
		}

		slot = MAX_WEAPON_SLOTS-1;
	}

	gpActiveSel = NULL;
}

void CHudAmmo::UserCmd_Autobuy()
{
	char *afile = (char*)gEngfuncs.COM_LoadFile("autobuy.txt", 5, NULL);
	char *pfile = afile;
	char token[256];
	char szCmd[1024];

	if( !pfile )
	{
		ConsolePrint( "Can't open autobuy.txt file.\n" );
		return;
	}

	strncpy(szCmd, "cl_setautobuy", sizeof(szCmd));

	while((pfile = gEngfuncs.COM_ParseFile( pfile, token )))
	{
		// append space first
		strcat(szCmd, " ");
		strcat(szCmd, token);
	}

	ConsolePrint( szCmd );
	gEngfuncs.pfnServerCmd( szCmd );

	gEngfuncs.COM_FreeFile( afile );
}

void CHudAmmo::UserCmd_Rebuy()
{
	char *afile = (char*)gEngfuncs.COM_LoadFile("rebuy.txt", 5, NULL);
	char *pfile = afile;
	char token[64];
	char szCmd[1024];
	int lastCh;

	if( !pfile )
	{
		ConsolePrint( "Can't open rebuy.txt file.\n" );
		return;
	}

	// start with \"
	strncpy(szCmd, "cl_setrebuy \"", sizeof(szCmd));



	while((pfile = gEngfuncs.COM_ParseFile( pfile, token )))
	{
		strcat(szCmd, token);
		// append space after token
		strcat(szCmd, " ");
	}
	// replace last space with ", before terminator
	lastCh = strlen(szCmd);
	szCmd[lastCh - 1] = '\"';

	ConsolePrint(szCmd);
	gEngfuncs.pfnServerCmd(szCmd);

	gEngfuncs.COM_FreeFile( afile );
}


//-------------------------------------------------------------------------
// Drawing code
//-------------------------------------------------------------------------

int CHudAmmo::Draw(float flTime)
{
	int a, x, y, r, g, b;
	int AmmoWidth;
	static bool switchCrosshairType = false;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;

	// place it here, so pretty dynamic crosshair will work even in spectator!
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_CROSSHAIR && m_pWeapon)
	{
		SetCrosshair(m_pWeapon->hAutoaim, m_pWeapon->rcAutoaim, 255, 255, 255);	
		switchCrosshairType = true;
	}
	else if( gHUD.m_iFOV > 40 )
	{
		if (m_pWeapon && gHUD.m_iFOV < 90 && (m_pWeapon->iId == WEAPON_SKULL5 ||
			m_pWeapon->iId == WEAPON_SKULL6 ||
			m_pWeapon->iId == WEAPON_CROSSBOW ||
			m_pWeapon->iId == WEAPON_KINGCOBRA ||
			m_pWeapon->iId == WEAPON_KINGCOBRAG ||
			m_pWeapon->iId == WEAPON_DESTROYER))
		{
			if (!switchCrosshairType)
			{
				SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);
				switchCrosshairType = true;
			}
		}
		else
		{
			if (switchCrosshairType)
			{
				SetCrosshair(0, nullrc, 0, 0, 0);
				switchCrosshairType = false;
			}
			// draw a dynamic crosshair
			DrawCrosshair(flTime);
		}
	}
	else
	{
		if (m_pWeapon && m_pWeapon->iId == WEAPON_SFSNIPER)
			return 1;

		if (m_pWeapon && !switchCrosshairType)
		{
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);
			switchCrosshairType = true;
		}
	}

	if ( (gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL )) )
		return 1;

	// Draw Weapon Menu
	/*if (gHUD.m_hudstyle->value == 2)
		DrawNEWHudWList(flTime);
	else*/
		DrawWList(flTime);

	if (m_flLastBuffHit > flTime)
	{
		/*SPR_Set(m_hBuffHit, 255, 0, 0);
		SPR_DrawAdditive(0, ScreenWidth / 2 - m_iBuffHitWidth / 2, ScreenHeight / 2 - m_iBuffHitHeight / 2, NULL);*/

		int iWidth = ScreenWidth * 0.05f;
		int iX = (ScreenWidth - iWidth) / 2;
		int iY = (ScreenHeight - iWidth) / 2;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
		model_t* buffHit = (struct model_s*)gEngfuncs.GetSpritePointer(m_hBuffHit);
		gEngfuncs.pTriAPI->SpriteTexture(buffHit, 0);

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		gEngfuncs.pTriAPI->Vertex3f(iX * gHUD.m_flScale, iY * gHUD.m_flScale, 0);
		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * gHUD.m_flScale, iY * gHUD.m_flScale, 0);
		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * gHUD.m_flScale, (iY + iWidth) * gHUD.m_flScale, 0);
		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		gEngfuncs.pTriAPI->Vertex3f(iX * gHUD.m_flScale, (iY + iWidth) * gHUD.m_flScale, 0);


		gEngfuncs.pTriAPI->End();
	}

	// Draw ammo pickup history
	if (gHUD.m_hudstyle->value == 2)
		gHR.DrawNEWHudAmmoHistory(flTime);
	else
		gHR.DrawAmmoHistory(flTime);


	if (!m_pWeapon)
		return 0;

	WEAPON *pw = m_pWeapon; // shorthand
	if (gHUD.m_hudstyle->value == 1)
		DrawWpnList(flTime);
	else if (gHUD.m_hudstyle->value == 2)
		DrawNEWHudCurrentWpn(flTime);

	// SPR_Draw Ammo
	if ((pw->iAmmoType < 0) && (pw->iAmmo2Type < 0))
		return 0;

	if (gHUD.m_hudstyle->value == 2)
	{
		DrawNEWHudAmmo(flTime);
		return 1;
	}

	int iFlags = DHN_DRAWZERO; // draw 0 values
	AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

	a = max( MIN_ALPHA, (int)m_fFade );

	if (m_fFade > 0)
		m_fFade -= (gHUD.m_flTimeDelta * 20);

	// Does this weapon have a clip?
	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight/2;

	if (gHUD.m_hudstyle->value == 1)
	{
		int iIconWidth = m_pWeapon->rcAmmo.right - m_pWeapon->rcAmmo.left;
		x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;
		m_pTexture_Black->Draw2DQuadScaled(x, y - gHUD.m_iFontHeight / 2, x + (8 * AmmoWidth) + iIconWidth, ScreenHeight, 0, 0, 1, 1, 255, 255, 255, 100);
	}
	// Does weapon have any ammo at all?
	if (m_pWeapon->iAmmoType > 0)
	{
		DrawUtils::UnpackRGB(r, g, b, gHUD.m_hudstyle->value == 1 ? RGB_WHITE : RGB_YELLOWISH);
		DrawUtils::ScaleColors(r, g, b, a);

		int iIconWidth = m_pWeapon->rcAmmo.right - m_pWeapon->rcAmmo.left;

		if (pw->iClip >= 0)
		{
			// room for the number and the '|' and the current ammo

			x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;

			x = DrawUtils::DrawHudNumber(x, y, iFlags | DHN_3DIGITS, pw->iClip, r, g, b);

			int iBarWidth =  AmmoWidth/10;

			x += AmmoWidth/2;

			DrawUtils::UnpackRGB(r,g,b, gHUD.m_hudstyle->value == 1 ? RGB_WHITE : RGB_YELLOWISH);

			// draw the | bar
			FillRGBA(x, y, iBarWidth, gHUD.m_iFontHeight, r, g, b, a);

			x += iBarWidth + AmmoWidth/2;;

			// GL Seems to need this
			DrawUtils::ScaleColors(r, g, b, a );

			if (gWR.CountAmmo(pw->iAmmoType) > 1500)
			{
				SPR_Set(gHUD.GetSprite(m_iInfinite), r, g, b);
				SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_iInfinite));
				x = ScreenWidth - AmmoWidth - iIconWidth;
			}
			else
				x = DrawUtils::DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);


		}
		else
		{
			if (pw->iSlot < 3)
			{
				// No clip weapon, draws blue special ammo
				DrawUtils::UnpackRGB(r, g, b, RGB_LIGHTBLUE);
				//DrawUtils::ScaleColors(r, g, b, a);
			}


			// SPR_Draw a bullets only line

		

			if (gWR.CountAmmo(pw->iAmmoType) > 1500)
			{
				x = ScreenWidth - AmmoWidth - iIconWidth;
				int width = gHUD.GetSpriteRect(m_iInfinite).right - gHUD.GetSpriteRect(m_iInfinite).left;
				x -= width;

				SPR_Set(gHUD.GetSprite(m_iInfinite), r, g, b);
				SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_iInfinite));
				x = ScreenWidth - AmmoWidth - iIconWidth;
			}
			else
			{
				if (m_pWeapon->iId != WEAPON_PATROLDRONE)
				{
					x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
					x = DrawUtils::DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);
				}
			}
		}

		// Draw the ammo Icon
		if (m_pWeapon->iId != WEAPON_PATROLDRONE)
		{
			int iOffset = (m_pWeapon->rcAmmo.bottom - m_pWeapon->rcAmmo.top) / 8;
			SPR_Set(m_pWeapon->hAmmo, r, g, b);
			SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo);
		}
	}

	// Does weapon have seconday ammo?
	if (pw->iAmmo2Type > 0)
	{
		// No clip weapon, draws blue special ammo
		DrawUtils::UnpackRGB(r, g, b, RGB_LIGHTBLUE);
		//DrawUtils::ScaleColors(r, g, b, a);

		int iIconWidth = m_pWeapon->rcAmmo2.right - m_pWeapon->rcAmmo2.left;

		// Do we have secondary ammo?
		if ((pw->iAmmo2Type != 0) && (gWR.CountAmmo(pw->iAmmo2Type) > 0))
		{
			y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight/4;
			x = ScreenWidth - 4 * AmmoWidth - iIconWidth;

			if (gWR.CountAmmo(pw->iAmmo2Type) >= 255)
			{
				x = ScreenWidth - AmmoWidth - iIconWidth;
				int width = gHUD.GetSpriteRect(m_iInfinite).right - gHUD.GetSpriteRect(m_iInfinite).left;
				x -= width;

				SPR_Set(gHUD.GetSprite(m_iInfinite), r, g, b);
				SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_iInfinite));
				x = ScreenWidth - AmmoWidth - iIconWidth;
			}
			else
				x = DrawUtils::DrawHudNumber(x, y, iFlags|DHN_3DIGITS, gWR.CountAmmo(pw->iAmmo2Type), r, g, b);

			// Draw the ammo Icon
			SPR_Set(m_pWeapon->hAmmo2, r, g, b);
			int iOffset = (m_pWeapon->rcAmmo2.bottom - m_pWeapon->rcAmmo2.top)/8;
			SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo2);
		}
	}

	return 1;
}

#define WEST_XPOS (ScreenWidth / 2 - flCrosshairDistance - iLength + 1)
#define EAST_XPOS (flCrosshairDistance + ScreenWidth / 2)
#define EAST_WEST_YPOS (ScreenHeight / 2)

#define NORTH_YPOS (ScreenHeight / 2 - flCrosshairDistance - iLength + 1)
#define SOUTH_YPOS (ScreenHeight / 2 + flCrosshairDistance)
#define NORTH_SOUTH_XPOS (ScreenWidth / 2)


void CHudAmmo::DrawCrosshair( float flTime )
{
	WeaponCrosshairData data;
	int iLength, weaponid;
	float flCrosshairDistance;

	if( !m_pWeapon )
		return;

	weaponid = m_pWeapon->iId;

	if(    weaponid == WEAPON_AWP
		|| weaponid == WEAPON_SCOUT
		|| weaponid == WEAPON_SG550
		|| weaponid == WEAPON_G3SG1	
		|| weaponid == WEAPON_XM8SHARPSHOOTER
		|| weaponid == WEAPON_AS50
		|| weaponid == WEAPON_AS50G
		|| weaponid == WEAPON_BENDITA
		|| weaponid == WEAPON_SKULL5
		|| weaponid == WEAPON_Z4B_AWPNVIDIA
		|| weaponid == WEAPON_M95
		|| weaponid == WEAPON_M95XMAS
		|| weaponid == WEAPON_WA2000
		|| weaponid == WEAPON_Z4B_XM2010PC
		|| weaponid == WEAPON_Z4B_M1887SE
		|| weaponid == WEAPON_SFSNIPER
		|| weaponid == WEAPON_CHEYTACLRRS
		|| weaponid == WEAPON_M400
		|| weaponid == WEAPON_M82
		|| weaponid == WEAPON_SL8
		|| weaponid == WEAPON_SL8G
		|| weaponid == WEAPON_SL8EX
		|| weaponid == WEAPON_TRG42
		|| weaponid == WEAPON_TRG42G
		|| weaponid == WEAPON_M24
		|| weaponid == WEAPON_AW50
		|| weaponid == WEAPON_SPRIFLE
		|| weaponid == WEAPON_BPGM
		|| weaponid == WEAPON_ZGUN
		|| weaponid == WEAPON_HUNTBOW
		|| weaponid == WEAPON_CHAINSAW
		|| weaponid == WEAPON_CHAINSAWM
		|| weaponid == WEAPON_HEROCHAINSAW
		|| weaponid == WEAPON_Z4B_BARRETTD
		|| weaponid == WEAPON_BUFFAWP
		|| weaponid == WEAPON_DESTROYER
		|| weaponid == WEAPON_STARCHASERSR
		|| weaponid == WEAPON_MOSIN
		|| weaponid == WEAPON_CARTBLUES)
		return;

	if ( g_iWeaponFlags & WPNSTATE_SHIELD_DRAWN )
		return;

	data = GetWeaponCrosshairData();

	if ( data.iAccuracyFlags && m_pClDynamicCrosshair->value && !(gHUD.m_iHideHUDDisplay & 1) )
	{
		if ( g_iPlayerFlags & FL_ONGROUND || !( data.iAccuracyFlags & ACCURACY_AIR ) )
		{
			if ( ( g_iPlayerFlags & FL_DUCKING ) && ( data.iAccuracyFlags & ACCURACY_DUCK ) )
			{
				data.iDistance *= 0.5;
			}
			else
			{
				if ( (data.iAccuracyFlags & ACCURACY_SPEED) && (g_flPlayerSpeed >= data.iAccuracySpeed) )
					data.iDistance *= 1.5;
			}
		}
		else
		{
			data.iDistance *= 2;
		}
		if ( data.iAccuracyFlags & ACCURACY_MULTIPLY_BY_14 )
			data.iDistance *= 1.4;
		if ( data.iAccuracyFlags & ACCURACY_MULTIPLY_BY_14_2 )
			data.iDistance *= 1.4;
	}

	if ( m_iAmmoLastCheck >= g_iShotsFired )
	{
		m_flCrosshairDistance -= (m_flCrosshairDistance * 0.013 + 0.1 );
		m_iAlpha += 2;
	}
	else
	{
		m_flCrosshairDistance = min( m_flCrosshairDistance + data.iDeltaDistance, 15.0f );
		m_iAlpha = max( m_iAlpha - 40, 120 );
	}

	if ( g_iShotsFired > 600 )
		g_iShotsFired = 1;

	CalcCrosshairColor();
	CalcCrosshairDrawMode();
	CalcCrosshairSize();

	m_iAmmoLastCheck = g_iShotsFired;
	m_flCrosshairDistance = max( m_flCrosshairDistance, (float)data.iDistance);
	iLength = (m_flCrosshairDistance - data.iDistance) * 0.5 + 5;

	if ( m_iAlpha > 255 )
		m_iAlpha = 255;

	if ( ScreenWidth != m_iCrosshairScaleBase )
	{
		flCrosshairDistance = ScreenWidth * m_flCrosshairDistance / m_iCrosshairScaleBase;
		iLength = ScreenWidth * iLength / m_iCrosshairScaleBase;
	}
	else
	{
		flCrosshairDistance = m_flCrosshairDistance;
	}


	// drawing
	/*if ( gHUD.m_NVG.m_iFlags )
	{
		FillRGBABlend(WEST_XPOS, EAST_WEST_YPOS,	iLength, 1, 250, 50, 50, m_iAlpha);
		FillRGBABlend(EAST_XPOS, EAST_WEST_YPOS,	iLength, 1, 250, 50, 50, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, NORTH_YPOS, 1, iLength, 250, 50, 50, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, SOUTH_YPOS, 1, iLength, 250, 50, 50, m_iAlpha);
	}
	else if ( m_bAdditive )
	{
		FillRGBA(WEST_XPOS, EAST_WEST_YPOS,		iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBA(EAST_XPOS, EAST_WEST_YPOS,		iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBA(NORTH_SOUTH_XPOS,	NORTH_YPOS,	1, iLength, m_R, m_G, m_B, m_iAlpha);
		FillRGBA(NORTH_SOUTH_XPOS, SOUTH_YPOS,	1, iLength, m_R, m_G, m_B, m_iAlpha);
	}
	else
	{
		FillRGBABlend(WEST_XPOS, EAST_WEST_YPOS,	iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBABlend(EAST_XPOS, EAST_WEST_YPOS,	iLength, 1, m_R, m_G, m_B, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, NORTH_YPOS, 1, iLength, m_R, m_G, m_B, m_iAlpha);
		FillRGBABlend(NORTH_SOUTH_XPOS, SOUTH_YPOS, 1, iLength, m_R, m_G, m_B, m_iAlpha);
	}*/
	if (data.iAccuracyFlags & CROSSHAIR_COLOR_CUSTOM)
	{
		if(weaponid == WEAPON_JANUS11 || weaponid == WEAPON_JANUS3 || weaponid == WEAPON_Z4B_DEATHRAY)
			DrawCrosshairEx(flTime, weaponid, iLength, flCrosshairDistance, m_bAdditive, 170, 83, 196, m_iAlpha);
	}
	else
	{
		if (gHUD.m_NVG.m_iFlags)
			DrawCrosshairEx(flTime, weaponid, iLength, flCrosshairDistance, false, 250, 50, 50, m_iAlpha);
		else
			DrawCrosshairEx(flTime, weaponid, iLength, flCrosshairDistance, m_bAdditive, m_R, m_G, m_B, m_iAlpha);
	}
	
	return;
}



int CHudAmmo::DrawCrosshairEx(float flTime, int weaponid, int iBarSize, float flCrosshairDistance, bool bAdditive, int r, int g, int b, int a)
{
	bool bDrawPoint = false;
	bool bDrawCircle = false;
	bool bDrawCross = false;

	void(*pfnFillRGBA)(int x, int y, int w, int h, int r, int g, int b, int a) = (bAdditive == false) ? gEngfuncs.pfnFillRGBABlend : gEngfuncs.pfnFillRGBA;

	switch ((int)cl_crosshair_type->value)
	{
	case 1:
	{
		bDrawPoint = true;
		bDrawCross = true;
		break;
	}

	case 2:
	{
		bDrawPoint = true;
		bDrawCircle = true;
		break;
	}

	case 3:
	{
		bDrawPoint = true;
		bDrawCircle = true;
		bDrawCross = true;
		break;
	}

	case 4:
	{
		bDrawPoint = true;
		break;
	}

	default:
	{
		bDrawCross = true;
		break;
	}
	}

	if (bDrawCircle)
	{
		float radius = (iBarSize / 2) + flCrosshairDistance;
#if 0
		switch (m_iCrosshairScaleBase)
		{
		case 640: radius += 0.1; break;
		case 800: radius += 0.8; break;
		case 1024: radius += 0.5; break;
		}
#endif
		int count = (int)((cos(0.7853981852531433) * radius) + 0.5);

		for (int i = 0; i < count; i++)
		{
			int size = sqrt((radius * radius) - (float)(i * i));

			pfnFillRGBA((ScreenWidth / 2) + i, (ScreenHeight / 2) + size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + i, (ScreenHeight / 2) - size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - i, (ScreenHeight / 2) + size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - i, (ScreenHeight / 2) - size, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + size, (ScreenHeight / 2) + i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) + size, (ScreenHeight / 2) - i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - size, (ScreenHeight / 2) + i, 1, 1, r, g, b, a);
			pfnFillRGBA((ScreenWidth / 2) - size, (ScreenHeight / 2) - i, 1, 1, r, g, b, a);
		}
	}

	if (bDrawPoint)
		pfnFillRGBA((ScreenWidth / 2) - 1, (ScreenHeight / 2) - 1, 3, 3, r, g, b, a);

	if (bDrawCross)
	{
		pfnFillRGBA((ScreenWidth / 2) - (int)flCrosshairDistance - iBarSize + 1, ScreenHeight / 2, iBarSize, 1, r, g, b, a);
		pfnFillRGBA((ScreenWidth / 2) + (int)flCrosshairDistance, ScreenHeight / 2, iBarSize, 1, r, g, b, a);
		pfnFillRGBA(ScreenWidth / 2, (ScreenHeight / 2) - (int)flCrosshairDistance - iBarSize + 1, 1, iBarSize, r, g, b, a);
		pfnFillRGBA(ScreenWidth / 2, (ScreenHeight / 2) + (int)flCrosshairDistance, 1, iBarSize, r, g, b, a);
	}

	return 1;
}


void CHudAmmo::CalcCrosshairSize()
{
	static char prevSize[64] = { 0 };
	const char *size = m_pClCrosshairSize->string;

	if( !strncmp( prevSize, size, sizeof(prevSize) ) )
		return;

	strncpy( prevSize, size, sizeof(prevSize) );

	if( !stricmp(size, "auto") )
	{
		if( ScreenWidth <= 640 )
		{
			m_iCrosshairScaleBase = 1024;
		}
		else if( ScreenWidth <= 1024 )
		{
			m_iCrosshairScaleBase = 800;
		}
		else
		{
			m_iCrosshairScaleBase = 640;
		}
	}
	else if( !stricmp( size, "small" ))
	{
		m_iCrosshairScaleBase = 1024;
	}
	else if( !stricmp( size, "medium" ))
	{
		m_iCrosshairScaleBase = 800;
	}
	else if( !stricmp( size, "large" ))
	{
		m_iCrosshairScaleBase = 640;
	}
	return;
}

void CHudAmmo::CalcCrosshairDrawMode()
{
	static float prevDrawMode = -1;
	float drawMode = m_pClCrosshairTranslucent->value;

	if( drawMode == prevDrawMode )
		return;

	if ( drawMode == 0.0f )
	{
		m_bAdditive = 0;
	}
	else if ( drawMode == 1.0f )
	{
		m_bAdditive = 1;
	}
	else
	{
		gEngfuncs.Con_Printf("usage: cl_crosshair_translucent <1|0>\n");
		gEngfuncs.Cvar_Set("cl_crosshair_translucent", (char*)"1");
	}

	prevDrawMode = drawMode;
}

void CHudAmmo::CalcCrosshairColor()
{
	static char prevColors[64] = { 0 };
	const char *colors = m_pClCrosshairColor->string;

	if( strncmp( prevColors, colors, 64 ) )
	{
		strncpy( prevColors, colors, 64 );

		sscanf( colors, "%d %d %d", &m_cvarR, &m_cvarG, &m_cvarB);

		m_R = m_cvarR = bound( 0, m_cvarR, 255 );
		m_G = m_cvarG = bound( 0, m_cvarG, 255 );
		m_B = m_cvarB = bound( 0, m_cvarB, 255 );
	}
}

//
// Draws the ammo bar on the hud
//
int DrawBar(int x, int y, int width, int height, float f)
{
	int r, g, b;

	f = bound( 0, f, 1 );

	if (f)
	{
		int w = f * width;

		// Always show at least one pixel if we have ammo.
		if (w <= 0)
			w = 1;
		DrawUtils::UnpackRGB(r, g, b, RGB_GREENISH);
		FillRGBA(x, y, w, height, r, g, b, 255);
		x += w;
		width -= w;
	}

	DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);

	FillRGBA(x, y, width, height, r, g, b, 128);

	return (x + width);
}



void DrawAmmoBar(WEAPON *p, int x, int y, int width, int height)
{
	if ( !p )
		return;

	if (p->iAmmoType != -1)
	{
		if (!gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gWR.CountAmmo(p->iAmmoType)/(float)p->iMax1;

		x = DrawBar(x, y, width, height, f);


		// Do we have secondary ammo too?

		if (p->iAmmo2Type != -1)
		{
			f = (float)gWR.CountAmmo(p->iAmmo2Type)/(float)p->iMax2;

			x += 5; //!!!

			DrawBar(x, y, width, height, f);
		}
	}
}



int CHudAmmo::DrawNEWHudWList(float flTime)
{



	return 1;
}


//
// Draw Weapon Menu
//
int CHudAmmo::DrawWList(float flTime)
{
	int r,g,b,x,y,a,i;

	if ( !gpActiveSel )
		return 0;

	int iActiveSlot;

	if ( gpActiveSel == (WEAPON *)1 )
		iActiveSlot = -1;	// current slot has no weapons
	else
		iActiveSlot = gpActiveSel->iSlot;

	x = gHUD.m_Radar.GetRadarSize() + 10; //!!!
	y = 10; //!!!


	// Ensure that there are available choices in the active slot
	if ( iActiveSlot > 0 )
	{
		if ( !gWR.GetFirstPos( iActiveSlot ) )
		{
			gpActiveSel = (WEAPON *)1;
			iActiveSlot = -1;
		}
	}

	// Draw top line
	for ( i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		int iWidth;

		DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);

		if ( iActiveSlot == i )
			a = 255;
		else
			a = 192;

		DrawUtils::ScaleColors(r, g, b, 255);
		SPR_Set(gHUD.GetSprite(m_HUD_bucket0 + i), r, g, b );

		// make active slot wide enough to accommodate gun pictures
		if ( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos(iActiveSlot);
			if ( p )
				iWidth = p->rcActive.right - p->rcActive.left;
			else
				iWidth = giBucketWidth;
		}
		else
			iWidth = giBucketWidth;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_bucket0 + i));

		x += iWidth + 5;
	}


	a = 128; //!!!
	x = gHUD.m_Radar.GetRadarSize() + 10; //!!!;

	// Draw all of the buckets
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		y = giBucketHeight + 10;

		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		if ( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos( i );
			int iWidth = giBucketWidth;
			if ( p )
				iWidth = p->rcActive.right - p->rcActive.left;

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				p = gWR.GetWeaponSlot( i, iPos );

				if ( !p || !p->iId )
					continue;


				// if active, then we must have ammo.
				if ( gWR.HasAmmo(p) )
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH );
					DrawUtils::ScaleColors(r, g, b, 192);
				}
				else
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_REDISH);
					DrawUtils::ScaleColors(r, g, b, 128);
				}


				if ( gpActiveSel == p )
				{
					SPR_Set(p->hActive, r, g, b );
					SPR_DrawAdditive(0, x, y, &p->rcActive);

					SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b );
					SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
				}
				else
				{
					// Draw Weapon if Red if no ammo
					SPR_Set( p->hInactive, r, g, b );
					SPR_DrawAdditive( 0, x, y, &p->rcInactive );
				}

				// Draw Ammo Bar

				DrawAmmoBar(p, x + giABWidth/2, y, giABWidth, giABHeight);

				y += p->rcActive.bottom - p->rcActive.top + 5;
			}

			x += iWidth + 5;

		}
		else
		{
			// Draw Row of weapons.

			DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				WEAPON *p = gWR.GetWeaponSlot( i, iPos );

				if ( !p || !p->iId )
					continue;

				if ( gWR.HasAmmo(p) )
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);
					a = 128;
				}
				else
				{
					DrawUtils::UnpackRGB(r,g,b, RGB_REDISH);
					a = 96;
				}

				FillRGBA( x, y, giBucketWidth, giBucketHeight, r, g, b, a );

				y += giBucketHeight + 5;
			}

			x += giBucketWidth + 5;
		}
	}

	return 1;

}

int CHudAmmo::DrawNEWHudAmmo(float flTime)
{
	if (!m_pWeapon)
		return 0;

	WEAPON* pw = m_pWeapon; // shorthand

	int AmmoWidth = gHUD.GetSpriteRect(gHUD.m_NEWHUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_NEWHUD_number_0).left;
	int r, g, b, a;
	a = max(MIN_ALPHA, (int)m_fFade);

	if (m_fFade > 0)
		m_fFade -= (gHUD.m_flTimeDelta * 20);

	// Does this weapon have a clip?
	int iY = ScreenHeight - 15 - gHUD.m_NEWHUD_iFontHeight;
	int iX = ScreenWidth - 5;

	// Does weapon have any ammo at all?
	if (m_pWeapon->iAmmoType > 0)
	{
		DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
		DrawUtils::ScaleColors(r, g, b, a);

		int iIconWidth = m_pWeapon->rcAmmo.right - m_pWeapon->rcAmmo.left;

		if (pw->iClip >= 0)
		{
			// room for the number and the '|' and the current ammo


			iX -= (8 * AmmoWidth) - iIconWidth;

			DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);

			int iBarWidth = AmmoWidth / 10;
			// draw the | bar
			FillRGBA(iX, iY, iBarWidth, gHUD.m_NEWHUD_iFontHeight, r, g, b, a);

			iX -= DrawUtils::GetNEWHudNumberWidth(0, pw->iClip, FALSE, 3, 0) + iBarWidth + 1;
			DrawUtils::DrawNEWHudNumber(0, iX, iY, pw->iClip, r, g, b, 255, FALSE, 3);

			iX = ScreenWidth - 5 - iIconWidth;
			// GL Seems to need this
			iX -= DrawUtils::GetNEWHudNumberWidth(0, gWR.CountAmmo(pw->iAmmoType), FALSE, 4, 0);
			if (gWR.CountAmmo(pw->iAmmoType) > 1500)
			{
				SPR_Set(gHUD.GetSprite(m_iInfinite), r, g, b);
				SPR_DrawAdditive(0, iX, iY, &gHUD.GetSpriteRect(m_iInfinite));
			}
			else
				DrawUtils::DrawNEWHudNumber(0, iX, iY, gWR.CountAmmo(pw->iAmmoType), r, g, b, 255, FALSE, 4);
		
		}
		else
		{
			if (pw->iSlot < 3)
				DrawUtils::UnpackRGB(r, g, b, RGB_LIGHTBLUE);

			iX = ScreenWidth - 5 - iIconWidth;
			// GL Seems to need this
			if (gWR.CountAmmo(pw->iAmmoType) > 1500)
			{
				int width = gHUD.GetSpriteRect(m_iInfinite).right - gHUD.GetSpriteRect(m_iInfinite).left;
				iX -= width;

				SPR_Set(gHUD.GetSprite(m_iInfinite), r, g, b);
				SPR_DrawAdditive(0, iX, iY, &gHUD.GetSpriteRect(m_iInfinite));
			}
			else
			{
				if (m_pWeapon->iId != WEAPON_PATROLDRONE)
				{
					iX -= DrawUtils::GetNEWHudNumberWidth(0, gWR.CountAmmo(pw->iAmmoType), FALSE, 3, 0);
					DrawUtils::DrawNEWHudNumber(0, iX, iY, gWR.CountAmmo(pw->iAmmoType), r, g, b, 255, FALSE, 3);
				}
			}
		}

		// Draw the ammo Icon
		if (m_pWeapon->iId != WEAPON_PATROLDRONE)
		{
			iY = ScreenHeight - 15 - gHUD.m_NEWHUD_iFontHeight;
			iX = ScreenWidth - 5;
			SPR_Set(m_pWeapon->hAmmo, r, g, b);
			SPR_DrawAdditive(0, iX - iIconWidth, iY, &m_pWeapon->rcAmmo);
		}
	}

	// Does weapon have seconday ammo?
	if (pw->iAmmo2Type > 0)
	{
		// No clip weapon, draws blue special ammo
		DrawUtils::UnpackRGB(r, g, b, RGB_LIGHTBLUE);
		//DrawUtils::ScaleColors(r, g, b, a);

		int iIconWidth = m_pWeapon->rcAmmo2.right - m_pWeapon->rcAmmo2.left;

		// Do we have secondary ammo?
		if ((pw->iAmmo2Type != 0) && (gWR.CountAmmo(pw->iAmmo2Type) > 0))
		{
			iY = ScreenHeight - 15 - gHUD.m_NEWHUD_iFontHeight;
			iX = ScreenWidth - 215;

			if (gWR.CountAmmo(pw->iAmmo2Type) >= 255)
			{
				int width = gHUD.GetSpriteRect(m_iInfinite).right - gHUD.GetSpriteRect(m_iInfinite).left;
				iX -= width;

				SPR_Set(gHUD.GetSprite(m_iInfinite), r, g, b);
				SPR_DrawAdditive(0, iX, iY, &gHUD.GetSpriteRect(m_iInfinite));
			}
			else
			{
				iX -= DrawUtils::GetNEWHudNumberWidth(0, gWR.CountAmmo(pw->iAmmo2Type), FALSE, 3, 0);
				DrawUtils::DrawNEWHudNumber(0, iX, iY, gWR.CountAmmo(pw->iAmmo2Type), r, g, b, 255, FALSE, 3);
			}
			// Draw the ammo Icon
			iX = ScreenWidth - 215;
			SPR_Set(m_pWeapon->hAmmo2, r, g, b);
			SPR_DrawAdditive(0, iX, iY, &m_pWeapon->rcAmmo2);
		}
	}

	return 1;
}

int CHudAmmo::DrawNEWHudCurrentWpn(float flTime)
{
	int r, g, b, x, y, a, i;
	wrect_t rcSelect = gHUD.GetSpriteRect(m_iWeaponSelect);
	int iSelectionWidth = rcSelect.right - rcSelect.left;
	int iSelectionHeight = rcSelect.top - rcSelect.bottom;

	//int iH = m_iWeaponList->h();
	//int iW = m_iWeaponList->w();

	int iH = m_iWeapon_OffBG->h();
	int iW = m_iWeapon_OffBG->w();

	//wpn is 170w 45h  newslection is 170w 61h  list is 170w 22h
	a = 128;
	int iX = ScreenWidth - 5;	//x2
	int iY = ScreenHeight - 93;	//y2

	if (m_pWeapon)
	{
		m_iWeapon_OffBG->Draw2DQuadScaled(iX - iW, iY - iH, iX, iY);

		SPR_Set(m_pWeapon->hInactive, 255, 255, 255);
		iW = m_pWeapon->rcInactive.right - m_pWeapon->rcInactive.left;
		iH = m_pWeapon->rcInactive.bottom - m_pWeapon->rcInactive.top;
		SPR_DrawAdditive(0, iX - iW, iY - iH, &m_pWeapon->rcInactive);

		int iLength, iHeight;
		iX = ScreenWidth - 5 - m_iWeapon_OffBG->w();
		iY = ScreenHeight - 93 - m_iWeapon_OffBG->h();

		gEngfuncs.pfnDrawSetTextColor(1.0f, 1.0f, 1.0f);

		char szWeaponInfo[64];

		sprintf(szWeaponInfo, "%d %s", m_pWeapon->iSlot + 1, m_pWeapon->szName);

		gEngfuncs.pfnDrawConsoleStringLen(szWeaponInfo, &iLength, &iHeight);
		gEngfuncs.pfnDrawConsoleString(iX, iY + 1, szWeaponInfo);
	}

	return 1;

}

//
// Draw Weapon Menu
//
int CHudAmmo::DrawWpnList(float flTime)
{
	int r, g, b, x, y, a, i;


	a = 128; //!!!
	x = ScreenWidth - g_iSelectionWidth * 1.1; //!!!;
	y = ScreenHeight - g_iSelectionHeight * 7;

	// Draw all of the buckets
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes

		WEAPON* p = gWR.GetFirstPos(i);
		for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
		{
			p = gWR.GetWeaponSlot(i, iPos);

			if (!p || !p->iId)
				continue;


			// if active, then we must have ammo.
			if (gWR.HasAmmo(p))
			{
				DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
				DrawUtils::ScaleColors(r, g, b, 192);
			}
			else
			{
				DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
				DrawUtils::ScaleColors(r, g, b, 128);
			}

			if (p == m_pWeapon)
			{
				SPR_Set(p->hActive, r, g, b);
				SPR_DrawAdditive(0, x, y, &p->rcActive);

				SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b);
				SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
			}
			else
			{
				// Draw Weapon if Red if no ammo
				SPR_Set(p->hInactive, r, g, b);
				SPR_DrawAdditive(0, x, y, &p->rcInactive);
			}


			// Draw Ammo Bar

			DrawAmmoBar(p, x + giABWidth / 2, y, giABWidth, giABHeight);

			x -= p->rcActive.right - p->rcActive.left - 5;
		}
		x = ScreenWidth - g_iSelectionWidth * 1.1;
		y += g_iSelectionHeight * 1.2;
	}

		/*{
			// Draw Row of weapons.
			for (i = 0; i < MAX_WEAPON_SLOTS; i++)
			{
				y = giBucketHeight + 10;
				DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);

				for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
				{
					WEAPON* p = gWR.GetWeaponSlot(i, iPos);

					if (!p || !p->iId)
						continue;

					if (gWR.HasAmmo(p))
					{
						DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
						a = 128;
					}
					else
					{
						DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
						a = 96;
					}

					FillRGBA(x, y, giBucketWidth, giBucketHeight, r, g, b, a);

					y += giBucketHeight + 5;
				}


				}
			}
		}*/

		return 1;

}

/*
=================================
	GetSpriteList

Finds and returns the matching 
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
=================================
*/
client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount)
{
	if (!pList)
		return NULL;

	int i = iCount;
	client_sprite_t *p = pList;

	while(i--)
	{
		if ((!strcmp(psz, p->szName)) && (p->iRes == iRes))
			return p;
		p++;
	}

	return NULL;
}

bool CHudAmmo::FHasSecondaryAmmo() const
{
	return m_pWeapon && (m_pWeapon->iAmmo2Type > 0) && (gWR.CountAmmo(m_pWeapon->iAmmo2Type) > 0);
}

}