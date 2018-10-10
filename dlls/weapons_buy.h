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

#ifndef WEAPONS_BUY_H
#define WEAPONS_BUY_H
#ifdef _WIN32
#pragma once
#endif



enum AutoBuyClassType
{
	AUTOBUYCLASS_NONE		= 0,
	AUTOBUYCLASS_PRIMARY		= (1 << 0),
	AUTOBUYCLASS_SECONDARY		= (1 << 1),
	AUTOBUYCLASS_AMMO		= (1 << 2),
	AUTOBUYCLASS_ARMOR		= (1 << 3),
	AUTOBUYCLASS_DEFUSER		= (1 << 4),
	AUTOBUYCLASS_PISTOL		= (1 << 5),
	AUTOBUYCLASS_SMG		= (1 << 6),
	AUTOBUYCLASS_RIFLE		= (1 << 7),
	AUTOBUYCLASS_SNIPERRIFLE	= (1 << 8),
	AUTOBUYCLASS_SHOTGUN		= (1 << 9),
	AUTOBUYCLASS_MACHINEGUN		= (1 << 10),
	AUTOBUYCLASS_GRENADE		= (1 << 11),
	AUTOBUYCLASS_NIGHTVISION	= (1 << 12),
	AUTOBUYCLASS_SHIELD		= (1 << 13),
};

enum AmmoCostType
{
	AMMO_338MAG_PRICE	= 125,
	AMMO_357SIG_PRICE	= 50,
	AMMO_45ACP_PRICE	= 25,
	AMMO_50AE_PRICE		= 40,
	AMMO_556MM_PRICE	= 60,
	AMMO_57MM_PRICE		= 50,
	AMMO_762MM_PRICE	= 80,
	AMMO_9MM_PRICE		= 20,
	AMMO_BUCKSHOT_PRICE	= 65,
};

enum WeaponCostType
{
	AK47_PRICE	= 2500,
	AWP_PRICE	= 4750,
	DEAGLE_PRICE	= 650,
	G3SG1_PRICE	= 5000,
	SG550_PRICE	= 4200,
	GLOCK18_PRICE	= 400,
	M249_PRICE	= 5750,
	M3_PRICE	= 1700,
	M4A1_PRICE	= 3100,
	AUG_PRICE	= 3500,
	MP5NAVY_PRICE	= 1500,
	P228_PRICE	= 600,
	P90_PRICE	= 2350,
	UMP45_PRICE	= 1700,
	MAC10_PRICE	= 1400,
	SCOUT_PRICE	= 2750,
	SG552_PRICE	= 3500,
	TMP_PRICE	= 1250,
	USP_PRICE	= 500,
	ELITE_PRICE	= 800,
	FIVESEVEN_PRICE	= 750,
	XM1014_PRICE	= 3000,
	GALIL_PRICE	= 2000,
	FAMAS_PRICE	= 2250,
	SHIELDGUN_PRICE	= 2200,
};

enum WeaponClassType
{
	WEAPONCLASS_NONE,
	WEAPONCLASS_KNIFE,
	WEAPONCLASS_PISTOL,
	WEAPONCLASS_GRENADE,
	WEAPONCLASS_SUBMACHINEGUN,
	WEAPONCLASS_SHOTGUN,
	WEAPONCLASS_MACHINEGUN,
	WEAPONCLASS_RIFLE,
	WEAPONCLASS_SNIPERRIFLE,
	WEAPONCLASS_MAX,
};

enum AmmoBuyAmount
{
	AMMO_338MAG_BUY		= 10,
	AMMO_357SIG_BUY		= 13,
	AMMO_45ACP_BUY		= 12,
	AMMO_50AE_BUY		= 7,
	AMMO_556NATO_BUY	= 30,
	AMMO_556NATOBOX_BUY	= 30,
	AMMO_57MM_BUY		= 50,
	AMMO_762NATO_BUY	= 30,
	AMMO_9MM_BUY		= 30,
	AMMO_BUCKSHOT_BUY	= 8,
};

enum ItemCostType
{
	ASSAULTSUIT_PRICE	= 1000,
	FLASHBANG_PRICE		= 200,
	HEGRENADE_PRICE		= 300,
	SMOKEGRENADE_PRICE	= 300,
	KEVLAR_PRICE		= 650,
	HELMET_PRICE		= 350,
	NVG_PRICE		= 1250,
	DEFUSEKIT_PRICE		= 200,
};

struct AutoBuyInfoStruct
{
	int m_class;
	char *m_command;
	char *m_classname;
};

struct WeaponAliasInfo
{
	char *alias;
	WeaponIdType id;
};

struct WeaponBuyAliasInfo
{
	char *alias;
	WeaponIdType id;
	char *failName;
};

struct WeaponClassAliasInfo
{
	char *alias;
	WeaponClassType id;
};

struct WeaponInfoStruct
{
	int id;
	int cost;
	int clipCost;
	int buyClipSize;
	int gunClipSize;
	int maxRounds;
	int ammoType;
	char *entityName;
};

struct WeaponBuyAmmoConfig
{
	const char *classname;
	int cost;
};

extern AutoBuyInfoStruct g_autoBuyInfo[35];

// WeaponType
WeaponIdType AliasToWeaponID(const char *alias);
const char *BuyAliasToWeaponID(const char *alias, WeaponIdType &id);
const char *WeaponIDToAlias(int id);
WeaponClassType AliasToWeaponClass(const char *alias);
WeaponClassType WeaponIDToWeaponClass(int id);
bool IsPrimaryWeapon(int id);
bool IsSecondaryWeapon(int id);
WeaponInfoStruct *GetWeaponInfo(int weaponID);
bool CanBuyWeaponByMaptype(int playerTeam, WeaponIdType weaponID, bool useAssasinationRestrictions);

#endif // WEAPONTYPE_H
