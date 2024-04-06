#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamemode/zbs/monster_entity.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {

	LINK_ENTITY_TO_CLASS(spearex, CSpearEX);

	static constexpr float AmmoSpeed = 2000.0;
	static constexpr float AutoAimDegree = 7.0;
	static constexpr const auto& AmmoExpTime = 0.8s;

	static constexpr float ExpRadius = 3.0;
	static constexpr int TraceLife = 4;
	static constexpr int TraceColorR = 50;
	static constexpr int TraceColorG = 185;
	static constexpr int TraceColorB = 200;
	static constexpr int TraceBrightness = 200;
	static constexpr int TraceWidth = 3;	//3.5

	static constexpr float ChargingExpRadius = 2.5;
	static constexpr int ChargingShotTraceR = 255;
	static constexpr int ChargingShotTraceG = 85;
	static constexpr int ChargingShotTraceB = 50;
	static constexpr int ChargingShotTraceBrightness = 100;
	static constexpr int ChargingShotTraceWidth = 1;
	static constexpr float ChargingShotExpScale = 0.9;

	static constexpr float FullChargingExpRadius = 4.0;
	static constexpr int FullChargingShotTraceR = 255;
	static constexpr int FullChargingShotTraceG = 85;
	static constexpr int FullChargingShotTraceB = 50;
	static constexpr int FullChargingShotTraceBrightness = 150;
	static constexpr int FullChargingShotTraceWidth = 5;
	static constexpr float FullChargingShotExpScale = 1.6;

	static constexpr float ExpKickMinZ = 199.0;

	static constexpr float ExpKickPower = 0.0;
	static constexpr float ExpKickPowerZombie = 0.0;
	static constexpr float ExpKickPowerScenario = 0.0;

	void CSpearEX::AddPlayerToCrowd(CBaseMonster* pPlayer)
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

	void CSpearEX::Update(void)
	{
		if (m_iType == 0 && m_hTarget)
		{
			edict_t* pEnt = m_hTarget.Get();

			Vector vecAim, vecRight, vecUp, vecNewDir;
			vecAim = pev->velocity.Normalize();
			Vector vecAngle = UTIL_VecToAngles(vecAim);
			Vector vecAngleConst = UTIL_VecToAngles(vecAim);

			Vector vecIdeal = m_hTarget->BodyTarget(vecRight) - pev->origin;
			float lenVecIdeal = vecIdeal.Length();
			float flAmmoSpeed = AmmoSpeed * std::clamp(lenVecIdeal * 0.002, 0.1, 1.0);
			float NeedAngle;

			int iCalc = 0;
			iCalc = UTIL_MissileCalcDirection(pev->velocity, vecIdeal, 7.0, vecNewDir);
			
			if (iCalc && vecNewDir.Length())
			{
				m_vecVelocity = vecNewDir * flAmmoSpeed;

				pev->angles = UTIL_VecToAngles(vecNewDir);
			}
		

			//if (m_flTimeNextAutoAim != invalid_time_point && m_flTimeNextAutoAim < gpGlobals->time)
			//	m_flTimeNextAutoAim = gpGlobals->time + 0.1s;

			//	float angDiff = acos((DotProduct(vecAim, vecIdeal) / lenVecIdeal)) * 180.0 / M_PI;
			//	float flAngleSpeed = min(5.0, angDiff * 0.5 * flAmmoSpeed / AmmoSpeed);
			//	if (lenVecIdeal < 50)
			//	{
			//		vecAngle = UTIL_VecToAngles(vecIdeal);
			//		flAngleSpeed = angDiff;
			//	}
			//	else
			//	{

			//		NeedAngle = UTIL_AnglesRotateTo(pev->origin, vecAngleConst, pEnt->v.origin, vecAngle, flAngleSpeed);

			//	}
			//	//float flAngleSpeed = angDiff * 1.5 / lenVecIdeal / flAmmoSpeed / 0.01;



			//UTIL_MakeVectors(vecAngle);

			//pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);

			//m_vecVelocity = gpGlobals->v_forward * flAmmoSpeed;
		}

		if (gpGlobals->time > m_flTimeExplode)
		{
			Explode(0.0s, false);

			return;
		}

		if (!(pev->effects & EF_NODRAW))
		{
			pev->velocity = m_vecVelocity;
			m_vecDirection = pev->velocity.Normalize();
		}
			
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

	void CSpearEX::Explode(duration_t flTimeRemove, bool bDetonate)
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


				if (pHitEntity->pev->flags & FL_MONSTER)
					flRatio = 1.28;
				else

					flRatio = 1.0;

				pHitEntity->TakeDamage(pev, pevOwner, m_flHitDamage * flRatio, DMG_BULLET);
			}
		}

		CBaseEntity* pEntity = NULL;

		float flRadius;
		if (m_iType == 2)
		{
			flRadius = FullChargingExpRadius * 39.37;
		}
		else if (m_iType == 1)
		{
			flRadius = ChargingExpRadius * 39.37;
		}
		else
			flRadius = ExpRadius * 39.37;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->pev->health <= 0)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			if (ENT(pEntity->pev) == pev->owner)
				continue;

			if (pEntity->IsPlayer() && ENT(pEntity->pev) != pev->owner && pEntity->m_iTeam == m_iTeam)
				continue;

			Vector vecDir;

			float flDamage = UTIL_CalcDamage(vecSrc, pEntity->Center(), m_flExplodeDamage, flRadius, vecDir);

			if (flDamage == 0.0)
				continue;

			pEntity->m_LastHitGroup = HITGROUP_CHEST;

			pEntity->TakeDamage(pev, pevOwner, flDamage, DMG_EXPLOSION);

			if (pEntity->IsPlayer())
			{
				pEntity->pev->velocity += vecDir * (flDamage / m_flExplodeDamage) * m_flExplodeForce;

				if (pEntity->pev->velocity.z <= ExpKickMinZ)
					pEntity->pev->velocity.z = ExpKickMinZ;
			}
		}

		if (m_iType == 2)
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(vecSrc.x);
			WRITE_COORD(vecSrc.y);
			WRITE_COORD(vecSrc.z);
			WRITE_SHORT(m_iExplodeSprite);
			WRITE_BYTE(int(FullChargingShotExpScale * 10));
			WRITE_BYTE(150);
			WRITE_BYTE(30);
			MESSAGE_END();

			UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/speargunex_exp2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		else if (m_iType == 1)
		{

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(vecSrc.x);
			WRITE_COORD(vecSrc.y);
			WRITE_COORD(vecSrc.z);
			WRITE_SHORT(m_iExplodeSprite);
			WRITE_BYTE(int(ChargingShotExpScale * 10));
			WRITE_BYTE(150);
			WRITE_BYTE(30);
			MESSAGE_END();

			UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/speargunex_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		else
		{
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
		}

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

	void CSpearEX::FollowThink(void)
	{
		Update();

		pev->nextthink = gpGlobals->time + 0.032s;
	}

	void CSpearEX::IgniteThink(void)
	{
		Update();

		if (m_iType == 2)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iBeamModelIndex);
			WRITE_BYTE(TraceLife);
			WRITE_BYTE(FullChargingShotTraceWidth);
			WRITE_BYTE(FullChargingShotTraceR);
			WRITE_BYTE(FullChargingShotTraceG);
			WRITE_BYTE(FullChargingShotTraceB);
			WRITE_BYTE(FullChargingShotTraceBrightness);
			MESSAGE_END();
		}
		else if (m_iType == 1)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iBeamModelIndex);
			WRITE_BYTE(TraceLife);
			WRITE_BYTE(ChargingShotTraceWidth);
			WRITE_BYTE(ChargingShotTraceR);
			WRITE_BYTE(ChargingShotTraceG);
			WRITE_BYTE(ChargingShotTraceB);
			WRITE_BYTE(ChargingShotTraceBrightness);
			MESSAGE_END();
		}
		else
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iBeamModelIndex);
			WRITE_BYTE(TraceLife);
			WRITE_BYTE(TraceWidth);
			WRITE_BYTE(TraceColorR);
			WRITE_BYTE(TraceColorG);
			WRITE_BYTE(TraceColorB);
			WRITE_BYTE(TraceBrightness);
			MESSAGE_END();

			//UTIL_MakeVectors(pev->angles);

			//Vector vecSrc, vecEnd, vecEnd1;
			//Vector vecAngleCheckDir = gpGlobals->v_forward.Normalize();
			//float angCheck = cos(AutoAimDegree * M_PI / 180.0);
			//vecSrc = pev->origin;
			//CBaseEntity* pEntity = nullptr;
			//TraceResult tr;
			//float Dis = 4096;
			//while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 4096)) != nullptr)
			//{
			//	if (pEntity->edict() == pev->owner)
			//		continue;

			//	if (pEntity->IsDormant())
			//		continue;

			//	if (pEntity->pev->takedamage == DAMAGE_NO)
			//		continue;

			//	if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
			//		continue;

			//	vecEnd = pEntity->BodyTarget(vecSrc);

			//	vecEnd1 = pEntity->pev->origin;

			//	if (DotProduct((vecEnd - vecSrc).Normalize(), vecAngleCheckDir) < angCheck)
			//		continue;

			//	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

			//	if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
			//		continue;

			//	float flDelta = (vecSrc - vecEnd).Length();

			//	if (flDelta < Dis)
			//	{
			//		m_hTarget.Set(pEntity->edict());
			//		Dis = flDelta;
			//	}

			//}

			/*if (m_hTarget.Get() != NULL)
			{
				MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(m_hTarget->pev->origin.x);
				WRITE_COORD(m_hTarget->pev->origin.y);
				WRITE_COORD(m_hTarget->pev->origin.z + 30);
				WRITE_SHORT(m_iExplodeSprite);
				WRITE_BYTE(5);
				WRITE_BYTE(150);
				WRITE_BYTE(15);
				MESSAGE_END();
			}
		*/

		}

		SetThink(&CSpearEX::FollowThink);

		pev->nextthink = gpGlobals->time + 0.032s;
	}

	void CSpearEX::CrowdCallback(CBaseEntity* entityPushing, CBaseEntity* entityBehind)
	{
		if (m_flPushDuration == UTIL_WeaponTimeBase())
			return;

		if (m_hLockingEntity == entityPushing)
		{
			if ((entityBehind->IsPlayer() || entityBehind->Classify() == CLASS_PLAYER_ALLY) && g_pGameRules->PlayerRelationship(m_pOwner, entityPushing) != GR_TEAMMATE)
			{
				Vector vecDir = UTIL_CalcCrowdForceDir(entityPushing, entityBehind);

				if (vecDir.x || vecDir.y || vecDir.z)
				{
					entityBehind->pev->velocity = vecDir * m_flCrowdForce;
					entityBehind->pev->velocity.z = m_flCrowdBoostVel;


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
					else
						AddPlayerToCrowd((CMonster*)entityBehind);

					if (entityBehind->Classify() == CLASS_PLAYER_ALLY)
					{
						((CMonster*)entityBehind)->m_flNextSpearPush = gpGlobals->time + 0.45s;
					}

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
					Vector vecDir = UTIL_CalcCrowdForceDir(entityPushing, entityBehind);
					vecDir.z = 0;

					if (vecDir.x || vecDir.y || vecDir.z)
					{
						entityBehind->pev->velocity = vecDir * entityPushing->pev->velocity.Length() * 1.01;

						((CBasePlayer*)entityBehind)->m_flNextSpearPush = gpGlobals->time + SPEAR_PLAYER_PUSH_INTERVAL;
					}
				}
			}
		}
		else if (entityPushing->Classify() == CLASS_PLAYER_ALLY && entityBehind->Classify() == CLASS_PLAYER_ALLY && SPEAR_PLAYER_PUSH_INTERVAL != 0s)
		{
			CMonster* pPlayer = static_cast<CMonster*>(entityBehind);

			if ((pPlayer->m_hSpearAttached != nullptr || pPlayer->m_hSpearAttached == this))
			{
				AddPlayerToCrowd(pPlayer);

				if (pPlayer->m_flNextSpearPush > gpGlobals->time && gpGlobals->time > pPlayer->m_flNextSpearPush)
				{
					Vector vecDir = UTIL_CalcCrowdForceDir(entityPushing, entityBehind);
					vecDir.z = 0;

					if (vecDir.x || vecDir.y || vecDir.z)
					{
						entityBehind->pev->velocity = vecDir * entityPushing->pev->velocity.Length() * 1.01;

						((CMonster*)entityBehind)->m_flNextSpearPush = gpGlobals->time + SPEAR_PLAYER_PUSH_INTERVAL;
					}
				}
			}
		}
	}

	void CSpearEX::MaterialSound(void)
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

	void CSpearEX::Precache(void)
	{
		m_iArrowModel = PRECACHE_MODEL("models/speargunex_spear.mdl");
		m_iExplodeModel = PRECACHE_MODEL("models/speargunex_spear2.mdl");
		m_iExplodeSprite = PRECACHE_MODEL("sprites/ef_speargun_explo.spr");


		m_iBeamModelIndex = PRECACHE_MODEL("sprites/smoke.spr");
	}

	void CSpearEX::HitDamage(CBaseEntity* pOther)
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		ClearMultiDamage();

		pOther->TraceAttack(VARS(pev->owner), m_flHitDamage, m_vecDirection, &tr, DMG_BULLET | DMG_NEVERGIB);

		m_LastHitGroup = pOther->m_LastHitGroup;
		m_flHitDamage = gMultiDamage.amount;
	}

	void CSpearEX::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_BBOX;
		pev->gravity = 0.5;
		pev->friction = 0;

		SET_MODEL(ENT(pev), "models/speargunex_spear.mdl");

		pev->mins = pev->maxs = Vector(0.008, 0.008, 0.008);

		UTIL_SetSize(pev, pev->mins, pev->maxs);
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CSpearEX::SpearTouch);
		SetThink(&CSpearEX::IgniteThink);

		pev->nextthink = gpGlobals->time;

		m_flTimeStopPushing = invalid_time_point;
		m_flTimeNextAutoAim = gpGlobals->time;
		m_flPushVelocityModifier = 1.0;
		if (m_iType > 0)
		{
			pev->skin = 1;
		}

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

	void CSpearEX::SpearTouch(CBaseEntity* pOther)
	{
		if (m_hLockingEntity.Get() != NULL)
			return;

		if (m_hTarget.Get() != NULL)
		{
			m_hTarget = NULL;
		}

		m_hEntityHit = pOther;

		TraceResult tr = UTIL_GetGlobalTrace();

		m_LastHitGroup = tr.iHitgroup;

		if (m_iType != 0)
		{
			HitDamage(pOther);

			pev->origin -= m_vecDirection * 5.0;
			Explode(0.0s, true);
			return;
		}

		if (pOther->IsBSPModel())
		{
			MaterialSound();
			//when fisrt hit 0.8s explode
			if (gpGlobals->time + AmmoExpTime <= m_flTimeExplode)
				m_flTimeExplode = gpGlobals->time + AmmoExpTime;

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
					WRITE_COORD(pev->angles[0]);
					WRITE_COORD(pev->angles[1]);
					WRITE_COORD(pev->angles[2]);
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

		if (!pOther->IsPlayer() && pOther->Classify() != CLASS_PLAYER_ALLY)	// !CanAddKickRate(pOther)
		{
			HitDamage(pOther);

			pev->origin -= m_vecDirection * 5.0;
			Explode(0.0s, true);
			return;
		}

		CMonster* pMonster = dynamic_cast<CMonster*>(pOther);
		if (pMonster)
		{
			if (pMonster->m_iClass >= BOSS_CLASS_BIG00)
			{
				HitDamage(pOther);

				pev->origin -= m_vecDirection * 5.0;
				Explode(0.0s, true);
				return;
			}
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

					if (gpGlobals->time + AmmoExpTime <= m_flTimeExplode)
						m_flTimeExplode = gpGlobals->time + AmmoExpTime;

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

		if (pOther->IsAlive() && pOther->Classify() == CLASS_PLAYER_ALLY)
		{
			if (pMonster)
			{
				if (pOther->pev->takedamage != DAMAGE_NO)
				{
					pev->solid = SOLID_NOT;

					if (pMonster->m_hSpearAttached.Get() == NULL)
					{
						m_hLockingEntity = pOther;
						pMonster->m_hSpearAttached = this;

						SetTouch(NULL);

						m_flTimeStopPushing = gpGlobals->time + m_flPushDuration;

						pev->effects |= EF_NODRAW;
						pev->velocity = g_vecZero;

						if (gpGlobals->time + AmmoExpTime <= m_flTimeExplode)
							m_flTimeExplode = gpGlobals->time + AmmoExpTime;

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

	CSpearEX* CSpearEX::Create(int iType)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("spearex"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CSpearEX* pSpear = (CSpearEX*)Instance(pent);
		if (pSpear)
		{
			pSpear->pev->classname = MAKE_STRING("spearex");
			pSpear->m_iType = iType;
			pSpear->Spawn();
		}

		return pSpear;
	}


}
