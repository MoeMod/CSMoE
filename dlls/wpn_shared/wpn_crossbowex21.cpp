/*
wpn_CCrossBowEX21.cpp - Port of Zombie IV Mod
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

#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define WPNSTATE_CROSSBOWEX_LED		(1<<1)
#define WPNSTATE_CROSSBOWEX_ZOOM	(1<<2)

    class CCrossBowEX21 : public LinkWeaponTemplate< CCrossBowEX21,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
            TPrecacheEvent,
            TDeployDefault,
			TPrimaryAttackRifle,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_crossbowex21.mdl";
    static constexpr const char *P_Model = "models/p_crossbowex21.mdl";
    static constexpr const char *W_Model = "models/w_crossbowex21.mdl";
    static constexpr const char *EventFile = "events/crossbowex21.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crossbowex21";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 50;
	enum
	{
		ANIM_IDLE,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_IDLE_LED,
		ANIM_SHOOT1_LED,
		ANIM_SHOOT2_LED,
		ANIM_RELOAD_LED,
		ANIM_ZOOM_IN,
		ANIM_ZOOM_IDLE,
		ANIM_ZOOM_SHOOT,
		ANIM_ZOOM_OUT,
		ANIM_ZOOM_OUT_LED,
	};
    static constexpr float PlayerMaxSpeed = 205;
    static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
    static constexpr int Penetration = 1;
	static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
	static constexpr const auto& SpreadCalcWalking = 0.025 * A + 0.07;
	static constexpr const auto& SpreadCalcDucking = 0.0 * A;
	static constexpr const auto& SpreadCalcDefault = 0.02 * A;
	static constexpr const auto& CycleTime = 0.165s;
	static constexpr int DamageDefault = 37;
	static constexpr int DamageZB = 375;
	static constexpr int DamageZBS = 520;
	static constexpr const auto& AccuracyCalc = (N * N * N / 210.0) + 0.3;
	static constexpr float AccuracyDefault = 0.19;
	static constexpr float AccuracyMax = 1.0;
	static constexpr float RangeModifier = 1.0;
	static constexpr auto BulletType = BULLET_PLAYER_556MM;
	/*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 1000.0f, 700.0f, 450.0f, 250.0f, 0.4f };

	time_point_t m_flTimePlayIdleSound;
	time_point_t m_flTimeHolster;
	time_point_t m_flTimeNextCharge;
	int m_iRecordAmmo;
	int m_iSummonModelIndex01;
	int m_iExpIndex;
	int m_iExpModelIndex;
	bool m_bChargeFinish;
	int m_iDefaultAmmo2;

#ifndef CLIENT_DLL
	std::vector<EHANDLE> m_pEnemylist;
#endif

	KnockbackData GetKnockBackData() override
	{
		//wingshoot 0.9
		if (m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON)
			return { 0.0f, 0.0f, 0.0f, 0.0f, 0.9f };
		else
			return { 1000.0f, 700.0f, 450.0f, 250.0f, 0.4f };
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_CCrossBowEX21" , 175 }; }
#endif
	float GetArmorRatioModifier() override { return 1.75; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

	void Spawn() override
	{
		m_bChargeFinish = false;
		m_iRecordAmmo = 0;
		m_iDefaultAmmo2 = 0;
		m_flTimeHolster = gpGlobals->time;
		m_flTimePlayIdleSound = invalid_time_point;
		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_SOUND("weapons/CCrossBowEX21-2.wav");
		PRECACHE_SOUND("weapons/CCrossBowEX21_special_on.wav");
		PRECACHE_SOUND("weapons/CCrossBowEX21_loop.wav");
		PRECACHE_SOUND("weapons/CCrossBowEX21_loop_end.wav");
		PRECACHE_MODEL("models/CCrossBowEX21_bullet.mdl");
		m_iExpModelIndex = PRECACHE_MODEL("models/ef_CCrossBowEX21_explosion.mdl");

		PRECACHE_MODEL("sprites/muzzleflash163.spr");
		PRECACHE_MODEL("sprites/muzzleflash165.spr");
		PRECACHE_MODEL("sprites/muzzleflash166.spr");
		PRECACHE_MODEL("sprites/muzzleflash169.spr");
		PRECACHE_MODEL("sprites/muzzleflash171.spr");

		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_particle.spr");	//hit wall
		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_ring.spr");	//attachment[2] client
		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_laserbeam.spr");
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_CCrossBowEX21_explosion.spr");

		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_charging2.spr");	//attachment[2]
		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_charging.spr");	//attachment[4/5]

		m_iSummonModelIndex01 = PRECACHE_MODEL("sprites/ef_CCrossBowEX21_summon01.spr");
		
		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "CCrossBowEX21BuckShot";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = "CrossBowEX21";
		p->iMaxAmmo2 = 10;
		p->iMaxClip = 50;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_CCrossBowEX21;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
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
		SetThink(nullptr);
		m_flTimeHolster = gpGlobals->time;
		RemoveWing();
		RemoveWingParticle();
		StopIdleSound();
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	BOOL Deploy(void)
	{
#ifndef CLIENT_DLL
		if (m_flTimeHolster != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHolster) / 2.0s;
		}
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;
#endif
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
		return DefaultDeploy("models/v_crossbowex21.mdl", "models/p_crossbowex21.mdl", m_iWeaponState & WPNSTATE_CROSSBOWEX_LED ? ANIM_DRAW_LED , ANIM_DRAW, "shotgun", UseDecrement() != FALSE, 0.1s, 1.0s);
	}

	void PrimaryAttack() override
	{
		if (m_iWeaponState & WPNSTATE_CROSSBOWEX_ZOOM && !m_pEnemylist.empty())
			PrimaryAttack_InstantDamage();
		else
			Base::PrimaryAttack();
	}

	void PrimaryAttack_InstantDamage()
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

#ifndef CLIENT_DLL
			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
			}
#endif
			return;
		}

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		for (CBaseEntity* pEntity : m_pEnemylist)
		{
			if (!pEntity)
				continue;

			Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
			Vector vecSrc = m_pPlayer->GetGunPosition();

			TraceResult tr;
			UTIL_TraceLine(vecSrc, vecDirection * 8192, missile, ENT(m_pPlayer->pev), &tr);

			float flDamage = GetDamage_PrimaryAttack_Instant() * (1 - tr.flFraction);

			ClearMultiDamage();
			pEntity->TraceAttack(m_pPlayer->pev, flDamage, vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_pPlayer->pev->origin);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(vecSrc.x);
			WRITE_COORD(vecSrc.y);
			WRITE_COORD(vecSrc.z);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/smoke.spr"));
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_BYTE(10);//Life
			WRITE_BYTE(7);//Width
			WRITE_BYTE(0);//Noise
			WRITE_BYTE(255);
			WRITE_BYTE(255);
			WRITE_BYTE(255);
			WRITE_BYTE(20);
			WRITE_BYTE(0);
			MESSAGE_END();
		}
#endif
	}

	void ItemPostFrame() override
	{
		if (!m_iWeaponState & WPNSTATE_CROSSBOWEX_ZOOM)
		{
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 10)
			{
				if (m_flTimeNextCharge <= gpGlobals->time)
				{
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
					m_flTimeNextCharge = gpGlobals->time + 2.0s;
				}
			}
		}

		if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
		{
			if (m_pPlayer->m_iFOV != 90)
			{
#ifndef CLIENT_DLL
				m_pEnemylist.clear();
#endif
				SendWeaponAnim(ANIM_SCAN_EACTIVATE, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
				m_iWeaponState = 0;
				m_flNextThink = invalid_time_point;
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
				pev->iuser1 = 0;
			}
			else
			{
				m_iWeaponState |= WPNSTATE_KRONOS12_ZOOM_ACTIVATE;
				m_flNextThink = gpGlobals->time + 5.0s;
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 65;
			}
		}

		if (m_iWeaponState & WPNSTATE_KRONOS12_ZOOM_ACTIVATE)
		{
			PrimaryAttack_FindTargets();
		}

		if (!m_iWeaponState)
		{
			if (pev->iuser1 >= KRONOS12_CHARGE_AMMO)
			{
				SendWeaponAnim(ANIM_SCAN_ACTIVATE, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
				m_iWeaponState |= WPNSTATE_KRONOS12_SCAN_ACTIVATE;
				pev->iuser1 = 0;
			}
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_CROSSBOWEX_ZOOM)
		{
#ifndef CLIENT_DLL
			m_pEnemylist.clear();
#endif
			
			m_iWeaponState &~ = WPNSTATE_CROSSBOWEX_ZOOM;
			if(m_iWeaponState & WPNSTATE_CROSSBOWEX_LED)
				SendWeaponAnim(ANIM_ZOOM_OUT_LED, UseDecrement() != FALSE);
			else
				SendWeaponAnim(ANIM_ZOOM_OUT, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
			m_flNextThink = invalid_time_point;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			return;
		}

		if (m_iWeaponState & WPNSTATE_CROSSBOWEX_LED)
		{
			SendWeaponAnim(ANIM_ZOOM_IN, UseDecrement() != FALSE);
			m_flNextThink = gpGlobals->time + 0.7s;
			m_pPlayer->m_flNextAttack = 0.7s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
		}
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		int iClipSize = 30;
		int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		if (!j)
			return;

		if (m_pPlayer->m_iFOV != 90)
		{
			m_iWeaponState = 0;
			m_flNextThink = invalid_time_point;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			pev->iuser1 = 0;
		}

		if (DefaultReload(50, m_iWeaponState & WPNSTATE_CROSSBOWEX_LED ? ANIM_RELOAD_LED : ANIM_RELOAD, 3.23s))
		{

#ifndef CLIENT_DLL
			m_pEnemylist.clear();
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		}
	}

	void WeaponIdle() override
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);		
#ifndef CLIENT_DLL
			if (m_pWing)
			{
				if (m_pWing->pev->body == 0 && m_pPlayer->pev->button & (IN_FORWARD) && m_pPlayer->pev->velocity.Length2D() > 235)
				{
					CreateWingParticle();
					m_pWing->pev->body = 1;
				}
				else
				{
					RemoveWingParticle();
					m_pWing->pev->body = 0;
				}				
			}			
#endif

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON)
			SendWeaponAnim(ANIM_WINGLOOP, UseDecrement() != FALSE);
		else
			SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	}

	void StopIdleSound(void)
	{
#ifndef CLIENT_DLL
		int player = m_pPlayer->entindex();

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(79);
		WRITE_SHORT(player);
		WRITE_SHORT(CHAN_ITEM);
		WRITE_STRING("weapons/CCrossBowEX21_loop.wav");
		MESSAGE_END();
#endif
	}

	float GetSecondaryDamage()
	{
		float flDamage = 37;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2620;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4100;
#endif
		return flDamage;
	}


};

LINK_ENTITY_TO_CLASS(weapon_CCrossBowEX21, CCrossBowEX21)

}