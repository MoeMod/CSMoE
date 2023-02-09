#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "weapons.h"
#include "weapons_precache.h"

#include "cbase/cbase_memory.h"

#include "luash_sv/luash_sv.h"
#include "luash/luash.hpp"
#include "luash/luash_exp.hpp"


// called by worldspawn
namespace sv {

template<class T>
void W_PrecacheOther()
{
	auto pEntity = CreateClassPtr<T>();
	if (!pEntity)
	{
		ALERT(at_console, "NULL Ent (%s) in UTIL_PrecacheOther\n", typeid(T).name());
		return;
	}

	pEntity->Precache();

	REMOVE_ENTITY(pEntity->edict());
}

template<class T>
void PrecacheOtherWeapon()
{
	auto pEntity = CreateClassPtr<T>();
	if (!pEntity)
	{
		ALERT(at_console, "NULL Ent (%s) in UTIL_PrecacheOther\n", typeid(T).name());
		return;
	}

	pEntity->Precache();

	if (pEntity != NULL)
	{
		ItemInfo II;
		Q_memset(&II, 0, sizeof(II));

		pEntity->Precache();
		if (((CBasePlayerItem *)pEntity)->GetItemInfo(&II))
		{
			CBasePlayerItem::ItemInfoArray[ II.iId ] = II;

			if (II.pszAmmo1 != NULL && *II.pszAmmo1 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo1);
			}

			if (II.pszAmmo2 != NULL && *II.pszAmmo2 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo2);
			}
		}
	}

	REMOVE_ENTITY(pEntity->edict());
}

