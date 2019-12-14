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
#include "weapons/RadiusDamage.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using WeaponTemplate::Varibles::A;
using WeaponTemplate::Varibles::N;

struct Z4B_AK47X_WeaponData
{
	static constexpr const auto &DefaultReloadTime = 2.0s;
	static constexpr int ZoomFOV = 80;
	static constexpr const char* V_Model = "models/z4b/v_ak47x.mdl";
	static constexpr const char* P_Model = "models/z4b/pw_ak47x.mdl";
	static constexpr const char* W_Model = "models/z4b/pw_ak47x.mdl";
	static constexpr const char *EventFile = "events/ak47x.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "z4b_ak47x";
	static constexpr const char *AnimExtension = "ak47";
	static constexpr int MaxClip = 35;
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
	static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
	static constexpr const auto& SpreadCalcWalking = 0.07 * A + 0.04;
	static constexpr const auto& SpreadCalcDucking = 0.0 * A;
	static constexpr const auto& SpreadCalcDefault = 0.0275 * A;
	static constexpr const auto &CycleTime = 0.09s;
	static constexpr int DamageDefault = 36;
	static constexpr int DamageZB = 57;
	static constexpr int DamageZBS = 57;
	static constexpr const auto& AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyDefault = 0.2;
	static constexpr float AccuracyMax = 1.25;
	static constexpr float RangeModifier = 0.98;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };

	KickBackData KickBackWalking = { 0.9f, 0.35f, 0.225f, 0.05f, 5.5f, 2.5f, 8 };
	KickBackData KickBackNotOnGround = { 1.5f, 0.9f, 0.4f, 0.3f, 7.0f, 4.5f, 8 };
	KickBackData KickBackDucking = { 0.55f, 0.3f, 0.12f, 0.023f, 3.5f, 1.15f, 8 };
	KickBackData KickBackDefault = { 0.665f, 0.35f, 0.155f, 0.035f, 4.0f, 1.55f, 7 };

	static constexpr auto RadiusDamageRadius = 90;
	KnockbackData RadiusDamageKnockback = { 800.0f, 1000.0, 650.0f, 500.0f, 0.9f };

};

#ifndef CLIENT_DLL
	class CZ4B_AK47XGrenade : public CBaseEntity, public Z4B_AK47X_WeaponData
	{
	public:
		void Spawn() override
		{
			Precache();

			m_fSequenceLoops = 0;
			//ph26 = 0;
			SetThink(&CZ4B_AK47XGrenade::IgniteThink);
			SetTouch(&CZ4B_AK47XGrenade::OnTouch);
			SET_MODEL(this->edict(), "sprites/laserminigun3.spr");

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

			pev->classname = MAKE_STRING("d_ak47x");
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/laserminigun3.spr");
			m_iSprExplo = PRECACHE_MODEL("sprites/laserminigun_hit3.spr");
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

			RadiusDamage(*this, pev->origin, this, pev->owner);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSprExplo);	// sprite index
			WRITE_BYTE(6);			// scale in 0.1's
			WRITE_BYTE(70);			// brightness
			MESSAGE_END();

			Remove();
		}

		void EXPORT IgniteThink()
		{
			SetThink(&CZ4B_AK47XGrenade::FollowThink);
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

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, TeamName iTeam, int type = 0)
		{
			std::tie(m_flTouchDamage, RadiusDamageAmount, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, iTeam);
			m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
			m_iType = type;
		}

		int m_fSequenceLoops;
		time_point_t m_flRemoveTime;
		float m_flMaxFrames;
		float m_flTouchDamage;
		float RadiusDamageAmount;
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
	LINK_ENTITY_TO_CLASS(ak47x_grenade, CZ4B_AK47XGrenade)
#endif

	
    class CZ4B_AK47X : public LinkWeaponTemplate< CZ4B_AK47X,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TSecondaryAttackZoom,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault,
            TGetDamageDefault
    >, public Z4B_AK47X_WeaponData
    {
    public:


	void PrimaryAttack(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return Base::PrimaryAttack();

		FireLaserBomb(0, 0.5s, FALSE);
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

		--m_iClip;

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
		CZ4B_AK47XGrenade* pEnt = static_cast<CZ4B_AK47XGrenade*>(CBaseEntity::Create("ak47x_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			float flTouchDamage = g_pModRunning->DamageTrack() == DT_NONE ? 24 : 240;
			float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 40 : 400;
			pEnt->Init(gpGlobals->v_forward * 1000, flTouchDamage, flExplodeDamage, m_pPlayer->m_iTeam);
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
			KickBack(5.0, 2.0, 0.8, 0.4, 9.0, 9.0, 5);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(5.0, 3.0, 0.9, 0.5, 9.0, 9.0, 5);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(3.0, 1.0, 0.35, 0.125, 5.5, 5.5, 5);
		else
			KickBack(4.5, 1.8, 0.75, 0.375, 8.75, 8.75, 5);
	}
};
LINK_ENTITY_TO_CLASS(z4b_ak47x, CZ4B_AK47X)
}