/*
thunderpistol.cpp
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
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#endif

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CThunderPistol : public LinkWeaponTemplate< CThunderPistol,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
            TPrecacheEvent,
            TDeployDefault,
			TReloadDefault,
            TPrimaryAttackRifle,
            TFirePistol,
			TGetDamageDefault,
			TWeaponIdleDefault
    >
    {
    public:
	static constexpr const auto& DefaultReloadTime = 2.37s;
    static constexpr const char *V_Model = "models/v_thunderpistol.mdl";
    static constexpr const char *P_Model = "models/p_thunderpistol.mdl";
    static constexpr const char *W_Model = "models/w_thunderpistol.mdl";
    static constexpr const char *EventFile = "events/thunderpistol.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_thunderpistol";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 1;

#define WPNSTATE_THUNDERPISTOL_MODEB	1

	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOTA,
		ANIM_SHOOTB,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.1*A+0.1;
    static constexpr const auto &SpreadCalcWalking = 0.045*A+0.02;
    static constexpr const auto& SpreadCalcDucking = 0.0*A;
    static constexpr const auto &SpreadCalcDefault = 0.01*A+0.01;

	static constexpr const auto& CycleTime = 0.7s;
    static constexpr int DamageDefault = 109;
    static constexpr int DamageZB = 1542;
    static constexpr int DamageZBS = 1070;

    static constexpr float AccuracyDefault = 0.15;
    /*static constexpr*/ int Penetration = 10;
    KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_thunderpistol" , 100 }; }
#endif
	float GetArmorRatioModifier() override { return 1.5; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int m_iEfIndex;
	int m_iExpIndex;
	int m_iExpBIndex;
	int m_iDefaultAmmo2;
	time_point_t m_flNextThink;

	void Spawn()
	{
		m_flNextThink = invalid_time_point;
		m_iDefaultAmmo2 = 100;
		Base::Spawn();
	}

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "thunderpistol";
		p->iMaxAmmo1 = 50;
		p->pszAmmo2 = "cloaking_thunderpistol";
		p->iMaxAmmo2 = 100;
		p->iMaxClip = 1;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_THUNDERPISTOL;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	}

	float GetMaxSpeed(void)
	{
		if (m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB)
			return 330;

		return 250;
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void Holster(int skiplocal)
	{
		m_pPlayer->m_iBuff &= ~BUFF_CLOAKING;
		if (m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB)
			m_iWeaponState &= ~WPNSTATE_THUNDERPISTOL_MODEB;
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void Precache(void)
	{
		m_iEfIndex = PRECACHE_MODEL("models/ef_thunderpistol.mdl");
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_thunderpistol_explosion.spr");
		m_iExpBIndex = PRECACHE_MODEL("sprites/ef_thunderpistol_explosionb.spr");
		PRECACHE_MODEL("sprites/ef_thunderpistol_laserbeam.spr");
		PRECACHE_MODEL("sprites/ef_thunderpistol_laserbeamb.spr");
		PRECACHE_MODEL("sprites/ef_thunderpistol_transform.spr");
		PRECACHE_MODEL("sprites/ef_thunderpistol_transform_left.spr");
		PRECACHE_MODEL("sprites/muzzleflash83.spr");
		PRECACHE_MODEL("sprites/muzzleflash84.spr");

		PRECACHE_SOUND("weapons/thundergun_2.wav");
		Base::Precache();
	}

	void SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB)
		{
			m_pPlayer->m_iBuff &= ~BUFF_CLOAKING;
			m_iWeaponState &= ~WPNSTATE_THUNDERPISTOL_MODEB;
			m_pPlayer->m_flCloaking = gpGlobals->time + 0.5s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;
			m_pPlayer->ResetMaxSpeed();
		}
		else
		{
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/thundergun_2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#ifndef CLIENT_DLL
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, (m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB) ? TRUE : FALSE, TRUE);

				UTIL_TempModel({ 0, 0, 0 }, g_vecZero, g_vecZero, m_iEfIndex, 20, 0, 255, 0, 0, 255, kRenderTransAdd, m_pPlayer, false, 0, 5, 30, 0);
#endif
				m_iWeaponState |= WPNSTATE_THUNDERPISTOL_MODEB;
				m_pPlayer->ResetMaxSpeed();
				m_pPlayer->m_iBuff |= BUFF_CLOAKING;
				m_pPlayer->m_flCloaking = m_flNextThink = gpGlobals->time + 1s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;
			}
		}
	}

	void ItemPostFrame()
	{
		if (m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB)
		{
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
			{
				if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
				{
					m_flNextThink = gpGlobals->time + 0.25s;
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
				}
			}
		}

		if (m_iWeaponState && !m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		{
			m_pPlayer->m_iBuff &= ~BUFF_CLOAKING;
			m_flNextThink = invalid_time_point;
			m_iWeaponState &= ~WPNSTATE_THUNDERPISTOL_MODEB;
			m_pPlayer->ResetMaxSpeed();
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

	BOOL Deploy()
	{
		m_pPlayer->m_iBuff &= ~BUFF_CLOAKING;
		m_iWeaponState &= ~WPNSTATE_THUNDERPISTOL_MODEB;
		return Base::Deploy();
	}

#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
	{
		Vector vecEnd = tr->vecEndPos - vecDir * 10;
		int iOriginalPenetration = m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB ? 1 : 10;

		if (iOriginalPenetration == iPenetration)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecEnd);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(vecEnd.x);
			WRITE_COORD(vecEnd.y);
			WRITE_COORD(vecEnd.z);
			WRITE_SHORT(m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB ? m_iExpBIndex : m_iExpIndex);
			WRITE_BYTE(6);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, 3 * 39.37)) != NULL)
			{
				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->pev == m_pPlayer->pev)
					continue;

				pEntity->TraceAttack(m_pPlayer->pev, GetExpDamage(), vecDir, tr, DMG_BULLET | DMG_NEVERGIB);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (target->m_bIsZombie)
					{
						if (!(target->m_iBuff & BUFF_IGNORE_KNOCKBACK))
						{
							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(target->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(1.5s, m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB ? 70.0f : 100.0f);
								strategy->SetStunGravityTime(1.5s);
							}
						}

					}
				}

				
			}
		}
	}
#endif

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		m_iShotsFired++;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 240.0 + 0.1;

			if (m_flAccuracy > 1.0)
				m_flAccuracy = 1.0;
		}

		m_flLastFire = gpGlobals->time;

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

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

		bool bShootB = m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 10;

		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, bShootB ? 1 : 10, BULLET_PLAYER_50AE, GetDamage(), 1.0, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), bShootB ? TRUE : FALSE, FALSE);
#endif
		if (m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB)
		{
			if (bShootB)
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 10;
				if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
			}
				
			m_pPlayer->m_flCloaking = gpGlobals->time + 0.5s;
			m_iWeaponState &= ~WPNSTATE_THUNDERPISTOL_MODEB;
			m_pPlayer->m_iBuff &= ~BUFF_CLOAKING;
			m_pPlayer->ResetMaxSpeed();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		
		m_pPlayer->pev->punchangle.x -= 2;
    }

	float GetExpDamage() const
	{
		float flDamage = 5.0f;
		if (m_iWeaponState & WPNSTATE_THUNDERPISTOL_MODEB)
		{
			flDamage = 10.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 100.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1000.0f;
#endif
		}
		else
		{
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 50.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 500.0f;
#endif
		}

		return flDamage;
	}

};
LINK_ENTITY_TO_CLASS(weapon_thunderpistol, CThunderPistol)
}