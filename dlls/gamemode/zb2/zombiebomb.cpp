#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "zombiebomb.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"

#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(zombiebomb, CZombibomb);

void CZombibomb::Precache()
{
	PRECACHE_MODEL("models/w_zombibomb.mdl");
}

void CZombibomb::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	pev->gravity = 1.0;

	UTIL_SetSize(pev, Vector(-8, -8, 0), Vector(8, 8, 8));
	
	SetTouch(&CZombibomb::OnTouch);

	SET_MODEL(edict(), "models/w_zombibomb.mdl");
}

void CZombibomb::OnTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	CBasePlayer *p = static_cast<CBasePlayer *>(pOther);
	

	if (p->m_bIsVIP || !p->m_bIsZombie)
		return;
	
	CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)p->m_rgpPlayerItems[GRENADE_SLOT];
	if (pWeapon != NULL && !Q_strcmp(STRING(pWeapon->pev->classname), "weapon_zombibomb"))
		return;

	p->GiveNamedItem("weapon_zombibomb");
	pev->effects |= EF_NODRAW;
	Kill();
}


void CZombibomb::Kill()
{
	SetTouch(NULL);

	UTIL_Remove(this);
}

}
