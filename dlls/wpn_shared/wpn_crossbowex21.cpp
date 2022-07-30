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

#define WPNSTATE_CROSSBOWEX_LED		1
#define WPNSTATE_CROSSBOWEX_ZOOM	2

    class CCrossBowEX21 : public LinkWeaponTemplate< CCrossBowEX21,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
            TPrecacheEvent
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_crossbowex21.mdl";
    static constexpr const char *P_Model = "models/p_crossbowex21.mdl";
    static constexpr const char *W_Model = "models/w_crossbowex21.mdl";
    static constexpr const char *EventFile = "events/crossbowex21.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crossbowex21";
    static constexpr const char *AnimExtension = "mp5";
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
		ANIM_DRAW_LED,
		ANIM_ZOOM_IN,
		ANIM_ZOOM_IDLE,
		ANIM_ZOOM_SHOOT,
		ANIM_ZOOM_OUT,
		ANIM_ZOOM_OUT_LED,
	};
    static constexpr float PlayerMaxSpeed = 250;
    KnockbackData KnockBack = { 1000.0f, 700.0f, 450.0f, 250.0f, 0.4f };

	time_point_t m_flTimeHolster;
	time_point_t m_flTimeNextCharge;
	time_point_t m_flNextThink;

	int m_iBeam;
	int m_iExpIndex;
	int m_iDefaultAmmo2;

#ifndef CLIENT_DLL
	std::vector<EHANDLE> m_pEnemylist;
	std::vector<EHANDLE> m_pGroupList;
#endif

	KnockbackData GetKnockBackData() override
	{
		return { 1000.0f, 700.0f, 450.0f, 250.0f, 0.4f };
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_crossbowex21" , 300 }; }
#endif
	float GetArmorRatioModifier() override { return 1.75; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

	void Spawn() override
	{
		m_iDefaultAmmo2 = 0;
		m_flTimeHolster = gpGlobals->time;
		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_SOUND("weapons/crossbowex21_beep.wav");
		PRECACHE_SOUND("weapons/crossbowex21_zoom.wav");
		PRECACHE_SOUND("weapons/crossbowex21-1.wav");
		PRECACHE_SOUND("weapons/crossbowex21-2.wav");
		PRECACHE_SOUND("weapons/crossbowex21-1_exp.wav");
		PRECACHE_SOUND("weapons/crossbowex21-2_exp.wav");
		
		PRECACHE_MODEL("sprites/muzzleflash172.spr");
		PRECACHE_MODEL("sprites/muzzleflash173.spr");

		PRECACHE_MODEL("sprites/ef_crossbowex_hit_1.spr");	//hit wall
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_crossbowex_hit_2.spr");	//attachment[2] client
		PRECACHE_MODEL("sprites/ef_crossbowex_shoot1.spr");
		m_iBeam = PRECACHE_MODEL("sprites/ef_crossbowex_shoot2.spr");
		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "crossbowex21";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = "crossbowex21_b";
		p->iMaxAmmo2 = 10;
		p->iMaxClip = 50;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_CROSSBOWEX21;
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
		m_iWeaponState &= ~WPNSTATE_CROSSBOWEX_ZOOM;
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	BOOL Deploy(void)
	{
		if (m_flTimeHolster != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHolster) / 1.0s;
		}		

		ResetStateLED();

		return DefaultDeploy("models/v_crossbowex21.mdl", "models/p_crossbowex21.mdl", m_iWeaponState & WPNSTATE_CROSSBOWEX_LED ? ANIM_DRAW_LED : ANIM_DRAW, "mp5", UseDecrement() != FALSE);
	}

	void ResetStateLED()
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 10)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 10;
			m_iWeaponState |= WPNSTATE_CROSSBOWEX_LED;
		}
		else
			m_iWeaponState &= ~WPNSTATE_CROSSBOWEX_LED;
	}

	void PrimaryAttack() override
	{
		duration_t flCycleTime;

		if (m_pPlayer->pev->fov == 90)
			flCycleTime = 0.3s;
		else
			flCycleTime = 1.0s;

		if (m_pPlayer->pev->fov == 90)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				CrossbowEX21Fire((0.04) * m_flAccuracy + 0.035, flCycleTime, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 120)
				CrossbowEX21Fire((0.001) * m_flAccuracy + 0.02, flCycleTime, FALSE);
			else if (m_pPlayer->pev->fov != 90)
				CrossbowEX21Fire((0.0) * m_flAccuracy + 0.0, flCycleTime, FALSE);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				CrossbowEX21Fire((0.001) * m_flAccuracy + 0.01, flCycleTime, FALSE);
			else
				CrossbowEX21Fire((0.001) * m_flAccuracy + 0.018, flCycleTime, FALSE);
		}
		else
		{
#ifndef CLIENT_DLL
			PrimaryAttack_InstantDamage();
#endif
		}
	}

	void CrossbowEX21Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoaim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;

		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210.0 + 0.25;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

