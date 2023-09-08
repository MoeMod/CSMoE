/*
knife_balrog9.cpp
Copyright (C) 2021 Original Author BTE Team


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
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	
#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

	class CKnifeBalrog9 : public LinkWeaponTemplate<CKnifeBalrog9,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_balrog9";
		static constexpr const char* V_Model = "models/v_balrog9.mdl";
		static constexpr const char* P_Model = "models/p_balrog9.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum balrog9_e
		{
			ANIM_IDLE = 0,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_SLASH3,
			ANIM_SLASH4,
			ANIM_SLASH5,
			ANIM_DRAW,
			ANIM_CHARGE_START1,
			ANIM_CHARGE_FINISH1,
			ANIM_CHARGE_IDLE1,
			ANIM_CHARGE_IDLE2,
			ANIM_CHARGE_ATTACK1,
			ANIM_CHARGE_ATTACK2,
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_balrog9.mdl");
			PRECACHE_MODEL("models/p_balrog9.mdl");

			PRECACHE_MODEL("sprites/muzzleflash20.spr");

			PRECACHE_SOUND("weapons/balrog9_slash1.wav");
			PRECACHE_SOUND("weapons/balrog9_slash2.wav");
			PRECACHE_SOUND("weapons/balrog9_hit1.wav");
			PRECACHE_SOUND("weapons/balrog9_hit2.wav");
			PRECACHE_SOUND("weapons/balrog9_hitwall.wav");
			PRECACHE_SOUND("weapons/balrog9_charge_attack2.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;
			m_iState = 0;
		
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			
			m_flTimeCheckCharge = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "dragontail", UseDecrement() != FALSE))
			{
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			}
			return TRUE;
		}

		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		void Holster(int skiplocal) override;
		void DelaySecondaryAttack();
		void OnAttack2Released();
		void PunchAxis(float x, float y, float x_min, float y_min);

		BOOL CanDrop() override { return false; }

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 35;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 150.0f;	//original 1836
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1240.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 58;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 350.0f;	//1785
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 245.0f;
#endif
			return flDamage;
		}

		float  GetChargeAttackDamage() const
		{
			float flDamage = 118;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1180.0f;	//580+500
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 906.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeCheckCharge;

		int m_iState;	//pev_iuser1
	};

	LINK_ENTITY_TO_CLASS(knife_balrog9, CKnifeBalrog9)

	void CKnifeBalrog9::Spawn()
	{
		m_iKnifeID = KNIFE_BALROG9;
		pev->classname = MAKE_STRING("knife_balrog9");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_flTimeCheckCharge = invalid_time_point;
		m_iClip = WEAPON_NOCLIP;
		m_iState = 0;
		FallInit();
	}

	int CKnifeBalrog9::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = NULL;
		p->iMaxAmmo1 = -1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

	void CKnifeBalrog9::PunchAxis(float x, float y, float x_min = -100.0, float y_min = -100.0)
	{
#ifndef CLIENT_DLL
		Vector vecPunchangle = m_pPlayer->pev->punchangle;
		vecPunchangle[0] += x;
		vecPunchangle[1] += y;

		vecPunchangle[0] = min(max(vecPunchangle[0], x_min), -x_min);
		vecPunchangle[1] = min(max(vecPunchangle[1], y_min), -y_min);

		m_pPlayer->pev->punchangle = vecPunchangle;	
#endif
	}

	void CKnifeBalrog9::PrimaryAttack(void)
	{
		if (m_iState)
		{
			OnAttack2Released();
			return;
		}

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/balrog9-slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

		Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
		int iAnim = RANDOM_LONG(ANIM_SLASH1, ANIM_SLASH5);
		SendWeaponAnim(iAnim, UseDecrement() != FALSE);

		bool fDidHit = FALSE;
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/balrog9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

		switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 50, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{
			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog9_hitwall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			break;
		}
		}

		if (fDidHit)
		{
			if (iAnim % 2)
				PunchAxis(0.8, 0.8);
			else
				PunchAxis(-0.8, -0.8);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.33s;
		}
			
		else
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8s;
#endif

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.25s;				
		
	}

	void CKnifeBalrog9::SecondaryAttack(void)
	{
		if (!m_iState)
		{
			SendWeaponAnim(ANIM_CHARGE_START1, UseDecrement() != FALSE);

			m_flTimeCheckCharge = gpGlobals->time + 0.73s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_iState = 1;
		}
		else
		{
			if (m_flTimeCheckCharge < gpGlobals->time)
			{
				switch (m_iState)
				{
				case 1:
				{
					SendWeaponAnim(ANIM_CHARGE_IDLE1, UseDecrement() != FALSE);

					m_iState = 2;

					m_flTimeCheckCharge = gpGlobals->time + 1.0s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
					break;
				}
				case 2:
				{
					SendWeaponAnim(ANIM_CHARGE_FINISH1, UseDecrement() != FALSE);

					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/balrog9_charge_finish1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

					m_iState = 3;

					m_flTimeCheckCharge = gpGlobals->time + 0.33s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33s;
					break;
				}
				case 3:
				{
					SendWeaponAnim(ANIM_CHARGE_IDLE2, UseDecrement() != FALSE);

					m_iState = 4;

					m_flTimeCheckCharge = gpGlobals->time + 99999s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
					break;	
				}
				}
			}
		}
	}

	void CKnifeBalrog9::OnAttack2Released(void)
	{
		switch (m_iState)
		{
		case 1:
		case 2:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iSwing = 0;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

			SendWeaponAnim(ANIM_CHARGE_ATTACK1, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/balrog9-slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

			DelaySecondaryAttack();

			break;
		}
		case 3:
		case 4:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iSwing = 1;
			SendWeaponAnim(ANIM_CHARGE_ATTACK2, UseDecrement() != FALSE);

			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/balrog9_v_charge_attack1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			DelaySecondaryAttack();

			break;
		}
		}
		m_iState = 0;
	}

	void CKnifeBalrog9::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL
		float flDamage, flRadius, flAngleDegrees;
		flAngleDegrees = m_iSwing ? 360 : 60;
		flDamage = m_iSwing ? GetChargeAttackDamage() : GetSecondaryAttackDamage();
		flRadius = m_iSwing ? 90 : 85;
		
		CBaseEntity* pEntity = NULL;
		TraceResult tr;
		
		Vector vecEnd, vecAngle;
		vecAngle = m_pPlayer->pev->v_angle;
		UTIL_MakeVectors(vecAngle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		if (m_iSwing)
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, vecSrc + gpGlobals->v_forward * 35, g_vecZero, 0.0, 0.0, 21, 0, TRUE, FALSE);

		float c = cos(flAngleDegrees * 0.5 * M_PI / flRadius);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (!AngleCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
				continue;

			float flRatio = 1.0;
			int bitsDamageType = DMG_BULLET | DMG_NEVERGIB;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}

			ClearMultiDamage();
			pEntity->TraceAttack(m_pPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		
			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
			if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
			{
				Vector vecDir = (pPlayer->pev->origin - m_pPlayer->pev->origin);
				if (m_iSwing)
				{
					ApplyKnockbackData(pPlayer, vecDir, { 500, 400, 400, 500, 1.0f });
					pPlayer->pev->velocity.z = 200;
				}
				else
				{
					ApplyKnockbackData(pPlayer, vecDir, { 400, 300, 300, 400, 1.0f });
					pPlayer->pev->velocity.z = 150;
				}		
			}
		}
#endif
		m_iState = 0;
		m_iSwing = 0;
	}

	float CKnifeBalrog9::GetMaxSpeed(void)
	{
			return 250;
	}

	void CKnifeBalrog9::Holster(int skiplocal)
	{
		m_iState = 0;
		SetThink(nullptr);

		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeBalrog9::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_iState && !(m_pPlayer->pev->button & IN_ATTACK2))
			OnAttack2Released();

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	}
}
