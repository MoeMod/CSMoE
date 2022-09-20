/*
coilgun.cpp
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

#define COILGUNBOLT_MAXHIT	1

	class CCoilGunBolt : public CBaseEntity
	{
	public:
		KnockbackData GetKnockBackData() override { return { 375.0f, 250.0f, 350.0f, 150.0f, 0.4f }; }
		int GetWeaponsId(void) override { return WEAPON_COILGUN; }
		float GetArmorRatioModifier() override { return 1.5; }

		static CCoilGunBolt* CreateBolt(void)
		{
			edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("coilgun_bolt"));

			if (FNullEnt(pent))
			{
				ALERT(at_console, "NULL Ent in Create!\n");
				return NULL;
			}

			CCoilGunBolt* pBolt = (CCoilGunBolt*)Instance(pent);

			pBolt->Spawn();

			return pBolt;
		}

		void Spawn() override
		{
			Precache();

			pev->movetype = MOVETYPE_BOUNCE;
			pev->solid = SOLID_BBOX;
			pev->gravity = 0.001;

			SET_MODEL(ENT(pev), "models/crossbow_bolt.mdl");
			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			SetTouch(&CCoilGunBolt::BoltTouch);
			SetThink(&CCoilGunBolt::IgniteThink);

			pev->nextthink = gpGlobals->time;
			m_DirectDmg = TEAM_UNASSIGNED;
		}

		void Precache(void)
		{
			PRECACHE_MODEL("models/crossbow_bolt.mdl");
			PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
			PRECACHE_SOUND("weapons/xbow_hit1.wav");

			m_iStreak = PRECACHE_MODEL("sprites/streak.spr");
			m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
		}

		void EXPORT BoltTouch(CBaseEntity* pOther)
		{
			
			if (pOther->pev->takedamage == DAMAGE_NO)
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));

				if (m_iHitCount >= COILGUNBOLT_MAXHIT)
				{		
					SetThink(&CBaseEntity::SUB_Remove);
					pev->nextthink = gpGlobals->time;

					if (FClassnameIs(pOther->pev, "worldspawn"))
					{
						Vector vecDir = pev->velocity.Normalize();
						Vector vecSrc = pev->origin - vecDir * 12.0;
						UTIL_SetOrigin(pev, vecSrc);

						pev->angles = UTIL_VecToAngles(vecSrc);
						pev->solid = SOLID_NOT;
						pev->movetype = MOVETYPE_FLY;
						pev->velocity = g_vecZero;
						pev->avelocity.z = 0.0;
						pev->angles.z = RANDOM_LONG(0, 360);

						pev->nextthink = gpGlobals->time + 10.0s;
					}

					if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
					{
						TraceResult tr = UTIL_GetGlobalTrace();

						UTIL_GunshotDecalTrace(&tr, DamageDecal(DMG_BULLET | DMG_NEVERGIB), false, NULL);
					}

					UTIL_Remove(this);
					return;
				}

				m_iHitCount++;

				if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
				{
					TraceResult tr = UTIL_GetGlobalTrace();

					UTIL_GunshotDecalTrace(&tr, DamageDecal(DMG_BULLET | DMG_NEVERGIB), false, NULL);
				}

			}
			else
			{
				SetThink(NULL);
				SetTouch(NULL);

				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_KILLBEAM);
				WRITE_SHORT(this->entindex());
				MESSAGE_END();

				TraceResult tr = UTIL_GetGlobalTrace();
				entvars_t* pevOwner = VARS(pev->owner);

				ClearMultiDamage();

				pOther->TraceAttack(pevOwner, m_DirectDmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

				ApplyMultiDamage(pev, pevOwner);

				pev->velocity = g_vecZero;

				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hitbod1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				UTIL_Remove(this);
			}
		}

		void EXPORT IgniteThink(void)
		{
			if (pev->waterlevel == 0)
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_BEAMFOLLOW);
				WRITE_SHORT(entindex());
				WRITE_SHORT(m_iTrail);
				WRITE_BYTE(1);
				WRITE_BYTE(1);
				WRITE_BYTE(244);
				WRITE_BYTE(244);
				WRITE_BYTE(244);
				WRITE_BYTE(100);
				MESSAGE_END();			
			}

			pev->nextthink = gpGlobals->time + 0.1s;
		}

	public:
		int m_iHitCount;
		int m_iStreak;
		int m_iTrail;
		float m_DirectDmg;
	};

	LINK_ENTITY_TO_CLASS(coilgun_bolt, CCoilGunBolt);
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CCoilGun : public LinkWeaponTemplate< CCoilGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MP5N>::template type,
			TPrecacheEvent,
			TDeployDefault,
			TReloadDefault,
			TGetDamageDefault,
			TPrimaryAttackRifle,
			TFireRifle,
			TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.5s;
    static constexpr const char *V_Model = "models/v_coilgun.mdl";
    static constexpr const char *P_Model = "models/p_coilgun.mdl";
    static constexpr const char *W_Model = "models/w_coilgun.mdl";
    static constexpr const char *EventFile = "events/coilgun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_coilgun";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 100;
	enum
	{
		ANIM_IDLE1 = 0,	
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
	static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr int DamageDefault = 33;
	static constexpr int DamageZB = 58;
    static constexpr int DamageZBS = 97;
   
	static constexpr const auto& SpreadCalcNotOnGround = 0.22 * A;
	static constexpr const auto& SpreadCalcDefault = 0.04 * A;

	static constexpr float AccuracyDefault = 0.0;
	static constexpr float AccuracyMax = 1.0;

	static constexpr const auto& CycleTime = 0.225s;

    KnockbackData KnockBack = { 375.0f, 250.0f, 350.0f, 150.0f, 0.4f };

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "coilbolts";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 100;
		p->iSlot = 0;
		p->iPosition = 16;
		p->iId = m_iId = WEAPON_COILGUN;
		p->iFlags = 0;
		p->iWeight = MP5NAVY_WEIGHT;

		return 1;
	}
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_coilbolts" , 150 }; }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 0 }; }

	float GetArmorRatioModifier() override { return 1.5; }

	float GetMaxSpeed(void) override
	{
		return 240;
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;

		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210.0 + 0.35;

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
		if (m_pPlayer->pev->effects & EF_MUZZLEFLASH)
			m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, flSpread, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
		Vector vecBoltDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
		Vector vecAngles = UTIL_VecToAngles(vecBoltDir);

		CCoilGunBolt* pBolt = CCoilGunBolt::CreateBolt();

		pBolt->pev->origin = vecSrc;
		pBolt->pev->angles = vecAngles;
		pBolt->pev->owner = m_pPlayer->edict();
		pBolt->m_iTeam = m_pPlayer->m_iTeam;
		pBolt->m_DirectDmg = GetDamage();

		float flSpeed;
		if (m_pPlayer->pev->waterlevel == 3)
			flSpeed = 1300.0;
		else
			flSpeed = 1500.0;

		pBolt->pev->velocity = vecBoltDir * flSpeed;
		pBolt->pev->speed = flSpeed;
		pBolt->pev->avelocity.z = 10.0;
#endif

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType], FALSE, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(0.125, 0.65, 0.55, 0.475, 5.5, 4.0, 10);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.55, 0.3, 0.225, 0.03, 3.75, 2.5, 10);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.25, 10);
		else
			KickBack(0.275, 0.2, 0.15, 0.0225, 3.0, 1.5, 10);
	}
};

LINK_ENTITY_TO_CLASS(weapon_coilgun, CCoilGun)
}