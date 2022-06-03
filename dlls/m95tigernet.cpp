#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "m95tigernet.h"


namespace sv {
	LINK_ENTITY_TO_CLASS(m95tiger_net, CM95TigerNet)

	void CM95TigerNet::Spawn()
	{
		Precache();

		SetThink(&CM95TigerNet::FlyThink);
		SetTouch(&CM95TigerNet::OnTouch);
		SET_MODEL(this->edict(), "models/ef_m95tiger_net.mdl");

		pev->rendermode = kRenderTransAdd;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 255;
		pev->framerate = 1.0;
		pev->solid = SOLID_TRIGGER;
		pev->movetype = MOVETYPE_FLY;

		m_flRemoveTime = invalid_time_point;
		UTIL_SetSize(pev, Vector(-16.0, -16.0, -16.0), Vector(16.0, 16.0, 16.0));
		pev->classname = MAKE_STRING("m95tiger_net");//nodamage

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CM95TigerNet::Precache()
	{
		PRECACHE_MODEL("models/ef_m95tiger_net.mdl");
	}

	void CM95TigerNet::OnTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pev->owner == pOther->edict())
			return;

		if (pOther->IsBSPModel())
		{
			m_flRemoveTime = gpGlobals->time + 0.1s;
			return;
		}

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

		CBaseEntity* pEntity = NULL;
		if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
		{
			TraceResult tr;

			Vector vecEnd;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 75.0)) != NULL)
			{
				if (pEntity->IsBSPModel())
					continue;

				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;
					
					player->pev->velocity = g_vecZero;
					player->m_tLockBuffTime = gpGlobals->time + m_flLockTime;			
					player->m_flLockGravity = 10.0f;
					player->m_flLockSpeed = 1.0f;
					player->ResetMaxSpeed();			
				}

				PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, g_vecZero, 0.0, 0.0, 23, pEntity->entindex(), TRUE, FALSE);
			}
		}

		Remove();
	}

	void CM95TigerNet::FlyThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;
		if (m_flRemoveTime != invalid_time_point && m_flRemoveTime <= gpGlobals->time)
		{
			Remove();
			return;
		}
	}

	void CM95TigerNet::Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
}
