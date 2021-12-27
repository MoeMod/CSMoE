#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "teleportentity.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"

#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(teleportentity, CTeleportEntity);

void CTeleportEntity::Spawn(void)
{
	Precache();
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255;
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	pev->sequence = 0;
	pev->framerate = 1.0;
	SET_MODEL(ENT(pev), "models/teleport_zombie_mark.mdl");
	SetTouch(&CTeleportEntity::OnTouch);
	SetThink(&CTeleportEntity::OnThink);

	UTIL_SetSize(pev, Vector(-5, -5, -2), Vector(5, 5, 2));
}

void CTeleportEntity::Precache(void)
{
	PRECACHE_MODEL("models/teleport_zombie_mark.mdl");
}

void CTeleportEntity::Kill(void)
{
	SetThink(NULL);
	SetTouch(NULL);

	UTIL_Remove(this);
}

void EXPORT CTeleportEntity::OnThink()
{
	CBasePlayer* owner = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev->owner));
	if (!owner->IsAlive())
		Kill();
}

void EXPORT CTeleportEntity::OnTouch(CBaseEntity* pOther)
{
	CBasePlayer* owner = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev->owner));
	pev->movetype = MOVETYPE_PUSHSTEP;
	if (!owner->IsAlive())
		Kill();
}

void CTeleportEntity::Play_EfTeleport()
{
	MESSAGE_BEGIN(MSG_ALL, gmsgHeadIcon, NULL, this->pev);
	WRITE_BYTE(7);	//type
	WRITE_SHORT(ENTINDEX(this->edict()));	//entity
	WRITE_BYTE(1);
	MESSAGE_END();
}
}