void W_Precache()
{
	CBasePlayerItem::ItemInfoArray.clear();
	//Q_memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
	Q_memset(CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray));
	/*Q_memset(CBasePlayerItem::ItemInfoArray, 0, ARRAYSIZE(CBasePlayerItem::ItemInfoArray));
	Q_memset(CBasePlayerItem::AmmoInfoArray, 0, ARRAYSIZE(CBasePlayerItem::AmmoInfoArray));*/
	giAmmoIndex = 0;

	// custom items...

	// common world objects
	UTIL_PrecacheOther("item_suit");
	UTIL_PrecacheOther("item_battery");
	UTIL_PrecacheOther("item_antidote");
	UTIL_PrecacheOther("item_security");
	UTIL_PrecacheOther("item_longjump");
	UTIL_PrecacheOther("item_kevlar");
	UTIL_PrecacheOther("item_assaultsuit");
	UTIL_PrecacheOther("item_thighpack");

	// awp magnum
	UTIL_PrecacheOtherWeapon("weapon_awp");
	UTIL_PrecacheOther("ammo_338magnum");

	UTIL_PrecacheOtherWeapon("weapon_g3sg1");
	UTIL_PrecacheOtherWeapon("weapon_ak47");
	UTIL_PrecacheOtherWeapon("weapon_scout");
	UTIL_PrecacheOther("ammo_762nato");

	// m249
	UTIL_PrecacheOtherWeapon("weapon_m249");
	UTIL_PrecacheOther("ammo_556natobox");

	UTIL_PrecacheOtherWeapon("weapon_m4a1");
	UTIL_PrecacheOtherWeapon("weapon_sg552");
	UTIL_PrecacheOtherWeapon("weapon_aug");
	UTIL_PrecacheOtherWeapon("weapon_sg550");
	UTIL_PrecacheOther("ammo_556nato");

	// shotgun
	UTIL_PrecacheOtherWeapon("weapon_m3");
	UTIL_PrecacheOtherWeapon("weapon_xm1014");
	UTIL_PrecacheOther("ammo_buckshot");

	UTIL_PrecacheOtherWeapon("weapon_usp");
	UTIL_PrecacheOtherWeapon("weapon_mac10");
	UTIL_PrecacheOtherWeapon("weapon_ump45");
	UTIL_PrecacheOther("ammo_45acp");

	UTIL_PrecacheOtherWeapon("weapon_fiveseven");
	UTIL_PrecacheOtherWeapon("weapon_p90");
	UTIL_PrecacheOther("ammo_57mm");

	// deagle
	UTIL_PrecacheOtherWeapon("weapon_deagle");
	UTIL_PrecacheOther("ammo_50ae");

	// p228
	UTIL_PrecacheOtherWeapon("weapon_p228");
	UTIL_PrecacheOther("ammo_357sig");

	// knife
	UTIL_PrecacheOtherWeapon("weapon_knife");

	UTIL_PrecacheOtherWeapon("weapon_glock18");
	UTIL_PrecacheOtherWeapon("weapon_mp5navy");
	UTIL_PrecacheOtherWeapon("weapon_tmp");
	UTIL_PrecacheOtherWeapon("weapon_elite");
	UTIL_PrecacheOther("ammo_9mm");

	UTIL_PrecacheOtherWeapon("weapon_flashbang");
	UTIL_PrecacheOtherWeapon("weapon_hegrenade");
	UTIL_PrecacheOtherWeapon("weapon_smokegrenade");
	UTIL_PrecacheOtherWeapon("weapon_c4");
	UTIL_PrecacheOtherWeapon("weapon_galil");
	UTIL_PrecacheOtherWeapon("weapon_famas");

    {
        using namespace luash;
        xpeval(LuaSV_Get(), LuaSV_ExceptionHandler, (
                _if(_G[_S("W_Precache")])._then(
                        _G[_S("W_Precache")]()
                )._end
        ));
    }

	if (g_pGameRules->IsDeathmatch())
	{
		// container for dropped deathmatch weapons
		UTIL_PrecacheOther("weaponbox");
	}

	g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");	// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL("sprites/WXplo1.spr");		// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");		// smoke
	g_sModelIndexSmokeBeam = PRECACHE_MODEL("sprites/smoke.spr");
	g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");		// bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr");	// initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr");		// splattered blood

	g_sModelIndexSmokePuff = PRECACHE_MODEL("sprites/smokepuff.spr");
	g_sModelIndexFireball2 = PRECACHE_MODEL("sprites/eexplo.spr");
	g_sModelIndexFireball3 = PRECACHE_MODEL("sprites/fexplo.spr");
	g_sModelIndexFireball4 = PRECACHE_MODEL("sprites/fexplo1.spr");
	g_sModelIndexRadio = PRECACHE_MODEL("sprites/radio.spr");
	g_sModelIndexZombiebomb_exp = PRECACHE_MODEL("sprites/zombiebomb_exp.spr");
	g_sModelIndexBoomerEffect = PRECACHE_MODEL("models/ef_boomer.mdl");
	g_sModelIndexBoomerEffectEx = PRECACHE_MODEL("sprites/ef_boomer_ex.spr");
	g_sModelIndexBoomerSprite = PRECACHE_MODEL("sprites/spr_boomer.spr");
	g_sModelIndexPosionEffect = PRECACHE_MODEL("models/ef_poison03.mdl");
	g_sModelIndexGuillotineGibs = PRECACHE_MODEL("models/gibs_guilotine.mdl");
	g_sModelIndexZb3Respawn = PRECACHE_MODEL("sprites/zb3_respawn.spr");

	g_sModelIndexEventBombExp = PRECACHE_MODEL("sprites/heart_bombexp.spr");
	g_sModelIndexEventBombExp2 = PRECACHE_MODEL("sprites/cakeexp.spr");
	g_sModelIndexMoonCakeExp = PRECACHE_MODEL("sprites/mooncake.spr");
	g_sModelIndexHolyWater = PRECACHE_MODEL("sprites/ef_hwater.spr");

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
	PRECACHE_SOUND("zombi/zombi_bomb_exp.wav");	// grenade

	PRECACHE_SOUND("weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND("weapons/bullet_hit2.wav");	// hit by bullet

	PRECACHE_SOUND("items/weapondrop1.wav");	// weapon falls to the ground
	PRECACHE_SOUND("weapons/generic_reload.wav");

    g_pModRunning->W_Precache();
}

}
