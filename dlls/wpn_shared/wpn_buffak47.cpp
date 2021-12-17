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

struct BuffAK47_WeaponData
{
	static constexpr const auto &DefaultReloadTime = 2.0s;
	static constexpr int ZoomFOV = 80;
	static constexpr const char* V_Model = "models/v_buffak.mdl";
	static constexpr const char* P_Model = "models/p_buffak.mdl";
	static constexpr const char* W_Model = "models/w_buffak.mdl";
	static constexpr const char *EventFile = "events/ak47x.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_buffak";
	static constexpr const char *AnimExtension = "ak47";
	static constexpr int MaxClip = 50;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3
	};
	static constexpr float PlayerMaxSpeed = 235;
	static constexpr float ArmorRatioModifier = 1.4f;
	static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
	static constexpr const auto& SpreadCalcWalking = 0.07 * A + 0.04;
	static constexpr const auto& SpreadCalcDucking = 0.0 * A;
	static constexpr const auto& SpreadCalcDefault = 0.0275 * A;
	static constexpr const auto &CycleTime = 0.09s;
	static constexpr int DamageDefault = 38;
	static constexpr int DamageZB = 115;
	static constexpr int DamageZBS = 115;
	static constexpr const auto& AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyDefault = 0.2;
	static constexpr float AccuracyMax = 1.25;
	static constexpr float RangeModifier = 0.98;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 0.9f, 0.35f, 0.225f, 0.05f, 5.5f, 2.5f, 8 };
	KickBackData KickBackNotOnGround = { 1.5f, 0.9f, 0.4f, 0.3f, 7.0f, 4.5f, 8 };
	KickBackData KickBackDucking = { 0.55f, 0.3f, 0.12f, 0.023f, 3.5f, 1.15f, 8 };
	KickBackData KickBackDefault = { 0.665f, 0.35f, 0.155f, 0.035f, 4.0f, 1.55f, 7 };

	static constexpr auto RadiusDamageRadius = 90;
	KnockbackData RadiusDamageKnockback = { 800.0f, 1100.0, 650.0f, 500.0f, 0.9f };

};

#ifndef CLIENT_DLL
	class CBuffAK47Grenade : public CBaseEntity, public BuffAK47_WeaponData
	{
	public:
		void Spawn() override
		{
			Precache();

			m_fSequenceLoops = 0;
			//ph26 = 0;
			SetThink(&CBuffAK47Grenade::FollowThink);
			SetTouch(&CBuffAK47Grenade::OnTouch);
			SET_MODEL(this->edict(), "sprites/muzzleflash19.spr");

			//ph32 = ?
			pev->rendermode = kRenderTransAdd;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 180;
			pev->framerate = 10;
			pev->scale = 0.05;
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);

			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			pev->classname = MAKE_STRING("d_buffak");
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/ef_buffak_hit.spr");
			PRECACHE_MODEL("sprites/muzzleflash19.spr");
			m_iSprExplo = PRECACHE_MODEL("sprites/ef_buffak_hit.spr");
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSprExplo);	// sprite index
			WRITE_BYTE(6);			// scale in 0.1's
			WRITE_BYTE(70);			// brightness
			MESSAGE_END();

			if (pev->owner == pOther->edict())
			{
				SetTouch(NULL);
				SetThink(NULL);
				Remove();
				return;
			}
				

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

			SetTouch(NULL);
			SetThink(NULL);
			Remove();
		}

		void EXPORT FollowThink()
		{
			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flMaxFrames)
				pev->frame = fmod(pev->frame, m_flMaxFrames);

			pev->nextthink = gpGlobals->time + 0.01s;
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
	LINK_ENTITY_TO_CLASS(buffak_grenade, CBuffAK47Grenade)
#endif

	
    class CBuffAK47 : public LinkWeaponTemplate< CBuffAK47,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TSecondaryAttackZoom,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault,
            TGetDamageDefault
    >, public BuffAK47_WeaponData
    {
    public:

	void Precache(void) override
	{
		Base::Precache();

		PRECACHE_MODEL("sprites/muzzleflash40.spr");
		PRECACHE_MODEL("sprites/muzzleflash41.spr");
		PRECACHE_MODEL("sprites/muzzleflash42.spr");

	}

	void PrimaryAttack(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return Base::PrimaryAttack();

		FireLaserBomb(0, 0.5s, FALSE);
	}


	void Reload(void) override
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(50, ANIM_RELOAD, 2.0s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.2;
			m_iShotsFired = 0;
			m_bDelayFire = false;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		}
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
		CBuffAK47Grenade* pEnt = static_cast<CBuffAK47Grenade*>(CBaseEntity::Create("buffak_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			float flTouchDamage = g_pModRunning->DamageTrack() == DT_NONE ? 70 : 280;
			float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 10 : 400;
			pEnt->Init(gpGlobals->v_forward * 2000, flTouchDamage, flExplodeDamage, m_pPlayer->m_iTeam);
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
LINK_ENTITY_TO_CLASS(weapon_buffak, CBuffAK47)
}