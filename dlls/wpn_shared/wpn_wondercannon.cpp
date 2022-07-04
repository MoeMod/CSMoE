/*
wpn_wondercannon.cpp - Port of Zombie IV Mod
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "studio.h"

#include "decals.h"
#include "gamerules.h"
#include "wpn_wondercannon.h"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_wondercannon, CWonderCannon)

	void CWonderCannon::Spawn(void)
	{
		pev->classname = MAKE_STRING("weapon_wondercannon");

		Precache();
		m_iId = WEAPON_WONDERCANNON;
		SET_MODEL(ENT(pev), "models/w_wondercannon.mdl");
		m_tNextPlayEvent = invalid_time_point;
		m_iDefaultAmmo = 30;
		m_iDefaultAmmo2 = WONDERCANNON_MAXMINE;
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;

		m_flSecondaryAttackTimer = m_flSecondaryAttackTimer2 = invalid_time_point;

		FallInit();
	};

	int CWonderCannon::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "wondercannon_ammo";
		p->iMaxAmmo1 = 120;
		p->pszAmmo2 = "WonderCannonMineAmmo";
		p->iMaxAmmo2 = 3;
		p->iMaxClip = 30;
		p->iSlot = 0;
		p->iPosition = 4;
		p->iId = m_iId = WEAPON_WONDERCANNON;
		p->iFlags = 0;
		p->iWeight = AK47_WEIGHT;

		return 1;
	};

	void CWonderCannon::Precache(void)
	{
		PRECACHE_MODEL("models/v_wondercannon.mdl");
		PRECACHE_MODEL("models/p_wondercannon.mdl");
		PRECACHE_MODEL("models/w_wondercannon.mdl");

		PRECACHE_SOUND("weapons/wondercannon-1.wav");
		PRECACHE_SOUND("weapons/wondercannon_comd_shoot.wav");	
		PRECACHE_SOUND("weapons/wondercannon_cmod_charging.wav");

		m_iHitEffect = PRECACHE_MODEL("sprites/ef_wondercannon_hit.spr");
		PRECACHE_MODEL("sprites/ef_wondercannon_bomb_set.spr");
		PRECACHE_MODEL("sprites/muzzleflash109.spr");
		PRECACHE_MODEL("sprites/muzzleflash111.spr");
		PRECACHE_MODEL("sprites/muzzleflash113.spr");
		PRECACHE_MODEL("sprites/muzzleflash116.spr");

		m_usFireWonderCannon = PRECACHE_EVENT(1, "events/wondercannon.sc");
	};

	int CWonderCannon::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	BOOL CWonderCannon::Deploy(void)
	{
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_tNextPlayEvent = invalid_time_point;
		pev->skin = 0;
		m_flSecondaryAttackTimer = m_flSecondaryAttackTimer2 = invalid_time_point;

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireWonderCannon, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, TRUE);

		if (DefaultDeploy("models/v_wondercannon.mdl", "models/p_wondercannon.mdl", WONDERCANNON_DRAW, "ak47", UseDecrement() != FALSE, 0.1s, 1.5s))
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
			return TRUE;
		}
		return FALSE;
	};

	void CWonderCannon::Reload(void)
	{
		m_flSecondaryAttackTimer = invalid_time_point;
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(30, WONDERCANNON_RELOAD, 2.0s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.2;
			m_bDelayFire = false;
			m_iShotsFired = 0;
		}
	};

	void CWonderCannon::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(WONDERCANNON_IDLE1, UseDecrement() != FALSE);
	};

	float CWonderCannon::GetMaxSpeed(void)
	{
		return 240;
	};


	void CWonderCannon::PrimaryAttack(void)
	{
		duration_t flCycleTime = 0.75s;
		m_tNextPlayEvent = gpGlobals->time + 0.3s;
		pev->skin = 1;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			WonderCannonFire(0.05 + (0.2) * m_flAccuracy, flCycleTime, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			WonderCannonFire(0.01 + (0.02) * m_flAccuracy, flCycleTime, FALSE);
		else
			WonderCannonFire(0.03 + (0.0) * m_flAccuracy, flCycleTime, FALSE);

	};
	void CWonderCannon::WonderCannonFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0) + 0.3;

		//Max Accuracy
		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_WONDERCANNON, GetDamageA(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireWonderCannon, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.55, 0.3, 0.3, 3.75, 1.5, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1, 0.3, 0.2, 0.06, 3.75, 1.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.75, 0.1, 0.1, 0.018, 3.75, 1.5, 9);
		else
			KickBack(0.85, 0.2, 0.2, 0.02, 3.75, 1.5, 7);

	};

	float CWonderCannon::GetDamageA() const
	{
		float flDamage = 1.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 380.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 580.0f;
#endif
		return flDamage;
	}

#ifndef CLIENT_DLL
	void CWonderCannon::DoExpAttachVicStart(CBaseEntity* pVictim)
	{
		if (pVictim->m_iTeam != m_pPlayer->m_iTeam)
			UTIL_TempModel(Vector(0, 0, 0), pev->angles, g_vecZero, m_iHitEffect, 30, 0, 200, 0, 0, 195, kRenderTransAdd, pVictim, false, 0, 8, 0, 0);

		CWonderCannonChain* pNewChain = static_cast<CWonderCannonChain*>(CBaseEntity::Create("wondercannon_chain", pVictim->pev->origin, g_vecZero, ENT(m_pPlayer->pev)));

		if (pNewChain)
		{
			CUtlVector<CBaseEntity*>* pEnemyList = new CUtlVector<CBaseEntity*>();
			pNewChain->Init(m_pPlayer, this, pVictim, pEnemyList);
		}
	};
#endif

#ifndef CLIENT_DLL
	LINK_ENTITY_TO_CLASS(wondercannon_mine, CWonderCannonMine)

	void CWonderCannonMine::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), "models/bomb_wondercannon.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->angles = g_vecZero;
		pev->gravity = 2.5;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_TOSS;
		pev->frame = 0;

		pev->classname = MAKE_STRING("wondercannon_mine");
		pev->nextthink = gpGlobals->time + 0.01s;
		SetThink(&CWonderCannonMine::MineThink);
	}

	void CWonderCannonMine::Init(CBasePlayer* pOwner, CWonderCannon* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;
	}

	void CWonderCannonMine::Precache(void)
	{
		PRECACHE_MODEL("sprites/ef_wondercannon_bomb_set.spr");
		PRECACHE_MODEL("models/ef_wondercannon_area.mdl");
		PRECACHE_MODEL("models/bomb_wondercannon.mdl");

		m_iExp[0] = PRECACHE_MODEL("sprites/ef_wondercannon_hit3.spr");
		m_iExp[1] = PRECACHE_MODEL("sprites/ef_wondercannon_bomb_ex.spr");

		PRECACHE_SOUND("weapons/wondercannon_comd_drop.wav");
	}

	void CWonderCannonMine::Remove()
	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
		WRITE_SHORT(entindex());
		MESSAGE_END();

		SetThink(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}

	void CWonderCannonMine::Explode(bool IsManual)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iExp[0]);
		WRITE_BYTE(10);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 5.0);
		WRITE_SHORT(m_iExp[1]);
		WRITE_BYTE(5);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		DoBombExp(IsManual);

		Remove();
	}
	void CWonderCannonMine::DoBombExp( bool IsManual)
	{
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 39.37 * 5)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity == m_pOwner)
				continue;

			if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
				continue;

			pEntity->TakeDamage(pev, m_pOwner->pev, GetDamage(IsManual), DMG_NEVERGIB | DMG_EXPLOSION | DMG_NOMORALE);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(3.5s, 55.0);
						strategy->SetStunGravityTime(3.5s);
					}
				}
			}
		}
	}

	float CWonderCannonMine::GetDamage(bool IsManual) const
	{
		float flDamage = IsManual ? 110.0f : 100.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			//flDamage = IsManual ? 28600.0f : 18600.0f;
			flDamage = IsManual ? 16900.0f : 10900.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = IsManual ? 39400.0f : 24400.0f;

#endif
		return flDamage;
	}

	void EXPORT CWonderCannonMine::MineThink(void)
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

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];

		if (!pWeapon)
		{
			Remove();
			return;
		}

		if (pWeapon->m_iId != WEAPON_WONDERCANNON)
		{
			Remove();
			return;
		}

		if (pev->flags & FL_ONGROUND)
		{
			if (m_iState == 2)
			{
				CBaseEntity* pEntity = NULL;

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 39.37 * 1.5)) != NULL)
				{
					//a lazy way
					if (pEntity != m_pOwner)
						continue;

					Vector2D vecVelocity = pEntity->pev->velocity.Make2D();
					float flSpeed = vecVelocity.Length();
					vecVelocity = vecVelocity.Normalize();
					pEntity->pev->velocity.x = vecVelocity.x * flSpeed * 2;
					pEntity->pev->velocity.y = vecVelocity.y * flSpeed * 2;
					pEntity->pev->velocity.z = 400;

					Explode(false);
					return;
				}

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 39.37 * 2.8)) != NULL)
				{
					if (pEntity->IsDormant())
						continue;

					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pEntity == m_pOwner)
						continue;

					if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
						continue;
			
					Explode(false);
					
				}
			}

			if (m_flNextAnim != invalid_time_point && m_flNextAnim < gpGlobals->time)
			{
				pev->sequence = 1;
				//play model & spr

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, this->pev);
				WRITE_BYTE(18);
				WRITE_BYTE(0);
				WRITE_SHORT(this->entindex());
				MESSAGE_END();

				m_iState = 2;
				m_flNextAnim = invalid_time_point;		
			}

			if (!m_iState)
			{
				pev->sequence = 0;
				pev->frame = 0;
				ResetSequenceInfo();

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, this->pev);
				WRITE_BYTE(18);
				WRITE_BYTE(1);
				WRITE_SHORT(this->entindex());
				MESSAGE_END();

				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/wondercannon_comd_drop.wav", VOL_NORM, ATTN_NORM);

				m_flNextAnim = gpGlobals->time + 0.5s;
				
				m_iState = 1;
			}
		}
	}

#endif
	int CWonderCannon::CheckBombActive()
	{
		int iCheck = 0;
#ifndef CLIENT_DLL
		for (int i = 0; i < WONDERCANNON_MAXMINE; i++)
		{
			if (m_pWonderCannonMine[i])
				iCheck++;
		}
#endif
		return iCheck;
	}
	//

	void CWonderCannon::SecondaryAttack()
	{
#if 1	
		if (m_flSecondaryAttackTimer == invalid_time_point)
			m_flSecondaryAttackTimer = gpGlobals->time;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/wondercannon_cmod_charging.wav", VOL_NORM, ATTN_NORM);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
#endif
	}

	void CWonderCannon::ExpCurrentBomb()
	{
#ifndef CLIENT_DLL
		for (int i = 0; i < WONDERCANNON_MAXMINE; i++)
		{
			if (m_pWonderCannonMine[i])
			{
				if (m_pWonderCannonMine[i])
				{
					m_pWonderCannonMine[i]->Explode(true);
				}
			}			
		}
#endif
		SetThink(nullptr);
	}

	void CWonderCannon::ShootBomb()
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1); //5
#endif
		//Make Bomb...Lazy to do 
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		int iMineId = 0;
		for (iMineId = 0; iMineId < WONDERCANNON_MAXMINE; iMineId++)
		{
			if (!m_pWonderCannonMine[iMineId])
				break;
		}

		m_pWonderCannonMine[iMineId] = static_cast<CWonderCannonMine*>(CBaseEntity::Create("wondercannon_mine", vecSrc, UTIL_VecToAngles(gpGlobals->v_forward), ENT(m_pPlayer->pev)));

		if (m_pWonderCannonMine[iMineId])
		{
			m_pWonderCannonMine[iMineId]->Init(m_pPlayer, this);
			m_pWonderCannonMine[iMineId]->pev->velocity = gpGlobals->v_forward * 1050;		
		}
#endif

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	}

	void CWonderCannon::ItemPostFrame(void)
	{	
		if (m_tNextPlayEvent != invalid_time_point && m_tNextPlayEvent < gpGlobals->time)
		{	
			pev->skin = 0;
			m_tNextPlayEvent = invalid_time_point;
		}

		if (m_flSecondaryAttackTimer2 != invalid_time_point && m_flSecondaryAttackTimer2 < gpGlobals->time)
		{
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireWonderCannon, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, TRUE);
			m_flSecondaryAttackTimer2 = invalid_time_point;
		}
		
		if (m_flSecondaryAttackTimer > invalid_time_point)
		{
			if (this->m_pPlayer->pev->button & IN_ATTACK2) // 0x800
			{
				if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
					return CBasePlayerWeapon::ItemPostFrame();
				const int v16 = m_pPlayer->pev->weaponanim;		
				if (gpGlobals->time > m_flSecondaryAttackTimer + WONDERCANNON_CHECKBOMBTIME)
				{
					if (gpGlobals->time - m_flSecondaryAttackTimer >= WONDERCANNON_CHECKBOMBTIME + 1.0s)
					{
						if (v16 != WONDERCANNON_CMOD_IDLE) // 9
						{
							SendWeaponAnim(WONDERCANNON_CMOD_IDLE, UseDecrement() != FALSE);
							return CBasePlayerWeapon::ItemPostFrame();
						}
					}
					else if (v16 != WONDERCANNON_CMOD_START) // 7
					{
						SendWeaponAnim(WONDERCANNON_CMOD_START, UseDecrement() != FALSE);
						return CBasePlayerWeapon::ItemPostFrame();
					}
				}
			}
			else
			{
				if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
				{
					//exp current bomb
#ifndef CLIENT_DLL
					if (CheckBombActive())
					{
						SendWeaponAnim(WONDERCANNON_BMOD_ON, UseDecrement() != FALSE);

						m_flSecondaryAttackTimer2 = gpGlobals->time + 1.5s;
						PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireWonderCannon, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, TRUE);

						SetThink(&CWonderCannon::ExpCurrentBomb);
						pev->nextthink = gpGlobals->time + 1.0s;
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
					}
					else
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1s;
#else
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
#endif
					m_flSecondaryAttackTimer = invalid_time_point;
					return CBasePlayerWeapon::ItemPostFrame();
				}

				if (gpGlobals->time - m_flSecondaryAttackTimer < WONDERCANNON_CHECKBOMBTIME)
				{
					//exp current bomb
#ifndef CLIENT_DLL
					if (CheckBombActive())
					{
						SendWeaponAnim(WONDERCANNON_BMOD_ON, UseDecrement() != FALSE);

						m_flSecondaryAttackTimer2 = gpGlobals->time + 1.5s;
						PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireWonderCannon, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, TRUE);

						SetThink(&CWonderCannon::ExpCurrentBomb);
						pev->nextthink = gpGlobals->time + 1.0s;
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
					}
					else
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0s;
#else
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
#endif
					m_flSecondaryAttackTimer = invalid_time_point;
					return CBasePlayerWeapon::ItemPostFrame();
				}
				else if (gpGlobals->time - m_flSecondaryAttackTimer >= WONDERCANNON_CHECKBOMBTIME)
				{
					SendWeaponAnim(WONDERCANNON_CMOD_SHOOT, UseDecrement() != FALSE); // 8

					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/wondercannon_comd_shoot.wav", VOL_NORM, ATTN_NORM);

					m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
					ShootBomb();
					m_flSecondaryAttackTimer = invalid_time_point;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
					return CBasePlayerWeapon::ItemPostFrame();
				}
				else if (m_pPlayer->pev->weaponanim != WONDERCANNON_CMOD_START) // 7
				{
					SendWeaponAnim(WONDERCANNON_CMOD_START, UseDecrement() != FALSE); // 7
					return CBasePlayerWeapon::ItemPostFrame();
				}
			}
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}
	

	void CWonderCannon::Holster(int skiplocal)
	{
		SetThink(nullptr);
		return CBasePlayerWeapon::Holster();
	}
	
};