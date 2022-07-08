#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

namespace sv {

/*
	Default impl for classic weapons.
*/

// moved from player.cpp
const char *GetCSModelName(WeaponIdType item_id)
{
	const char *modelName = NULL;
	switch (item_id) {
		case WEAPON_P228:
			modelName = "models/w_p228.mdl";
			break;
		case WEAPON_SCOUT:
			modelName = "models/w_scout.mdl";
			break;
		case WEAPON_HEGRENADE:
			modelName = "models/w_hegrenade.mdl";
			break;
		case WEAPON_XM1014:
			modelName = "models/w_xm1014.mdl";
			break;
		case WEAPON_C4:
			modelName = "models/w_backpack.mdl";
			break;
		case WEAPON_MAC10:
			modelName = "models/w_mac10.mdl";
			break;
		case WEAPON_AUG:
			modelName = "models/w_aug.mdl";
			break;
		case WEAPON_SMOKEGRENADE:
			modelName = "models/w_smokegrenade.mdl";
			break;
		case WEAPON_ELITE:
			modelName = "models/w_elite.mdl";
			break;
		case WEAPON_FIVESEVEN:
			modelName = "models/w_fiveseven.mdl";
			break;
		case WEAPON_UMP45:
			modelName = "models/w_ump45.mdl";
			break;
		case WEAPON_SG550:
			modelName = "models/w_sg550.mdl";
			break;
		case WEAPON_GALIL:
			modelName = "models/w_galil.mdl";
			break;
		case WEAPON_FAMAS:
			modelName = "models/w_famas.mdl";
			break;
		case WEAPON_USP:
			modelName = "models/w_usp.mdl";
			break;
		case WEAPON_GLOCK18:
			modelName = "models/w_glock18.mdl";
			break;
		case WEAPON_AWP:
			modelName = "models/w_awp.mdl";
			break;
		case WEAPON_MP5N:
			modelName = "models/w_mp5.mdl";
			break;
		case WEAPON_M249:
			modelName = "models/w_m249.mdl";
			break;
		case WEAPON_M3:
			modelName = "models/w_m3.mdl";
			break;
		case WEAPON_M4A1:
			modelName = "models/w_m4a1.mdl";
			break;
		case WEAPON_TMP:
			modelName = "models/w_tmp.mdl";
			break;
		case WEAPON_G3SG1:
			modelName = "models/w_g3sg1.mdl";
			break;
		case WEAPON_FLASHBANG:
			modelName = "models/w_flashbang.mdl";
			break;
		case WEAPON_DEAGLE:
			modelName = "models/w_deagle.mdl";
			break;
		case WEAPON_SG552:
			modelName = "models/w_sg552.mdl";
			break;
		case WEAPON_AK47:
			modelName = "models/w_ak47.mdl";
			break;
		case WEAPON_KNIFE:
			modelName = "models/w_knife.mdl";
			break;
		case WEAPON_P90:
			modelName = "models/w_p90.mdl";
			break;
		case WEAPON_SHIELDGUN:
			modelName = "models/w_shield.mdl";
			break;
		default:
			ALERT(at_console, "CBasePlayer::PackDeadPlayerItems(): Unhandled item %d not creating weaponbox\n", item_id);
	}

	return modelName;
}

float GetArmorRatioModifier(WeaponIdType iGunType)
{
	float flRatio = 1.0f;

	switch (iGunType) {
		case WEAPON_AUG:
		case WEAPON_M4A1:
			flRatio *= 1.4;
			break;
		case WEAPON_AWP:
		case WEAPON_M400:
		case WEAPON_TRG42:
		case WEAPON_TRG42G:
		case WEAPON_CHEYTACLRRS:
			flRatio *= 1.95;
			break;
		case WEAPON_G3SG1:
			flRatio *= 1.65;
			break;
		case WEAPON_SG550:
			flRatio *= 1.45;
			break;
		case WEAPON_M249:
			flRatio *= 1.5;
			break;
		case WEAPON_ELITE:
			flRatio *= 1.05;
			break;
		case WEAPON_DEAGLE:
		case WEAPON_ANACONDA:
			flRatio *= 1.5;
			break;
		case WEAPON_GLOCK18:
			flRatio *= 1.05;
			break;
		case WEAPON_FIVESEVEN:
		case WEAPON_P90:
			flRatio *= 1.5;
			break;
		case WEAPON_MAC10:
			flRatio *= 0.95;
			break;
		case WEAPON_P228:
			flRatio *= 1.25;
			break;
		case WEAPON_SCOUT:
		case WEAPON_KNIFE:
			flRatio *= 1.7;
			break;
		case WEAPON_FAMAS:
		case WEAPON_SG552:
			flRatio *= 1.4;
			break;
		case WEAPON_GALIL:
		case WEAPON_AK47:
			flRatio *= 1.55;
			break;
		case WEAPON_M82:
			flRatio *= 1.92;
			break;
		case WEAPON_AW50:
			flRatio *= 1.94;
			break;
		case WEAPON_M24:
			flRatio *= 1.94;
			break;
		case WEAPON_SFSNIPER:
		case WEAPON_M95TIGER:
			flRatio *= 1.99;
			break;
		default:
			break;
	}
	return flRatio;
}

WeaponBuyAmmoConfig GetBuyAmmoConfig(WeaponIdType id)
{
	int cost = 0;
	const char *classname = nullptr;

	switch (id) {
		case WEAPON_AWP:
		case WEAPON_M400:
		case WEAPON_TRG42:
		case WEAPON_TRG42G:
			cost = AMMO_338MAG_PRICE;
			classname = "ammo_338magnum";
			break;
		case WEAPON_SCOUT:
		case WEAPON_G3SG1:
		case WEAPON_AK47:
		case WEAPON_M82:
		case WEAPON_M24:
			cost = AMMO_762MM_PRICE;
			classname = "ammo_762nato";
			break;
		case WEAPON_XM1014:
		case WEAPON_M3:
			cost = AMMO_BUCKSHOT_PRICE;
			classname = "ammo_buckshot";
			break;
		case WEAPON_MAC10:
		case WEAPON_UMP45:
		case WEAPON_USP:
			cost = AMMO_45ACP_PRICE;
			classname = "ammo_45acp";
			break;
		case WEAPON_M249:
			cost = AMMO_556MM_PRICE;
			classname = "ammo_556natobox";
			break;
		case WEAPON_FIVESEVEN:
		case WEAPON_P90:
			cost = AMMO_57MM_PRICE;
			classname = "ammo_57mm";
			break;
		case WEAPON_ELITE:
		case WEAPON_GLOCK18:
		case WEAPON_MP5N:
		case WEAPON_TMP:
			cost = AMMO_9MM_PRICE;
			classname = "ammo_9mm";
			break;
		case WEAPON_DEAGLE:
		case WEAPON_ANACONDA:
			cost = AMMO_50AE_PRICE;
			classname = "ammo_50ae";
			break;
		case WEAPON_P228:
			cost = AMMO_357SIG_PRICE;
			classname = "ammo_357sig";
			break;
		case WEAPON_AUG:
		case WEAPON_SG550:
		case WEAPON_GALIL:
		case WEAPON_FAMAS:
		case WEAPON_M4A1:
		case WEAPON_SG552:
			cost = AMMO_556MM_PRICE;
			classname = "ammo_556nato";
			break;
		default:
			// ...
			break;
	}
	return {classname, cost};
}

float CBasePlayerItem::GetArmorRatioModifier()
{
	return ::sv::GetArmorRatioModifier(m_iId);
}

// gets weapons' W_Model, should be overridden by custom weapon.
const char *CBasePlayerItem::GetCSModelName()
{
	return ::sv::GetCSModelName(m_iId);
}

WeaponBuyAmmoConfig CBasePlayerItem::GetBuyAmmoConfig()
{
	return ::sv::GetBuyAmmoConfig(m_iId);
}

}
