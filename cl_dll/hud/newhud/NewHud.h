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
#pragma once
#include "hud_sub.h"
namespace cl {
extern vec3_t g_velocity;
extern long g_iDamage[MAX_CLIENTS + 1];
extern long g_iDamageTotal[MAX_CLIENTS + 1];
extern double g_flDamageInAll;


const std::map<ZombieClassType, std::string> g_szZombiePortrait =
{
	{ ZOMBIE_CLASS_HUMAN,  "zombi" },
	{ ZOMBIE_CLASS_TANK,  "zombi" },
	{ ZOMBIE_CLASS_SPEED, "speed_zombi", },
	{ ZOMBIE_CLASS_HEAVY,  "heavy_zombi" },
	{ ZOMBIE_CLASS_PC, "pc_zombi" },
	{ ZOMBIE_CLASS_HEAL, "heal_zombi" },
	{ ZOMBIE_CLASS_DEIMOS, "deimos_zombi" },
	{ ZOMBIE_CLASS_DEIMOS2, "deimos2_zombi" },
	{ ZOMBIE_CLASS_ZBS_DEIMOS2, "zbs_deimos2_zombi" },
	{ ZOMBIE_CLASS_TELEPORT, "teleport_zombi" },
	{ ZOMBIE_CLASS_BOOSTER, "booster_zombi" },
	{ ZOMBIE_CLASS_CHINA, "china_zombi" },
	{ ZOMBIE_CLASS_PASS, "pass_zombi" },
	{ ZOMBIE_CLASS_FLYING, "flyingzombi" },
	{ ZOMBIE_CLASS_REVIVAL, "revival_zombi" },
	{ ZOMBIE_CLASS_STAMPER, "stamper_zombi" },
	{ ZOMBIE_CLASS_RESIDENT, "resident_zombi" },
	{ ZOMBIE_CLASS_WITCH, "witch_zombi" },
	{ ZOMBIE_CLASS_AKSHA, "akshazombi" },
	{ ZOMBIE_CLASS_BOOMER, "boomer_zombi" },
	{ ZOMBIE_CLASS_NEMESIS, "csmoe_nemesis" },
	{ ZOMBIE_CLASS_SPIDER, "zombi_spider" },
	{ ZOMBIE_CLASS_DEATHKNIGHT, "zombi_deathknight" },
	{ ZOMBIE_CLASS_MEATWALL, "zombi_meatwall" },
	{ ZOMBIE_CLASS_SIREN, "zombi_siren" },
};

class CHudNewHud : public CHudBase
{
public:
	CHudNewHud();
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	int DrawNewHudCharacterBG(float flTime);
	int DrawApexHudCharacterBG(float flTime);

	void InitHUDData(void);
	CHudMsgFunc(NewHudMsg);

private:
	SharedTexture m_iCharacterStateBG;

	UniqueTexture m_iCharacterBG;
	UniqueTexture m_iCharacterBG_New_Bottom;
	UniqueTexture m_iCharacterBG_New_Top;

	SharedTexture m_iNum_Character;

	wrect_t m_iNum_CharacterC[10];

	UniqueTexture m_iIcon_Speed;
	UniqueTexture m_iIcon_Damage;

	UniqueTexture m_iWeaponBG;


	UniqueTexture m_iCharacter;
	std::string m_szLastModel;
	int m_iLastZombieClass;
};
}