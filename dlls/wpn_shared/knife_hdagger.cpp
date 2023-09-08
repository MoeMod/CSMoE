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

class CKnifeHDagger: public LinkWeaponTemplate<CKnifeHDagger,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "knife_hdagger";
	static constexpr const char *V_Model = "models/v_hdagger.mdl";
	static constexpr const char *P_Model = "models/p_hdagger.mdl";
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
        ANIM_SLASH2
	};

	void Precache() override
	{
		Base::Precache();
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
		float flDamage = 65;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_hdagger, CKnifeHDagger)

void CKnifeHDagger::Spawn()
{
	m_iKnifeID = KNIFE_HDAGGER;
	pev->classname = MAKE_STRING("knife_hdagger");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeHDagger::PrimaryAttack(void)
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

#ifndef CLIENT_DLL
	switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 48, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev, true))
	{
		case HIT_NONE:
		{
			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 3))
			{
			case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM); break;
			case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM); break;
			case 3: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM); break;
			}
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			break;
		}
	}
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.42s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
}

void CKnifeHDagger::SecondaryAttack(void)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_STAB_MISS, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
	switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 35, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);	
		break;
	}
	case HIT_PLAYER:
	{
		SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_stab.wav", VOL_NORM, ATTN_NORM);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		break;
	}
	}
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
}

}
