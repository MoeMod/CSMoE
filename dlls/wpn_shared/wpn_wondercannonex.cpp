/*
wpn_wondercannonex.cpp - Port of Zombie IV Mod
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

	LINK_ENTITY_TO_CLASS(weapon_wondercannonex, CWonderCannonEX)

	void CWonderCannonEX::Spawn(void)
	{
		pev->classname = MAKE_STRING("weapon_wondercannonex");

		Precache();
		m_iId = WEAPON_WONDERCANNONEX;
		SET_MODEL(ENT(pev), "models/w_wondercannonex.mdl");
		m_tNextPlayEvent = invalid_time_point;
		m_iDefaultAmmo = 30;
		m_iDefaultAmmo2 = WONDERCANNONEX_MAXMINE;
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;

		m_flSecondaryAttackTimer = invalid_time_point;

		FallInit();
	};

	int CWonderCannonEX::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "wondercannon_ammo";
		p->iMaxAmmo1 = 120;
		p->pszAmmo2 = "wondercannonMineAmmo";
		p->iMaxAmmo2 = 3;
		p->iMaxClip = 30;
		p->iSlot = 0;
		p->iPosition = 4;
		p->iId = m_iId = WEAPON_WONDERCANNONEX;
		p->iFlags = 0;
		p->iWeight = AK47_WEIGHT;

		return 1;
	};

	void CWonderCannonEX::Precache(void)
	{
		PRECACHE_MODEL("models/v_wondercannonex.mdl");
		PRECACHE_MODEL("models/p_wondercannonex.mdl");
		PRECACHE_MODEL("models/w_wondercannonex.mdl");

		PRECACHE_SOUND("weapons/wondercannonex-1.wav");
		PRECACHE_SOUND("weapons/wondercannon_comd_shoot.wav");	
		PRECACHE_SOUND("weapons/wondercannon_cmod_charging.wav");

		m_iHitEffect = PRECACHE_MODEL("sprites/ef_wondercannonex_hit1.spr");
		PRECACHE_MODEL("sprites/ef_wondercannonex_light.spr");
		PRECACHE_MODEL("sprites/muzzleflash109.spr");
		PRECACHE_MODEL("sprites/muzzleflash113.spr");
		PRECACHE_MODEL("sprites/muzzleflash116.spr");
		PRECACHE_MODEL("sprites/muzzleflash255.spr");

		m_usFireWonderCannon = PRECACHE_EVENT(1, "events/wondercannon.sc");
	};

	int CWonderCannonEX::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	BOOL CWonderCannonEX::Deploy(void)
	{
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_tNextPlayEvent = invalid_time_point;
		pev->skin = 0;
		m_flSecondaryAttackTimer = invalid_time_point;

		//PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFirewondercannonex, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, TRUE);

		if (DefaultDeploy("models/v_wondercannonex.mdl", "models/p_wondercannonex.mdl", WONDERCANNON_DRAW, "ak47", UseDecrement() != FALSE, 0.1s, 1.5s))
		{
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
			return TRUE;
		}
		return FALSE;
	};

	void CWonderCannonEX::Reload(void)
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

	void CWonderCannonEX::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(WONDERCANNON_IDLE1, UseDecrement() != FALSE);
	};

	float CWonderCannonEX::GetMaxSpeed(void)
	{
		return 240;
	};


	void CWonderCannonEX::PrimaryAttack(void)
	{
		duration_t flCycleTime = 0.57s;
		m_tNextPlayEvent = gpGlobals->time + 0.3s;
		pev->skin = 1;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			WonderCannonEXFire(0.05 + (0.2) * m_flAccuracy, flCycleTime, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			WonderCannonEXFire(0.01 + (0.02) * m_flAccuracy, flCycleTime, FALSE);
		else
			WonderCannonEXFire(0.03 + (0.0) * m_flAccuracy, flCycleTime, FALSE);

	};
	void CWonderCannonEX::WonderCannonEXFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
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
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamageA(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

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

	float CWonderCannonEX::GetDamageA() const
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
	void CWonderCannonEX::FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
	{
		int iOriginalPenetration = 2;

		if (iOriginalPenetration == iPenetration)
		{
			if (CBaseEntity* pEntity = gMultiDamage.pEntity)
			{
				if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					DoExpAttachVicStart(pEntity);
				}
			}
		}
	}

	void CWonderCannonEX::DoExpAttachVicStart(CBaseEntity* pVictim)
	{
		if (pVictim->m_iTeam != m_pPlayer->m_iTeam)
			UTIL_TempModel(Vector(0, 0, 0), pev->angles, g_vecZero, m_iHitEffect, 30, 0, 200, 0, 0, 195, kRenderTransAdd, pVictim, false, 0, 8, 0, 0);

		CWonderCannonChain* pNewChain = CWonderCannonChain::Create(1, pVictim->pev->origin, g_vecZero, ENT(m_pPlayer->pev));

		if (pNewChain)
		{
			CUtlVector<CBaseEntity*>* pEnemyList = new CUtlVector<CBaseEntity*>();
			pNewChain->Init(m_pPlayer, this, pVictim, pEnemyList);
		}
	};
#endif


	int CWonderCannonEX::CheckBombActive()
	{
		int iCheck = 0;
#ifndef CLIENT_DLL
		for (int i = 0; i < WONDERCANNONEX_MAXMINE; i++)
		{
			if (m_pWonderCannonMine[i])
				iCheck++;
		}
#endif
		return iCheck;
	}
	//

	void CWonderCannonEX::SecondaryAttack()
	{
#if 1	
		if (m_flSecondaryAttackTimer == invalid_time_point)
			m_flSecondaryAttackTimer = gpGlobals->time;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/wondercannon_cmod_charging.wav", VOL_NORM, ATTN_NORM);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
#endif
	}

	void CWonderCannonEX::ExpCurrentBomb()
	{
#ifndef CLIENT_DLL
		for (int i = 0; i < WONDERCANNONEX_MAXMINE; i++)
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

	void CWonderCannonEX::ShootBomb()
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1); //5
#endif
		//Make Bomb...Lazy to do 
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		int iMineId = 0;
		for (iMineId = 0; iMineId < WONDERCANNONEX_MAXMINE; iMineId++)
		{
			if (!m_pWonderCannonMine[iMineId])
				break;
		}

		m_pWonderCannonMine[iMineId] = CWonderCannonMine::Create(1, vecSrc, UTIL_VecToAngles(gpGlobals->v_forward), ENT(m_pPlayer->pev));

		if (m_pWonderCannonMine[iMineId])
		{
			m_pWonderCannonMine[iMineId]->Init(m_pPlayer, this);
			m_pWonderCannonMine[iMineId]->pev->velocity = gpGlobals->v_forward * 1050;		
		}
#endif

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	}

	void CWonderCannonEX::ItemPostFrame(void)
	{	
		if (m_tNextPlayEvent != invalid_time_point && m_tNextPlayEvent < gpGlobals->time)
		{	
			pev->skin = 0;
			m_tNextPlayEvent = invalid_time_point;
		}

		if (m_flSecondaryAttackTimer > invalid_time_point)
		{
			if (this->m_pPlayer->pev->button & IN_ATTACK2) // 0x800
			{
				if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
					return CBasePlayerWeapon::ItemPostFrame();
				const int v16 = m_pPlayer->pev->weaponanim;		
				if (gpGlobals->time > m_flSecondaryAttackTimer + WONDERCANNONEX_CHECKBOMBTIME)
				{
					if (gpGlobals->time - m_flSecondaryAttackTimer >= WONDERCANNONEX_CHECKBOMBTIME + 1.0s)
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

						SetThink(&CWonderCannonEX::ExpCurrentBomb);
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

				if (gpGlobals->time - m_flSecondaryAttackTimer < WONDERCANNONEX_CHECKBOMBTIME)
				{
					//exp current bomb
#ifndef CLIENT_DLL
					if (CheckBombActive())
					{
						SendWeaponAnim(WONDERCANNON_BMOD_ON, UseDecrement() != FALSE);

						SetThink(&CWonderCannonEX::ExpCurrentBomb);
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
				else if (gpGlobals->time - m_flSecondaryAttackTimer >= WONDERCANNONEX_CHECKBOMBTIME)
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
	

	void CWonderCannonEX::Holster(int skiplocal)
	{
		SetThink(nullptr);
		return CBasePlayerWeapon::Holster();
	}
	
};