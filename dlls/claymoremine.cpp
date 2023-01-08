#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"

namespace sv {

	LINK_ENTITY_TO_CLASS(claymore_mine, CClayMoreMine)


		const float CLAYMORE_MINE_EXP_RADIUS = 39.37 * 12.7;
	const float CLAYMORE_MINE_DETECT_RADIUS = 39.37 * 5.08;


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

		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/claymore_exp.wav", VOL_NORM, ATTN_NORM);

		if (!IsManual)
		{
			if (m_pWeapon)
			{
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];
				pWeapon->Respawn();
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


		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, this->pev->origin, g_vecZero, CLAYMORE_MINE_EXP_RADIUS, 0.0, 28, 0, FALSE, FALSE);

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
				if (flRad < 0.6) continue;

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

			float flRad = CalcRadAngle(this, pEntity);
			if (flRad < 0.6)
			{
				m_bSuperDamage = false;
				continue;
			}
			else
				m_bSuperDamage = true;

			float flRange = (this->pev->origin - pEntity->pev->origin).Length();
			Vector vecpEntityOrigin2 = pEntity->pev->origin;;

			if (!Q_strcmp(STRING(pEntity->pev->classname), "func_breakable"))
			{
				vecpEntityOrigin2 = pEntity->pev->origin;
				vecpEntityOrigin2 += (pEntity->pev->mins + pEntity->pev->maxs) * 0.5;
				flRange = (this->pev->origin - vecpEntityOrigin2).Length();
			}

			float flDamage = (float)(GetDamage(IsManual) - ((GetDamage(IsManual) / CLAYMORE_MINE_EXP_RADIUS) * flRange));
			if (m_bSuperDamage)
				flDamage *= 2.0;

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

			if ((pEntity->pev->origin - pev->origin).Length() < CLAYMORE_MINE_EXP_RADIUS / 1.5)
			{
				Vector2D vecDir2D = (pEntity->pev->origin.Make2D() - pev->origin.Make2D()).Normalize();
				pEntity->pev->velocity.x = vecDir2D.x * 1000;
				pEntity->pev->velocity.y = vecDir2D.y * 1000;
				pEntity->pev->velocity.z = 600;
			}
		}
		if (bAlreadyBoom)
		{
			Explode(IsManual);
			return 1;
		}
		return 1;
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
			flDamage = IsManual ? 110.0f : 100.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = IsManual ? 16900.0f : 10900.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = IsManual ? 39400.0f : 24400.0f;
		}
		return flDamage;
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
			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pev);
			WRITE_BYTE(33);
			WRITE_BYTE(m_iType);
			WRITE_SHORT(entindex());
			MESSAGE_END();

			UTIL_TempModel(pev->origin, pev->angles, g_vecZero, m_iLaser, 999, 0, 1.0, false, 0, 255, kRenderTransAdd, this, false, 0, 1.0, 1, 0);
			m_flDelayTrigger = gpGlobals->time + 1.03s;

		}

	};
}
