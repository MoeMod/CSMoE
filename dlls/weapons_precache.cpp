#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "weapons.h"
#include "weapons_precache.h"

#include "cbase/cbase_memory.h"
#include "cbase/cbase_typelist.h"


// called by worldspawn
namespace sv {

bool Weapons_PrecacheOne(const char *classname)
{
	if(!Q_strnicmp(classname, "weapon_", 7) || !Q_strnicmp(classname, "knife_", 6) || !Q_strnicmp(classname, "z4b_", 4))
		return UTIL_PrecacheOtherWeapon(classname), true;
	else if(!Q_strnicmp(classname, "ammo_", 5) || !Q_strnicmp(classname, "item_", 5))
		return UTIL_PrecacheOther(classname), true;
	return false;
}

bool Weapons_PrecacheOne(const EntityMetaData& md)
{
	return Weapons_PrecacheOne(md.ClassName);
}

template<class...Types>
std::size_t Weapons_PrecacheAll(TypeList<Types...>)
{
	auto il = { Weapons_PrecacheOne(GetEntityMetaDataFor(type_identity<Types>()))... };
	return std::count(il.begin(), il.end(), true);
}

void W_Precache()
{
	Q_memset(CBasePlayerItem::ItemInfoArray, 0, ARRAYSIZE(CBasePlayerItem::ItemInfoArray));
	Q_memset(CBasePlayerItem::AmmoInfoArray, 0, ARRAYSIZE(CBasePlayerItem::AmmoInfoArray));
	giAmmoIndex = 0;

	// custom items...
	auto count = Weapons_PrecacheAll(AllEntityTypeList());

	if (g_pGameRules->IsDeathmatch())
	{
		// container for dropped deathmatch weapons
		UTIL_PrecacheOther("weaponbox");
	}

	g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");	// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL("sprites/WXplo1.spr");		// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");		// smoke
	g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");		// bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr");	// initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr");		// splattered blood

	g_sModelIndexSmokePuff = PRECACHE_MODEL("sprites/smokepuff.spr");
	g_sModelIndexFireball2 = PRECACHE_MODEL("sprites/eexplo.spr");
	g_sModelIndexFireball3 = PRECACHE_MODEL("sprites/fexplo.spr");
	g_sModelIndexFireball4 = PRECACHE_MODEL("sprites/fexplo1.spr");
	g_sModelIndexRadio = PRECACHE_MODEL("sprites/radio.spr");

	g_sModelIndexCTGhost = PRECACHE_MODEL("sprites/b-tele1.spr");
	g_sModelIndexTGhost = PRECACHE_MODEL("sprites/c-tele1.spr");
	g_sModelIndexC4Glow = PRECACHE_MODEL("sprites/ledglow.spr");

	g_sModelIndexLaser = PRECACHE_MODEL((char*)g_pModelNameLaser);
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");

	// used by explosions
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_MODEL("sprites/explode1.spr");

	PRECACHE_SOUND("weapons/debris1.wav");		// explosion aftermaths
	PRECACHE_SOUND("weapons/debris2.wav");		// explosion aftermaths
	PRECACHE_SOUND("weapons/debris3.wav");		// explosion aftermaths

	PRECACHE_SOUND("weapons/grenade_hit1.wav");	// grenade
	PRECACHE_SOUND("weapons/grenade_hit2.wav");	// grenade
	PRECACHE_SOUND("weapons/grenade_hit3.wav");	// grenade

	PRECACHE_SOUND("weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND("weapons/bullet_hit2.wav");	// hit by bullet

	PRECACHE_SOUND("items/weapondrop1.wav");	// weapon falls to the ground
	PRECACHE_SOUND("weapons/generic_reload.wav");
}

}
