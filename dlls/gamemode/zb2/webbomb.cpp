#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "webbomb.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"
#include "gamemode/mod_zb2.h"

#include <utility>

namespace sv {

	LINK_ENTITY_TO_CLASS(webbomb, CWebBomb);

	void CWebBomb::Spawn(void)
	{
		Precache();
		pev->movetype = MOVETYPE_BOUNCE;

		pev->classname = MAKE_STRING("webbomb");

		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/zb5_za_webbomb.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		SetTouch(&CWebBomb::OnTouch);
		SetThink(&CWebBomb::OnThink);

		m_flSlowdownTime = invalid_time_point;
	}

	void CWebBomb::Precache(void)
	{
		PRECACHE_MODEL("models/zb5_za_webbomb.mdl");
		m_iWebTrap = PRECACHE_MODEL("models/zb5_za_webtrap.mdl");
	}

	void EXPORT CWebBomb::OnThink(void)
	{
		if (!IsInWorld())
		{
			SetThink(NULL);
			UTIL_Remove(this);
			return;
		}

		if (m_flSlowdownTime != invalid_time_point && m_flSlowdownTime < gpGlobals->time)
		{
			SetThink(NULL);
			UTIL_Remove(this);
			return;
		}

		if (m_flSlowdownTime > gpGlobals->time)
		{
			Slowdown();
		}

		pev->nextthink = gpGlobals->time + 0.5s;
	}

	void EXPORT CWebBomb::OnTouch(CBaseEntity* pOther)
	{
		if (ENT(pOther->pev) == pev->owner)
			return;

		pev->velocity = g_vecZero;
		m_flSlowdownTime = gpGlobals->time + 5.0s;

		pev->solid = SOLID_NOT;
		pev->takedamage = DAMAGE_NO;

		UTIL_TempModel(pev->origin, g_vecZero, g_vecZero, m_iWebTrap, 50, 0, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

		SetTouch(NULL);
		pev->effects |= EF_NODRAW;
	}

	void CWebBomb::Slowdown()
	{
		TraceResult tr;
		CBaseEntity* pList[100];
		Vector vecStart;

		vecStart = pev->origin + Vector(0.0, 0.0, 1.0);

		const float flRange = 2.5 * 39.37;
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
			if (pPlayer->m_bIsZombie)
				continue;

			UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;

			auto strategy = dynamic_cast<CPlayerModStrategy_ZB2*>(pPlayer->m_pModStrategy.get());
			if (strategy)
			{
				strategy->SetSlowdownTime(1.0s);
			}

		}
	}

}