#ifndef CLIENT_DLL
			if (TheBots)
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif
			return;
		}

		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_iClip--;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192.0, 1, BULLET_PLAYER_556MM, GetDamage(), 1.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->pev->fov == 90 ? true : false, m_iWeaponState & WPNSTATE_CROSSBOWEX_LED);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.25, 0.25, 0.065, 0.065, 1.75, 1.75, 3);
		else if (m_pPlayer->pev->fov != 90)
			KickBack(0.11, 0.11, 0.06, 0.06, 0.6, 0.6, 2);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.1, 0.1, 0.05, 0.05, 0.5, 0.5, 1);
		else
			KickBack(0.2, 0.2, 0.06, 0.06, 1.0, 1.0, 2);
	}

	bool PrimaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection)
	{
		const float flRadius = 4096;

		if (!a2->IsPlayer() && a2->Classify() != CLASS_PLAYER_ALLY)
			return false;

		if (!a2->IsAlive())
			return false;

#ifndef CLIENT_DLL
		if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
			return false;
#endif

		if (a2->pev == m_pPlayer->pev)
			return false;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDelta = (a2->Center() - vecSrc).Normalize();

		UTIL_MakeVectors(vecAngleDirection);

		if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
			return false;

		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->pev->origin, a2->pev->origin, missile, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0)
		{
			if (!tr.pHit)
				return false;

			if (tr.pHit != a2->edict())
				return false;
		}

		return true;
	}

	void PrimaryAttack_FindTargets()
	{
#if 1
#ifndef CLIENT_DLL
		const float flRadius = 4096;

		m_pEnemylist.erase(std::remove_if(m_pEnemylist.begin(), m_pEnemylist.end(), [=](CBaseEntity* pEntity) {

			if (!pEntity->IsAlive() || bIsWallBetweenEntity(pEntity, m_pPlayer))
				return true;

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);

			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecDelta = (pEntity->Center() - vecSrc).Normalize();

			if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
				return true;

			return false;
			}), m_pEnemylist.end());


		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, flRadius)) != nullptr)
		{
			EHANDLE eh;
			eh.Set(pEntity->edict());

			if (std::find(m_pEnemylist.begin(), m_pEnemylist.end(), eh) != m_pEnemylist.end())
				continue;

			if (PrimaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
			{
				m_pEnemylist.push_back(eh);
			}
		}
#endif
#endif
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
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
		Vector vecDir = gpGlobals->v_forward;
		
		if (!m_pEnemylist.empty())
		{
			CBaseEntity* pEnt = NULL;
			float flMinDist = 9999.0f;
			for (CBaseEntity* pEntity : m_pEnemylist)
			{
				if (!pEntity)
					continue;

				if (std::find(m_pGroupList.begin(), m_pGroupList.end(), pEntity) != m_pGroupList.end())
					continue;

				Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin);

				float flDelta = vecDirection.Length();
				if (flDelta < flMinDist)
				{
					flMinDist = flDelta;
					vecDir = vecDirection.Normalize();
					pEnt = pEntity;
				}
			}

			if (pEnt)
			{
				EHANDLE eh;
				eh.Set(pEnt->edict());

				m_pGroupList.push_back(eh);

				UTIL_TempModel(pEnt->pev->origin, g_vecZero, g_vecZero, m_iExpIndex, 10, 0, 255, false, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 6, 0);
			}
			else
			{
				m_pGroupList.clear();
				float flMinDist = 9999.0f;
				for (CBaseEntity* pEntity : m_pEnemylist)
				{
					if (!pEntity)
						continue;

					if (std::find(m_pGroupList.begin(), m_pGroupList.end(), pEntity) != m_pGroupList.end())
						continue;

					Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin);

					float flDelta = vecDirection.Length();
					if (flDelta < flMinDist)
					{
						flMinDist = flDelta;
						vecDir = vecDirection.Normalize();
						pEnt = pEntity;
					}
				}

				if (pEnt)
				{
					EHANDLE eh;
					eh.Set(pEnt->edict());

					m_pGroupList.push_back(eh);

					UTIL_TempModel(pEnt->pev->origin, g_vecZero, g_vecZero, m_iExpIndex, 10, 0, 255, false, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 6, 0);
				}
			}
		}

		Vector vecSrc = m_pPlayer->GetGunPosition();
	
		PLAYBACK_EVENT_FULL(FEV_RELIABLE, m_pPlayer->edict(), m_usFire, 0.0, m_pPlayer->pev->origin, vecDir, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->m_iFOV == 90, m_iWeaponState & WPNSTATE_CROSSBOWEX_LED);

		vecDir = m_pPlayer->FireBullets3(vecSrc, vecDir, 0, 8192.0, 6, BULLET_PLAYER_556MM, GetSecondaryDamage(), 1.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

		if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;

		if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
			m_flNextThink = gpGlobals->time + 0.2s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.25, 0.25, 0.065, 0.065, 1.75, 1.75, 3);
		else if (m_pPlayer->pev->fov != 90)
			KickBack(0.11, 0.11, 0.06, 0.06, 0.6, 0.6, 2);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.1, 0.1, 0.05, 0.05, 0.5, 0.5, 1);
		else
			KickBack(0.2, 0.2, 0.06, 0.06, 1.0, 1.0, 2);
	}

	void ItemPostFrame() override
	{
		if (!(m_iWeaponState & WPNSTATE_CROSSBOWEX_ZOOM))
		{
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 10)
			{
				if (m_flTimeNextCharge <= gpGlobals->time)
				{
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

#ifndef CLIENT_DLL
					MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
					WRITE_BYTE(CBasePlayer::GetAmmoIndex("crossbowex21_b"));
					WRITE_BYTE(1);
					MESSAGE_END();
#endif

					if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 10)
					{
						m_iWeaponState |= WPNSTATE_CROSSBOWEX_LED;

						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;

						EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/crossbowex21_beep.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
					}
					else
						m_flTimeNextCharge = gpGlobals->time + 1.0s;
				}
			}		
		}

		if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
		{
			if (m_pPlayer->pev->fov != 90)
			{
#ifndef CLIENT_DLL
				m_pEnemylist.clear();
				m_pGroupList.clear();
#endif
				SendWeaponAnim(ANIM_ZOOM_OUT, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
				m_iWeaponState = 0;
				m_flNextThink = invalid_time_point;
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			}
			else
			{
				m_iWeaponState |= WPNSTATE_CROSSBOWEX_ZOOM;

				EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/crossbowex21_zoom.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 89;
				m_flNextThink = invalid_time_point;
			}
		}

		if (m_iWeaponState & WPNSTATE_CROSSBOWEX_ZOOM)
		{
			PrimaryAttack_FindTargets();
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_CROSSBOWEX_ZOOM)
		{
#ifndef CLIENT_DLL
			m_pEnemylist.clear();
			m_pGroupList.clear();
#endif	
			ResetStateLED();

			m_iWeaponState &= ~WPNSTATE_CROSSBOWEX_ZOOM;
			if(m_iWeaponState & WPNSTATE_CROSSBOWEX_LED)
				SendWeaponAnim(ANIM_ZOOM_OUT_LED, UseDecrement() != FALSE);
			else
				SendWeaponAnim(ANIM_ZOOM_OUT, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
			m_flNextThink = invalid_time_point;

			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			m_flTimeNextCharge = gpGlobals->time + 1.0s;
			return;
		}

		if (m_iWeaponState & WPNSTATE_CROSSBOWEX_LED)
		{
			SendWeaponAnim(ANIM_ZOOM_IN, UseDecrement() != FALSE);
			m_flNextThink = gpGlobals->time + 0.6s;
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

		if (m_pPlayer->pev->fov != 90)
		{
			int iClipSize = 50;
			int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

			if (!j)
			{
				SecondaryAttack();
				return;
			}			
		}

		ResetStateLED();
		m_iWeaponState &= ~WPNSTATE_CROSSBOWEX_ZOOM;

		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;

		if (DefaultReload(50, m_iWeaponState & WPNSTATE_CROSSBOWEX_LED ? ANIM_RELOAD_LED : ANIM_RELOAD, 3.23s))
		{
#ifndef CLIENT_DLL
			m_pEnemylist.clear();
			m_pGroupList.clear();
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flTimeNextCharge = gpGlobals->time + 1.0s;
		}
	}

	void WeaponIdle() override
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
			if (m_iWeaponState & WPNSTATE_CROSSBOWEX_LED)
			{
				if (m_iWeaponState & WPNSTATE_CROSSBOWEX_ZOOM)
					SendWeaponAnim(ANIM_ZOOM_IDLE, UseDecrement() != FALSE);
				else
					SendWeaponAnim(ANIM_IDLE_LED, UseDecrement() != FALSE);
			}
			else
			{
				SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
			}
		}
	}


	float GetDamage()
	{
		float flDamage = 37;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 180;	//370
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 520;
#endif
		return flDamage;
	}

	float GetSecondaryDamage()
	{
		float flDamage = 37;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1860;	//2620
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4100;
#endif
		return flDamage;
	}


};

LINK_ENTITY_TO_CLASS(weapon_crossbowex21, CCrossBowEX21)

}