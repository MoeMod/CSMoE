#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "zombiesmoke.h"
#include "gamemode/mod_zb2.h"
#include "vector.h"

namespace sv {

LINK_ENTITY_TO_CLASS(zombie_smoke, CZombieSmoke);

void CZombieSmoke::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->takedamage = DAMAGE_NO;
	pev->rendermode = 0;
	pev->renderamt = 0;
	pev->effects |= EF_NODRAW;

	SetThink(&CZombieSmoke::SmokeThink);
	pev->nextthink = gpGlobals->time + 0.1s;

	m_flReduceRate = 0.0;
}


void EXPORT CZombieSmoke::SmokeThink()
{
	pev->nextthink = gpGlobals->time + 0.1s;

	if (m_fKillTime < gpGlobals->time)
	{
		SetThink(&CBaseEntity::SUB_Remove);
		return;
	}

	CBaseEntity* pList[100];
	Vector vecStart;

	vecStart = pev->origin + Vector(0.0, 0.0, 1.0);

	const float flRange = 3 * 39.37;
	int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

	for (int i = 0; i < iCount; i++)
	{
		CBaseEntity* pEntity = pList[i];

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->edict() == pev->owner)
			continue;

		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (!pPlayer->m_bIsZombie)
			continue;

		auto strategy = dynamic_cast<CPlayerModStrategy_ZB2*>(pPlayer->m_pModStrategy.get());
		if (strategy)
		{
			strategy->SetInSmokeTime(1.0s, m_flReduceRate);
		}

	}

}

void CZombieSmoke::SetLifespan(duration_t flLifespawn)
{
	m_fKillTime = gpGlobals->time + flLifespawn;
}

void CZombieSmoke::SetReduceRate(float rate)
{
	m_flReduceRate = rate;
}


}