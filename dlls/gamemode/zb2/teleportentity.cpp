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

	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}

void CTeleportEntity::Precache(void)
{
	PRECACHE_MODEL("models/teleport_zombie_mark.mdl");
	m_iTeleportZombieOut = PRECACHE_MODEL("sprites/ef_teleportzombieout.spr");
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
	if (!owner->IsAlive() || (owner->m_ZombieClass && stricmp(owner->m_ZombieClass, "teleport")))
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
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_TEMPSPRITE);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(m_iTeleportZombieOut);
	WRITE_BYTE(8);
	WRITE_BYTE(255);
	WRITE_BYTE(8);
	MESSAGE_END();
}
}
