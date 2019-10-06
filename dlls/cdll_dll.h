/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef CDLL_DLL_H
#define CDLL_DLL_H
#ifdef _WIN32
#pragma once
#endif

#define MAX_CLIENTS 32
#define MAX_WEAPONS 32
#define MAX_WEAPON_SLOTS 5
#define MAX_ITEM_TYPES 6
#define MAX_ITEMS 4

#define DEFAULT_FOV			90		// the default field of view

#define HIDEHUD_WEAPONS			(1 << 0)
#define HIDEHUD_FLASHLIGHT		(1 << 1)
#define HIDEHUD_ALL			(1 << 2)
#define HIDEHUD_HEALTH			(1 << 3)
#define HIDEHUD_TIMER			(1 << 4)
#define HIDEHUD_MONEY			(1 << 5)
#define HIDEHUD_CROSSHAIR		(1 << 6)

#define STATUSICON_HIDE			0
#define STATUSICON_SHOW			1
#define STATUSICON_FLASH		2

#define TEAM_UNASSIGNED 0
#define TEAM_TERRORIST 1
#define TEAM_CT 2
#define TEAM_SPECTATOR 3

#define CLASS_UNASSIGNED 0
#define CLASS_URBAN 1
#define CLASS_TERROR 2
#define CLASS_LEET 3
#define CLASS_ARCTIC 4
#define CLASS_GSG9 5
#define CLASS_GIGN 6
#define CLASS_SAS 7
#define CLASS_GUERILLA 8
#define CLASS_VIP 9
#define CLASS_MILITIA 10
#define CLASS_SPETSNAZ 11

#define HUD_PRINTNOTIFY			1
#define HUD_PRINTCONSOLE		2
#define HUD_PRINTTALK			3
#define HUD_PRINTCENTER			4
#define HUD_PRINTRADIO			5

#define STATUS_NIGHTVISION_ON		1
#define STATUS_NIGHTVISION_OFF		0

#define ITEM_STATUS_NIGHTVISION		(1<<0)
#define ITEM_STATUS_DEFUSER		(1<<1)

#define PLAYER_DEAD (1<<0)
#define PLAYER_HAS_C4 (1<<1)
#define PLAYER_VIP (1<<2)
#define PLAYER_ZOMBIE (1<<3)

#define SIGNAL_BUY			(1<<0)
#define SIGNAL_BOMB			(1<<1)
#define SIGNAL_RESCUE			(1<<2)
#define SIGNAL_ESCAPE			(1<<3)
#define SIGNAL_VIPSAFETY		(1<<4)

#define DATA_IUSER3_CANSHOOT		(1<<0)
#define DATA_IUSER3_FREEZETIMEOVER	(1<<1)
#define DATA_IUSER3_INBOMBZONE		(1<<2)
#define DATA_IUSER3_HOLDINGSHIELD	(1<<3)
#define IUSER3_CANSHOOT (1<<0)
#define IUSER3_FREEZETIMEOVER (1<<1)
#define IUSER3_INBOMBZONE (1<<2)
#define IUSER3_HOLDINGSHIELD (1<<3)

#define MENU_KEY_1			(1<<0)
#define MENU_KEY_2			(1<<1)
#define MENU_KEY_3			(1<<2)
#define MENU_KEY_4			(1<<3)
#define MENU_KEY_5			(1<<4)
#define MENU_KEY_6			(1<<5)
#define MENU_KEY_7			(1<<6)
#define MENU_KEY_8			(1<<7)
#define MENU_KEY_9			(1<<8)
#define MENU_KEY_0			(1<<9)

#define MAX_AMMO_TYPES			32		// ???
#define MAX_AMMO_SLOTS			32		// not really slots

#define HUD_PRINTNOTIFY			1
#define HUD_PRINTCONSOLE		2
#define HUD_PRINTTALK			3
#define HUD_PRINTCENTER			4

#define WEAPON_SUIT			31
#define WEAPON_ALLWEAPONS		(~(1 << WEAPON_SUIT))

// custom enum
enum VGUIMenu
{
	VGUI_Menu_Team = 2,
	VGUI_Menu_MapBriefing = 4,

	VGUI_Menu_Class_T = 26,
	VGUI_Menu_Class_CT,
	VGUI_Menu_Buy,
	VGUI_Menu_Buy_Pistol,
	VGUI_Menu_Buy_ShotGun,
	VGUI_Menu_Buy_Rifle,
	VGUI_Menu_Buy_SubMachineGun,
	VGUI_Menu_Buy_MachineGun,
	VGUI_Menu_Buy_Item,
};

// custom enum
enum VGUIMenuSlot
{
	VGUI_MenuSlot_Buy_Pistol = 1,
	VGUI_MenuSlot_Buy_ShotGun,
	VGUI_MenuSlot_Buy_SubMachineGun,
	VGUI_MenuSlot_Buy_Rifle,
	VGUI_MenuSlot_Buy_MachineGun,
	VGUI_MenuSlot_Buy_PrimAmmo,
	VGUI_MenuSlot_Buy_SecAmmo,
	VGUI_MenuSlot_Buy_Item,
	VGUI_MenuSlot_Zbs_Upgrade,
};

#define MENU_TEAM 2
#define MENU_MAPBRIEFING 4
#define MENU_CLASS_T 26
#define MENU_CLASS_CT 27
#define MENU_BUY 28
#define MENU_BUY_PISTOL 29
#define MENU_BUY_SHOTGUN 30
#define MENU_BUY_RIFLE 31
#define MENU_BUY_SUBMACHINEGUN 32
#define MENU_BUY_MACHINEGUN 33
#define MENU_BUY_ITEM 34
// -- cs16client extension start -- //
#define MENU_RADIOA 35
#define MENU_RADIOB 36
#define MENU_RADIOC 37
#define MENU_RADIOSELECTOR 38
#define MENU_NUMERICAL_MENU -1
// -- cs16client extension end -- //

constexpr size_t MAX_PLAYER_NAME_LENGTH =		32;
constexpr size_t MAX_AUTOBUY_LENGTH =		256;
constexpr size_t MAX_REBUY_LENGTH =		256;
constexpr size_t MAX_MOTD_CHUNK = 60;
constexpr size_t MAX_MOTD_LENGTH = 1536;

#endif // CDLL_DLL_H
