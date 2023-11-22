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

		KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		float GetArmorRatioModifier() { return 1.65; }
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

			UTIL_SetSize(pev, { -2.0, -2.0, -2.0 }, { 2.0, 2.0, 2.0 });

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
			m_iExpSpr[0] = PRECACHE_MODEL("sprites/laserminigun_hit1.spr");
			m_iExpSpr[1] = PRECACHE_MODEL("sprites/laserminigun_hit2.spr");
			m_iExpSpr[2] = PRECACHE_MODEL("sprites/laserminigun_hit3.spr");
			PRECACHE_MODEL("sprites/laserminigun1.spr");
			PRECACHE_MODEL("sprites/laserminigun2.spr");
			PRECACHE_MODEL("sprites/laserminigun3.spr");

			PRECACHE_SOUND("weapons/laserminigun_exp1.wav");
			PRECACHE_SOUND("weapons/laserminigun_exp2.wav");
			PRECACHE_SOUND("weapons/laserminigun_exp3.wav");

			PRECACHE_SOUND("weapons/laserminigun-charge_shoot.wav");
		}

		void EXPORT SpearTouch(CBaseEntity* pOther)
		{
			if (!pOther)
				return;

			if (pOther->edict() == pev->owner)
				return;

			SetTouch(NULL);
			SetThink(NULL);

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iExpSpr[m_iState]);
			WRITE_BYTE(m_iState ? 10 : 6);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			if (m_iState > 1)
				EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/laserminigun_exp3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else if (m_iState > 0)
				EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/laserminigun_exp2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/laserminigun_exp1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			
			float flDamage = GetDirectDamage();

			entvars_t* pevOwner = VARS(pev->owner);

			if (!pOther->IsDormant())
			{
				if (pOther->pev->takedamage != DAMAGE_NO)
				{
					pOther->TakeDamage(pev, pevOwner, GetDirectDamage(), 0);
				}
			}

			Vector vecEnd;
			bool bInWater = UTIL_PointContents(pev->origin) == CONTENTS_WATER;

			TraceResult tr;
			CBaseEntity* pEntity = NULL;
			float flRadius = GetRadius() * 39.37;
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

				if (!friendlyfire.value && (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE))
					continue;

				if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				/*float flCurrentDamage = UTIL_CalcDamage(pev->origin, pEntity->pev->origin, GetExpDamage(), flRadius, NULL);

				if (flCurrentDamage <= 0.0)
					continue;

				pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_EXPLOSION);*/

				pEntity->TakeDamage(pev, pevOwner, GetExpDamage(), DMG_EXPLOSION);
			}

			UTIL_Remove(this);
		}

		CBasePlayer* m_pOwner;
		int m_iState;
		int m_iChargeTime;
		float m_flNumFrames;
		time_point_t m_flChargeTime;

	private:
		int m_iExpSpr[3];

		float GetDirectDamage()
		{
			float flDamage;
			if (m_iState > 1)
			{
				flDamage = 110.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 1800.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 7000.0f;
			}
			else if (m_iState > 0)
			{
				flDamage = 80.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 940.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 3500.0f;
			}
			else
			{
				flDamage = 50.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 470.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 1750.0f;
			}
			return flDamage;
		}

		float GetExpDamage()
		{
			float flDamage;
			if (m_iState > 1)
			{
				flDamage = 44.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 752.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 2800.0f;
			}
			else if (m_iState > 0)
			{
				flDamage = 32.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 376.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 1400.0f;
			}
			else
			{
				flDamage = 20.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 188.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 700.0f;
			}
			return flDamage;
		}

		float GetRadius()
		{
			float flRadius;
			if (m_iState > 1)
			{
				flRadius = 1.5f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRadius = 2.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRadius = 4.0f;
			}
			else if (m_iState > 0)
			{
				flRadius = 1.25f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRadius = 1.5f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRadius = 2.5f;
			}
			else
			{
				flRadius = 1.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRadius = 1.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRadius = 1.5f;
			}
			return flRadius;
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
            TGetDamageDefault
    >
    {
    public:
	static constexpr const auto& DefaultDeployTime = 2.5s;
    static constexpr const auto &DefaultReloadTime = 5.0s;
    static constexpr const char *V_Model = "models/v_laserminigun.mdl";
    static constexpr const char *P_Model = "models/p_laserminiguna.mdl";
	static constexpr const char* P_Model_2 = "models/p_laserminigunb.mdl";
    static constexpr const char *W_Model = "models/w_laserminigun.mdl";
    static constexpr const char *EventFile = "events/laserminigun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_laserminigun";
    static constexpr const char *AnimExtension = "m249";
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

	enum skin_e
	{
		SKIN_LOW,
		SKIN_HIGH,
		SKIN_EXTRME,
		SKIN_CHARGE,
	};

    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.65f;

    static constexpr float AccuracyDefault = 0.2;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

#ifndef CLIENT_DLL
	EntityHandle <CLasermingunChargeAmmo> m_pChargeAmmo;
#endif

	time_point_t m_flTimeCheckCharge;
	int m_iChargeState;
	int m_iAmmo2;
	time_point_t m_flTimeCheckAmmo2;

	void Spawn()
	{
		m_iChargeState = 0;
		m_iAmmo2 = 0;
		m_flTimeCheckCharge = invalid_time_point;
		m_flTimeCheckAmmo2 = gpGlobals->time;
		Base::Spawn();
	}

    void Precache(void)
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/laserminigun-charge_start.wav");
        PRECACHE_SOUND("weapons/laserminigun-charge_origin.wav");
        PRECACHE_SOUND("weapons/laserminigun-charge_shoot.wav");

        PRECACHE_SOUND("weapons/laserminigun-1.wav");

		PRECACHE_MODEL("models/p_laserminigunb.mdl");
		PRECACHE_MODEL("sprites/muzzleflash37.spr");
		PRECACHE_MODEL("sprites/muzzleflash38.spr");
		PRECACHE_MODEL("sprites/muzzleflash39.spr");

		PRECACHE_MODEL("sprites/ef_laser1.spr");
		PRECACHE_MODEL("sprites/ef_laser2.spr");
		PRECACHE_MODEL("sprites/ef_laser3.spr");
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "laserminigun";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 200;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_LASERMINIGUN;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_laserminigun" , 80 }; }
#endif
    float GetArmorRatioModifier() override { return 1.65; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 0 }; }

	void ItemPostFrame()
	{
		if (m_flTimeCheckAmmo2 < gpGlobals->time && m_flTimeCheckAmmo2 != invalid_time_point)
		{
			if (m_iAmmo2 > 0)
				m_iAmmo2--;

			m_flTimeCheckAmmo2 = gpGlobals->time + 0.1s;
		}

		if (m_iAmmo2 > 45)
			m_iWeaponState = 2;
		else if (m_iAmmo2 > 10)
			m_iWeaponState = 1;
		else
			m_iWeaponState = 0;

		if (pev->skin != SKIN_CHARGE)
			pev->skin = m_iWeaponState;

		CBasePlayerWeapon::ItemPostFrame();
	}

	void PrimaryAttack(void)
	{	
		if (m_iWeaponState > 1)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				LaserMiniGunFire(0.02 + (0.001) * m_flAccuracy, 0.06s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				LaserMiniGunFire(0.03 + (0.02) * m_flAccuracy, 0.06s, FALSE);
			else
				LaserMiniGunFire(0.04 + (0.1) * m_flAccuracy, 0.06s, FALSE);
		}
		else if (m_iWeaponState > 0)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				LaserMiniGunFire(0.0215 + (0.001) * m_flAccuracy, 0.06s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				LaserMiniGunFire(0.0315 + (0.02) * m_flAccuracy, 0.06s, FALSE);
			else
				LaserMiniGunFire(0.0415 + (0.1) * m_flAccuracy, 0.06s, FALSE);
		}
		else
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				LaserMiniGunFire(0.023 + (0.001) * m_flAccuracy, 0.06s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				LaserMiniGunFire(0.033 + (0.02) * m_flAccuracy, 0.06s, FALSE);
			else
				LaserMiniGunFire(0.043 + (0.1) * m_flAccuracy, 0.06s, FALSE);
		}
		
	}


	void LaserMiniGunFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0) + 0.3;

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

		m_iAmmo2++;

		if(m_iWeaponState > 0)
			m_flTimeCheckAmmo2 = gpGlobals->time + 1.0s;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_556MM, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		if (m_iWeaponState > 1)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBackCustom(1, 0.4, 0.2, 0.15, 3, 2, 0);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBackCustom(0.3, 0.075, 0.05, 0.02, 2.5, 1.5, 0);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBackCustom(0.175, 0.04, 0.03, 0.001, 1.5, 1, 0);
			else
				KickBackCustom(0.2, 0.05, 0.035, 0.015, 2.0, 1.25, 0);
		}
		else if (m_iWeaponState > 0)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBackCustom(1, 0.4, 0.2, 0.15, 2.9, 1.9, 0);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBackCustom(0.3, 0.075, 0.05, 0.02, 2.4, 1.4, 0);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBackCustom(0.175, 0.04, 0.03, 0.001, 1.4, 0.9, 0);
			else
				KickBackCustom(0.2, 0.05, 0.035, 0.015, 1.9, 1.15, 0);
		}
		else
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBackCustom(1, 0.4, 0.2, 0.15, 2.8, 1.8, 0);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBackCustom(0.3, 0.075, 0.05, 0.02, 2.3, 1.3, 0);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBackCustom(0.175, 0.04, 0.03, 0.001, 1.3, 0.8, 0);
			else
				KickBackCustom(0.2, 0.05, 0.035, 0.015, 1.8, 1.05, 0);
		}
	}


	void KickBackCustom(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change)
	{
		float up, lateral;
		int iRan;

		up = ((rand() % 10) * up_base) * 0.1;
		lateral = ((rand() % 10) * lateral_base) * 0.1;

		if (m_iShotsFired != 1)
		{
			up += (rand() % (m_iShotsFired * 2)) * up_modifier;
			lateral += (rand() % (m_iShotsFired * 2)) * lateral_modifier;
		}

		iRan = rand();

		if (!iRan || iRan % 2)
		{
			m_pPlayer->pev->punchangle.x += up;

			if (m_pPlayer->pev->punchangle.x > up_max)
				m_pPlayer->pev->punchangle.x = up_max;
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= up;

			if (m_pPlayer->pev->punchangle.x < -up_max)
				m_pPlayer->pev->punchangle.x = -up_max;
		}

		if (rand() % 2 == 1)
		{
			m_pPlayer->pev->punchangle.y += lateral;

			if (m_pPlayer->pev->punchangle.y > lateral_max)
				m_pPlayer->pev->punchangle.y = lateral_max;
		}
		else
		{
			m_pPlayer->pev->punchangle.y -= lateral;

			if (m_pPlayer->pev->punchangle.y < -lateral_max)
				m_pPlayer->pev->punchangle.y = -lateral_max;
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

		SendWeaponAnim(ANIM_IDLE1);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
	}

	void OnAttack2Released()
	{
		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);

		if (m_iChargeState < 2)
		{
			pev->skin = m_iWeaponState;
			SendWeaponAnim(ANIM_CHARGE_END);
		}
		else
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/laserminigun-charge_shoot.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			pev->skin = m_iWeaponState;
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
			if (m_iClip > (m_pChargeAmmo->m_iState + 1) * 10)
				m_iClip -= (m_pChargeAmmo->m_iState + 1) * 10;
			else
				m_iClip = 0;
#endif

		}

		m_iChargeState = 0;
	}

    void SecondaryAttack(void)
    {
		if (m_iClip <= 0)
			return;

		if (m_iChargeState)
		{
			if (gpGlobals->time > m_flTimeCheckCharge)
			{
				if (pev->skin != SKIN_CHARGE)
					pev->skin = SKIN_CHARGE;

				switch (m_iChargeState)
				{
				case 1:
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/laserminigun-charge_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

					SendWeaponAnim(ANIM_CHARGE_START);
					//blue
#ifndef CLIENT_DLL
					m_pChargeAmmo = CLasermingunChargeAmmo::Create(0, ENT(m_pPlayer->pev));
					if (m_pChargeAmmo)
						m_pChargeAmmo->m_pOwner = m_pPlayer;
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
			if (pev->skin != SKIN_CHARGE)
				pev->skin = SKIN_CHARGE;

			m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_2);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/laserminigun-charge_origin.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SendWeaponAnim(ANIM_CHARGE_ORIGIN);

			m_flTimeCheckCharge = gpGlobals->time + 1.0s;
			m_iChargeState = 1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		}
    }


	float GetDamage(void)
	{
		float flDamage;
		if (m_iWeaponState > 1)
		{
			flDamage = 48.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 50.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 113.0f;
#endif
		}
		else if (m_iWeaponState > 0)
		{
			flDamage = 30.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 40.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 103.0f;
#endif
		}
		else
		{
			flDamage = 28.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 40.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 93.0f;
#endif
		}
		return flDamage;
	}

    		 	
};
LINK_ENTITY_TO_CLASS(weapon_laserminigun, CLaserMiniGun)
}