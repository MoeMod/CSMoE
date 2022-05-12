#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
	LINK_ENTITY_TO_CLASS(sgmissile_cannon, CSGMissileCannon)

	void CSGMissileCannon::Spawn()
	{
		Precache();
		
		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);
		
		SET_MODEL(ENT(pev), "sprites/ef_sgmissile_line.spr");

		pev->frame = 0.0;
		pev->effects = 0;
		pev->skin = 0;
		pev->body = 0;
		pev->solid = SOLID_TRIGGER; // 2	
		pev->rendermode = kRenderTransAdd; // 5;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 90.0;
		pev->movetype = MOVETYPE_FLYMISSILE; // 9
		pev->angles.z = (float)RANDOM_LONG(5, 180);

		SetThink(&CBalrog11Cannon::AnimationThink);
		SetTouch(&CSGMissileCannon::SGMissileBcsTouch);
		m_vecOrigin = pev->origin;
		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->classname = MAKE_STRING("sgmissile");

		m_flNumFrames = (float)(MODEL_FRAMES(pev->modelindex) - 1);
		pev->framerate = RANDOM_FLOAT(1.0, 30.0);

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL);
		WRITE_BYTE(2);
		WRITE_COORD(m_vecOrigin.x);
		WRITE_COORD(m_vecOrigin.y);
		WRITE_COORD(m_vecOrigin.z);
		WRITE_SHORT(this->entindex());
		WRITE_BYTE(2);
		MESSAGE_END();
	}

	void CSGMissileCannon::Init(Vector vecDir, float flSpeed, duration_t tTimeRemove, float flDamage, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList, float flExplodeDamage, float flExplodeRadius, float flDirectForce)
	{
		m_vecDir = vecDir;
		m_flSpeed = flSpeed;
		m_tTimeRemove = tTimeRemove + gpGlobals->time;
		m_vecVelocity = vecDir * m_flSpeed;
		m_flDamage = flDamage;
		pev->velocity = m_vecVelocity;
		m_pEnemyList = pList;
		m_pGroupList = pGroupList;
		m_pGroupList->AddToTail(this);
		pev->avelocity.z = (float)RANDOM_LONG(5, 50);
		if (RANDOM_LONG(0, 1))
			pev->avelocity.z *= -1;
		m_flExplodeRadius = flExplodeRadius;
		m_flExplodeDamage = flExplodeDamage;
		m_flDirectForce = flDirectForce;
	}

	void CSGMissileCannon::Precache()
	{
		PRECACHE_MODEL("sprites/ef_sgmissile_line.spr");

		m_iEffectSprite = PRECACHE_MODEL("sprites/ef_sgmissile.spr");
		m_iExplosionSprite = PRECACHE_MODEL("sprites/dart_explosion.spr");

		PRECACHE_SOUND("weapons/sgmissile_exp.wav");
	}

	void CSGMissileCannon::Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u

		return UTIL_Remove(this);
	}

	void CSGMissileCannon::SGMissileBcsTouch(CBaseEntity* pOther)
	{
		if (ENT(pOther->pev) == pev->owner)
			return;

		if (m_pEnemyList->HasElement(pOther))
			return;

		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);

		if (pEntityOwner)
		{
			if (!pEntityOwner->m_bIsZombie)
			{
				if (!pOther->IsDormant())
				{
					if (pOther->pev->takedamage != DAMAGE_NO)
					{
						TraceResult tr = UTIL_GetGlobalTrace();

						if (tr.pHit && (CBaseEntity*)GET_PRIVATE(tr.pHit) == pOther)
						{
							entvars_t* pevOwner = VARS(pev->owner);

							ClearMultiDamage();

							pOther->TraceAttack(pevOwner, m_flExplodeDamage, pev->velocity, &tr, DMG_BULLET | DMG_NEVERGIB);

							ApplyMultiDamage(pev, pOwner->pev);

							if (pOther->IsPlayer() && ((CBasePlayer*)pOther)->m_bIsZombie && g_pGameRules->PlayerRelationship(pEntityOwner, pOther) != GR_TEAMMATE)
							{
								UTIL_AddKickRateBoost(pOther, (pOther->pev->origin.Make2D() - pOwner->pev->origin.Make2D()).Normalize(), 1.0, m_flDirectForce, 0.0);
							}

							m_pEnemyList->AddToTail(pOther);
							Explode();
						}
					}
				}
			}
		}

		if (pOther->IsBSPModel())
		{
			Explode();

			m_pGroupList->FindAndRemove(this);

			if (!m_pGroupList->Count())
			{
				delete m_pEnemyList;
				delete m_pGroupList;
			}

			UTIL_Remove(this);

			SetThink(NULL);
			SetTouch(NULL);
		}
	}

	
	void CSGMissileCannon::Explode(void)
	{
		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);

		if (!pEntityOwner)
			return;

		if (FNullEnt(pEntityOwner->edict()))
			return;

		Vector vecEffectPos = pev->origin - pev->velocity.Normalize() * 10;

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecEffectPos.x);
		WRITE_COORD(vecEffectPos.y);
		WRITE_COORD(vecEffectPos.z);
		WRITE_SHORT(m_iEffectSprite);
		WRITE_BYTE(5);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecEffectPos.x);
		WRITE_COORD(vecEffectPos.y);
		WRITE_COORD(vecEffectPos.z);
		WRITE_SHORT(m_iExplosionSprite);
		WRITE_BYTE(10);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
		MESSAGE_END();

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/sgmissile_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		CBaseEntity *pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flExplodeRadius)) != NULL)
		{
			if (pEntityOwner->m_bIsZombie)
				break;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
				continue;

			if (pEntity->pev == pEntityOwner->pev)
				continue;

			if (g_pGameRules->PlayerRelationship(pEntityOwner, pEntity) == GR_TEAMMATE)
				continue;

			float flDamage = UTIL_CalcDamage(pev->origin, pEntity->pev->origin, m_flExplodeDamage, m_flExplodeRadius, NULL);

			pEntity->TakeDamage(pev, pEntityOwner->pev, flDamage, DMG_EXPLOSION);
		}
	}
}
