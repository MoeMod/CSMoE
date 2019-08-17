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

#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

class CKnifeLcSword: public LinkWeaponTemplate<CKnifeLcSword,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_lc_sword";
	static constexpr const char *V_Model = "models/z4b/v_lc_sword.mdl";
	static constexpr const char *P_Model = "models/z4b/p_lc_sword.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 1.0f };

public:

	enum skullaxe_e
	{
		ANIM_IDLE1 = 0,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_DRAW,
		ANIM_STAB,
		ANIM_STAB_HIT
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/dragonsword_draw.wav");
		PRECACHE_SOUND("weapons/dragonsword_hit1.wav");
		PRECACHE_SOUND("weapons/dragonsword_hit2.wav");
		PRECACHE_SOUND("weapons/dragonsword_idle.wav");
		PRECACHE_SOUND("weapons/dragonsword_slash1.wav");
		PRECACHE_SOUND("weapons/dragonsword_slash2.wav");
		PRECACHE_SOUND("weapons/dragonsword_stab_hit.wav");
		PRECACHE_SOUND("weapons/dragonsword_wall.wav");
	}

	BOOL Deploy() override
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dragonsword_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		return Base::Deploy();
	}

	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

public:
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 76;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 720;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage() const
	{
		float flDamage = 72;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 680;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(z4b_lc_sword, CKnifeLcSword)

void CKnifeLcSword::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_iSwing++;
	if(m_iSwing & 1)
	{
		SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
		SetThink(&CKnifeLcSword::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.5s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	}
	else
	{
		SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);
		SetThink(nullptr);
		DelayPrimaryAttack();
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.22s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	}
}

void CKnifeLcSword::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

	SetThink(&CKnifeLcSword::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.658s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_iSwing = 0;
}

void CKnifeLcSword::DelayPrimaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	const float flRadius = m_iSwing & 1 ? 108:118;
	const float flAngle = m_iSwing & 1 ? 60 : 10;

	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		if(m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dragonsword_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dragonsword_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dragonsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dragonsword_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		break;
	}
	}
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

void CKnifeLcSword::DelaySecondaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 108, 72, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		SendWeaponAnim(ANIM_STAB_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		SendWeaponAnim(ANIM_STAB_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		SendWeaponAnim(ANIM_STAB_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
		fDidHit = TRUE;
		break;
	}
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

}
