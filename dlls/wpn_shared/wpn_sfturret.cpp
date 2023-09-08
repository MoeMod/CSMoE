#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_sfturret.h"

#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"

#ifndef CLIENT_DLL
#include "player/player_mod_strategy.h"
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_sfturret, CSfTurret)

		void CSfTurret::Spawn(void)
	{
		m_iGrenadeID = GRENADE_SFTURRET;
		Precache();
		m_iId = WEAPON_HEGRENADE;
		SET_MODEL(ENT(pev), "models/w_sfturret.mdl");

		m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE * 3;
		m_flStartThrow = invalid_time_point;
		m_flReleaseThrow = invalid_time_point;

		FallInit();
	}
	void CSfTurret::Precache(void)
	{
		PRECACHE_MODEL("models/v_sfturret.mdl");
		PRECACHE_MODEL("models/p_sfturret.mdl");
		PRECACHE_MODEL("models/w_sfturret.mdl");

		for (int i = 292; i < 324; i++)
		{
			char szString[64];
			sprintf(szString, "sprites/muzzleflash%d.spr", i);
			PRECACHE_MODEL(szString);
		}
		PRECACHE_SOUND("weapons/sfturret_idle_pullpin_fire.wav");
		PRECACHE_SOUND("weapons/sfturret_idle_pullpin_spark.wav");
		PRECACHE_SOUND("weapons/sfturret_mode_change.wav");
		PRECACHE_SOUND("weapons/sfturret_mode_change_FX.wav");
	}

	int CSfTurret::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "HEGrenade";
		p->iMaxAmmo1 = MAX_AMMO_HEGRENADE * 3;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 3;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_HEGRENADE;
		p->iWeight = HEGRENADE_WEIGHT;
		p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

		return 1;
	}
	BOOL CSfTurret::Deploy(void)
	{
		SetThink(nullptr);

		m_fMaxSpeed = 260;

		m_flStartThrow = invalid_time_point;
		m_flReleaseThrow = invalid_time_point;



		return DefaultDeploy("models/v_sfturret.mdl", "models/p_sfturret.mdl", SFTURRET_DRAW, "grenade", UseDecrement() != FALSE, 0.75s, 2.0s);
	}
	BOOL CSfTurret::CanHolster(void)
	{
		return CBasePlayerWeapon::CanHolster();
	}
	void CSfTurret::Holster(int skiplocal)
	{
		SetThink(nullptr);

		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_pPlayer->RemoveWeapon(m_iId);
			DestroyItem();
		}


		m_flStartThrow = invalid_time_point;
		m_flReleaseThrow = invalid_time_point;
	}
	void CSfTurret::PrimaryAttack(void)
	{
		if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		{
			m_flStartThrow = gpGlobals->time;
			m_flReleaseThrow = invalid_time_point;
			SendWeaponAnim(SFTURRET_PULLPIN, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (duration_t)(11 / 30.0);

			SetThink(&CSfTurret::AnimThink);
			pev->nextthink = gpGlobals->time + (duration_t)(11 / 30.0);
		}
	}
	void EXPORT CSfTurret::AnimThink(void)
	{
		pev->nextthink = gpGlobals->time + (duration_t)(10 / 30.0);

		if (m_flStartThrow != invalid_time_point)
			SendWeaponAnim(SFTURRET_IDLE_PULLPIN, UseDecrement() != FALSE);
	}
	void CSfTurret::SecondaryAttack(void)
	{
		if (!m_iState)
		{
			m_iState = 1;
			SendWeaponAnim(SFTURRET_MODECHANGE_LIGHTENING, UseDecrement() != FALSE);
		}
		else
		{
			m_iState = 0;
			SendWeaponAnim(SFTURRET_MODECHANGE_FIRE, UseDecrement() != FALSE);
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (duration_t)(31 / 30.0);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (duration_t)(31 / 30.0);

		SetThink(&CSfTurret::ChangeState);
		pev->nextthink = gpGlobals->time + (duration_t)(31 / 30.0);
	}
	void CSfTurret::ChangeState()
	{
		pev->skin = m_iState ? 1 : 0;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + (duration_t)(15 / 30.0);

		SetThink(nullptr);
	}
	void CSfTurret::WeaponIdle(void)
	{
		if (m_flReleaseThrow == invalid_time_point && m_flStartThrow != invalid_time_point)
			m_flReleaseThrow = gpGlobals->time;

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		SetThink(nullptr);

		if (m_flStartThrow != invalid_time_point)
		{

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
			SendWeaponAnim(SFTURRET_THROW, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1); //5
#endif

			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL

			m_pMine = CSfTurretNade::Create(m_iState, vecSrc, UTIL_VecToAngles(gpGlobals->v_forward), ENT(m_pPlayer->pev));

			if (m_pMine)
			{
				m_pMine->Init(m_pPlayer, this);
				m_pMine->pev->velocity = gpGlobals->v_forward * 1050;
			}
#endif

			m_flStartThrow = invalid_time_point;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] - 1 >= 0)
				--m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
				Deploy();
			else
				RetireWeapon();

			return;
		}
		else if (m_flReleaseThrow != invalid_time_point)
		{
			m_flStartThrow = invalid_time_point;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				m_flReleaseThrow = invalid_time_point;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
			}
			else
				RetireWeapon();

			return;
		}

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(m_iState ? SFTURRET_IDLE_B : SFTURRET_IDLE_A, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
	}
	BOOL CSfTurret::CanDeploy(void)
	{
		return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
	}
