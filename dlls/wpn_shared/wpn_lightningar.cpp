/*
lightningar.cpp
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


#ifndef CLIENT_DLL
namespace sv {
	class CLightningARElectroField : public CBaseAnimating
	{
	public:
		float GetArmorRatioModifier() { return 1.5; }
		int GetWeaponsId(void) { return WEAPON_LIGHTNINGAR; }

		float GetRadius() const
		{
			float flRadius = 2.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 2.5f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 2.5f;
			return flRadius * 39.37;
		}

		duration_t GetBModeCoolTime() const
		{
			duration_t BModeCoolTime = 0.7s;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				BModeCoolTime = 0.3s;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				BModeCoolTime = 0.3s;
			return BModeCoolTime;
		}


		void Spawn() override
		{
			Precache();

			SetThink(&CLightningARElectroField::FollowThink);
			SetTouch(&CLightningARElectroField::OnTouch);
			SET_MODEL(this->edict(), "models/ef_lightningar.mdl");

			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			pev->framerate = 1.0;
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flRemoveTime = gpGlobals->time + 3.0s;
			m_flDamageTime = gpGlobals->time;

			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_lightningar.mdl");
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			pev->velocity = g_vecZero;

			SetTouch(nullptr);

			RadiusDamage(true);
		}



		void RadiusDamage(bool exp)
		{
			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (!pAttackePlayer)
				return;

			TraceResult tr;
			CBaseEntity* pEntity = NULL;
			Vector vecEnd;

			float flDamage = exp ? m_flTouchDamage : m_flRadiusDamage;
			float flRadius = GetRadius();
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
			{
				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (g_pGameRules->PlayerRelationship(pAttackePlayer, pEntity) == GR_TEAMMATE)
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				pEntity->m_LastHitGroup = HITGROUP_CHEST;
				pEntity->TakeDamage(pev, pAttackePlayer->pev, flDamage, DMG_BULLET);
			}

		}

		void FadeOut()
		{
			if (pev->renderamt > 10)
			{
				pev->renderamt -= 40.0f;
				pev->nextthink = gpGlobals->time + 0.1s;
			}
			else
			{
				pev->renderamt = 0.0f;
				pev->nextthink = gpGlobals->time + 0.01s;
				SetThink(&CLightningARElectroField::SUB_Remove);
			}
		}

		void EXPORT FollowThink()
		{
			pev->nextthink = gpGlobals->time + 0.1s;

			if (m_flRemoveTime < gpGlobals->time)
			{
				SetThink(&CLightningARElectroField::FadeOut);
			}

			if (m_flDamageTime < gpGlobals->time)
			{
				m_flDamageTime = gpGlobals->time + GetBModeCoolTime();
				RadiusDamage(false);
			}
		
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int type)
		{
			std::tie(m_flTouchDamage, m_flRadiusDamage, m_iType) = std::make_tuple(flTouchDamage, flExplodeDamage, type);
			pev->velocity = std::move(vecVelocity);

			if (m_iType)
				UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/lightningar-2_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/lightningar-3_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		}

		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f }; }

		time_point_t m_flRemoveTime;
		time_point_t m_flDamageTime;
		float m_flTouchDamage;
		float m_flRadiusDamage;
		int m_iType;

	};
	LINK_ENTITY_TO_CLASS(lightningar_electro_field, CLightningARElectroField)
}
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CLightningAR : public LinkWeaponTemplate< CLightningAR,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
			TPrimaryAttackRifle
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_lightningar.mdl";
    static constexpr const char *P_Model = "models/p_lightningar_a.mdl";
	static constexpr const char* P_Model_B = "models/p_lightningar_b.mdl";
    static constexpr const char *W_Model = "models/w_lightningar.mdl";
    static constexpr const char *EventFile = "events/lightningar.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_lightningar";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 50;
	enum
	{
		ANIM_IDLE1_EMPTY = 0,
		ANIM_IDLE1,
		ANIM_CHARGE_IDLE1,
		ANIM_CHARGE_IDLE2,
		ANIM_CHARGE_FINISH,
		ANIM_RELOAD_EMPTY,
		ANIM_RELOAD,
		ANIM_DRAW_EMPTY,
		ANIM_DRAW,
		ANIM_SHOOT_EMPTY,
		ANIM_CHARGE_SHOOT1,
		ANIM_CHARGE_SHOOT2,
		ANIM_CHARGE_SHOOT_LAST,
	};

    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.5f;

	static constexpr float PrimaryAttackWalkingMiniumSpeed = 130;
	static constexpr const auto& SpreadCalcNotOnGround = 0.04 * A + 0.035;
	static constexpr const auto& SpreadCalcWalking = 0.01 * A + 0.03;
	static constexpr const auto& SpreadCalcDefault = 0.02 * A;
	static constexpr const auto& SpreadCalcDucking = 0.01 * A;

	static constexpr const auto& ReloadTime = 2.4s;
	static constexpr const auto& CycleTime = 0.35s;
	static constexpr int DamageDefault = 36;
	static constexpr int DamageZB = 280;
	static constexpr int DamageZBS = 600;

	static constexpr float AccuracyDefault = 0.15;
	static constexpr float AccuracyMax = 1.8;

    KnockbackData KnockBack = { 500.0f, 300.0f, 300.0f, 120.0f, 0.5f };

	time_point_t m_flTimeCheckCharge;
	int m_iChargeState;
	int m_iDefaultAmmo2;
	int m_iChargeShotsFired;
	int m_iTargetShotsFired;

	void Spawn()
	{
		m_iDefaultAmmo2 = 0;
		m_iChargeState = 0;
		m_flTimeCheckCharge = invalid_time_point;
		m_iChargeShotsFired = 0;
		m_iTargetShotsFired = 10;
		Base::Spawn();
	}

    void Precache(void)
    {
        Base::Precache();
   
		PRECACHE_MODEL(P_Model_B);
		PRECACHE_MODEL("sprites/muzzleflash215.spr");
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "lightningar";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = "lightningar2";
        p->iMaxAmmo2 = 3;
        p->iMaxClip = 50;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_LIGHTNINGAR;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;
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

	float GetTouchDamage() const
	{
		float flDamage = 10.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 2250.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 3000.0f;
#endif
		return flDamage;
	}

	float GetRadiusDamage() const
	{
		float flDamage = 5.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 200.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 300.0f;
#endif
		return flDamage;
	}

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_lightningar" , 70 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { 0, 4, 3, 0 }; }

	void FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL isGlock18)
	{
		m_iClip--;
		m_iChargeShotsFired++;

		if (m_iChargeShotsFired >= m_iTargetShotsFired && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 3)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
			WRITE_BYTE(CBasePlayer::GetAmmoIndex("lightningar2"));
			WRITE_BYTE(1);
			MESSAGE_END();
#endif

			m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_B);

			m_iChargeShotsFired = 0;
		}

		if (m_iClip < 0)
		{
			m_iClip = 0;
			shotsFired = 2;
			shootTime = invalid_time_point;
			return;
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0, FALSE);

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		shotsFired++;

		if (shotsFired != 2)
		{
			shootTime = gpGlobals->time + 0.04s;
		}
		else
			shootTime = invalid_time_point;
	}


	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_iFamasShotsFired = 0;

		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210.0) + 0.18;

		if (m_flAccuracy > 1.8)
			m_flAccuracy = 1.8;

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
		m_iChargeShotsFired++;

		if (m_iChargeShotsFired >= m_iTargetShotsFired && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 3)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
			WRITE_BYTE(CBasePlayer::GetAmmoIndex("lightningar2"));
			WRITE_BYTE(1);
			MESSAGE_END();
#endif

			m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_B);

			m_iChargeShotsFired = 0;
		}

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;

		if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.4, 0.2, 0.05, 0.02, 2, 0.6, 2);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1, 0.45, 0.22, 0.18, 4, 3, 1);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.1, 0.1, 0.03, 0.01, 1.5, 0.3, 10);
		else
			KickBack(0.25, 0.2, 0.04, 0.01, 1.8, 0.5, 4);

		m_fBurstSpread = 0.016;
		m_iFamasShotsFired++;
		m_flFamasShoot = gpGlobals->time + 0.04s;

	}

	BOOL Deploy(void) override
	{
		m_flAccuracy = AccuracyDefault;
		m_iChargeState = 0;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
		{
			if (DefaultDeploy(V_Model, P_Model_B, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE))
			{
				return TRUE;
			}
		}
		else
		{
			if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW_EMPTY, AnimExtension, UseDecrement() != FALSE))
			{
				return TRUE;
			}
		}

		return FALSE;
	}


	void Reload(void)
	{

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(MaxClip, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? ANIM_RELOAD : ANIM_RELOAD_EMPTY, ReloadTime))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.24;
			m_bDelayFire = false;
			m_iShotsFired = 0;
			m_iTargetShotsFired = RANDOM_LONG(7, 11);
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();

		if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2))
		{
			OnAttack2Released();
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
		{
			SendWeaponAnim(ANIM_IDLE1);

			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningar_idle1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
		}
		else
		{
			SendWeaponAnim(ANIM_IDLE1_EMPTY);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
		}
	}

	void OnAttack2Released()
	{
		if (m_iChargeState > 1)
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningar-3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	
			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

			Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

			CLightningARElectroField* pEnt = static_cast<CLightningARElectroField*>(CBaseEntity::Create("lightningar_electro_field", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
			if (pEnt)
			{
				pEnt->Init(vecProjDir * 1000, GetTouchDamage(), GetRadiusDamage(), 1);
			}
#endif
		}
		else
		{
#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

			Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 100.0;;

			CLightningARElectroField* pEnt = static_cast<CLightningARElectroField*>(CBaseEntity::Create("lightningar_electro_field", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
			if (pEnt)
			{
				pEnt->Init(g_vecZero, GetTouchDamage(), GetRadiusDamage(), 0);
			}
#endif

			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningar-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

		}

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

		SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0 ? ANIM_CHARGE_SHOOT_LAST : ANIM_CHARGE_SHOOT2);

		m_iChargeShotsFired = 0;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.57s;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
			m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);

		m_iChargeState = 0;
	}

    void SecondaryAttack(void)
    {
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
			return;

		if (m_iChargeState)
		{
			if (gpGlobals->time > m_flTimeCheckCharge)
			{
				switch (m_iChargeState)
				{
				case 1:
				{			
					SendWeaponAnim(ANIM_CHARGE_FINISH);

					m_iChargeState = 2;
					m_flTimeCheckCharge = gpGlobals->time + 0.4s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
					break;
				}
				case 2:
				{
					SendWeaponAnim(ANIM_CHARGE_IDLE2);

					UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningar_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

					m_iChargeState = 3;
					m_flTimeCheckCharge = gpGlobals->time + 4.0s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
					break;
				}
				case 3:
				{
					UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/lightningar_idle3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

					m_flTimeCheckCharge = gpGlobals->time + 4.0s;
				}

				default:
					break;
				}
			}
		}
		else
		{

			SendWeaponAnim(ANIM_CHARGE_IDLE1);

			m_flTimeCheckCharge = gpGlobals->time + 0.5s;
			m_iChargeState = 1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		}
    }
		 	
};
LINK_ENTITY_TO_CLASS(weapon_lightningar, CLightningAR)
}