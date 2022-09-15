/*
wpn_destroyer.cpp - Port of Zombie IV Mod
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

#ifndef CLIENT_DLL
	class CDestroyerExplode : public CBaseEntity
	{
	public:
		float GetArmorRatioModifier() { return 1.99; }
		int GetWeaponsId(void) { return WEAPON_DESTROYER; }
		KnockbackData GetKnockBackData() override { return { 2500.0f, 600.0f, 600.0f, 1600.0f, 0.8f }; }

		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/shell_destroyer.mdl");
			SetThink(&CDestroyerExplode::ExplodeThink);

			pev->rendermode = kRenderTransAdd; // 5;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 0.0;
			pev->nextthink = gpGlobals->time + 0.95s;
			pev->solid = SOLID_NOT; // 2
			pev->movetype = MOVETYPE_NONE; // 9
			pev->effects |= EF_NODRAW;

		}

		void Precache() override
		{
			m_iExpEffect = PRECACHE_MODEL("sprites/ef_destroyerexp.spr");

			PRECACHE_SOUND("weapons/destroyer_exp.wav");
		}

		float GetDamage() const
		{
			float flDamage = 30.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 90.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 350.0f;
#endif
			return flDamage;
		}

		float GetRadius() const
		{
			float flRadius = 1 * 39.37f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 3 * 39.37f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4 * 39.37f;
#endif
			return flRadius;
		}

		void RadiusDamage()
		{
			float flDamage = GetDamage();
			float flRadius = GetRadius();
			CBaseEntity* pEntity = NULL;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
			{
				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->m_iTeam == m_iTeam)
					continue;

				pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_GENERIC | DMG_EXPLOSION);
			}

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 20.0);
			WRITE_SHORT(m_iExpEffect);
			WRITE_BYTE(5);
			WRITE_BYTE(30);
			WRITE_BYTE(0);
			MESSAGE_END();

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/destroyer_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);
		}

		void EXPORT ExplodeThink(void)
		{
			RadiusDamage();


			SetThink(nullptr);
			UTIL_Remove(this);
		}

		int m_iExpEffect;	
	};

	LINK_ENTITY_TO_CLASS(destroyer_explode, CDestroyerExplode);
#endif

    class CDestroyer : public LinkWeaponTemplate< CDestroyer,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
			TSecondaryAttackSniperZoom2,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.375s;
    static constexpr int ZoomFOV1 = 55;
    static constexpr int ZoomFOV2 = 10;
    static constexpr const char *V_Model = "models/v_destroyer.mdl";
    static constexpr const char *P_Model = "models/p_destroyer.mdl";
    static constexpr const char *W_Model = "models/w_destroyer.mdl";
    static constexpr const char *EventFile = "events/destroyer.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_destroyer";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 3;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};

    static constexpr float RangeModifier = 1.0;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;
    KnockbackData KnockBack = { 2500.0f, 600.0f, 600.0f, 1600.0f, 0.8f };

	float GetArmorRatioModifier() override { return 1.99; }

	void Precache() override
	{
		PRECACHE_MODEL("sprites/muzzleflash6.spr");
		PRECACHE_MODEL("sprites/muzzleflash46.spr");
		PRECACHE_MODEL("sprites/muzzleflash47.spr");
		PRECACHE_MODEL("sprites/muzzleflash48.spr");

		PRECACHE_SOUND("weapons/destroyer-1.wav");

		m_iShellId = PRECACHE_MODEL("models/shell_destroyer.mdl");
		return Base::Precache();
	}
	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "20mm";
		p->iMaxAmmo1 = 30;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 3;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_DESTROYER;
		p->iFlags = 0;
		p->iWeight = AWP_WEIGHT;

		return 1;
	}

	void PrimaryAttack(void)
	{

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Fire(0.7, 0.75s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			Fire(0.7, 0.75s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			Fire(0.03, 0.75s, FALSE);
		else
			Fire(0.15, 0.75s, FALSE);
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 230;
		return 150;
	}


#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
	{
		Vector vecEnd = tr->vecEndPos - vecDir * 5;
		int iOriginalPenetration = 10;

		if (iOriginalPenetration == iPenetration)
		{
			CDestroyerExplode* pEnt = static_cast<CDestroyerExplode*>(CBaseEntity::Create("destroyer_explode", tr->vecEndPos, g_vecZero, ENT(m_pPlayer->pev)));
			if (pEnt)
			{
				pEnt->m_iTeam = m_pPlayer->m_iTeam;
			}
		}
	}
#endif

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov == 90)
			flSpread += 0.1;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.375 + 0.6;

			float flMaxAccuracy = m_pPlayer->m_iFOV == 90 ? 0.9 : 1.0;

			if (m_pPlayer->m_iFOV == 90)
				flMaxAccuracy = 0.9;
			else
				flMaxAccuracy = 1.0;

			if (m_flAccuracy > flMaxAccuracy)
				m_flAccuracy = flMaxAccuracy;
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

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 10, BULLET_PLAYER_338MAG, GetDamage(), 1.0, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

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

		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 10.0) + m_pPlayer->pev->punchangle.x * 0.75;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 2.0);
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 9.0) + m_pPlayer->pev->punchangle.x * 0.75;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 3.0);
		}
	}

	float GetDamage() const
	{
		float flDamage = 120.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 460.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1750.0f;
#endif
		return flDamage;
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_20mm" , 100 }; }
#endif

};
LINK_ENTITY_TO_CLASS(weapon_destroyer, CDestroyer)
}