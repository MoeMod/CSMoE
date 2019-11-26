/*
z4b_m4a1mw.cpp
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

#ifndef CLIENT_DLL
	class CZ4B_M4A1MWGrenade : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();

			m_fSequenceLoops = 0;
			//ph26 = 0;
			SetThink(&CZ4B_M4A1MWGrenade::IgniteThink);
			SetTouch(&CZ4B_M4A1MWGrenade::OnTouch);
			SET_MODEL(this->edict(), "sprites/z4b/m4a1mw_laser.spr");

			//ph32 = ?
			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 180;
			pev->framerate = 0.5;
			pev->scale = 0.2;
			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_FLY;
			pev->nextthink = gpGlobals->time + 0.1s;
			m_flRemoveTime = gpGlobals->time + 500ms;
			m_flMaxFrames = 300.0;
			UTIL_SetSize(pev, { -0.1, -0.1, -0.1 }, { 0.1, 0.1, 0.1 });

			pev->classname = MAKE_STRING("d_m4a1mw");
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/z4b/m4a1mw_laser.spr");
			PRECACHE_SOUND("weapons/m4a1mw_hit.wav");
			m_iSprExplo = PRECACHE_MODEL("sprites/z4b/m4a1mw_hit.spr");
		}

		KnockbackData GetKnockBackData()
		{
			return {  };
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			if (pev->owner == pOther->edict())
				return;

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (pAttackePlayer &&
				pOther->pev->takedamage != DAMAGE_NO &&
				pOther->IsAlive()
				)
			{
				Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

				TraceResult tr;
				UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
				tr.iHitgroup = HITGROUP_CHEST; // ...

				ClearMultiDamage();
				pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
				ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);
			}

			RadiusDamage();
			Remove();
		}

		void EXPORT IgniteThink()
		{
			SetThink(&CZ4B_M4A1MWGrenade::FollowThink);
			pev->nextthink = gpGlobals->time + 100ms;
		}

		void EXPORT FollowThink(void)
		{
			UTIL_MakeAimVectors(pev->angles);

			pev->scale += 0.01;

			constexpr auto Frames = 33;
			pev->frame = (static_cast<int>(pev->frame) + 1) % Frames;
			pev->nextthink = gpGlobals->time + 45ms;

			if (gpGlobals->time > m_flRemoveTime)
			{
				if ((pev->renderamt -= 15) < 5)
				{
					Remove();
				}
			}
		}

		void RadiusDamage()
		{
			const float flRadius = m_flExplodeRadius;
			const float flDamage = m_flExplodeDamage;
			const Vector vecSrc = pev->origin;
			entvars_t* const pevAttacker = VARS(pev->owner);
			entvars_t* const pevInflictor = this->pev;
			int bitsDamageType = DMG_BULLET;

			TraceResult tr;
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

					/*if (pEntity->pev == pevAttacker)
						continue;*/

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

						CBasePlayer* pVictim = dynamic_cast<CBasePlayer*>(pEntity);
						if (pVictim->m_bIsZombie) // Zombie Knockback...
						{
							ApplyKnockbackData(pVictim, vecSpot - vecSrc, { 450.0f, 350.0, 400.0f, 200.0f, 0.6f });
						}
					}
				}
			}

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSprExplo);
			WRITE_BYTE(3);
			WRITE_BYTE(70);
			MESSAGE_END();

			EMIT_SOUND_DYN(edict(), CHAN_AUTO, "weapons/m4a1mw_hit.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, float flExplodeRadius, TeamName iTeam, int type = 0)
		{
			std::tie(m_flTouchDamage, m_flExplodeDamage, m_flExplodeRadius, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, flExplodeRadius, iTeam);
			m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
			m_iType = type;
		}

		int m_fSequenceLoops;
		time_point_t m_flRemoveTime;
		float m_flMaxFrames;
		float m_flTouchDamage;
		float m_flExplodeDamage;
		float m_flExplodeRadius;
		TeamName m_iTeam;
		int m_iSprEffect;
		int m_iSprExplo;
		Vector m_vecStartVelocity;
		int m_iType;

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};
	LINK_ENTITY_TO_CLASS(m4a1mw_grenade, CZ4B_M4A1MWGrenade)
#endif

	
    class CZ4B_M4A1MW : public LinkWeaponTemplate< CZ4B_M4A1MW,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M4A1>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TSecondaryAttackZoom,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault,
            TGetDamageDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.0s;
    static constexpr int ZoomFOV = 65;
	static constexpr const char* V_Model = "models/z4b/v_m4a1mw.mdl";
	static constexpr const char* P_Model = "models/z4b/p_m4a1mw.mdl";
	static constexpr const char* W_Model = "models/z4b/w_m4a1mw.mdl";
    static constexpr const char *EventFile = "events/m4a1mw.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_m4a1mw";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 36;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3
	};
	static constexpr float MaxSpeed = 235;
	static constexpr float ArmorRatioModifier = 1.4f;
	static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.035;
	static constexpr const auto& SpreadCalcWalking = 0.025 * A + 0.07;
	static constexpr const auto& SpreadCalcDucking = 0.0 * A;
	static constexpr const auto& SpreadCalcDefault = 0.02 * A;
    static constexpr const auto &CycleTime = 0.09s;
    static constexpr int DamageDefault = 33;
    static constexpr int DamageZB = 46;
    static constexpr int DamageZBS = 46;
	static constexpr const auto& AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyDefault = 0.19;
	static constexpr float AccuracyMax = 1.25;
    static constexpr float RangeModifier = 0.96;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    static constexpr int Penetration = 1;
    KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };

	KickBackData KickBackWalking = { 1.0f, 0.45f, 0.28f, 0.045f, 3.75f, 3.0f, 7 };
	KickBackData KickBackNotOnGround = { 1.2f, 0.5f, 0.23f, 0.15f, 5.5f, 3.5f, 6 };
	KickBackData KickBackDucking = { 0.6f, 0.3f, 0.2f, 0.0125f, 3.25f, 2.0f, 7 };
	KickBackData KickBackDefault = { 0.65f, 0.35f, 0.25f, 0.015f, 3.5f, 2.25f, 7 };

	void PrimaryAttack(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return Base::PrimaryAttack();

		FireLaserBomb(0, 0.2s, FALSE);
	}

	void FireLaserBomb(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = AccuracyCalc(N = m_flAccuracy);

		if (m_flAccuracy > AccuracyMax)
			m_flAccuracy = AccuracyMax;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_iClip = std::max(m_iClip - 3, 0);

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

		Vector vecDir;

#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		CZ4B_M4A1MWGrenade* pEnt = static_cast<CZ4B_M4A1MWGrenade*>(CBaseEntity::Create("m4a1mw_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			float flTouchDamage = g_pModRunning->DamageTrack() == DT_NONE ? 15 : 120;
			float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 30 : 200;
			pEnt->Init(gpGlobals->v_forward * 1000, flTouchDamage, flExplodeDamage, 75, m_pPlayer->m_iTeam);
		}
#endif

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

		int flags = 0;

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 0.2s;

		if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.7, 0.275, 0.425, 0.0335, 3.5, 2.5, 8);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.9, 0.4, 0.3, 7.0, 4.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.5, 0.12, 0.24, 0.001, 3.15, 2.1, 6);
		else
			KickBack(0.595, 0.225, 0.32, 0.0115, 3.25, 2.1, 7);
	}
};
LINK_ENTITY_TO_CLASS(z4b_m4a1mw, CZ4B_M4A1MW)
}