#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"

#include "wpn_shared/wpn_claymore.h"

namespace sv {

	LINK_ENTITY_TO_CLASS(claymore_mine, CClayMoreMine)


		const float CLAYMORE_MINE_EXP_RADIUS = 39.37 * 12.7;
	const float CLAYMORE_MINE_DETECT_RADIUS = 39.37 * 5.08;
	const float CLAYMORE_MINE_BACK_EXP_RADIUS = 39.37 * 2.54;
	const float CLAYMORE_MINE_SIDE_EXP_RADIUS = 39.37 * 10.16;


	CClayMoreMine* CClayMoreMine::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("claymore_mine"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CClayMoreMine* pMine = (CClayMoreMine*)Instance(pent);

		if (pMine)
		{
			pMine->m_iType = iType;
			pMine->pev->owner = pentOwner;
			pMine->pev->origin = vecOrigin;
			pMine->pev->angles = vecAngles;
			pMine->Spawn();
		}

		return pMine;
	}

	void CClayMoreMine::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), m_iType ? "models/w_claymore.mdl" : "models/w_claymore.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->gravity = 1.0;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_TOSS;
		pev->frame = 0;
		pev->health = 800.0;
		pev->takedamage = DAMAGE_YES;

		m_iState = 0;

		pev->classname = MAKE_STRING("claymore_mine");
		pev->nextthink = gpGlobals->time + 0.01s;
		SetThink(&CClayMoreMine::MineThink);
	}

	void CClayMoreMine::Init(CBasePlayer* pOwner, CClayMore* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;
	}

	void CClayMoreMine::Precache(void)
	{
		m_iLaser = PRECACHE_MODEL("models/w_claymore_laser.mdl");
		m_iGibs[0] = PRECACHE_MODEL("models/gibs_greencrate.mdl");
		m_iGibs[1] = PRECACHE_MODEL("models/gibs3.mdl");


		PRECACHE_MODEL("sprites/claymore_explosion1.spr");
		PRECACHE_MODEL("sprites/claymore_explosion2.spr");
		PRECACHE_MODEL("sprites/claymore_explosion3.spr");
	}

	void CClayMoreMine::Remove()
	{

		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
		WRITE_SHORT(entindex());
		MESSAGE_END();

		SetThink(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}

	void CClayMoreMine::Explode(bool IsManual)
	{

		if (!IsManual)
		{
			if (m_pWeapon)
			{
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];
				if (pWeapon->m_iId == WEAPON_CLAYMORE)
				{
					//pWeapon->RetireWeapon();

					CClayMore* pLinkWeapon = dynamic_cast<CClayMore*>(pWeapon);
					if (pLinkWeapon)
					{
						pLinkWeapon->m_bAlreadySetUp = false;
						pLinkWeapon->Holster(false);
						pLinkWeapon->RetireWeapon();
					}
				}
			}
		}

		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_WORLDDECAL);	// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_BYTE(DECAL_INDEX("{scorch1"));
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_DLIGHT);	// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_BYTE(5);
		WRITE_BYTE(255);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		WRITE_BYTE(2);
		WRITE_BYTE(0);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 250.0f);
		WRITE_SHORT(MODEL_INDEX("sprites/claymore_explosion3.spr"));
		WRITE_BYTE(3);			// scale * 10
		WRITE_BYTE(25);		// framerate
		WRITE_BYTE(TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);	// flags
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 230.0);
		WRITE_SHORT(MODEL_INDEX("sprites/bunkerbuster_smoke.spr"));
		WRITE_BYTE(10);
		WRITE_BYTE(255);
		WRITE_BYTE(25);
		MESSAGE_END();

		Vector vecVelocity;
		Vector vecGibOrg;

		vecGibOrg.x = pev->absmin.x + pev->size.x * (RANDOM_FLOAT(0, 1));
		vecGibOrg.y = pev->absmin.y + pev->size.y * (RANDOM_FLOAT(0, 1));
		// absmin.z is in the floor because the engine subtracts 1 to enlarge the box
		vecGibOrg.z = pev->absmin.z + pev->size.z * (RANDOM_FLOAT(0, 1)) + 1;

		// make the gib fly away from the attack vector
		vecVelocity = pev->angles * -1;

		// mix in some noise
		vecVelocity.x += RANDOM_FLOAT(-0.25, 0.25);
		vecVelocity.y += RANDOM_FLOAT(-0.25, 0.25);
		vecVelocity.z += RANDOM_FLOAT(-0.25, 0.25);

		vecVelocity = vecVelocity * RANDOM_FLOAT(50, 80);

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BREAKMODEL);
		WRITE_COORD(pev->origin.x);		// position
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(pev->size.x);	// size
		WRITE_COORD(pev->size.y);
		WRITE_COORD(pev->size.z);
		WRITE_COORD(vecVelocity.x += RANDOM_FLOAT(-0.25, 0.25));	// velocity
		WRITE_COORD(vecVelocity.y += RANDOM_FLOAT(-0.25, 0.25));
		WRITE_COORD(vecVelocity.z += RANDOM_FLOAT(-0.25, 0.25));
		WRITE_BYTE(10);			// randomization
		WRITE_SHORT(m_iGibs[1]);		// model id#
		WRITE_BYTE(10);			// # of shards, let client decide
		WRITE_BYTE(15);			// duration, 2.5 seconds
		WRITE_BYTE(BREAK_CONCRETE);		// flags
		MESSAGE_END();

		vecVelocity = { RANDOM_FLOAT(-80.0, 80.0) * 10, RANDOM_FLOAT(-60.0, 60.0) * 10, RANDOM_FLOAT(20.0, 50.0) * 5 };

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BREAKMODEL);
		WRITE_COORD(pev->origin.x);		// position
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(pev->size.x);	// size
		WRITE_COORD(pev->size.y);
		WRITE_COORD(pev->size.z);
		WRITE_COORD(vecVelocity.x);	// velocity
		WRITE_COORD(vecVelocity.y);
		WRITE_COORD(vecVelocity.z);
		WRITE_BYTE(20);			// randomization
		WRITE_SHORT(m_iGibs[0]);		// model id#
		WRITE_BYTE(10);			// # of shards, let client decide
		WRITE_BYTE(25);			// duration, 2.5 seconds
		WRITE_BYTE(BREAK_METAL);		// flags
		MESSAGE_END();

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, this->pev->origin, g_vecZero, CLAYMORE_MINE_EXP_RADIUS, 0.0, 28, 0, FALSE, FALSE);
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/claymore_exp.wav", VOL_NORM, ATTN_NORM);

		Remove();
	}
	int CClayMoreMine::CheckCanBoom(bool IsManual, bool IsDetection)
	{
		//lambda->GetRadAngle
		auto CalcRadAngle = [](CBaseEntity* pClayMore, CBaseEntity* pEntity)
		{
			Vector vecClayMoreOrigin, vecpEntityOrigin;
			if (pClayMore->pev->solid == SOLID_BSP)
			{
				Vector vecMins = pClayMore->pev->mins, vecMaxs = pClayMore->pev->maxs;
				vecClayMoreOrigin = (vecMaxs - vecMins) / 2 + vecMins;
			}
			else
				vecClayMoreOrigin = pClayMore->pev->origin;

			if (pEntity->pev->solid == SOLID_BSP)
			{
				Vector vecMins = pEntity->pev->mins, vecMaxs = pEntity->pev->maxs;
				vecpEntityOrigin = (vecMaxs - vecMins) / 2 + vecMins;
			}
			else
				vecpEntityOrigin = pEntity->pev->origin;

			Vector vecClayMoreAngles = pClayMore->pev->angles;

			vec2_t vecOriginCross;

			for (int i = 0; i < 2; i++)
				vecOriginCross[i] = vecpEntityOrigin[i] - vecClayMoreOrigin[i];

			float flLength2d = sqrt(vecOriginCross[0] * vecOriginCross[0] + vecOriginCross[1] * vecOriginCross[1]);
			if (flLength2d < 0.0)
				vecOriginCross = { 0.0,0.0 };
			else
				vecOriginCross = vecOriginCross * (1.0 / flLength2d);

			UTIL_MakeVectors(vecClayMoreAngles);
			Vector vecForward = gpGlobals->v_forward;

			float flRad = vecOriginCross[0] * vecForward[0] + vecOriginCross[1] * vecForward[1];

			return flRad;
		};

		if (IsDetection)
		{
			//DetectionLaser
			Vector pOrigin = pev->origin;
			int iIdDetected = 0;

			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, CLAYMORE_MINE_DETECT_RADIUS)) != NULL)
			{
				if (pEntity->IsDormant())
					continue;

				if (this->pev->classname == pEntity->pev->classname)
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				//队友不触发检测
				if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
					continue;

				//墙体/玻璃不能触发检测
				if (!Q_strcmp(STRING(pEntity->pev->classname), "func_breakable"))
					continue;


				float flRad = CalcRadAngle(this, pEntity);
				if (flRad < (1 - (0.35 - 0.075))) continue;

				iIdDetected = pEntity->entindex();
			}

			return iIdDetected;
		}

		CBaseEntity* pEntity = NULL;
		bool bAlreadyBoom = true;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, CLAYMORE_MINE_EXP_RADIUS)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (this->pev->classname == pEntity->pev->classname)
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE && m_pOwner != pEntity)
				continue;

			if (UTIL_IsWallBetweenEntity(pEntity, this))
				continue;

			//弧度角计算
			float flRad = CalcRadAngle(this, pEntity);
			if (flRad < 0.65 + 0.075)
			{
				//玩家处于阔剑地雷的非正面范围,默认受到低伤害...
				m_iDamageType = 4;

				if (flRad > 0.5 + 0.075 / 2 && flRad < 0.65 + 0.075 / 2)
				{
					//玩家处于阔剑地雷的非正面范围 -> 侧面范围，玩家受到高伤害，但是应略低于玩家处于正面范围时的伤害...
					if (CLAYMORE_MINE_SIDE_EXP_RADIUS < (this->pev->origin - pEntity->pev->origin).Length())
						continue;

					//UTIL_ClientPrintAll(HUD_PRINTTALK, "CurrentPos-> Side Sphere");
					m_iDamageType = 2;
				}
				else if (flRad < 0.5)
				{
					if (flRad > -0.15 && flRad < 0 || (flRad > 0 && flRad < 0.15))
					{
						//玩家处于阔剑地雷的非正面范围 -> 侧面范围，玩家受到高伤害，但是应略低于玩家处于正面范围时的伤害...
						if (CLAYMORE_MINE_SIDE_EXP_RADIUS < (this->pev->origin - pEntity->pev->origin).Length())
							continue;

						//UTIL_ClientPrintAll(HUD_PRINTTALK, "CurrentPos-> Side Sphere ");
						m_iDamageType = 2;
					}
					else
					{
						//玩家处于阔剑地雷的非正面范围 -> 背面范围 ，如果玩家在背面范围 并且 距离阔剑地雷的距离 > 安全距离,不造成伤害... 否则受到低伤害...
						if (CLAYMORE_MINE_BACK_EXP_RADIUS < (this->pev->origin - pEntity->pev->origin).Length())
							continue;

						//UTIL_ClientPrintAll(HUD_PRINTTALK, "CurrentPos-> Back Sphere");
						m_iDamageType = 4;
					}

				}
			}
			else
			{
				//玩家处于阔剑地雷的正面范围，玩家受到最高伤害，但是随距离递减
				if (CLAYMORE_MINE_EXP_RADIUS < (this->pev->origin - pEntity->pev->origin).Length())
					continue;

				//UTIL_ClientPrintAll(HUD_PRINTTALK, "Front Sphere");

				m_iDamageType = 1;
			}

			float flRange = (this->pev->origin - pEntity->pev->origin).Length();
			Vector vecpEntityOrigin2 = pEntity->pev->origin;;

			if (!Q_strcmp(STRING(pEntity->pev->classname), "func_breakable"))
			{
				vecpEntityOrigin2 = pEntity->pev->origin;
				vecpEntityOrigin2 += (pEntity->pev->mins + pEntity->pev->maxs) * 0.5;
				flRange = (this->pev->origin - vecpEntityOrigin2).Length();
			}

			float flDamage = (float)(GetDamage(IsManual) - ((GetDamage(IsManual) / CLAYMORE_MINE_EXP_RADIUS) * flRange));
			if (m_iDamageType)
				flDamage /= m_iDamageType;

			if (flDamage)
				pEntity->TakeDamage(pev, m_pOwner->pev, flDamage, DMG_NEVERGIB | DMG_EXPLOSION | DMG_NOMORALE);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(3.5s);
						strategy->SetStunGravityTime(3.5s);
					}
				}
			}

			if ((pEntity->pev->origin - pev->origin).Length() < (m_iDamageType == 1 ? CLAYMORE_MINE_EXP_RADIUS : (m_iDamageType == 2 ? CLAYMORE_MINE_SIDE_EXP_RADIUS : CLAYMORE_MINE_BACK_EXP_RADIUS)))
			{
				Vector2D vecDir2D = (pEntity->pev->origin.Make2D() - pev->origin.Make2D()).Normalize();
				pEntity->pev->velocity.x = vecDir2D.x * (m_iDamageType == 1 ? 2500 : (m_iDamageType == 2 ? 1700 : 500));
				pEntity->pev->velocity.y = vecDir2D.y * (m_iDamageType == 1 ? 2500 : (m_iDamageType == 2 ? 1700 : 500));
				pEntity->pev->velocity.z = 150 * 2.0;
			}

			MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pEntity->pev);
			WRITE_SHORT((3 << 12) * 5);
			WRITE_SHORT((2 << 14));
			WRITE_SHORT((3 << 12) * 5);
			MESSAGE_END();
		}
		if (bAlreadyBoom)
		{
			Explode(IsManual);
			return 1;
		}
		return 1;
	}

	void EXPORT CClayMoreMine::MineThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.01s;

		if (!m_pOwner)
		{
			Remove();
			return;
		}

		if (m_pOwner->m_bIsZombie || !m_pOwner->IsAlive())
		{
			Remove();
			return;
		}

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];

		if (!pWeapon)
		{
			Remove();
			return;
		}

		if (pWeapon->m_iId != GetWeaponsId())
		{
			Remove();
			return;
		}

		if (pev->flags & FL_ONGROUND)
		{
			if (!m_iState)
				m_iState = 1;

			if (m_iState == 1)
			{
				if (m_bLaserMode == true)
				{
					if (m_flDelayTrigger != invalid_time_point && m_flDelayTrigger < gpGlobals->time)
					{
						int iEnemyDetected = CheckCanBoom(false, true);
						if (iEnemyDetected)
						{
							CBaseEntity* pEnemy = UTIL_PlayerByIndex(iEnemyDetected);
							if (pEnemy && pEnemy->IsPlayer())
							{
								CBasePlayer* pEnemyPlayer = (CBasePlayer*)pEnemy;
								if (pEnemyPlayer && pEnemyPlayer->IsAlive())
								{
									CheckCanBoom(false, false);
									return;
								}
							}
						}
					}
				}
			}
		}
	}
	void CClayMoreMine::ChangeMode(CBaseEntity* pMine, bool State)
	{
		m_bLaserMode = State;
		if (!m_bLaserMode)
		{
			MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
			WRITE_SHORT(entindex());
			MESSAGE_END();

			m_flDelayTrigger = invalid_time_point;
		}
		else
		{
			UTIL_TempModel(Vector(0, 0, 0), pev->angles, g_vecZero, m_iLaser, 255, 0, 10, 0, 0, 255, kRenderTransAdd, this, false, 0, 0, 256, 0);
			m_flDelayTrigger = gpGlobals->time + 0.8s;
		}

	}
	int CClayMoreMine::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
	{
		auto pAttack = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pevAttacker));

		if (!pAttack)
			return 0;

		if (pAttack->IsPlayer() && pAttack->m_bIsZombie)
		{

			if (bitsDamageType & DMG_BULLET || bitsDamageType & DMG_EXPLOSION)
				flDamage *= 2.0;
			else
				return 0;
		}

		if (pAttack->IsPlayer() && !pAttack->m_bIsZombie)
		{
			if (g_pGameRules->PlayerRelationship(m_pOwner, pAttack) == GR_TEAMMATE)
				return 0;
			if (bitsDamageType & DMG_EXPLOSION)
				flDamage *= 2.0;
			else
				return 0;

		}
		return 0;

		return CBaseEntity::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}
	void CClayMoreMine::Killed(entvars_t* pevAttacker, int iGib)
	{
		if (m_pWeapon)
		{
			CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];
			if (pWeapon->m_iId == WEAPON_CLAYMORE)
			{
				CClayMore* pLinkWeapon = dynamic_cast<CClayMore*>(pWeapon);
				if (pLinkWeapon)
				{
					pLinkWeapon->m_bAlreadySetUp = false;
					pLinkWeapon->Holster(false);
					pLinkWeapon->RetireWeapon();
				}
			}
		}

		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_WORLDDECAL);	// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_BYTE(DECAL_INDEX("{scorch1"));
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SPARKS);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 10.0f);
		MESSAGE_END();

		RadiusDamage(pev->origin, this->pev, m_pOwner->pev, GetDamage(false) / 4.0, CLAYMORE_MINE_EXP_RADIUS / 2.0, CLASS_NONE, DMG_NEVERGIB | DMG_EXPLOSION | DMG_NOMORALE);


		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		Remove();
	}

	float CClayMoreMine::GetDamage(bool IsManual) const
	{
		float flDamage = 1.0f;
		if (m_iType)
		{
			flDamage = IsManual ? 110.0f : 100.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = IsManual ? 29200.0f : 19200.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = IsManual ? 40000.0f : 25000.0f;
		}
		else
		{
			flDamage = IsManual ? 500.0f : 400.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = IsManual ? 8400.0f : 8000.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = IsManual ? 12900.0f : 12000.0f;
		}
		return flDamage;
	}
}
