#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

// moved from player.cpp
const char *GetCSModelName(int item_id)
{
	const char *modelName = NULL;
	switch (item_id)
	{
	case WEAPON_P228:		modelName = "models/w_p228.mdl"; break;
	case WEAPON_SCOUT:		modelName = "models/w_scout.mdl"; break;
	case WEAPON_HEGRENADE:		modelName = "models/w_hegrenade.mdl"; break;
	case WEAPON_XM1014:		modelName = "models/w_xm1014.mdl"; break;
	case WEAPON_C4:			modelName = "models/w_backpack.mdl"; break;
	case WEAPON_MAC10:		modelName = "models/w_mac10.mdl"; break;
	case WEAPON_AUG:		modelName = "models/w_aug.mdl"; break;
	case WEAPON_SMOKEGRENADE:	modelName = "models/w_smokegrenade.mdl"; break;
	case WEAPON_ELITE:		modelName = "models/w_elite.mdl"; break;
	case WEAPON_FIVESEVEN:		modelName = "models/w_fiveseven.mdl"; break;
	case WEAPON_UMP45:		modelName = "models/w_ump45.mdl"; break;
	case WEAPON_SG550:		modelName = "models/w_sg550.mdl"; break;
	case WEAPON_GALIL:		modelName = "models/w_galil.mdl"; break;
	case WEAPON_FAMAS:		modelName = "models/w_famas.mdl"; break;
	case WEAPON_USP:		modelName = "models/w_usp.mdl"; break;
	case WEAPON_GLOCK18:		modelName = "models/w_glock18.mdl"; break;
	case WEAPON_AWP:		modelName = "models/w_awp.mdl"; break;
	case WEAPON_MP5N:		modelName = "models/w_mp5.mdl"; break;
	case WEAPON_M249:		modelName = "models/w_m249.mdl"; break;
	case WEAPON_M3:			modelName = "models/w_m3.mdl"; break;
	case WEAPON_M4A1:		modelName = "models/w_m4a1.mdl"; break;
	case WEAPON_TMP:		modelName = "models/w_tmp.mdl"; break;
	case WEAPON_G3SG1:		modelName = "models/w_g3sg1.mdl"; break;
	case WEAPON_FLASHBANG:		modelName = "models/w_flashbang.mdl"; break;
	case WEAPON_DEAGLE:		modelName = "models/w_deagle.mdl"; break;
	case WEAPON_SG552:		modelName = "models/w_sg552.mdl"; break;
	case WEAPON_AK47:		modelName = "models/w_ak47.mdl"; break;
	case WEAPON_KNIFE:		modelName = "models/w_knife.mdl"; break;
	case WEAPON_P90:		modelName = "models/w_p90.mdl"; break;
	case WEAPON_SHIELDGUN:		modelName = "models/w_shield.mdl"; break;
	default:
		ALERT(at_console, "CBasePlayer::PackDeadPlayerItems(): Unhandled item- not creating weaponbox\n");
	}

	return modelName;
}

// gets weapons' W_Model, should be overridden by custom weapon.
const char *CBasePlayerItem::GetCSModelName()
{
	return ::GetCSModelName(m_iId);
}