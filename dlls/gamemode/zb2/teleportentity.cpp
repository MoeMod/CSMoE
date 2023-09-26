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
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->sequence = 0;
	pev->framerate = 1.0f;
	pev->friction = 0.7f;
	pev->nextthink = gpGlobals->time + 2s;
	SET_MODEL(ENT(pev), "models/teleport_zombie_mark.mdl");
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	SetThink(&CTeleportEntity::OnThink);
	SetTouch(&CTeleportEntity::OnTouch);
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
	if (!owner->IsAlive() || owner->m_iZombieClass != ZOMBIE_CLASS_TELEPORT || !owner->m_bIsZombie)
	{
		Kill();
		return;
	}
	pev->nextthink = gpGlobals->time + 2s;
}

void EXPORT CTeleportEntity::OnTouch(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther->edict() == pev->owner)
		return;

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.5f;
	}
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
