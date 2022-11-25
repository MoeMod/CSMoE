/*
laserminigun.cpp
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
	class CLasermingunChargeAmmo : public CBaseEntity
	{
	public:
		CLasermingunChargeAmmo()
		{
			m_iState = 0;
		}

		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f }; }
		float GetArmorRatioModifier() { return 1.5; }
		int GetWeaponsId(void) { return WEAPON_LASERMINIGUN; }

		static CLasermingunChargeAmmo* Create(int iState, edict_t* pentOwner)
		{
			edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("laserminiguncharge"));

			if (FNullEnt(pent))
			{
				ALERT(at_console, "NULL Ent in Create!\n");
				return NULL;
			}

			CLasermingunChargeAmmo* pAmmo = (CLasermingunChargeAmmo*)Instance(pent);

			if (pAmmo)
			{
				pAmmo->m_iState = iState;
				pAmmo->pev->owner = pentOwner;
				pAmmo->Spawn();
			}

			return pAmmo;
		}

		void Spawn() override
		{
			Precache();

			SetThink(&CLasermingunChargeAmmo::IgniteThink);
			
			SET_MODEL(this->edict(), "sprites/laserminigun1.spr");

			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_FOLLOW;
			pev->aiment = pev->owner;
			pev->body = 1;

			pev->animtime = gpGlobals->time;
			pev->framerate = 30.0;
			pev->scale = 0.15;
			pev->frame = 0;
			m_flNumFrames = 8.0;

			m_iChargeTime = 4;

			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			pev->nextthink = gpGlobals->time + 0.1s;
			m_flChargeTime = gpGlobals->time + 0.5s;
		}

		void Launch()
		{
			SetThink(NULL);

			pev->aiment = nullptr;
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->solid = SOLID_BBOX;

			SetTouch(&CLasermingunChargeAmmo::SpearTouch);
			SetThink(&CLasermingunChargeAmmo::FollowThink);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void EXPORT FollowThink()
		{
			pev->nextthink = gpGlobals->time + 0.1s;

			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flNumFrames)
				pev->frame = fmod(pev->frame, m_flNumFrames);

			if (!IsInWorld())
			{
				SetTouch(NULL);
				SetThink(NULL);

				UTIL_Remove(this);
				return;
			}
		}

		void EXPORT IgniteThink()
		{
			pev->nextthink = gpGlobals->time + 0.02s;

			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flNumFrames)
				pev->frame = fmod(pev->frame, m_flNumFrames);

			if (m_iChargeTime > 0)
			{
				if (m_flChargeTime != invalid_time_point && m_flChargeTime < gpGlobals->time)
				{	
					m_iChargeTime--;
					if (m_iChargeTime == 1)
					{
						m_iState = 1;
						SET_MODEL(this->edict(), "sprites/laserminigun2.spr");

						UTIL_SetSize(pev, g_vecZero, g_vecZero);

					}

					if (m_iChargeTime == 0)
					{
						m_flChargeTime = gpGlobals->time + 1.2s;
						return;
					}

					pev->scale += 0.08;
					m_flChargeTime = gpGlobals->time + 0.4s;				
				}
			}
			else
			{
				if (m_flChargeTime != invalid_time_point && m_flChargeTime < gpGlobals->time)
				{
					m_iState = 2;
					SET_MODEL(this->edict(), "sprites/laserminigun3.spr");

					UTIL_SetSize(pev, g_vecZero, g_vecZero);

					m_flChargeTime = invalid_time_point;
				}
			}

		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/laserminigun_hit.spr");
			PRECACHE_MODEL("sprites/laserminigun_hit1.spr");
			PRECACHE_MODEL("sprites/laserminigun_hit2.spr");
			PRECACHE_MODEL("sprites/laserminigun_hit3.spr");
			PRECACHE_MODEL("sprites/laserminigun1.spr");
			PRECACHE_MODEL("sprites/laserminigun2.spr");
			PRECACHE_MODEL("sprites/laserminigun3.spr");

			PRECACHE_SOUND("weapons/laserminigun_exp1.wav");
			PRECACHE_SOUND("weapons/laserminigun_exp2.wav");
			PRECACHE_SOUND("weapons/laserminigun_exp3.wav");

			PRECACHE_SOUND("weapons/laserminigun-charge_shoot.wav");
		}

		void RadiusDamage()
		{
			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackPlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

			if (!pAttackPlayer)
				return;

			if (pAttackPlayer->m_bIsZombie)
				return;

			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
			float flDamage = GetScytheDamage();
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 32.0)) != NULL)
			{
				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->edict() == pev->owner)
					continue;

				if (UTIL_IsWallBetweenEntity(pEntity, this))
					continue;

				pEntity->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_BULLET);
			}
		}

		void EXPORT SpearTouch(CBaseEntity* pOther)
		{
			if (!pOther)
				return;

			if (pOther->edict() == pev->owner)
				return;

			SetTouch(NULL);
			SetThink(NULL);

			float flDamage = GetDirectDamage();

			entvars_t* pevOwner = VARS(pev->owner);

			if (!pOther->IsDormant())
			{
				if (pOther->pev->takedamage != DAMAGE_NO)
				{
					TraceResult tr = UTIL_GetGlobalTrace();

					ClearMultiDamage();
					tr.iHitgroup = HITGROUP_CHEST;
					pOther->TraceAttack(pevOwner, flDamage, pev->velocity, &tr, DMG_BULLET | DMG_SLASH);
					ApplyMultiDamage(pev, pevOwner);
				}
			}

			UTIL_Remove(this);
		}

		CBasePlayer* m_pOwner;
		int m_iState;
		int m_iChargeTime;
		float m_flNumFrames;
		time_point_t m_flChargeTime;

	private:
		float GetDirectDamage()
		{
			float flDamage = 75;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 850;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 2050;
			return flDamage;
		}

		float GetScytheDamage()
		{
			float flDamage = 65;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 520;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1350;
			return flDamage;
		}

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};

	LINK_ENTITY_TO_CLASS(laserminiguncharge, CLasermingunChargeAmmo)
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CLaserMiniGun : public LinkWeaponTemplate< CLaserMiniGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 4.7s;
    static constexpr const char *V_Model = "models/v_laserminigun.mdl";
    static constexpr const char *P_Model = "models/p_laserminigun.mdl";
    static constexpr const char *W_Model = "models/w_laserminigun.mdl";
    static constexpr const char *EventFile = "events/laserminigun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_laserminigun";
    static constexpr const char *AnimExtension = "laserminigun";
    static constexpr int MaxClip = 200;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_CHARGE_ORIGIN,
        ANIM_CHARGE_START,
        ANIM_CHARGE_LOOP,
        ANIM_CHARGE_SHOOT,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_CHARGE_END,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.045;
    static constexpr const auto &SpreadCalcWalking = 0.095*A + 0.045;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 32;
    static constexpr int DamageZB = 38;
    static constexpr int DamageZBS = 38;
    static constexpr const auto &AccuracyCalc = (N * N * N / 175.0) + 0.4;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.9;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.1, 0.5, 0.3, 0.06, 4.0, 3.0, 8 };
	KickBackData KickBackNotOnGround = { 1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8 };
	KickBackData KickBackDucking = { 0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9 };
	KickBackData KickBackDefault = { 0.8, 0.35, 0.3, 0.03, 3.75, 3.0, 9 };

#ifndef CLIENT_DLL
	EntityHandle <CLasermingunChargeAmmo> m_pChargeAmmo;
#endif

	time_point_t m_flTimeCheckCharge;
	int m_iChargeState;

	void Spawn()
	{
		m_iChargeState = 0;
		m_flTimeCheckCharge = invalid_time_point;
		Base::Spawn();
	}

    void Precache(void)
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/laserminigun-charge_start.wav");
        PRECACHE_SOUND("weapons/laserminigun-charge_origin.wav");
        PRECACHE_SOUND("weapons/laserminigun-charge_shoot.wav");

        PRECACHE_SOUND("weapons/laserminigun-1.wav");

		PRECACHE_MODEL("sprites/muzzleflash37.spr");
		PRECACHE_MODEL("sprites/muzzleflash38.spr");
		PRECACHE_MODEL("sprites/muzzleflash39.spr");
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556NatoBox";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = M249_MAX_CLIP;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_LASERMINIGUN;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M249); }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 195 }; }


    float GetSecondaryAttackDamage() const
    {
        float flDamage = 50;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1150;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 600;
#endif
        return flDamage;
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

		SendWeaponAnim(ANIM_IDLE1);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
	}

	void OnAttack2Released()
	{
		if (m_iChargeState < 2)
		{
			SendWeaponAnim(ANIM_CHARGE_END);
		}
		else
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/laserminigun-charge_shoot.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SendWeaponAnim(ANIM_CHARGE_SHOOT);
#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecAmmoDir = gpGlobals->v_forward;
			Vector vecAngles = UTIL_VecToAngles(vecAmmoDir);

			if (m_pChargeAmmo)
			{
				m_pChargeAmmo->pev->origin = vecSrc;
				m_pChargeAmmo->Launch();
				m_pChargeAmmo->pev->velocity = vecAmmoDir.Normalize() * 1000.0;
			}
#endif
		}

		m_iChargeState = 0;
	}

    void SecondaryAttack(void)
    {
		if (m_iChargeState)
		{
			if (gpGlobals->time > m_flTimeCheckCharge)
			{
				switch (m_iChargeState)
				{
				case 1:
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/laserminigun-charge_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

					SendWeaponAnim(ANIM_CHARGE_START);
					//blue
#ifndef CLIENT_DLL
					m_pChargeAmmo = CLasermingunChargeAmmo::Create(0, ENT(m_pPlayer->pev));
#endif
					m_iChargeState = 2;
					m_flTimeCheckCharge = gpGlobals->time + 3.0s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
					break;
				}
				case 2:
				{
					//red
					SendWeaponAnim(ANIM_CHARGE_LOOP);
					m_iChargeState = 3;
					m_flTimeCheckCharge = gpGlobals->time + 9999.0s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
					break;
				}

				default:
					break;
				}
			}
		}
		else
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/laserminigun-charge_origin.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SendWeaponAnim(ANIM_CHARGE_ORIGIN);

			m_flTimeCheckCharge = gpGlobals->time + 1.0s;
			m_iChargeState = 1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		}
    }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_laserminigun, CLaserMiniGun)
}