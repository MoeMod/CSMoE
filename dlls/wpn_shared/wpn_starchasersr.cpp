/*
z4b_starchasersr.cpp - Port of Zombie IV Mod
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
	class CStarChaserSRAmmo : public CBaseEntity
	{
	public:
		KnockbackData GetKnockBackData() override { return { 2500.0f, 600.0f, 600.0f, 1600.0f, 0.5f }; }
		float GetArmorRatioModifier() { return 1.5; }
		int GetWeaponsId(void) { return WEAPON_STARCHASERSR; }

		void Spawn() override
		{
			Precache();

			SET_MODEL(ENT(pev), "sprites/ef_starchasersr_star.spr");
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->solid = SOLID_BBOX;
			pev->rendermode = kRenderTransAdd;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 180;

			m_flNumFrames = float(MODEL_FRAMES(pev->modelindex) - 1);
			pev->framerate = 10;
			pev->scale = 0.2;

			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(-0.1, -0.1, -0.1), Vector(0.1, 0.1, 0.1));

			SetTouch(&CStarChaserSRAmmo::AmmoTouch);
			SetThink(&CStarChaserSRAmmo::AnimationThink);

			pev->nextthink = gpGlobals->time + 0.01s;
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/ef_starchasersr_star.spr");
		}

		void EXPORT AmmoTouch(CBaseEntity* pOther)
		{
			if (FClassnameIs(pOther->pev, "starchasersr_ammo"))
				return;
			CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pev->owner);

			if (!pPlayer || FNullEnt(pPlayer->edict()) || !pPlayer->IsPlayer() || pPlayer->m_bIsZombie)
			{
				SetTouch(NULL);
				SetThink(NULL);

				UTIL_Remove(this);
				return;
			}

			if (!pOther->IsDormant() && pOther->pev->takedamage != DAMAGE_NO)
			{
				pOther->TakeDamage(pev, pPlayer->pev, GetHitDamage(), 0);
			}

			SetTouch(NULL);
			SetThink(NULL);
			UTIL_Remove(this);
		}

		void EXPORT AnimationThink()
		{
			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flNumFrames)
				pev->frame = fmod(pev->frame, m_flNumFrames);

			pev->nextthink = gpGlobals->time + 0.01s;	
		}

		float GetHitDamage() const
		{
			float flDamage = 60;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 396;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 654;
#endif
			return flDamage;
		}


	private:
		float m_flNumFrames;

	};
	LINK_ENTITY_TO_CLASS(starchasersr_ammo, CStarChaserSRAmmo)
#endif
    class CStarChaserSR : public LinkWeaponTemplate< CStarChaserSR,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
            TPrecacheEvent,
            TDeployDefault,
			TSecondaryAttackSniperZoom1,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.0s;
	static constexpr auto ZoomFOV = 40;
    static constexpr const char *V_Model = "models/v_starchasersr.mdl";
    static constexpr const char *P_Model = "models/p_starchasersr.mdl";
    static constexpr const char *W_Model = "models/w_starchasersr.mdl";
    static constexpr const char *EventFile = "events/starchasersr.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_starchasersr";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 15;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};

    static constexpr float RangeModifier = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;
    KnockbackData KnockBack = { 2500.0f, 600.0f, 600.0f, 1600.0f, 0.5f };

	float GetArmorRatioModifier() override { return 1.5; }

#define STARCHASERSR_AMMO_COUNT	7

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_starchasersr" , 300 }; }
#endif

	int m_iEfHit;

	void Precache() override
	{
		PRECACHE_MODEL("sprites/muzzleflash73.spr");

		m_iEfHit = PRECACHE_MODEL("sprites/ef_starchasersr_explosion.spr");
		PRECACHE_MODEL("sprites/ef_starchasersr.spr");
		PRECACHE_MODEL("sprites/ef_starchasersr_line.spr");

		return Base::Precache();
	}
	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "StarChaserSR";
		p->iMaxAmmo1 = 150;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 15;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_STARCHASERSR;
		p->iFlags = 0;
		p->iWeight = SG550_WEIGHT;

		return 1;
	}

	void PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Fire((0.45) * (1 - m_flAccuracy), 1.0s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			Fire((0.15) * (1 - m_flAccuracy), 1.0s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			Fire((0.04) * (1 - m_flAccuracy), 1.0s, FALSE);
		else
			Fire((0.05) * (1 - m_flAccuracy), 1.0s, FALSE);
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 260;
		return 220;
	}


#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
	{
		Vector vecEnd = tr->vecEndPos - vecDir * 5;
		int iOriginalPenetration = 1;

		if (iOriginalPenetration == iPenetration)
		{
			float flExplodeDamage = GetExplodeDamage();
		
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecEnd);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(vecEnd.x);
			WRITE_COORD(vecEnd.y);
			WRITE_COORD(vecEnd.z);
			WRITE_SHORT(m_iEfHit);
			WRITE_BYTE(6);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			CBaseEntity* pEntity = NULL;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr->vecEndPos, 1.7 * 39.37)) != NULL)
			{
				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->edict() == pev->owner)
					continue;

				if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
					continue;

				pEntity->TakeDamage(pev, VARS(pev->owner), flExplodeDamage, DMG_BULLET);
			}

			EMIT_SOUND_DYN(this->edict(), CHAN_STATIC, "weapons/starchasersr_exp.wav", VOL_NORM, ATTN_NORM, 0, 94);

			for (int i = 0; i < STARCHASERSR_AMMO_COUNT; i++)
			{
				Vector vEnd;
				Vector vecStart = vecEnd;
				vEnd[0] = vecStart[0] * RANDOM_FLOAT(-4.0, 4.0);
				vEnd[1] = vecStart[1] * RANDOM_FLOAT(-4.0, 4.0);
				vEnd[2] = vecStart[2] * RANDOM_FLOAT(-4.0, 4.0);

				Vector vecAmmoVelocity = UTIL_GetSpeedVector(vecStart, vEnd, 1000);

				CStarChaserSRAmmo* pAmmo = (CStarChaserSRAmmo*)CBaseEntity::Create("starchasersr_ammo", vecEnd, UTIL_VecToAngles(vecDir), m_pPlayer->edict());

				pAmmo->pev->velocity = vecAmmoVelocity;
				pAmmo->m_iTeam = m_pPlayer->m_iTeam;
			}
		}
	}
#endif


	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov == 90)
			flSpread += 0.01;

		m_iShotsFired++;
		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0 + 0.3;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;

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

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_338MAG, GetDamage(), 0.99, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
#endif
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 10.0) + m_pPlayer->pev->punchangle.x * 0.25;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 2.0);
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 9.0) + m_pPlayer->pev->punchangle.x * 0.25;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 3.0);
		}
	}

	float GetDamage() const
	{
		float flDamage = 119.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 791.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1307.0f;
#endif
		return flDamage;
	}

	void Reload(void) override
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(15, ANIM_RELOAD, 3.53s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.2;
			m_iShotsFired = 0;
			if (m_pPlayer->pev->fov != 90)
				SecondaryAttack();
		}
	}

	float GetExplodeDamage() const
	{
		float flDamage = 60;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 396;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 654;
#endif
		return flDamage;
	}


};
LINK_ENTITY_TO_CLASS(weapon_starchasersr, CStarChaserSR)
}