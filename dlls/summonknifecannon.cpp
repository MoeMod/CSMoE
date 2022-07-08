#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {

	LINK_ENTITY_TO_CLASS(summonknife_cannon, CSummonKnifeCannon);


	float UTIL_QuickRandomFloat(float low, float high);

	static Vector CalcCrowdForceDir(CBaseEntity* pPushing, CBaseEntity* pBehind)
	{
		Vector vecPushingDir = pPushing->pev->velocity.Normalize();
		Vector vecDir = (pBehind->pev->origin - pPushing->pev->origin).Normalize();
		float dot = DotProduct(vecPushingDir, vecDir);

		if (dot < 0)
			return g_vecZero;

		if (dot > cos(25 * M_PI / 180))
			return vecPushingDir;

		return vecDir;
	}


	CSummonKnifeCannon* CSummonKnifeCannon::Create(void)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("summonknife_cannon"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CSummonKnifeCannon* pCannon = (CSummonKnifeCannon*)Instance(pent);
		if (pCannon)
		{
			pCannon->pev->classname = MAKE_STRING("summonknife_cannon");

			pCannon->Spawn();
		}

		return pCannon;
	}

	void CSummonKnifeCannon::Update(void)
	{
		pev->frame += pev->framerate * gpGlobals->frametime / 1s;

		if (pev->frame > m_flNumFrames)
			pev->frame = fmod(pev->frame, m_flNumFrames);

		if (gpGlobals->time > m_flTimeExplode)
		{
			Explode(0.0s, false);

			return;
		}

		if (!(pev->effects & EF_NODRAW))
			pev->velocity = m_vecVelocity;

		if (m_bLockingTarget)
		{
			pev->origin = m_vecLockingPos;

			m_bLockingTarget = FALSE;
		}

		if (gpGlobals->time > m_flTimeStopPushing)
			return;

		if (m_hLockingEntity.Get() != NULL)
		{
			if (!m_hLockingEntity->IsAlive())
			{
				Explode(0.0s, false);
			}
			else
			{
				m_hLockingEntity->pev->velocity = m_vecVelocity.Normalize() * m_flPushForce;

				if (m_flPushBoostVel > m_hLockingEntity->pev->velocity.z)
					m_hLockingEntity->pev->velocity.z = m_flPushBoostVel;

				m_hLockingEntity->pev->velocity = m_hLockingEntity->pev->velocity * m_flPushVelocityModifier;
			}
		}

		for (int i = 0; i < m_iNumCrowdMonsters; i++)
		{
			if (!m_rgCrowdMonsters[i].Get())
				continue;

			if ((CBaseEntity*)m_rgCrowdMonsters[i] && m_rgCrowdMonsters[i]->IsAlive())
				m_rgCrowdMonsters[i]->pev->velocity = m_rgvecCrowdPushVelocity[i];
		}
	}

	void CSummonKnifeCannon::Explode(duration_t flTimeRemove, bool bDetonate)
	{
		m_iExplosionCount++;

		if (m_iExplosionCount > 2)
			return;

		entvars_t* pevOwner = VARS(pev->owner);

		Vector vecSrc;
		CBaseEntity* pHitEntity = (CBaseEntity*)GET_PRIVATE(m_hEntityHit.Get());

		if (bDetonate || !pHitEntity || pHitEntity->IsBSPModel())
			vecSrc = pev->origin;
		else
			vecSrc = pHitEntity->pev->origin;

	
		if (pHitEntity && pHitEntity->IsPlayer())
		{
			CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pHitEntity);
			if (pPlayer->pev->takedamage != DAMAGE_NO)
			{
				pPlayer->m_LastHitGroup = m_LastHitGroup;

				float flRatio = 1.0;

				pPlayer->TakeDamage(pev, pevOwner, m_flHitDamage * flRatio, DMG_BULLET | DMG_NEVERGIB);
			}
		}

		CBaseEntity* pEntity = NULL;
		const float flRadius = 3.0 * 39.37;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->pev->health <= 0)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			if (pEntity == m_pOwner)
				continue;

			if (pEntity->IsPlayer() && ENT(pEntity->pev) != pev->owner && g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
				continue;

			Vector vecDir;

			float flDamage = UTIL_CalcDamage(vecSrc, pEntity->Center(), m_flExplodeDamage, flRadius, vecDir);

			if (flDamage == 0.0)
				continue;

			pEntity->TakeDamage(pev, pevOwner, flDamage, DMG_EXPLOSION | DMG_NEVERGIB);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);
				pPlayer->m_LastHitGroup = HITGROUP_CHEST;
				pEntity->pev->velocity += vecDir * (flDamage / m_flExplodeDamage) * m_flExplodeForce;

				if (pEntity->pev->velocity.z <= 199)
					pEntity->pev->velocity.z = 199;
			}
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(m_iExplodeModel);
		WRITE_BYTE(6);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		if (flTimeRemove <= 0.3s)
		{
			pev->effects |= EF_NODRAW;

			SetThink(&CBaseEntity::SUB_Remove);
			SetTouch(NULL);

			pev->nextthink = gpGlobals->time + flTimeRemove;
		}
		else
		{
			UTIL_Remove(this);
		}
	}

	void CSummonKnifeCannon::IgniteThink(void)
	{
		Update();
		pev->nextthink = gpGlobals->time + 0.032s;
	}

	void CSummonKnifeCannon::Precache(void)
	{
		m_iArrowModel = PRECACHE_MODEL("models/ef_summonknife_cannon.mdl");
		m_iExplodeModel = PRECACHE_MODEL("sprites/ef_summon_charging_exp.spr");
	}

	void CSummonKnifeCannon::HitDamage(CBaseEntity* pOther)
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		ClearMultiDamage();

		pOther->TraceAttack(VARS(pev->owner), m_flHitDamage, m_vecDirection, &tr, DMG_BULLET | DMG_NEVERGIB);
		if (pOther->IsPlayer())
		{
			CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pOther);
			m_LastHitGroup = pPlayer->m_LastHitGroup;
		}
		
		m_flHitDamage = gMultiDamage.amount;
	}

	void CSummonKnifeCannon::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_BBOX;
		pev->gravity = 0.5;
		pev->friction = 0;

		SET_MODEL(ENT(pev), "models/ef_summonknife_cannon.mdl");

		pev->mins = pev->maxs = Vector(0.008, 0.008, 0.008);

		UTIL_SetSize(pev, pev->mins, pev->maxs);
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CSummonKnifeCannon::CannonTouch);
		SetThink(&CSummonKnifeCannon::IgniteThink);

		pev->nextthink = gpGlobals->time;

		m_flNumFrames = (float)(MODEL_FRAMES(pev->modelindex) - 1);
		pev->framerate = 1.0;
		m_flTimeStopPushing = invalid_time_point;
		m_flPushVelocityModifier = 1.0;

		m_hLockingEntity = NULL;
		m_hEntityHit = NULL;
		m_pOwner = NULL;
		m_LastHitGroup = HITGROUP_GENERIC;

		for (int i = 0; i < SPEAR_MAX_CROWD_MONSTERS; i++)
			m_rgCrowdMonsters[i] = NULL;

		m_iNumCrowdMonsters = 0;

		for (int i = 0; i < SPEAR_MAX_CROWD_PLAYERS; i++)
			m_rgCrowdPlayers[i] = NULL;

		m_iNumCrowdPlayers = 0;

		m_flTimeNextBleed = invalid_time_point;
		m_iExplosionCount = 0;
	}

	void CSummonKnifeCannon::CannonTouch(CBaseEntity* pOther)
	{
		if (m_hLockingEntity.Get() != NULL)
			return;

		m_hEntityHit = pOther;

		TraceResult tr = UTIL_GetGlobalTrace();

		m_LastHitGroup = tr.iHitgroup;

		if (pOther->IsBSPModel())
		{	
			SetTouch(NULL);

			pev->effects |= EF_NODRAW;
			pev->velocity = g_vecZero;
			pev->origin -= m_vecDirection * 5;

			HitDamage(pOther);

			pev->origin -= m_vecDirection * 5.0;
			Explode(0.0s, true);

			return;
		}

		if (!pOther->IsPlayer())	// !CanAddKickRate(pOther)
		{
			SetTouch(NULL);

			pev->effects |= EF_NODRAW;
			pev->velocity = g_vecZero;
			pev->origin -= m_vecDirection * 5;

			HitDamage(pOther);

			pev->origin -= m_vecDirection * 5.0;
			Explode(0.0s, true);
			return;
		}

		if (pOther->IsPlayer() && g_pGameRules->PlayerRelationship(m_pOwner, pOther) == GR_TEAMMATE)
		{
			pev->origin += m_vecDirection * 0.5 * 39.37;

			m_vecLockingPos = pev->origin;
			m_bLockingTarget = TRUE;

			pev->nextthink = gpGlobals->time;

			return;
		}

		HitDamage(pOther);

		if (pOther->IsAlive() && pOther->IsPlayer())
		{
			CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pOther);
			if (pOther->pev->takedamage != DAMAGE_NO)
			{
				pev->solid = SOLID_NOT;

				if (pPlayer->m_hSpearAttached.Get() == NULL)
				{
					m_hLockingEntity = pOther;
					pPlayer->m_hSpearAttached = this;

					SetTouch(NULL);

					m_flTimeStopPushing = gpGlobals->time + m_flPushDuration;

					pev->effects |= EF_NODRAW;
					pev->velocity = g_vecZero;

					if (gpGlobals->time + 0.8s <= m_flTimeExplode)
						m_flTimeExplode = gpGlobals->time + 0.8s;

					if (m_flTimeExplode - gpGlobals->time + 0.5s > UTIL_WeaponTimeBase())
					{
						int life = (m_flTimeExplode - gpGlobals->time + 0.5s) / 1s;
						Vector vecOffset = pev->origin - pOther->pev->origin;

						MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
						WRITE_BYTE(TE_FOLLOWINGARROW);
						WRITE_SHORT(pOther->entindex());
						WRITE_SHORT(m_iArrowModel);
						WRITE_COORD(vecOffset.x);
						WRITE_COORD(vecOffset.y);
						WRITE_COORD(vecOffset.z);
						WRITE_COORD(pev->angles.x - pOther->pev->angles.x);
						WRITE_COORD(pev->angles.y - pOther->pev->angles.y);
						WRITE_COORD(pev->angles.z - pOther->pev->angles.z);
						WRITE_BYTE(life);
						MESSAGE_END();
					}

					return;
				}

				pev->origin -= m_vecDirection * 5.0;
				Explode(0.0s, true);

				return;
			}
		}
	}
}
