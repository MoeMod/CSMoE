#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_shared/wpn_bunkerbuster.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {

	LINK_ENTITY_TO_CLASS(bunkerbuster_ent, CBunkerBusterBase)


		void CBunkerBusterBase::OnThink(void)
	{
		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);
		if (!pAttackPlayer)
			return;

		if (!pAttackPlayer->IsAlive())
		{
			SetThink(NULL);
			SetTouch(NULL);
			UTIL_Remove(this);
		}
		if (pAttackPlayer->m_bIsZombie)
		{
			SetThink(NULL);
			SetTouch(NULL);
			UTIL_Remove(this);
			return;
		}

		if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
		{
			SetThink(NULL);
			SetTouch(NULL);
			UTIL_Remove(this);
		}
		if (!IsInWorld())
		{
			SetThink(NULL);
			UTIL_Remove(this);
			return;
		}

		if (m_iType == 1)
		{
			pev->nextthink = gpGlobals->time + BunkerBuster_BombingInterval;

			if (m_flArriveTime != invalid_time_point && m_flArriveTime < gpGlobals->time)
			{
				pev->effects &= ~EF_NODRAW;

				float flRefDistance; float flCurrentDistance; float flDistanceDifference;

				//Aim to target
				Vector m_vecOrigin;
				Vector m_vecAngles;

				m_vecOrigin = m_vecStart;
				m_vecOrigin = m_vecEnd - m_vecOrigin;

				m_vecAngles = UTIL_VecToAngles(m_vecOrigin);
				m_vecAngles.x = m_vecAngles.z = 0;

				//Turn the ent.
				pev->angles = pev->v_angle = m_vecAngles;

				//Arrive? Redirect origin within the map sphere.
				//飞机到达位置后，重新定位地图最大高度.
				Vector vecCurrentOrg;
				vecCurrentOrg = pev->origin;
				vecCurrentOrg.z = GetMaxHeight(vecCurrentOrg);
				pev->origin = vecCurrentOrg;
				m_vecEnd.z = m_vecStart.z = vecCurrentOrg.z;

				m_vecVelocity = UTIL_GetSpeedVector(m_vecStart, m_vecEnd, BunkerBuster_PlaneSpeed);
				pev->velocity = m_vecVelocity;


				if (m_flCanThrowTime != invalid_time_point && m_flCanThrowTime < gpGlobals->time)
				{
					if (m_iMissileCounts)
					{
						m_iMissileCounts--;
						//BunkerBusterMode = 0 ->Pinpoint Bombing
						if (!BunkerBusterMode)
						{
							//定点轰炸：根据飞机已经飞行的距离 & 飞机总飞行距离计算相对距离，每隔20.0u？的距离投放一颗导弹.
							m_vecStart.z = m_vecEnd.z = vecCurrentOrg.z;

							flRefDistance = (m_vecStart - m_vecEnd).Length();
							flCurrentDistance = (m_vecStart - vecCurrentOrg).Length();

							// Get difference as integer.
							flDistanceDifference = abs(round(flRefDistance)) - abs(round(flCurrentDistance));

							float flTargetDis = 0.0f;
							flTargetDis = (m_vecEnd - m_vecStart).Length() / BunkerBusterMissileCounts;

							if (flDistanceDifference >= (/*BunkerBuster_BombingDistanc*/flTargetDis))
							{
								CBunkerBusterBase* pEntMissile = CBunkerBusterBase::CreateBaseEnt(2, vecCurrentOrg, g_vecZero, ENT(pAttackPlayer->pev), pAttackPlayer->m_iTeam);
								if (pEntMissile)
								{
									Vector vecEnd;
									vecEnd = vecCurrentOrg;
									vecEnd.z -= 8192.0;

									Vector vecVelocity;

									//GetSpeedVector(vecCurrentOrg, vecEnd, BunkerBuster_MissileSpeed, vecVelocity);
									vecVelocity = UTIL_GetSpeedVector(vecCurrentOrg, vecEnd, BunkerBuster_MissileSpeed);
									pEntMissile->InitB52Missile(vecVelocity, vecCurrentOrg);

									pEntMissile->m_iTouchCounts = BunkerBuster_MissilePenetration;
									PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntMissile->pev), m_usFireBunkerBuster, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, 1, 0, FALSE, FALSE);

								}
							}
						}
						else
						{
							//非定点轰炸，飞机到达后便开始直线轰炸，直到离开地图.
							CBunkerBusterBase* pEntMissile = CBunkerBusterBase::CreateBaseEnt(2, vecCurrentOrg, g_vecZero, ENT(pAttackPlayer->pev), pAttackPlayer->m_iTeam);
							if (pEntMissile)
							{

								Vector vecEnd;
								vecEnd = vecCurrentOrg;
								vecEnd.z -= 8192.0;

								Vector vecVelocity;

								//GetSpeedVector(vecCurrentOrg, vecEnd, BunkerBuster_MissileSpeed, vecVelocity);
								vecVelocity = UTIL_GetSpeedVector(vecCurrentOrg, vecEnd, BunkerBuster_MissileSpeed);
								pEntMissile->InitB52Missile(vecVelocity, vecCurrentOrg);

								pEntMissile->m_iTouchCounts = BunkerBuster_MissilePenetration;
								PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntMissile->pev), m_usFireBunkerBuster, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, 1, 0, FALSE, FALSE);
							}
						}
					}
				}
			}
		}
		if (m_iType == 2)
		{
			Vector vecCurrentOrg;
			vecCurrentOrg = pev->origin;

			if (!m_bFireEnabled)
			{
				pev->nextthink = gpGlobals->time + 0.1s;

				if (pev->movetype == MOVETYPE_NOCLIP && !m_bNoInit)
				{
					if (abs((vecCurrentOrg.z - pAttackPlayer->pev->origin.z)) <= 500.0)
					{
						pev->movetype = MOVETYPE_TOSS;
					}
				}
				//只要导弹还未落地，持续绘制拖尾.
				//导弹能穿透，但是拖尾不能？OL貌似可以？
				if ((UTIL_PointContents(vecCurrentOrg) == CONTENTS_EMPTY) && (UTIL_PointContents(vecCurrentOrg) != CONTENTS_SKY))
				{
					if (!(pev->flags & FL_ONGROUND))
					{
						MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecCurrentOrg);
						WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
						WRITE_COORD(vecCurrentOrg.x);		// Send to PAS because of the sound
						WRITE_COORD(vecCurrentOrg.y);
						WRITE_COORD(vecCurrentOrg.z);
						WRITE_SHORT(MODEL_INDEX("sprites/bunkerbuster_smoke.spr"));
						WRITE_BYTE(RANDOM_LONG(4, 9));			// scale * 10
						WRITE_BYTE(25);		// framerate
						WRITE_BYTE(TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);	// flags
						MESSAGE_END();
					}
				}
			}
			else
			{
				pev->nextthink = gpGlobals->time + BunkerBuster_FireRate;

				if (m_flFireTime != invalid_time_point && m_flFireTime < gpGlobals->time)
				{
					m_bFireEnabled = false;
					m_flFireTime = invalid_time_point;

					UTIL_Remove(this);
					return;
				}
				vecCurrentOrg = pev->origin;

				MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecCurrentOrg);
				WRITE_BYTE(TE_FIREFIELD);
				WRITE_COORD(vecCurrentOrg.x);
				WRITE_COORD(vecCurrentOrg.y);
				WRITE_COORD(vecCurrentOrg.z + RANDOM_FLOAT(100.0, 125.0));
				WRITE_SHORT(GetRangeFire()); // Radius
				WRITE_SHORT(MODEL_INDEX("sprites/bunkerbuster_fire.spr"));
				WRITE_BYTE(8); // count
				WRITE_BYTE(TEFIRE_FLAG_PLANAR | TEFIRE_FLAG_ADDITIVE);
				WRITE_BYTE(2); // duration
				MESSAGE_END();


				CBaseEntity* pEntity = NULL;
				while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecCurrentOrg, GetRangeFire())) != NULL)
				{
					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pAttackPlayer && pAttackPlayer->IsPlayer())
					{
						if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
							continue;
					}

					pEntity->TakeDamage(pev, VARS(pev->owner), GetDamageFire(), DMG_NEVERGIB | DMG_BURN | DMG_NOMORALE | DMG_EXPLOSION);
				}
			}
		}
	}
	void CBunkerBusterBase::OnTouch(CBaseEntity* other)
	{
		if (m_iType != 2)
			return;

		Vector vecOrigin; Vector vecOriginCopy; Vector vecEnd;
		vecOriginCopy = vecOrigin = pev->origin;
		Vector vecDir = Vector(0, 0, -8192);

		TraceResult tr;
		if ((UTIL_PointContents(vecOrigin) != CONTENTS_WATER))
		{
			//BunkerBuster_MissilePenetration = x 最多穿透x次
			for (int i = 0; i < BunkerBuster_MissilePenetration; i++)
			{
				if (!m_iTouchCounts)
					break;

				//慢慢试探实体下方是不是地面 如果不是就慢慢-z坐标
				for (int j = 0; j < 256; j++)
				{
					vecOriginCopy.z -= 2.0;
					if ((UTIL_PointContents(vecOriginCopy) != CONTENTS_SOLID))
						break;
				};
				vecEnd = vecDir + vecOriginCopy;

				UTIL_TraceLine(vecOriginCopy, vecEnd, ignore_monsters, ENT(0), &tr);

				vecOriginCopy = tr.vecEndPos;

				if (tr.flFraction == 1.0)
					break;

				if ((UTIL_PointContents(vecOriginCopy) == CONTENTS_SOLID))
					break;

				if (!vecOriginCopy.Length())
					vecOriginCopy.z = 1.0;

				Vector vecNewOrigin;
				vecNewOrigin = pev->origin;

				vecNewOrigin.z = vecOriginCopy.z;

				CBunkerBusterBase* pEntMissile2 = CBunkerBusterBase::CreateBaseEnt(2, vecNewOrigin, g_vecZero, pev->owner, CBaseEntity::Instance(pev->owner)->m_iTeam);
				if (pEntMissile2)
				{
					pEntMissile2->m_bNoInit = true;
					pEntMissile2->pev->movetype = MOVETYPE_TOSS;

					Vector vecEnd;
					vecEnd = vecNewOrigin;
					vecEnd.z -= 8192.0;

					Vector vecVelocity;
					vecVelocity = UTIL_GetSpeedVector(vecNewOrigin, vecEnd, BunkerBuster_MissileSpeed);

					pEntMissile2->InitB52Missile(vecVelocity, vecNewOrigin);

					pEntMissile2->CreateRandomFire(vecNewOrigin);
					pEntMissile2->CreateExplosion(vecNewOrigin);

					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntMissile2->pev), m_usFireBunkerBuster, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, 1, 0, FALSE, FALSE);

				}
			}

			CreateExplosion(vecOrigin);
			CreateRandomFire(vecOrigin);


		}
		else
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/black_smoke3.spr"));
			WRITE_BYTE(RANDOM_LONG(40, 60)); // scale * 10
			WRITE_BYTE(RANDOM_LONG(10, 20)); // framerate
			MESSAGE_END();

			pev->effects |= EF_NODRAW; // 0x80u
			SetThink(NULL);
			SetTouch(NULL);
			UTIL_Remove(this);

			return;
		}


		pev->effects |= EF_NODRAW; // 0x80u
		pev->movetype = MOVETYPE_NONE;
		pev->solid = SOLID_NOT;
		SetTouch(NULL);

		return;

	}
	void CBunkerBusterBase::CreateRandomFire(Vector vecOrigin)
	{
		m_bFireEnabled = true;
		m_flFireTime = gpGlobals->time + BunkerBuster_FireTime;

		/*
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z + 100.0);
		WRITE_SHORT(MODEL_INDEX("sprites/bunkerbuster_fire.spr"));
		WRITE_BYTE(10);
		WRITE_BYTE(255);
		WRITE_BYTE(15);
		MESSAGE_END();
		*/




	}
	void CBunkerBusterBase::CreateExplosion(Vector vecOrigin)
	{
		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pOwnerPlayer = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pOwnerPlayer = static_cast<CBasePlayer*>(pOwner);

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z + 230.0);
		WRITE_SHORT(MODEL_INDEX("sprites/bunkerbuster_explosion.spr"));
		WRITE_BYTE(25);
		WRITE_BYTE(255);
		WRITE_BYTE(25);
		MESSAGE_END();

		if (!pOwnerPlayer)
			return;

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireBunkerBuster, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, 2, 0, FALSE, FALSE);
		//SetThink(&CBunkerBusterBase::SoundAfterThink);
		//pev->nextthink = gpGlobals->time + 2.0s;

		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, GetRangeMissile())) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pOwnerPlayer && pOwnerPlayer->IsPlayer())
			{
				if (pEntity != pOwnerPlayer)
				{
					if (g_pGameRules->PlayerRelationship(pOwnerPlayer, pEntity) == GR_TEAMMATE)
						continue;
				}

			}

			pEntity->TakeDamage(pev, VARS(pev->owner), GetDamageMissile(), DMG_NEVERGIB | DMG_EXPLOSION);
		}


		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CBaseEntity* player = UTIL_PlayerByIndex(i);
			if (player)
			{
				if (player->IsPlayer() && player->IsAlive())
				{
					if ((player->pev->origin - pev->origin).Length() <= GetRangeMissile() * 2.5)
					{
						MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, player->edict());
						WRITE_SHORT((3 << 12) * 10);
						WRITE_SHORT((2 << 14));
						WRITE_SHORT((3 << 12) * 10);
						MESSAGE_END();
					}

				}
			}

		}




	}
	void CBunkerBusterBase::SoundAfterThink(void)
	{
		pev->nextthink = gpGlobals->time + (!m_iSoundCounts ? 0.1s : 5.0s);

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireBunkerBuster, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, 2, 1, FALSE, FALSE);
		m_iSoundCounts++;
		if (m_iSoundCounts >= 2)
			SetThink(NULL);


	}

	CBunkerBusterBase* CBunkerBusterBase::CreateBaseEnt(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("bunkerbuster_ent"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CBunkerBusterBase* pEnt = (CBunkerBusterBase*)Instance(pent);

		if (pEnt)
		{
			pEnt->m_iType = iType;
			pEnt->pev->origin = vecOrigin;
			pEnt->pev->angles = vecAngles;
			pEnt->pev->owner = pentOwner;
			pEnt->m_iTeam = iTeam;

			pEnt->Spawn();

		}


		return pEnt;
	}

	void CBunkerBusterBase::InitB52(Vector vecVelocity, Vector vecStart, Vector vecEnd)
	{

		m_vecStart = vecStart;
		m_vecEnd = vecEnd;
		m_vecVelocity = vecVelocity;

		//Extend the vector.
		Vector vecDir;

		//VectorSubtract(vecStart, vecEnd, vecDir);
		//VectorNormalize(vecDir);
		vecDir = m_vecEnd - m_vecStart;

		if (!vecDir.Length())
		{
			CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pOwnerPlayer = nullptr;
			if (pOwner && pOwner->IsPlayer())
				pOwnerPlayer = static_cast<CBasePlayer*>(pOwner);
			if (pOwnerPlayer)
			{
				Vector vecMorigin;
				vecMorigin = pOwnerPlayer->pev->origin;
				vecDir = m_vecStart - vecMorigin;

				if (!vecDir.Length())
				{
					vecDir.x = RANDOM_FLOAT(-1.0, 1.0);
					vecDir.y = RANDOM_FLOAT(-1.0, 1.0);
				}
			}
		}
		vecDir = vecDir.Normalize();


		Vector vecTemp;
		Vector vecScale;
		vecTemp = vecDir;
		VectorScale(vecTemp, -BunkerBuster_PlaneOffSet, vecScale);
		VectorAdd(m_vecStart, vecScale, m_vecStart);
		//vecScale = vecTemp * BunkerBuster_PlaneOffSet;
		//m_vecStart = m_vecStart + vecScale;

		//SetOrigin

		Vector vecCurrentOrg;
		vecCurrentOrg = m_vecStart;
		vecCurrentOrg.z = GetMaxHeight(vecCurrentOrg);
		m_vecEnd.z = m_vecStart.z = vecCurrentOrg.z;

		pev->origin = vecCurrentOrg;


		//Aim to target
		Vector m_vecOrigin;
		Vector m_vecAngles;

		m_vecOrigin = m_vecStart;
		m_vecOrigin = m_vecEnd - m_vecOrigin;

		m_vecAngles = UTIL_VecToAngles(m_vecOrigin);
		m_vecAngles.x = m_vecAngles.z = 0;

		//Turn the ent.
		pev->angles = pev->v_angle = m_vecAngles;
		//pev->velocity = m_vecVelocity;

		pev->effects = EF_NODRAW; // 0x80u

		SetThink(&CBunkerBusterBase::OnThink);
		pev->nextthink = gpGlobals->time + 0.1s;


	}

	void CBunkerBusterBase::InitB52Missile(Vector vecVelocity, Vector vecStart)
	{
		if (!m_bNoInit)
		{
			vecStart.z = GetMaxHeight(vecStart);
			vecStart.z -= 150.0;
		}
		else
		{
			vecStart.z -= 20.0;
		}

		pev->origin = vecStart;
		pev->velocity = vecVelocity;

		if (!m_bNoInit)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(MODEL_INDEX("sprites/smoke.spr"));
			WRITE_BYTE(7);
			WRITE_BYTE(5);
			WRITE_BYTE(224);
			WRITE_BYTE(224);
			WRITE_BYTE(255);
			WRITE_BYTE(190);
			MESSAGE_END();
		}


		SetThink(&CBunkerBusterBase::OnThink);
		SetTouch(&CBunkerBusterBase::OnTouch);
		pev->nextthink = gpGlobals->time + (!m_bNoInit ? 0.1s : 0.05s);


	}


	float CBunkerBusterBase::GetDamageMissile(void)
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 20000;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 50000;
#endif
		return 125.0;
	}
	float CBunkerBusterBase::GetDamageFire(void)
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 500;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 1500;
#endif
		return 5.0;
	}
	float CBunkerBusterBase::GetRangeFire(void)
	{
		return 200.0f;
	}
	float CBunkerBusterBase::GetRangeMissile(void)
	{
		return 300.0f;
	}

	void CBunkerBusterBase::GetSpeedVector(Vector vecStart, Vector vecEnd, float flSpeed, Vector vecVelocity)
	{
		vecVelocity = vecEnd - vecStart;
		float SpeedRate = sqrt(flSpeed * flSpeed / (vecVelocity.x * vecVelocity.x + vecVelocity.y * vecVelocity.y + vecVelocity.z * vecVelocity.z));
		vecVelocity *= SpeedRate;

	}

	float CBunkerBusterBase::GetMaxHeight(Vector vecReturn)
	{

		/*const*/ int pcCurrent = 0;
		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

		//CONTENTS_EMPTY
		while (UTIL_PointContents(vecReturn) == CONTENTS_EMPTY)
		{
			vecReturn.z += 5.0;
		}

		pcCurrent = UTIL_PointContents(vecReturn);

		auto bModiCheck = false;
		if (pAttackPlayer)
		{
			float flPlayerZ = (pAttackPlayer->pev->origin.z);

			if (abs(flPlayerZ - ((pcCurrent == CONTENTS_SKY) ? 
				vecReturn.z - 100.0 : vecReturn.z)) < 200.0)
			{
				bModiCheck = true;
				return flPlayerZ + 150.0;
			}
		}

		return ((pcCurrent == CONTENTS_SKY)) ? 
			vecReturn.z - (bModiCheck ? 300.0 : 150.0) : vecReturn.z;
	}




	void CBunkerBusterBase::Precache(void)
	{
		PRECACHE_MODEL("models/bunkerbuster_missile.mdl");
		PRECACHE_MODEL("models/b52.mdl");

		PRECACHE_MODEL("sprites/bunkerbuster_smoke.spr");
		PRECACHE_MODEL("sprites/bunkerbuster_fire.spr");
		PRECACHE_MODEL("sprites/bunkerbuster_explosion.spr");
		PRECACHE_MODEL("sprites/black_smoke3.spr");



		PRECACHE_SOUND("weapons/bunkerbuster_whistling1.wav");
		PRECACHE_SOUND("weapons/bunkerbuster_whistling2.wav");
		PRECACHE_SOUND("weapons/bunkerbuster_whistling3.wav");

		PRECACHE_SOUND("weapons/bunkerbuster_target_siren.wav");
		PRECACHE_SOUND("weapons/bunkerbuster_fly.wav");
		PRECACHE_SOUND("weapons/bunkerbuster_fire.wav");
		PRECACHE_SOUND("weapons/bunkerbuster_explosion_1st.wav");
		PRECACHE_SOUND("weapons/bunkerbuster_explosion_after_1st.wav");


		m_usFireBunkerBuster = PRECACHE_EVENT(1, "events/bunkerbuster.sc");
	}

	void CBunkerBusterBase::Spawn(void)
	{

		Precache();
		pev->classname = MAKE_STRING("bunkerbuster_ent");

		if (m_iType == 1)
		{
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_NOCLIP;

			SET_MODEL(ENT(pev), "models/b52.mdl");

			m_flArriveTime = gpGlobals->time + 5.0s;
			m_flFlySoundTime = gpGlobals->time + 2.0s;
			m_flCanThrowTime = gpGlobals->time + 5.0s + ((BunkerBuster_PlaneOffSet / BunkerBuster_PlaneSpeed) * 1s) + 0.1s;//512 /800 + nexthink
			m_flRemoveTime = gpGlobals->time + BunkerBuster_PlaneRemoveTime;
			m_iMissileCounts = BunkerBusterMissileCounts;

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireBunkerBuster, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, 0, 0, FALSE, FALSE);

		}
		if (m_iType == 2)
		{
			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_NOCLIP;

			pev->takedamage = DAMAGE_NO;

			SET_MODEL(ENT(pev), "models/bunkerbuster_missile.mdl");

			Vector vecAngles;
			vecAngles.x -= 90;
			pev->angles = vecAngles;

			m_flRemoveTime = gpGlobals->time + BunkerBuster_MissileRemoveTime;

		}
		else if (m_iType == 3)
		{

		}
	}

}
