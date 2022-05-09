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

class CKnifeJKnife: public LinkWeaponTemplate<CKnifeJKnife,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "knife_jknife";
	static constexpr const char *V_Model = "models/v_jknife.mdl";
	static constexpr const char *P_Model = "models/p_jknife.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

	enum
	{
        ANIM_IDLE1,
        ANIM_NONE1,
        ANIM_NONE2,
        ANIM_DRAW,
        ANIM_STAB,
        ANIM_STAB_MISS,
        ANIM_SLASH1,
        ANIM_SLASH2,
		ANIM_STAB2,
		ANIM_STAB3,
	};

	void Precache() override
	{
		Base::Precache();
		
		PRECACHE_SOUND("weapons/jknife_slash1.wav");
		PRECACHE_SOUND("weapons/jknife_slash2.wav");
		PRECACHE_SOUND("weapons/jknife_hit1.wav");
		PRECACHE_SOUND("weapons/jknife_hit2.wav");
		PRECACHE_SOUND("weapons/jknife_hitwall1.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		return Base::Deploy();
	}

	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

public:
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 15;
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
		float flDamage = 38;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_jknife, CKnifeJKnife)

void CKnifeJKnife::Spawn()
{
	pev->classname = MAKE_STRING("knife_jknife");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeJKnife::PrimaryAttack(void)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_iSwing++;
	if(m_iSwing & 1)
		SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
	else
		SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);

	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/jknife_slash1.wav", VOL_NORM, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/jknife_slash2.wav", VOL_NORM, ATTN_NORM); break;
	}

#ifndef CLIENT_DLL
	switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 40, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev, true))
	{
		case HIT_NONE:
		{		
			break;
		}
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 1))
			{
			case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/jknife_hit1.wav", VOL_NORM, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/jknife_hit2.wav", VOL_NORM, ATTN_NORM); break;
			}
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/jknife_hitwall1.wav", VOL_NORM, ATTN_NORM);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			break;
		}
	}
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.35s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
}

void CKnifeJKnife::SecondaryAttack(void)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_STAB_MISS, UseDecrement() != FALSE);

	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/jknife_slash1.wav", VOL_NORM, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/jknife_slash2.wav", VOL_NORM, ATTN_NORM); break;
	}
#ifndef CLIENT_DLL
	switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 35, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{		
		break;
	}
	case HIT_PLAYER:
	{
		switch (pev->iuser1)
		{
		case 0:SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE); break;
		case 1:SendWeaponAnim(ANIM_STAB2, UseDecrement() != FALSE); break;
		case 2:SendWeaponAnim(ANIM_STAB3, UseDecrement() != FALSE); break;
		}

		switch (RANDOM_LONG(0, 1))
		{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/jknife_hit1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/jknife_hit2.wav", VOL_NORM, ATTN_NORM); break;
		}
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		switch (pev->iuser1)
		{
		case 0:SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE); break;
		case 1:SendWeaponAnim(ANIM_STAB2, UseDecrement() != FALSE); break;
		case 2:SendWeaponAnim(ANIM_STAB3, UseDecrement() != FALSE); break;
		}
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/jknife_hitwall1.wav", VOL_NORM, ATTN_NORM);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		break;
	}
	}
	pev->iuser1++;
	if (pev->iuser1 > 2)
		pev->iuser1 = 0;
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
}

}
