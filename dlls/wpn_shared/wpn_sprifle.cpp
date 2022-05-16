/*
sprifle.cpp
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

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CSprifle : public LinkWeaponTemplate< CSprifle,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG550>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
			TSecondaryAttackSniperZoom2,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.5s;
	static constexpr int ZoomFOV1 = 40;
	static constexpr int ZoomFOV2 = 10;
    static constexpr const char *V_Model = "models/v_sprifle.mdl";
    static constexpr const char *P_Model = "models/p_sprifle.mdl";
    static constexpr const char *W_Model = "models/w_sprifle.mdl";
    static constexpr const char *EventFile = "events/sprifle.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_sprifle";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 7;
	enum
	{
		ANIM_IDLE1 = 0,	
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};

    static constexpr int DamageDefault = 115;
    static constexpr int DamageZB = 170;
    static constexpr int DamageZBS = 170;
    static constexpr float AccuracyDefault = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;

    KnockbackData KnockBack = { 3000.0f, 280.0f, 700.0f, 450.0f, 0.3f };

	float GetArmorRatioModifier() override { return 1.95; }

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "338Magnum";
		p->iMaxAmmo1 = 30;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 7;
		p->iSlot = 0;
		p->iPosition = 16;
		p->iId = m_iId = WEAPON_SG550;
		p->iFlags = 0;
		p->iWeight = SG550_WEIGHT;

		return 1;
	}

	void StopIdleSound(void)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(79);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_SHORT(0);
		WRITE_STRING("weapons/sprifle_idle.wav");
		MESSAGE_END();
#endif
	}

	void Holster(int skiplocal)
	{
		CBasePlayerWeapon::Holster(skiplocal);

		StopIdleSound();
	}

	void PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Fire(0.8, 0.2s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			Fire(0.2, 0.2s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			Fire(0.001, 0.2s, FALSE);
		else
			Fire(0.003, 0.2s, FALSE);
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 210;

		return 150;
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_iShotsFired++;

		if (m_iShotsFired > 1)
		{
			m_iShotsFired = 1;

			return;
		}

		if (m_pPlayer->pev->fov == 90)
			flSpread += 0.05;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.5 + 0.4;

			if (m_flAccuracy > 0.99)
				m_flAccuracy = 0.99;
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

		StopIdleSound();

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread * (1 - m_flAccuracy), 8192, 7, BULLET_PLAYER_338MAG, GetDamage(), 0.99, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		if (m_pPlayer->m_iFOV == 90)
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 6.0, 8.0) + m_pPlayer->pev->punchangle.x * 0.25;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -4.0, 4.0);
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 4.0, 8.0) + m_pPlayer->pev->punchangle.x * 0.25;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 3.0);
		}
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_338magnum" , AMMO_338MAG_PRICE }; }
#endif
};

LINK_ENTITY_TO_CLASS(weapon_sprifle, CSprifle)
}