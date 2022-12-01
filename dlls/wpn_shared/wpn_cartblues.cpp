/*
z4b_cartblues.cpp - Port of Zombie IV Mod
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
#include "weapons/RadiusDamage.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CCartBlueS : public LinkWeaponTemplate< CCartBlueS,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG550>::template type,
            TPrecacheEvent,
            TReloadDefault,
			TSecondaryAttackZoom,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.2s;
    static constexpr int ZoomFOV = 25;
    static constexpr const char *V_Model = "models/v_cartblue.mdl";
    static constexpr const char *P_Model = "models/p_cartblue.mdl";
    static constexpr const char *W_Model = "models/w_cartblue.mdl";
    static constexpr const char *EventFile = "events/cartblues.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_cartblues";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE_L,
		ANIM_RELOAD_L,
		ANIM_DRAW_L,
		ANIM_SHOOT1_L,
		ANIM_SHOOT2_L,
		ANIM_SHOOT3_L,
		ANIM_CHANGE_H,
		ANIM_IDLE1,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1_H,
		ANIM_CHANGE_L,
	};

	static constexpr float AccuracyDefault = 0.1;

    KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };

	float GetArmorRatioModifier() override { return 1.4; }

	void Spawn()
	{
		Base::Spawn();
		m_pLink = nullptr;
		m_iSwing = 0;
	}

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "556Nato";
		p->iMaxAmmo1 = 90;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 30;
		p->iSlot = 0;
		p->iPosition = 12;
		p->iId = m_iId = WEAPON_CARTBLUES;
		p->iFlags = 0;
		p->iWeight = SG550_WEIGHT;

		return 1;
	}


#ifndef CLIENT_DLL
	void AttachToPlayer(CBasePlayer* pPlayer)
	{
		CBasePlayerWeapon::AttachToPlayer(pPlayer);

		// find the linked weapon
		while (!m_pLink)
		{
			for (int i = 0; i < MAX_ITEM_TYPES; ++i)
			{
				CBasePlayerItem* pItem = m_pPlayer->m_rgpPlayerItems[i];
				while (pItem != NULL)
				{
					if (!Q_stricmp(pItem->pszName(), "weapon_cartbluec"))
					{
						m_pLink = pItem;

						m_pLink->m_iSwing = 1;
						m_iSwing = 0;
					}

					pItem = pItem->m_pNext;
				}
			}
			if (!m_pLink)
				pPlayer->GiveNamedItem("weapon_cartbluec");

		}
	}

	~CCartBlueS()
	{
		CBaseEntity* pOther = m_pLink;
		if (pOther && pOther->m_pLink == this)
		{
			pOther->m_pLink = m_pLink = nullptr;
			pOther->SUB_Remove();
		}
	}
#endif


	BOOL Deploy() override
	{
		BOOL bResult;

		m_iShotsFired = 0;
		m_bDelayFire = false;
		m_flAccuracy = 0.1;

		if (m_iSwing)
		{
			bResult = DefaultDeploy(V_Model, P_Model, ANIM_CHANGE_H, "mp5", UseDecrement() != FALSE);
			m_pPlayer->m_flNextAttack = 5.0s;
		}
		else
		{
			bResult = DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "carbine", UseDecrement() != FALSE);
		}

		return bResult;
	}


	void ItemPostFrame()
	{
#ifndef CLIENT_DLL
		m_iSwing = 0;
		if (m_pLink)
			m_pLink->m_iSwing = 1;
#endif
		return CBasePlayerWeapon::ItemPostFrame();
	}


	void PrimaryAttack(void)
	{
		duration_t flCycleTime = 0.22s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Fire(0.65, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			Fire(0.1, flCycleTime, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			Fire(0.01, flCycleTime, FALSE);
		else
			Fire(0.04, flCycleTime, FALSE);
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 200;
		return 165;
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov == 90)
			flSpread += 0.05;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.3 + 0.4;

			if (m_flAccuracy > 0.6)
				m_flAccuracy = 0.6;
		}

		m_flLastFire = gpGlobals->time;
		flSpread *= (1.0 - m_flAccuracy);

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

		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_556MM, GetDamage(), 0.96, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;


		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 0.48, 0.8) + m_pPlayer->pev->punchangle.x * 0.75;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -0.85, 0.5);

	}

	float GetDamage() const
	{
		float flDamage = 115.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 250.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 550.0f;
#endif
		return flDamage;
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif

};
LINK_ENTITY_TO_CLASS(weapon_cartblues, CCartBlueS)
}