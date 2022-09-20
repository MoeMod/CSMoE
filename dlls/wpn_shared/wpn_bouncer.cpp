/*
wpn_bouncer.cpp - Port of Zombie IV Mod
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

#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
#define COILGUNBOLT_MAXHIT	3
	class CBouncerBullet : public CBaseEntity
	{
	public:
		KnockbackData GetKnockBackData() override { return { 1000.0f, 500.0f, 650.0f, 500.0f, 0.3f }; }
		int GetWeaponsId(void) override { return WEAPON_BOUNCER; }
		float GetArmorRatioModifier() override { return 1.0; }

		static CBouncerBullet* CreateBullet(void)
		{
			edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("bouncer_bullet"));

			if (FNullEnt(pent))
			{
				ALERT(at_console, "NULL Ent in Create!\n");
				return NULL;
			}

			CBouncerBullet* pBullet = (CBouncerBullet*)Instance(pent);

			pBullet->Spawn();

			return pBullet;
		}

		void Spawn() override
		{
			Precache();

			pev->movetype = MOVETYPE_BOUNCE;
			pev->solid = SOLID_BBOX;
			pev->gravity = 0.001;

			SET_MODEL(ENT(pev), "models/bouncer_bullet02.mdl");
			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			SetTouch(&CBouncerBullet::BulletTouch);
			SetThink(&CBouncerBullet::BulletThink);

			pev->nextthink = gpGlobals->time;
			m_DirectDmg = TEAM_UNASSIGNED;
		}

		void Precache(void)
		{
			PRECACHE_MODEL("models/bouncer_bullet02.mdl");
			PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
			PRECACHE_SOUND("weapons/xbow_hit1.wav");

			m_iStreak = PRECACHE_MODEL("sprites/streak.spr");
			m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
		}

		void EXPORT BulletTouch(CBaseEntity* pOther)
		{
			entvars_t* pevToucher = pOther->pev;

			if (FClassnameIs(pevToucher, "bouncer_bullet"))
				return;

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
				tr.iHitgroup = HITGROUP_CHEST;
				pOther->TraceAttack(pevOwner, m_DirectDmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
				
				ApplyMultiDamage(pev, pevOwner);

				pev->velocity = g_vecZero;

				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hitbod1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				UTIL_Remove(this);
			}
		}

		void EXPORT BulletThink(void)
		{
			if (pev->waterlevel == 0)
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_BEAMFOLLOW);
				WRITE_SHORT(entindex());
				WRITE_SHORT(m_iTrail);
				WRITE_BYTE(2);
				WRITE_BYTE(1);
				WRITE_BYTE(10);
				WRITE_BYTE(155);
				WRITE_BYTE(255);
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

	LINK_ENTITY_TO_CLASS(bouncer_bullet, CBouncerBullet);
#endif


    class CBouncer : public LinkWeaponTemplate< CBouncer,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_bouncer.mdl";
    static constexpr const char *P_Model = "models/p_bouncer.mdl";
    static constexpr const char *W_Model = "models/w_bouncer.mdl";
    static constexpr const char *EventFile = "events/bouncer.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_bouncer";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 25;

	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD_INSERT,
		ANIM_RELOAD_AFTER,
		ANIM_START_RELOAD,
		ANIM_DRAW
	};

    static constexpr float PlayerMaxSpeed = 260;
    KnockbackData KnockBack = { 1000.0f, 500.0f, 650.0f, 500.0f, 0.3f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_bouncer" , 65 }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		return { ACCURACY_AIR | ACCURACY_SPEED, 9, 4, 0 };
	}

	duration_t m_flPumpTime;

	void Precache() override
	{
		PRECACHE_SOUND("weapons/bouncer-1.wav");
		PRECACHE_MODEL("sprites/muzzleflash65.spr");

		m_iShellId = PRECACHE_MODEL("models/bouncer_bullet01.mdl");

		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "bouncer";
		p->iMaxAmmo1 = 50;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 25;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_BOUNCER;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	BOOL Deploy(void)
	{
		return DefaultDeploy("models/v_bouncer.mdl", "models/p_bouncer.mdl", ANIM_DRAW, "shotgun", UseDecrement() != FALSE);
	}

	void PrimaryAttack(void)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0)
		{
			Reload();

			if (m_iClip == 0)
				PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			return;
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		float flDamage, flDistance, flSpread;
		duration_t flCycleTime;
		int cShots;

		GetFireInfo(flDamage, flDistance, flSpread, cShots, flCycleTime);

#ifndef CLIENT_DLL
		FireBulletsEx(7, flDamage, flDistance, flSpread, m_pPlayer->GetGunPosition(), Vector(flSpread, flSpread, 0.0));
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);

		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
		else
			m_flTimeWeaponIdle = 0.7s;

		m_fInSpecialReload = 0;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1, 3);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.65s;
	}

#ifndef CLIENT_DLL
	void FireBulletsEx(int cShots, float flDamage, float flFireDistance, float flFireSpread, Vector vecSrc, Vector vecSpread)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		for (int iShot = 1; iShot <= cShots; iShot++)
		{
			// get circular gaussian spread
			float x, y, z;

			do {
				x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);

			Vector vecBoltDir = gpGlobals->v_forward + x * vecSpread.x * gpGlobals->v_right + y * vecSpread.y * gpGlobals->v_up;
			Vector vecAngles = UTIL_VecToAngles(vecBoltDir);

			CBouncerBullet* pBullet = CBouncerBullet::CreateBullet();

			pBullet->pev->origin = vecSrc + x * vecSpread.x * gpGlobals->v_right + y * vecSpread.y * gpGlobals->v_up;
			pBullet->pev->angles = vecAngles;
			pBullet->pev->owner = m_pPlayer->edict();
			pBullet->m_iTeam = m_pPlayer->m_iTeam;
			pBullet->m_DirectDmg = flDamage;

			float flSpeed;
			if (m_pPlayer->pev->waterlevel == 3)
				flSpeed = 1500.0;
			else
				flSpeed = 2000.0;

			pBullet->pev->velocity = vecBoltDir * flSpeed;
			pBullet->pev->speed = flSpeed;
			pBullet->pev->avelocity.z = 10.0;
		}
	}
#endif

	void GetFireInfo(float& flDamage, float& flFireDistance, float& flFireSpread, int& cShots, duration_t& flCycleTime)
	{
			flFireDistance = 8192;
			flFireSpread = 0.07;
			cShots = 6;
			flCycleTime = 1.0s;
			flDamage = 22;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 179;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 273;
#endif
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(ANIM_START_RELOAD, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.4s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.4s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(ANIM_RELOAD_INSERT, UseDecrement() != FALSE);

			m_flNextReload = UTIL_WeaponTimeBase() + 0.37s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.37s;
		}
		else
		{
			m_iClip++;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_fInSpecialReload = 1;
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
			m_flPumpTime = zero_duration;

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else if (m_fInSpecialReload != 0)
			{
				if (m_iClip != MaxClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
				{
					Reload();
				}
				else
				{
					SendWeaponAnim(ANIM_RELOAD_AFTER, UseDecrement() != FALSE);

					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
			else
				SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
		}
	}

};
LINK_ENTITY_TO_CLASS(weapon_bouncer, CBouncer)
}