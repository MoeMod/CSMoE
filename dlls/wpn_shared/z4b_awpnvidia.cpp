/*
z4b_awpnvidia.cpp - Port of Zombie IV Mod
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

    class CAWPNvidia : public LinkWeaponTemplate< CAWPNvidia,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
			TSecondaryAttackSniperZoom2,
            TPrimaryAttackRifle,
			TRecoilPunch,
            TWeaponIdleDefault,
            TGetDamageDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.2s;
    static constexpr int ZoomFOV1 = 40;
    static constexpr int ZoomFOV2 = 10;
    static constexpr const char *V_Model = "models/z4b/v_awpnvidia.mdl";
    static constexpr const char *P_Model = "models/z4b/pw_awpnvidia.mdl";
    static constexpr const char *W_Model = "models/z4b/pw_awpnvidia.mdl";
    static constexpr const char *EventFile = "events/awp.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_awpnvidia";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 10;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float MaxSpeed = 225;
    static constexpr float ArmorRatioModifier = 1.6f;
    static constexpr const auto &SpreadCalcNotOnGround = 0 * A + 1.0;
    static constexpr const auto &SpreadCalcWalking = 0 * A + 0.3;
    static constexpr const auto &SpreadCalcDefault = 0 * A + 0.002;
    static constexpr const auto &SpreadCalcDucking = 0 * A + 0.0;
    static constexpr const auto &CycleTime = 1.6s;
    static constexpr int DamageDefault = 96;
    static constexpr int DamageZB = 260;
    static constexpr int DamageZBS = 600;
    static constexpr float RangeModifier = 0.94;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;
    static constexpr int Penetration = 1;
	std::array<float, 3> RecoilPunchAngleDelta = { -3, 0, 0 };
    KnockbackData KnockBack = { 4000.0f, 600.0f, 1000.0f, 600.0f, 0.2f };

	void Precache() override
	{
		m_iSprBeam = PRECACHE_MODEL("sprites/zbeam1.spr");
		m_iSprEffect2 = PRECACHE_MODEL("sprites/plasmabomb.spr");
		return Base::Precache();
	}
    	
	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_bResumeZoom = true;
			m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
		}
		else // not zoomed, so bullets diverts more
			flSpread += 0.08;

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
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		const float flDistance = 8192;
		const int iPenetration = Penetration;
		const Bullet iBulletType = BulletType;
		const int iDamage = GetDamage();
		const float flRangeModifier = RangeModifier;
		const BOOL bPistol = ItemSlot == PISTOL_SLOT;

		const Vector vecSrc = m_pPlayer->GetGunPosition();
		const Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, m_pPlayer->pev, bPistol, m_pPlayer->random_seed);

		const Vector vecDirShooting = (gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y).Normalize();
		Make_Explosion(vecSrc, vecDirShooting);

		FireEvent(vecDir);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 1.9s;

		Recoil();
	}

	void Make_Explosion(const Vector& vecShootingSrc, const Vector& vecDir)
	{
#ifndef CLIENT_DLL
		const float flRadius = 88;
		const float flDamage = GetExplodeDamage();
		
		entvars_t* const pevAttacker = m_pPlayer->pev;
		entvars_t* const pevInflictor = m_pPlayer->pev;
		int bitsDamageType = DMG_BULLET;

		TraceResult tr;
		UTIL_TraceLine(vecShootingSrc, vecShootingSrc + vecDir * 8192, dont_ignore_monsters, ENT(pevAttacker), &tr);
		const Vector vecSrc = tr.vecEndPos;

		const float falloff = flRadius ? flDamage / flRadius : 1;
		const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage != DAMAGE_NO)
			{
				if (bInWater && !pEntity->pev->waterlevel)
					continue;

				if (!bInWater && pEntity->pev->waterlevel == 3)
					continue;

				if (pEntity->IsBSPModel())
					continue;

				if (pEntity->pev == pevAttacker)
					continue;

				Vector vecSpot = pEntity->BodyTarget(vecSrc);
				UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

				if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
				{
					if (tr.fStartSolid)
					{
						tr.vecEndPos = vecSrc;
						tr.flFraction = 0;
					}
					float flAdjustedDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;
					flAdjustedDamage = Q_max(0, flAdjustedDamage);

					if (tr.flFraction == 1.0f)
					{
						pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
					}
					else
					{
						tr.iHitgroup = HITGROUP_CHEST;
						ClearMultiDamage();
						pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
						ApplyMultiDamage(pevInflictor, pevAttacker);
					}

					CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
					if (pVictim->m_bIsZombie) // Zombie Knockback...
					{
						ApplyKnockbackData(pVictim, vecSpot - vecSrc, { 3000.0f, 700.0f, 1200.0f, 700.0f, 0.4f });
					}
				}
			}
		}

		int id = m_pPlayer->entindex();
		
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(id | 0x1000);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(1); // life
		WRITE_BYTE(30);  // width
		WRITE_BYTE(15);   // noise
		WRITE_BYTE(20);
		WRITE_BYTE(255);
		WRITE_BYTE(20);
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(25);		// speed
		MESSAGE_END();
		
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(id | 0x1000);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(1); // life
		WRITE_BYTE(20);  // width
		WRITE_BYTE(50);   // noise
		WRITE_BYTE(0);
		WRITE_BYTE(255);
		WRITE_BYTE(120);
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(25);		// speed
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(m_iSprEffect2);
		WRITE_BYTE(7);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();
#endif
	}

	void FireEvent(const Vector& vecDir)
	{
		int flags = 0;
		
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		 

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	}
    	
	float GetExplodeDamage() const
	{
		float flDamage = 50.f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 233.f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 100.f;
#endif
		return flDamage;
	}

	int m_iSprBeam;
	int m_iSprEffect2;
};
LINK_ENTITY_TO_CLASS(z4b_awpnvidia, CAWPNvidia)
}