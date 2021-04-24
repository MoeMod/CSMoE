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

class CKnifeKatana: public LinkWeaponTemplate<CKnifeKatana,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "knife_katana";
	static constexpr const char *V_Model = "models/v_katana.mdl";
	static constexpr const char *P_Model = "models/p_katana.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 1.0f };

public:

	enum
	{
		ANIM_IDLE1,
		ANIM_NONE1,
		ANIM_NONE2,
		ANIM_DRAW,
		ANIM_STAB,
		ANIM_STABMISS,
		ANIM_SLASH1,
		ANIM_SLASH2
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/mastercombat_hit1.wav");
		PRECACHE_SOUND("weapons/mastercombat_wall.wav");
		PRECACHE_SOUND("weapons/katana_draw.wav");
		PRECACHE_SOUND("weapons/katana_midslash1.wav");
		PRECACHE_SOUND("weapons/katana_midslash2.wav");
		PRECACHE_SOUND("weapons/katana_stap.wav");
	}

	BOOL Deploy() override
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		return Base::Deploy();
	}

	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void DelaySecondaryAttack();
	BOOL CanDrop() override { return false; }

public:
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 38;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 5.5f;
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
			flDamage *= 7.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 7.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_katana, CKnifeKatana)

void CKnifeKatana::PrimaryAttack(void)
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
	switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 50, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
		case HIT_NONE:
		{
			if (m_iSwing & 1)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katana_midslash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katana_midslash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			break;
			
		}
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.37s;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.37s;
			break;
			
		}
	}
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeKatana::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);
	SetThink(&CKnifeKatana::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.5s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.1s;
	m_iSwing = 0;
}

void CKnifeKatana::DelaySecondaryAttack(void)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 75, 90, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
		case HIT_NONE:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katana_stap.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
			fDidHit = TRUE;
			break;
		}
	}
#endif
}

}
