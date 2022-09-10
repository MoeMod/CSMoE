/*
crossbow.cpp
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
	class CCrossbowBolt : public CBaseEntity
	{
	public:
		KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.9f }; }
		int GetWeaponsId(void) override { return WEAPON_CROSSBOW; }
		float GetArmorRatioModifier() override { return 1.7; }

		static CCrossbowBolt* CreateBolt(void)
		{
			edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("crossbow_bolt"));

			if (FNullEnt(pent))
			{
				ALERT(at_console, "NULL Ent in Create!\n");
				return NULL;
			}

			CCrossbowBolt* pBolt = (CCrossbowBolt*)Instance(pent);

			pBolt->Spawn();

			return pBolt;
		}

		void Spawn() override
		{
			Precache();

			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_BBOX;
			pev->gravity = 0.5;

			SET_MODEL(ENT(pev), "models/crossbow_bolt.mdl");
			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			SetTouch(&CCrossbowBolt::BoltTouch);
			SetThink(&CCrossbowBolt::IgniteThink);

			pev->nextthink = gpGlobals->time + 0.2s;
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
			SetThink(NULL);
			SetTouch(NULL);

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_KILLBEAM);
			WRITE_SHORT(this->entindex());
			MESSAGE_END();

			if (pOther->pev->takedamage == DAMAGE_NO)
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));

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
			}
			else
			{
				TraceResult tr = UTIL_GetGlobalTrace();
				entvars_t* pevOwner = VARS(pev->owner);

				Vector vecStart = pevOwner->origin;
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_BEAMPOINTS);
				WRITE_COORD(vecStart.x);
				WRITE_COORD(vecStart.y);
				WRITE_COORD(vecStart.z);
				WRITE_COORD(tr.vecEndPos.x);
				WRITE_COORD(tr.vecEndPos.y);
				WRITE_COORD(tr.vecEndPos.z); // coord coord coord (end position) 
				WRITE_SHORT(g_sModelIndexLaser); // short (sprite index) 
				WRITE_BYTE(0); // byte (starting frame) 
				WRITE_BYTE(0); // byte (frame rate in 0.1's) 
				WRITE_BYTE(50); // byte (life in 0.1's) 
				WRITE_BYTE(10); // byte (line width in 0.1's) 
				WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
				WRITE_BYTE(0); // byte,byte,byte (color)
				WRITE_BYTE(0);
				WRITE_BYTE(255);
				WRITE_BYTE(255); // byte (brightness)
				WRITE_BYTE(15); // byte (scroll speed in 0.1's)
				MESSAGE_END();

				ClearMultiDamage();

				pOther->TraceAttack(pevOwner, m_DirectDmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

				ApplyMultiDamage(pev, pevOwner);

				pev->velocity = g_vecZero;

				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hitbod1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				UTIL_Remove(this);
			}
		}

		void EXPORT BubbleThink(void)
		{
			UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);

			if (pev->waterlevel == 0)
				SetThink(&CCrossbowBolt::FollowThink);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void EXPORT FollowThink(void)
		{
			if (pev->waterlevel != 0)
				SetThink(&CCrossbowBolt::BubbleThink);

			pev->nextthink = gpGlobals->time + 0.1s;
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
				WRITE_BYTE(255);
				WRITE_BYTE(255);
				WRITE_BYTE(255);
				WRITE_BYTE(200);
				MESSAGE_END();

				SetThink(&CCrossbowBolt::FollowThink);
			}
			else
			{
				SetThink(&CCrossbowBolt::BubbleThink);
			}

			pev->nextthink = gpGlobals->time + 0.1s;
		}

	public:
		int m_iStreak;
		int m_iTrail;
		float m_DirectDmg;
	};

	LINK_ENTITY_TO_CLASS(crossbow_bolt, CCrossbowBolt);
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CCrossbow : public LinkWeaponTemplate< CCrossbow,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG550>::template type,
			TPrecacheEvent,
			TDeployDefault,
			TReloadDefault,
			TGetDamageDefault,
			TPrimaryAttackRifle,
			TSecondaryAttackZoom,
			TFireRifle,
			TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.7s;
	static constexpr int ZoomFOV = 44;
    static constexpr const char *V_Model = "models/v_crossbow.mdl";
    static constexpr const char *P_Model = "models/p_crossbow.mdl";
    static constexpr const char *W_Model = "models/w_crossbow.mdl";
    static constexpr const char *EventFile = "events/crossbow.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crossbow";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 50;
	enum
	{
		ANIM_IDLE1 = 0,	
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
	static constexpr float ArmorRatioModifier = 1.7f;

    static constexpr int DamageDefault = 10;
	static constexpr int DamageZB = 70;
    static constexpr int DamageZBS = 70;
   
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 120;
	static constexpr const auto& SpreadCalcNotOnGround = 0.04 * A + 0.035;
	static constexpr const auto& SpreadCalcWalking = 0.001 * A + 0.02;
	static constexpr const auto& SpreadCalcDefault = 0.001 * A + 0.018;
	static constexpr const auto& SpreadCalcZoomed = 0 * A;
	static constexpr const auto& SpreadCalcDucking = 0.001 * A + 0.01;

	static constexpr float AccuracyDefault = 0.15;
	static constexpr float AccuracyMax = 1.0;

	static constexpr const auto& CycleTime1 = 0.14s;
	static constexpr const auto& CycleTime2 = 0.15s;

	duration_t CycleTime = CycleTime1;

    KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.9f };;

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "bolts";
		p->iMaxAmmo1 = 200;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 50;
		p->iSlot = 0;
		p->iPosition = 16;
		p->iId = m_iId = WEAPON_CROSSBOW;
		p->iFlags = 0;
		p->iWeight = SG550_WEIGHT;

		return 1;
	}
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_bolts" , 300 }; }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 0 }; }

	float GetArmorRatioModifier() override { return 1.7; }

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 240;

		return 170;
	}

	void PrimaryAttack() override
	{
		if (m_pPlayer->pev->fov != 90)
			CycleTime = CycleTime2;
		else
			CycleTime = CycleTime1;
		return Base::PrimaryAttack();
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
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
		if (m_pPlayer->pev->effects & EF_MUZZLEFLASH)
			m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, flSpread, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
		Vector vecBoltDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
		Vector vecAngles = UTIL_VecToAngles(vecBoltDir);

		CCrossbowBolt* pBolt = CCrossbowBolt::CreateBolt();

		pBolt->pev->origin = vecSrc;
		pBolt->pev->angles = vecAngles;
		pBolt->pev->owner = m_pPlayer->edict();
		pBolt->m_iTeam = m_pPlayer->m_iTeam;
		pBolt->m_DirectDmg = GetDamage();

		float flSpeed;
		if (m_pPlayer->pev->waterlevel == 3)
			flSpeed = 1500.0;
		else
			flSpeed = 2000.0;

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
			KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.3, 0.3, 0.07, 0.07, 2.0, 2.0, 3);
		else if (m_pPlayer->pev->fov != 90)
			KickBack(0.11, 0.11, 0.06, 0.06, 0.6, 0.6, 2);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.1, 0.1, 0.05, 0.05, 0.5, 0.5, 1);
		else
			KickBack(0.2, 0.2, 0.06, 0.06, 1.0, 1.0, 2);
	}
};

LINK_ENTITY_TO_CLASS(weapon_crossbow, CCrossbow)
}