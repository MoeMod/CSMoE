/*
knife_tomahawk.cpp
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

	class CKnifeTomaHawk : public LinkWeaponTemplate<CKnifeTomaHawk,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_tomahawk";
		static constexpr const char* V_Model = "models/v_tomahawk.mdl";
		static constexpr const char* P_Model = "models/p_tomahawk.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "tomahawk";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum tomahawk_e
		{
			ANIM_IDLE,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_DRAW,
			ANIM_STAB,
			ANIM_STAB_MISS,
			ANIM_MIDSLASH1,
			ANIM_MINDSLASH2,		
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_tomahawk.mdl");
			PRECACHE_MODEL("models/p_tomahawk.mdl");
			
			PRECACHE_SOUND("weapons/tomahawk_wall.wav");
			PRECACHE_SOUND("weapons/tomahawk_slash1.wav");
			PRECACHE_SOUND("weapons/tomahawk_slash1_hit.wav");
			PRECACHE_SOUND("weapons/tomahawk_slash2.wav");
			PRECACHE_SOUND("weapons/tomahawk_slash2_hit.wav");
			PRECACHE_SOUND("weapons/tomahawk_stab_miss.wav");
			PRECACHE_SOUND("weapons/tomahawk_stab_hit.wav");
			PRECACHE_SOUND("weapons/tomahawk_draw.wav");
		}
		
		BOOL Deploy(void) override
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_draw.wav", 0.3, 2.4);

			m_fMaxSpeed = 250;

			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "tomahawk", UseDecrement() != FALSE);	
		}

		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		BOOL CanDrop() override { return false; }

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 37.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 740.0f;	//original 1836
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 70.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 80.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1500.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 110.0f;
#endif
			return flDamage;
		}
		//pev->iuser1 Signal
	};

	LINK_ENTITY_TO_CLASS(knife_tomahawk, CKnifeTomaHawk)

	void CKnifeTomaHawk::Spawn()
	{
		m_iKnifeID = KNIFE_TOMAHAWK;
		pev->classname = MAKE_STRING("knife_tomahawk");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_iClip = WEAPON_NOCLIP;

		FallInit();
	}

	int CKnifeTomaHawk::GetItemInfo(ItemInfo* p)
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

	void CKnifeTomaHawk::PrimaryAttack(void)
	{
		SendWeaponAnim(m_iSwing++ % 2 ? ANIM_SLASH2 : ANIM_SLASH1, UseDecrement() != FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.175s;

#ifndef CLIENT_DLL
		Vector vecSrc, vecEnd;
		TraceResult tr;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		vecSrc = m_pPlayer->GetGunPosition();
		vecEnd = vecSrc + gpGlobals->v_forward * 45;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1.0)
		{
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction >= 1.0)
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, m_iSwing ? "weapons/tomahawk_slash1.wav" : "weapons/tomahawk_slash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				return;
			}

			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
			{
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));
			}

			vecEnd = tr.vecEndPos;
		}

		if (tr.flFraction < 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetPrimaryAttackDamage(), gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, m_iSwing ? "weapons/tomahawk_slash1_hit.wav" : "weapons/tomahawk_slash2_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, m_iSwing ? "weapons/tomahawk_slash1.wav" : "weapons/tomahawk_slash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#endif
	}

	void CKnifeTomaHawk::SecondaryAttack(void)
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

#ifndef CLIENT_DLL
		int bitsDamageType = DMG_BULLET | DMG_NEVERGIB;

		Vector vecSrc, vecEnd;
		TraceResult tr;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		vecSrc = m_pPlayer->GetGunPosition();
		vecEnd = vecSrc + gpGlobals->v_forward * 65;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1.0)
		{
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction >= 1.0)
			{
				SendWeaponAnim(ANIM_STAB_MISS, UseDecrement() != FALSE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				return;
			}

			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
			{
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));
			}

			vecEnd = tr.vecEndPos;
		}

		if (tr.flFraction < 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			ClearMultiDamage();

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				Vector2D vec2LOS;
				Vector vecForward = gpGlobals->v_forward;

				UTIL_MakeVectors(pEntity->pev->angles);

				vec2LOS = vecForward.Make2D();
				vec2LOS = vec2LOS.Normalize();

				if (DotProduct(vec2LOS, gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}

			Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
			UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 200.0, 200);

			pEntity->TraceAttack(m_pPlayer->pev, GetSecondaryAttackDamage() * flRatio, gpGlobals->v_forward, &tr, bitsDamageType);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_stab_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);
		}
		else
		{
			SendWeaponAnim(ANIM_STAB_MISS, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
#endif
	}


	float CKnifeTomaHawk::GetMaxSpeed(void)
	{
		return 250;
	}

	void CKnifeTomaHawk::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		
	}
}
