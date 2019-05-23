#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

namespace sv {

// called after weapon switches, to update client's custom weapon prediction
void CBasePlayerWeapon::UpdateItemInfo()
{
	CBasePlayer *player = m_pPlayer;
	if (player && player->m_pActiveItem == this) {
		MESSAGE_BEGIN(MSG_ONE, gmsgBTEWeapon, NULL, player->pev);
		WRITE_BYTE(BTE_Weapon_Active); // type, reserved.
		WRITE_STRING(STRING(this->pev->classname));
		MESSAGE_END();
	}

}

}