#ifndef CLIENT_DLL
	LINK_ENTITY_TO_CLASS(turret_sfturret, CSfTurretNade);

	CSfTurretNade* CSfTurretNade::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("turret_sfturret"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CSfTurretNade* pMine = (CSfTurretNade*)Instance(pent);

		if (pMine)
		{
			pMine->pev->owner = pentOwner;
			pMine->pev->origin = vecOrigin;
			pMine->pev->angles = vecAngles;

			pMine->SetState(iType);
			pMine->Spawn();
		}

		return pMine;
	}
	void CSfTurretNade::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), "models/w_sfturret.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		m_iGrenadeID = GRENADE_SFTURRET;
		pev->angles = g_vecZero;
		pev->gravity = 2.5;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_TOSS;
		pev->frame = 0;

		pev->skin = m_iState ? 1 : 0;

		m_flNextAnim = invalid_time_point;
		m_flDelayExplosion = invalid_time_point;
		m_flReadyExplosion = invalid_time_point;

		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		pev->classname = MAKE_STRING("turret_sfturret");
		pev->nextthink = gpGlobals->time + 0.01s;


		SetThink(&CSfTurretNade::TurretThink);
		SetTouch(&CSfTurretNade::TurretTouch);

		m_flSpawnTime = gpGlobals->time;
	}

	void CSfTurretNade::Init(CBasePlayer* pOwner, CSfTurret* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;

		std::vector < std::pair<int, time_point_t >> vecEnt;
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityByClassname(pEntity, "turret_sfturret")) != NULL)
		{
			CGrenade* grenade = dynamic_ent_cast<CGrenade*>(pEntity);
			if (grenade == nullptr)
				continue;

			if (grenade->m_iGrenadeID != GRENADE_SFTURRET)
				continue;

			CSfTurretNade* pGrenade = dynamic_ent_cast<CSfTurretNade*>(grenade);
			if (pGrenade == nullptr)
				continue;

			if (pGrenade->m_pOwner != this->m_pOwner)
				continue;

			vecEnt.emplace_back(pGrenade->entindex(), pGrenade->m_flSpawnTime);
		}

		if (!vecEnt.empty() && vecEnt.size() > 3)
		{
			std::sort(vecEnt.begin(), vecEnt.end(), [](const std::pair<int, time_point_t>& a, const std::pair<int, time_point_t>& b) {return a.second < b.second; });

			CSfTurretNade* pGrenade = (CSfTurretNade*)Instance(INDEXENT(vecEnt.at(0).first));
			if (pGrenade)
			{
				pGrenade->pev->effects |= EF_NODRAW;
				pGrenade->pev->flags |= FL_KILLME;
				pGrenade->SetThink(&CBaseEntity::SUB_Remove);
				pGrenade->SetTouch(NULL);
			}
		}
	}
	void CSfTurretNade::Precache(void)
	{
		m_vecEffect.push_back(PRECACHE_MODEL("sprites/ef_sfturret_fire_attack.spr"));
		m_vecEffect.push_back(PRECACHE_MODEL("sprites/ef_sfturret_fire_hit.spr"));
		m_vecEffect.push_back(PRECACHE_MODEL("sprites/ef_sfturret_lightning_attack.spr"));
		m_vecEffect.push_back(PRECACHE_MODEL("sprites/ef_sfturret_lightning_hit.spr"));
		m_vecEffect.push_back(PRECACHE_MODEL("sprites/ef_sfturret_explotion.spr"));



		PRECACHE_SOUND("weapons/sfturret_drop.wav");
		PRECACHE_SOUND("weapons/sfturret_exp_beep.wav");
		PRECACHE_SOUND("weapons/sfturret_exp_end.wav");
		PRECACHE_SOUND("weapons/sfturret_fire_exp.wav");
		PRECACHE_SOUND("weapons/sfturret_fire_loop.wav");
		PRECACHE_SOUND("weapons/sfturret_spark_exp.wav");
		PRECACHE_SOUND("weapons/sfturret_spark_loop.wav");
	};
	void EXPORT CSfTurretNade::TurretTouch(CBaseEntity* pOther)
	{
		if ((pOther->IsPlayer()) || pOther->IsAlive())
		{
			return;
		}

		Vector vecOrigin = pev->origin;
		Vector vecDir = pev->velocity.Normalize();
		Vector vecEnd = vecOrigin + vecDir * 512;

		Vector vecNormal;
		Vector vecNormalCopy;


		TraceResult tr;
		UTIL_TraceLine(vecOrigin, vecEnd, ignore_monsters, ENT(0), &tr);
		vecNormalCopy = vecNormal = tr.vecPlaneNormal;

		vecNormalCopy = UTIL_VecToAngles(vecNormal);
		vecNormalCopy.x -= 90.0;
		pev->angles = vecNormalCopy;
		pev->v_angle = vecNormal;
		pev->origin = pev->origin - vecNormal * 3;

		pev->movetype = MOVETYPE_NONE;

		m_iAnim = 1;
		pev->sequence = 0;
		pev->framerate = 1.0;
		pev->animtime = gpGlobals->time + 0.2s;
		pev->nextthink = gpGlobals->time + 0.2s;

		if (GetBodygroup(1) == 0)
		{
			SetBodygroup(1, 1);
			pev->skin = m_iState ? 1 : 0;
			pev->body = 0;
		}
		EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/sfturret_drop.wav", VOL_NORM, ATTN_NORM);

		SetTouch(nullptr);

		//UTIL_LogPrintf("vecPlaneNormal : %.1f %.1f %.1f, pev->angles : %.1f %.1f %.1f", vecNormal[0], vecNormal[1], vecNormal[2], pev->angles.x, pev->angles.y, pev->angles.z);

	}
	void EXPORT CSfTurretNade::TurretThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.2s;

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

		if (m_pOwner->m_iGrenadeID != GRENADE_SFTURRET)
		{
			Remove();
			return;
		}

		if (m_iAnim && !pev->movetype)
		{
			Vector vecOrigin = pev->origin;

			std::vector<CBaseEntity*> vecTempList;
			std::vector<CBaseEntity*> vecEnemiesList;

			if (m_iAnim == 1)
			{
				CBaseEntity* pEntity = NULL;

				int bInWater = (UTIL_PointContents(vecOrigin) == CONTENTS_WATER);

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, GetRange(-1, -1))) != NULL)
				{
					if (pEntity->edict() == pev->owner)
						continue;

					if (pEntity->IsDormant())
						continue;

					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pEntity->Classify() == CLASS_NONE)
						continue;

					if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
						continue;

					//队友不触发检测
					if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
						continue;

					m_iAnim = 2;
					m_flNextAnim = gpGlobals->time + (m_iState ? 10.0s : 2.0s) + 1.0s;
					m_flDelayExplosion = gpGlobals->time + (m_iState ? 10.0s : 2.0s) - 1.0s;
					break;
				}
			}
			else if (m_iAnim == 2)
			{
				if (m_flNextAnim != invalid_time_point)
				{
					if (m_flNextAnim > gpGlobals->time)
					{
						if (m_flDelayExplosion != invalid_time_point)
						{
							if ((m_flDelayExplosion + 1s) > gpGlobals->time)
							{
								vecTempList = RadiusDamage(GetDamage(m_iState, 0), GetRange(m_iState, 0), true, vecOrigin);

								//vecTempList = RadiusDamage(GetDamage(m_iState, 0), GetRange(m_iState, 0), false, vecOrigin);
								vecEnemiesList.assign(vecTempList.begin(), vecTempList.end());
								for (std::vector<CBaseEntity*>::iterator iter = vecEnemiesList.begin(); iter != vecEnemiesList.end(); ++iter)
								{
									auto pEle = *iter;
									if (pEle->IsPlayer())
									{
										CBasePlayer* pPlayer = (CBasePlayer*)pEle;

										if (pPlayer)
										{
											pPlayer->m_tLockBuffTime = gpGlobals->time + (m_iState ? 1.0s : 2.0s);
											pPlayer->m_flLockSpeed = m_iState ? 100.0f : 70.0f;
											pPlayer->ResetMaxSpeed();
										}
									}
									if (pEle->IsAlive())
									{
										MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pEle->pev->origin);
										WRITE_BYTE(TE_TEMPSPRITE);
										WRITE_COORD(pEle->pev->origin.x);
										WRITE_COORD(pEle->pev->origin.y);
										WRITE_COORD(pEle->pev->origin.z);
										WRITE_SHORT(m_vecEffect.at(m_iState ? 3 : 1));
										WRITE_BYTE(2);
										WRITE_BYTE(255);
										WRITE_BYTE(30);
										MESSAGE_END();
									}
								}
								if (!vecEnemiesList.empty())
								{
									EMIT_SOUND(ENT(pev), CHAN_AUTO, m_iState ? "weapons/sfturret_spark_exp.wav" : "weapons/sfturret_fire_exp.wav", VOL_NORM, ATTN_NORM);

									MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
									WRITE_BYTE(TE_TEMPSPRITE);
									WRITE_COORD(pev->origin.x);
									WRITE_COORD(pev->origin.y);
									WRITE_COORD(pev->origin.z + 5.0f);
									WRITE_SHORT(m_vecEffect.at(m_iState ? 2 : 0));
									WRITE_BYTE(3);
									WRITE_BYTE(255);
									WRITE_BYTE(30);
									MESSAGE_END();
								}
								if ((m_flDelayExplosion) <= gpGlobals->time)
								{
									if (m_flReadyExplosion == invalid_time_point)
									{
										EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/sfturret_exp_beep.wav", VOL_NORM, ATTN_NORM);
										if (pev->sequence != 1)
										{
											pev->sequence = 1;
											pev->body = 1;
											pev->animtime = gpGlobals->time;
										}
										m_flReadyExplosion = gpGlobals->time + 1.25s;
									}
								}
							}
						}
						if (m_flReadyExplosion != invalid_time_point && m_flReadyExplosion < gpGlobals->time)
						{
							m_flReadyExplosion = invalid_time_point;
							m_flDelayExplosion = invalid_time_point;

							RadiusDamage(GetDamage(m_iState, 1), GetRange(m_iState, 1), true, vecOrigin);
							OnExplosion();
						}
					}
				}
			}
		}
	}
	void CSfTurretNade::OnExplosion()
	{
		m_iAnim = 0;
		m_flNextAnim = invalid_time_point;
		m_flNextAnim = invalid_time_point;

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_vecEffect.at(4));
		WRITE_BYTE(15);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/sfturret_exp_end.wav", VOL_NORM, ATTN_NORM);

		return SUB_Remove();
	}
	std::vector<CBaseEntity*> CSfTurretNade::RadiusDamage(float flDamage, float flRadius, bool IsDamage, Vector vecOrigin)
	{
		std::vector<CBaseEntity*> vecTemp;

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

		bool IsUsingDefaultOrigin = !vecOrigin.Length() ? true : false;

		if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
		{
			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecDelta;
			Vector vecEnd;
			int bInWater = (UTIL_PointContents(IsUsingDefaultOrigin ? pev->origin : vecOrigin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, IsUsingDefaultOrigin ? pev->origin : vecOrigin, flRadius)) != NULL)
			{
				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
					continue;

				//队友不触发检测
				if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (!player)
						continue;

					if (pAttackPlayer->m_iTeam == player->m_iTeam)
						continue;

					vecEnd = player->BodyTarget(IsUsingDefaultOrigin ? pev->origin : vecOrigin);

					UTIL_TraceLine(IsUsingDefaultOrigin ? pev->origin : vecOrigin, vecEnd, ignore_monsters, pev->owner, &tr);

					if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
						continue;

					vecDelta = pEntity->pev->origin - (IsUsingDefaultOrigin ? pev->origin : vecOrigin);


					if (vecDelta.Length() > flRadius)
						continue;

					if (IsDamage)
						player->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_EXPLOSION | DMG_NEVERGIB | DMG_NOMORALE);

					vecTemp.push_back(player);
				}
				else
				{
					vecEnd = pEntity->BodyTarget(IsUsingDefaultOrigin ? pev->origin : vecOrigin);

					UTIL_TraceLine(IsUsingDefaultOrigin ? pev->origin : vecOrigin, vecEnd, ignore_monsters, pev->owner, &tr);

					if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
						continue;

					vecDelta = pEntity->pev->origin - (IsUsingDefaultOrigin ? pev->origin : vecOrigin);

					if (vecDelta.Length() > flRadius)
						continue;

					if (IsDamage)
						pEntity->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_EXPLOSION | DMG_NEVERGIB | DMG_NOMORALE);
					vecTemp.push_back(pEntity);
				}
			}
		}
		return vecTemp;
	}
	void CSfTurretNade::Remove()
	{
		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
		WRITE_SHORT(entindex());
		MESSAGE_END();

		SetThink(nullptr);
		return UTIL_Remove(this);
	}
	float CSfTurretNade::GetDamage(int iType, int iStage) const
	{
		float flDamage = 1.0f;

		if (!iStage)
			flDamage = 3.0f;
		else
			flDamage = 100.0f;

		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			if (iType)
			{
				if (!iStage)
					flDamage = 36.0f;
				else
					flDamage = 17143.0f;
			}
			else
			{
				if (!iStage)
					flDamage = 168.0f;
				else
					flDamage = 12000.0f;
			}
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			if (iType)
			{
				if (!iStage)
					flDamage = 280.0f;
				else
					flDamage = 1846.0f;
			}
			else
			{
				if (!iStage)
					flDamage = 92.0f;
				else
					flDamage = 2638.0f;
			}
		}
		return flDamage;
	}
	float CSfTurretNade::GetRange(int iType, int iStage) const
	{
		float flRange = 1.0f;
		if (iType == -1)
			flRange = 137.8f;
		else
			flRange = 196.85f;

		return flRange;
	}
#endif
}