/*
balrog1.cpp
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
    class CBalrog1 : public LinkWeaponTemplate< CBalrog1,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TPrimaryAttackRifle,
            TFirePistol,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_balrog1.mdl";
    static constexpr const char *P_Model = "models/p_balrog1.mdl";
    static constexpr const char *W_Model = "models/w_balrog1.mdl";
    static constexpr const char *EventFile = "events/balrog1.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_balrog1";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 10;

#define WPNSTATE_BALROG1_MODEB	1

	enum
	{
		ANIM_IDLEA = 0,
        ANIM_IDLEB,
        ANIM_SHOOTA,
		ANIM_SHOOTB,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_CHANGEA,
		ANIM_CHANGEB,
		ANIM_RELOADB,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.1*(1-A);
    static constexpr const auto &SpreadCalcWalking = 0.045*(1-A);
    static constexpr const auto& SpreadCalcDucking = 0.035*(1-A);
    static constexpr const auto &SpreadCalcDefault = 0.04*(1-A);

	static constexpr const auto& CycleTime = 0.22s;
    static constexpr int DamageDefault = 46;
    static constexpr int DamageZB = 74;
    static constexpr int DamageZBS = 175;

    static constexpr float AccuracyDefault = 0.95;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 400.0f, 300.0f, 350.0f, 150.0f, 0.6f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50aeahe" , 90 }; }
#endif
	float GetArmorRatioModifier() override { return 1.5; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int m_iExpIndex;

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50aeahe";
		p->iMaxAmmo1 = 90;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 10;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_BALROG1;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	}

	void Precache(void)
	{
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_balrog1.spr");
		Base::Precache();
	}

	void SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_BALROG1_MODEB)
		{
			m_iWeaponState &= ~WPNSTATE_BALROG1_MODEB;
			SendWeaponAnim(ANIM_CHANGEB, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3s;
		}
		else
		{
			m_iWeaponState |= WPNSTATE_BALROG1_MODEB;
			SendWeaponAnim(ANIM_CHANGEA, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2s;
		}
	}

	BOOL Deploy()
	{
		m_iWeaponState &= ~WPNSTATE_BALROG1_MODEB;
		return Base::Deploy();
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		int iClipSize = 10;
		int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		if (!j)
			return;

		int iAnim = ANIM_RELOAD;
		duration_t flReloadTime = 2.2s;
		if (m_iWeaponState & WPNSTATE_BALROG1_MODEB)
		{
			m_iWeaponState &= ~WPNSTATE_BALROG1_MODEB;
			iAnim = ANIM_RELOADB;
			flReloadTime = 3.0s;
		}


		if (DefaultReload(10, iAnim, flReloadTime))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.95;
		}
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		m_iShotsFired++;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.375 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.315;

			if (m_flAccuracy > 0.95)
				m_flAccuracy = 0.95;
			else if (m_flAccuracy < 0.57)
				m_flAccuracy = 0.57;
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

#ifndef CLIENT_DLL
		if (m_iWeaponState & WPNSTATE_BALROG1_MODEB)
		{
			m_iWeaponState &= ~WPNSTATE_BALROG1_MODEB;
			flCycleTime = 3.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3s;
			Balrog1Explosion(vecSrc, gpGlobals->v_forward, 0.0, 8192, 0.0, 135, m_pPlayer->pev, m_pPlayer->random_seed);
		}
		else
#endif
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
		}

		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, GetDamage(), 0.81, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

#ifdef CLIENT_DLL
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_BALROG1_MODEB) ? TRUE : FALSE, FALSE);
#else
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		
		m_pPlayer->pev->punchangle.x -= 2;
    }

	float GetBuffDamage() const
	{
		float flDamage = 1.0;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 30.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 85.0f;
#endif
		return flDamage;
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_BALROG1_MODEB)
			SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
		else
			SendWeaponAnim(ANIM_IDLEA, UseDecrement() != FALSE);
	}

#ifndef CLIENT_DLL
	Vector Balrog1Explosion(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, float flDamage, float flRadius, entvars_t* pevAttacker, int random_seed)
	{
		ClearMultiDamage();
		gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

		entvars_t* pevAttack = pevAttacker;

		if (pevAttacker == NULL)
			pevAttack = pev;

		Vector vecRight = gpGlobals->v_right;
		Vector vecUp = gpGlobals->v_up;

		float x, y, z;

		if (IsPlayer())
		{
			x = UTIL_SharedRandomFloat(random_seed, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 1, -0.5, 0.5);
			y = UTIL_SharedRandomFloat(random_seed + 2, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 3, -0.5, 0.5);
		}
		else
		{
			do
			{
				x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);
		}

		TraceResult tr;
		Vector vecDir = vecDirShooting + x * flSpread * vecRight + y * flSpread * vecUp;
		Vector vecEnd = vecSrc + vecDir * flDistance;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z + 10.0);
		WRITE_SHORT(m_iExpIndex);
		WRITE_BYTE(2 - int(flRadius * -0.1));
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		bool bInWater = UTIL_PointContents(tr.vecEndPos) == CONTENTS_WATER;
		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr.vecEndPos, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (pEntity->pev == pevAttack)
				continue;

			pEntity->TakeDamage(pevAttack, pevAttack, UTIL_CalcDamage(tr.vecEndPos, pEntity->pev->origin, flDamage, flRadius, NULL), DMG_EXPLOSION);

			if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
			{
				if (pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
					WRITE_BYTE(42);
					WRITE_SHORT(pEntity->entindex());
					WRITE_COORD(RANDOM_FLOAT(-5.0, 5.0));
					WRITE_COORD(RANDOM_FLOAT(-5.0, 5.0));
					WRITE_COORD(0.0);
					WRITE_SHORT(30);
					MESSAGE_END();

					pEntity->m_pBuffAttacker = m_pPlayer->pev;
					pEntity->m_iHealthDecreaseCount = 10;
					pEntity->m_flHealthDecreaseAmount = GetBuffDamage();
					pEntity->m_flHealthDecreaseInterval = 1.0s;
					pEntity->m_flHealthDecreaseStartTime = gpGlobals->time + 1.0s;
				}
				else
				{
					CBasePlayer* pInflictorPlayer = static_cast<CBasePlayer*>(pEntity);

					if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
					{
						if (!pInflictorPlayer->m_iHealthDecreaseCount) // fire burn
						{
							MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
							WRITE_BYTE(42);
							WRITE_SHORT(pEntity->entindex());
							WRITE_COORD(RANDOM_FLOAT(-5.0, 5.0));
							WRITE_COORD(RANDOM_FLOAT(-5.0, 5.0));
							WRITE_COORD(0.0);
							WRITE_SHORT(15);
							MESSAGE_END();

							pEntity->m_pBuffAttacker = m_pPlayer->pev;
							pEntity->m_iHealthDecreaseCount = 10;
							pEntity->m_flHealthDecreaseAmount = GetBuffDamage();
							pEntity->m_flHealthDecreaseInterval = 1.0s;
							pEntity->m_flHealthDecreaseStartTime = gpGlobals->time + 1.0s;
						}
					}
				}
			}

		}

		return tr.vecEndPos;
	}
#endif
};
LINK_ENTITY_TO_CLASS(weapon_balrog1, CBalrog1)
}