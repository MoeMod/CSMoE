/*
knife_skullaxe.cpp
Copyright (C) 2019 Moemod Hyakuya

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

class CKnifeHammer: public LinkWeaponTemplate<CKnifeHammer,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_hammer";
	static constexpr const char *V_Model = "models/v_hammer.mdl";
	static constexpr const char *P_Model = "models/p_hammer.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

	enum
	{
		HAMMER_IDLE,
		HAMMER_SLASH,
		HAMMER_DRAW,
		HAMMER_IDLE_STAB,
		HAMMER_STAB,
		HAMMER_DRAW_STAB,
		HAMMER_MOVE_STAB,
		HAMMER_MOVE_SLASH
	};
	void Precache() override
	{
		Base::Precache();
	
		PRECACHE_SOUND("weapons/hammer_draw.wav");
		PRECACHE_SOUND("weapons/hammer_hit_slash.wav");
		PRECACHE_SOUND("weapons/hammer_hit_stab.wav");
		PRECACHE_SOUND("weapons/hammer_swing.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = m_iWeaponState ? 120 : 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/hammer_draw.wav", 0.3, 2.4);

		return DefaultDeploy("models/v_hammer.mdl", "models/p_hammer.mdl", m_iWeaponState ? HAMMER_DRAW_STAB : HAMMER_DRAW, "hammer", UseDecrement() != FALSE);
	}

	void Spawn() override;
	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }
	void DelayPrimaryAttack();
public:
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 34;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage() const
	{
		float flDamage = 60;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_hammer, CKnifeHammer)

void CKnifeHammer::Spawn()
{
	m_iKnifeID = KNIFE_HAMMER;
	pev->classname = MAKE_STRING("knife_hammer");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeHammer::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	if (!m_iWeaponState)
	{
		SendWeaponAnim(HAMMER_SLASH, UseDecrement() != FALSE);

		SetThink(&CKnifeHammer::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.9s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.8s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s + 0.5s;

		return;
	}

	SendWeaponAnim(HAMMER_STAB, UseDecrement() != FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s + 0.5s;

#ifndef CLIENT_DLL
	Vector vecSrc, vecEnd;
	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	vecSrc = m_pPlayer->GetGunPosition();
	vecEnd = vecSrc + gpGlobals->v_forward * 80;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1.0)
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

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
				flRatio = 3.0;
		}

		pEntity->TraceAttack(m_pPlayer->pev, 500 * flRatio, gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);

		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
		{
			Vector2D vecDir2D = (pPlayer->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

			pPlayer->pev->velocity.x = vecDir2D.x * 800;
			pPlayer->pev->velocity.y = vecDir2D.y * 800;
			pPlayer->pev->velocity.z = 400;
		}

		if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_hit_stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#endif
}

void CKnifeHammer::SecondaryAttack(void)
{
	m_iWeaponState = m_iWeaponState ? 0 : 1;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s + 0.5s;

	if (m_iWeaponState)
	{
		SendWeaponAnim(HAMMER_MOVE_STAB, UseDecrement() != FALSE);

		m_fMaxSpeed = 120;
	}
	else
	{
		SendWeaponAnim(HAMMER_MOVE_SLASH, UseDecrement() != FALSE);

		m_fMaxSpeed = 250;
	}

	m_pPlayer->ResetMaxSpeed();
}


void CKnifeHammer::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	Vector vecSrc, vecEnd;
	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	vecSrc = m_pPlayer->GetGunPosition();
	vecEnd = vecSrc + gpGlobals->v_forward * 75;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1.0)
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

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
				flRatio = 3.0;
		}

		pEntity->TraceAttack(m_pPlayer->pev, 800 * flRatio, gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);

		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_hit_slash.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/hammer_swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#endif

	SetThink(NULL);
}

void CKnifeHammer::WeaponIdle()
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(m_iWeaponState ? HAMMER_IDLE_STAB : HAMMER_IDLE, UseDecrement() != FALSE);
}

}
