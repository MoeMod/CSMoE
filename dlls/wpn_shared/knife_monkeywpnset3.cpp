/*
knife_monkeywpnset3.cpp
Copyright (C) 2019 JustANoobQAQ

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

	class CKnifeMonkeyWpnSet3 : public LinkWeaponTemplate<CKnifeMonkeyWpnSet3,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
		TWeaponIdleDefault,
		TDeployDefault
	>
	{
	public:
		int iAnim = 2;
		static constexpr const char* ClassName = "knife_monkeywpnset3";
		static constexpr const char* V_Model = "models/v_monkeywpnset3.mdl";
		static constexpr const char* P_Model = "models/p_monkeywpnset3.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "dragonsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum monkeywpnset3_e
		{
			ANIM_IDLE1 = 0,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_DRAW,
			ANIM_STAB,
			ANIM_STAB_HIT,
		};

		void Precache() override
		{
			Base::Precache();

			PRECACHE_SOUND("weapons/skullaxe_wall.wav");
			PRECACHE_SOUND("weapons/monkeyswpnset3_hit.wav");
			PRECACHE_SOUND("weapons/monkeyswpnset3_hit1.wav");
			PRECACHE_SOUND("weapons/monkeyswpnset3_hit2.wav");
		}


		float GetMaxSpeed(void) override
		{
			if (m_iWeaponState)
				return 270;

			return 250;
		}

		BOOL Deploy() override
		{
			m_iWeaponState = 0;
			m_fMaxSpeed = 250;
			m_iSwing = 0;

			return Base::Deploy();
		}

		void Spawn() override;
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		BOOL CanDrop() override { return false; }

	public:

		void DelayPrimaryAttack();
		void DelaySecondaryAttack();
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 65.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 720.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4620.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 70.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1540.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 5390.0f;
#endif
			return flDamage;
		}
	};

	LINK_ENTITY_TO_CLASS(knife_monkeywpnset3, CKnifeMonkeyWpnSet3)

	void CKnifeMonkeyWpnSet3::Spawn()
	{
		m_iKnifeID = KNIFE_MONKEYWPNSET3;
		pev->classname = MAKE_STRING("knife_monkeywpnset3");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_iClip = WEAPON_NOCLIP;
		FallInit();
	}

	void CKnifeMonkeyWpnSet3::SecondaryAttack(void)
	{
		m_iSwing = 0;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

		SetThink(&CKnifeMonkeyWpnSet3::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.6s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.6s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	}

	void CKnifeMonkeyWpnSet3::PrimaryAttack(void)
	{

		if (!m_iSwing)
		{
			SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.75s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

			SetThink(&CKnifeMonkeyWpnSet3::DelayPrimaryAttack);
			pev->nextthink = gpGlobals->time + 0.2s;
		}
		else
		{
			SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

			SetThink(&CKnifeMonkeyWpnSet3::DelayPrimaryAttack);
			pev->nextthink = gpGlobals->time + 0.15s;
		}
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_iSwing++;
		m_iSwing %= 2;
	}

	void CKnifeMonkeyWpnSet3::DelaySecondaryAttack()
	{
#ifndef CLIENT_DLL
		Vector vecSrc, vecEnd;
		TraceResult tr;
		CBaseEntity* pEntity = NULL;
		BOOL fHit = FALSE;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(360.0 * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 197.0)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (!AngleCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 197.0, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct(vecForward2D, gpGlobals->v_forward.Make2D()) > 0.8)
					flRatio = 3.0;
			}

			m_pPlayer->m_iWeaponVolume = 128;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, m_iWeaponState ? "weapons/monkeywpnset3_slash3.wav" : "weapons/monkeywpnset3_slash3_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetSecondaryAttackDamage(), (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
			//UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 2000.0, 200);

			fHit = TRUE;
		}

		if (!fHit)
		{
			static int calced = 0;
			static float calcRes = 0.0f;

			if (!(calced & 1))
			{
				calced |= 1;
				calcRes = sqrt(512.0);
			}

			vecEnd = vecSrc + vecForward * (110.0 - calcRes);

			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
			else
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/dragonsword_stab_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
		}
		SetThink(NULL);
#endif
	}

	void CKnifeMonkeyWpnSet3::DelayPrimaryAttack()
	{
#ifndef CLIENT_DLL
		Vector vecSrc, vecEnd;
		TraceResult tr;
		CBaseEntity* pEntity = NULL;
		BOOL fHit = FALSE;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float flRadius = m_iSwing ? 138.0 : 148.0;
		float flAngle = m_iSwing ? 180.0 : 120.0;
		float c = cos(flAngle * 0.5 * M_PI / 180.0);

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

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct(vecForward2D, gpGlobals->v_forward.Make2D()) > 0.8)
					flRatio = 3.0;
			}

			m_pPlayer->m_iWeaponVolume = 128;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, m_iSwing ? "weapons/monkeyswpnset3_hit1.wav" : "weapons/monkeyswpnset3_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetPrimaryAttackDamage() * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			fHit = TRUE;
		}

		if (!fHit)
		{
			static int calced = 0;
			static float calcRes = 0.0f;

			if (!(calced & 1))
			{
				calced |= 1;
				calcRes = sqrt(512.0);
			}

			vecEnd = vecSrc + vecForward * (70.0 - calcRes);

			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
			else
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/monkeywpnset3_slash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
		}
		else
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6s;

		SetThink(NULL);
#endif
	}
}