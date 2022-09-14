#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_shared/wpn_speargun.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {

	LINK_ENTITY_TO_CLASS(spear, CSpear);


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


	CSpear* CSpear::Create(int iType)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("spear"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CSpear* pSpear = (CSpear*)Instance(pent);
		if (pSpear)
		{
			pSpear->pev->classname = MAKE_STRING("spear");
			pSpear->m_iType = iType;
			pSpear->Spawn();
		}

		return pSpear;	
	}

	void CSpear::AddPlayerToCrowd(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_hSpearAttached != this)
		{
			if (m_iNumCrowdPlayers >= SPEAR_MAX_CROWD_PLAYERS)
				return;

			m_rgCrowdPlayers[m_iNumCrowdPlayers] = pPlayer;
			pPlayer->m_hSpearAttached = this;

			m_iNumCrowdPlayers++;
		}

		pPlayer->m_flNextSpearPush = gpGlobals->time + 0.08s;
	}

	void CSpear::Update(void)
	{
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

	void CSpear::Explode(duration_t flTimeRemove, bool bDetonate)
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

	
		if (pHitEntity)
		{
			if (pHitEntity->pev->takedamage != DAMAGE_NO)
			{
				pHitEntity->m_LastHitGroup = m_LastHitGroup;

				float flRatio;

				/*
				if (IsZSMonster(pHitEntity))
					flRatio = 1.28;
				else
				*/
				flRatio = 1.0;

				pHitEntity->TakeDamage(pev, pevOwner, m_flHitDamage * flRatio, DMG_BULLET);
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

			if (pEntity->IsPlayer() && ENT(pEntity->pev) != pev->owner && pEntity->m_iTeam == m_iTeam)
				continue;

			Vector vecDir;

			float flDamage = UTIL_CalcDamage(vecSrc, pEntity->Center(), m_flExplodeDamage, flRadius, vecDir);

			if (flDamage == 0.0)
				continue;

			if (ENT(pEntity->pev) == pev->owner)
				flDamage *= m_flOwnerDamageModifier;


			pEntity->m_LastHitGroup = HITGROUP_CHEST;

			pEntity->TakeDamage(pev, pevOwner, flDamage, DMG_EXPLOSION);

			if (pEntity->IsPlayer())
			{
				pEntity->pev->velocity += vecDir * (flDamage / m_flExplodeDamage) * m_flExplodeForce;

				if (pEntity->pev->velocity.z <= 199)
					pEntity->pev->velocity.z = 199;
			}
		}

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(g_sModelIndexFireball2);
		WRITE_BYTE(2 - int(flRadius * -0.3));
		WRITE_BYTE(150);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_TEMPEXPLODEMODEL);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_COORD(400);
		WRITE_SHORT(m_iExplodeModel);
		WRITE_SHORT(30);
		WRITE_BYTE(int(0.3 * 10));
		WRITE_BYTE(1);
		MESSAGE_END();

		if (flTimeRemove <= 0.3s)
		{
			pev->effects |= EF_NODRAW;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_KILLBEAM);
			WRITE_SHORT(this->entindex());
			MESSAGE_END();

			SetThink(&CBaseEntity::SUB_Remove);
			SetTouch(NULL);

			pev->nextthink = gpGlobals->time + flTimeRemove;
		}
		else
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_KILLBEAM);
			WRITE_SHORT(this->entindex());
			MESSAGE_END();

			UTIL_Remove(this);
		}
	}

	void CSpear::FollowThink(void)
	{
		Update();

		pev->nextthink = gpGlobals->time + 0.032s;
	}

	void CSpear::IgniteThink(void)
	{
		Update();

		if (m_iType)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iBeamModelIndex);
			WRITE_BYTE(4);
			WRITE_BYTE(3);
			WRITE_BYTE(255);
			WRITE_BYTE(100);
			WRITE_BYTE(0);
			WRITE_BYTE(200);
			MESSAGE_END();
		}
		else
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iBeamModelIndex);
			WRITE_BYTE(4);
			WRITE_BYTE(3);
			WRITE_BYTE(50);
			WRITE_BYTE(185);
			WRITE_BYTE(200);
			WRITE_BYTE(200);
			MESSAGE_END();
		}

		SetThink(&CSpear::FollowThink);

		pev->nextthink = gpGlobals->time + 0.032s;
	}

	void CSpear::CrowdCallback(CBaseEntity* entityPushing, CBaseEntity* entityBehind)
	{
		if (m_flPushDuration == UTIL_WeaponTimeBase())
			return;

		if (m_hLockingEntity == entityPushing)
		{
			if (entityBehind->IsPlayer() && m_iTeam != entityBehind->m_iTeam)
			{
				Vector vecDir = CalcCrowdForceDir(entityPushing, entityBehind);

				if (vecDir.x || vecDir.y || vecDir.z)
				{
					entityBehind->pev->velocity = vecDir * m_flCrowdForce;
					entityBehind->pev->velocity.z = m_flCrowdBoostVel;

					/*
					if (entityBehind->Classify() == CLASS_ZBS_MONSTER)
						entityBehind->m_flNextSpearPush = gpGlobals->time + 0.45;
					*/

					bool bFound = false;

					for (int i = 0; i < m_iNumCrowdMonsters; i++)
					{
						if (m_rgCrowdMonsters[i].Get() == ENT(entityBehind->pev))
						{
							bFound = true;

							break;
						}
					}

					if (!bFound && m_iNumCrowdMonsters < SPEAR_MAX_CROWD_MONSTERS)
					{
						if (entityBehind->pev->takedamage && entityBehind->pev->health > 0)
						{
							entvars_t* pevOwner = VARS(pev->owner);

							entityBehind->m_LastHitGroup = HITGROUP_CHEST;

							entityBehind->TakeDamage(pev, pevOwner, m_flCrowdDamage, DMG_BULLET | DMG_NEVERGIB);

							if (gpGlobals->time >= m_flTimeNextBleed)
							{
								TraceResult tr = UTIL_GetGlobalTrace();

								if (g_pModRunning->DamageTrack() == DT_ZB)
									UTIL_BloodDrips(tr.vecEndPos, Vector(0, 0, 1), BLOOD_COLOR_RED, 30);

								m_flTimeNextBleed = gpGlobals->time + 0.05s;
							}
						}

						m_rgCrowdMonsters[m_iNumCrowdMonsters] = entityBehind;
						m_rgvecCrowdPushVelocity[m_iNumCrowdMonsters] = entityBehind->pev->velocity;
						m_rgflTimeCrowdMovement[m_iNumCrowdMonsters] = gpGlobals->time + 0.1s;

						m_iNumCrowdMonsters++;
					}

					if (entityBehind->IsPlayer())
						AddPlayerToCrowd((CBasePlayer*)entityBehind);
				}
			}
			else if (entityBehind->IsBSPModel() && UTIL_PointContents(entityBehind->pev->origin) != CONTENTS_WATER)
			{
				TraceResult tr = UTIL_GetGlobalTrace();

				float dot = DotProduct((entityPushing->pev->velocity * -1).Normalize(), tr.vecPlaneNormal);

				if (dot > cos(45.0f * 3.141592f / 180.0f))
				{
					float modifier = (1.0 - 0.5) * (1.0 - dot) + 0.5;

					if (m_flPushVelocityModifier > 0.01 && modifier != 0)
						m_flPushVelocityModifier *= modifier;
				}

				if (dot > cos(15.0f * 3.141592f / 180.0f))
					Explode(0.0s, false);
			}
		}
		else if (entityPushing->IsPlayer() && entityBehind->IsPlayer() && SPEAR_PLAYER_PUSH_INTERVAL != 0s)
		{
			CBasePlayer* pPlayer = static_cast<CBasePlayer*>(entityBehind);	
			if (g_pGameRules->PlayerRelationship(pPlayer, entityPushing) == GR_TEAMMATE && (pPlayer->m_hSpearAttached != nullptr || pPlayer->m_hSpearAttached == this))
			{
				AddPlayerToCrowd(pPlayer);

				if (pPlayer->m_flNextSpearPush > gpGlobals->time && gpGlobals->time > pPlayer->m_flNextSpearPush)
				{
					Vector vecDir = CalcCrowdForceDir(entityPushing, entityBehind);
					vecDir.z = 0;

					if (vecDir.x || vecDir.y || vecDir.z)
					{
						entityBehind->pev->velocity = vecDir * entityPushing->pev->velocity.Length() * 1.01;

						((CBasePlayer*)entityBehind)->m_flNextSpearPush = gpGlobals->time + SPEAR_PLAYER_PUSH_INTERVAL;
					}
				}
			}	
		}
	}

	void CSpear::MaterialSound(void)
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		char ch = UTIL_TextureHit(&tr, pev->origin - m_vecDirection, pev->origin + m_vecDirection);
		const char* rgsz[2];

		switch (ch)
		{
		case CHAR_TEX_WOOD:
			rgsz[0] = "weapons/speargun_wood1.wav";
			rgsz[1] = "weapons/speargun_wood2.wav";
			break;

		case CHAR_TEX_GRATE:
		case CHAR_TEX_METAL:
		case CHAR_TEX_COMPUTER:
			rgsz[0] = "weapons/speargun_metal1.wav";
			rgsz[1] = "weapons/speargun_metal2.wav";
			break;

		default:
			rgsz[0] = "weapons/speargun_stone1.wav";
			rgsz[1] = "weapons/speargun_stone2.wav";
			break;
		}

		UTIL_EmitAmbientSound(ENT(eoNullEntity), tr.vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
	}

	void CSpear::Precache(void)
	{
		if (m_iType)
		{
			m_iArrowModel = PRECACHE_MODEL("models/spearm.mdl");
			m_iExplodeModel = PRECACHE_MODEL("models/spearm2.mdl");
		}
		else
		{
			m_iArrowModel = PRECACHE_MODEL("models/spear.mdl");
			m_iExplodeModel = PRECACHE_MODEL("models/spear2.mdl");
		}
		
		m_iBeamModelIndex = PRECACHE_MODEL("sprites/smoke.spr");
	}

	void CSpear::HitDamage(CBaseEntity* pOther)
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		ClearMultiDamage();

		pOther->TraceAttack(VARS(pev->owner), m_flHitDamage, m_vecDirection, &tr, DMG_BULLET | DMG_NEVERGIB);

		m_LastHitGroup = pOther->m_LastHitGroup;		
		m_flHitDamage = gMultiDamage.amount;
	}

	void CSpear::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_BBOX;
		pev->gravity = 0.5;
		pev->friction = 0;

		if(m_iType)
			SET_MODEL(ENT(pev), "models/spearm.mdl");
		else
			SET_MODEL(ENT(pev), "models/spear.mdl");

		pev->mins = pev->maxs = Vector(0.008, 0.008, 0.008);

		UTIL_SetSize(pev, pev->mins, pev->maxs);
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CSpear::SpearTouch);
		SetThink(&CSpear::IgniteThink);

		pev->nextthink = gpGlobals->time;

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

	void CSpear::SpearTouch(CBaseEntity* pOther)
	{
		if (m_hLockingEntity.Get() != NULL)
			return;

		m_hEntityHit = pOther;

		TraceResult tr = UTIL_GetGlobalTrace();

		m_LastHitGroup = tr.iHitgroup;

		if (pOther->IsBSPModel())
		{
			MaterialSound();

			if (gpGlobals->time + 0.8s <= m_flTimeExplode)
				m_flTimeExplode = gpGlobals->time + 0.8s;

			if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
			{
				UTIL_DecalTrace(&tr, DamageDecal(DMG_BULLET | DMG_NEVERGIB));
				UTIL_GunshotDecalTrace(&tr, DamageDecal(DMG_BULLET | DMG_NEVERGIB), false, NULL);

				if (m_flTimeExplode <= gpGlobals->time)
				{
					SetTouch(NULL);

					pev->effects |= EF_NODRAW;
					pev->velocity = g_vecZero;
					pev->origin -= m_vecDirection * 5;
				}
				else
				{
					Vector vecEffectPos = pev->origin + m_vecDirection * UTIL_QuickRandomFloat(-7.0, -2.0);

					MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
					WRITE_BYTE(TE_ARROWMODEL);
					WRITE_COORD(vecEffectPos.x);
					WRITE_COORD(vecEffectPos.y);
					WRITE_COORD(vecEffectPos.z);
					WRITE_ANGLE(pev->angles[0]);
					WRITE_ANGLE(pev->angles[1]);
					WRITE_ANGLE(pev->angles[2]);
					WRITE_SHORT(m_iArrowModel);
					WRITE_BYTE(int((m_flTimeExplode - gpGlobals->time) / 1s * 10.0));
					WRITE_BYTE(10);
					WRITE_BYTE(1);
					WRITE_BYTE(200);
					WRITE_BYTE(4);
					WRITE_SHORT(-1);
					WRITE_BYTE(17);
					MESSAGE_END();

					SetTouch(NULL);

					pev->effects |= EF_NODRAW;
					pev->velocity = g_vecZero;
					pev->origin -= m_vecDirection * 5;
				}
			}
			else
			{
				SetTouch(NULL);

				pev->effects |= EF_NODRAW;
				pev->velocity = g_vecZero;
				pev->origin -= m_vecDirection * 5;
			}

			return;
		}

		if (!pOther->IsPlayer())	// !CanAddKickRate(pOther)
		{
			HitDamage(pOther);

			pev->origin -= m_vecDirection * 5.0;
			Explode(0.0s, true);
			return;
		}

		if (pOther->IsPlayer() && pOther->m_iTeam == m_iTeam)
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
