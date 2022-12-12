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

class CKnifeSFSword: public LinkWeaponTemplate<CKnifeSFSword,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_sfsword";
	static constexpr const char *V_Model = "models/v_sfsword.mdl";
	static constexpr const char *P_Model = "models/p_sfsword_on.mdl";
	static constexpr const char* P_Model_1 = "models/p_sfsword_off.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	int m_iSlash;

public:
	enum
	{
		SFSWORD_IDLE,
		SFSWORD_ON,
		SFSWORD_OFF,
		SFSWORD_DRAW,
		SFSWORD_STAB,
		SFSWORD_STAB_MISS,
		SFSWORD_SLASH1,
		SFSWORD_SLASH2,
		SFSWORD_SLASH3,
		SFSWORD_OFF_IDLE,
		SFSWORD_OFF_SLASH,
	};

	void Precache() override
	{
		Base::Precache();
	
		PRECACHE_SOUND("weapons/skullaxe_wall.wav");
		PRECACHE_SOUND("weapons/sfsword_off_hit.wav");
		PRECACHE_SOUND("weapons/sfsword_hit1.wav");
		PRECACHE_SOUND("weapons/sfsword_hit2.wav");
		PRECACHE_SOUND("weapons/sfsword_wall1.wav");
		PRECACHE_SOUND("weapons/sfsword_wall2.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iSlash = 0;
		m_iWeaponState = 0;

		return DefaultDeploy(V_Model, P_Model, SFSWORD_DRAW, "sfsword", UseDecrement() != FALSE, 1.5s);
	}

	void Spawn() override;
	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }
	void DelayPrimaryAttack();
	void DelayOffAttack();
	void DelayMidslash();

public:
	float GetPrimaryAttackDamage() const
	{
		float flDamage = m_iSlash ? 65 : 39;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	float GetMidslashDamage() const
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

LINK_ENTITY_TO_CLASS(knife_sfsword, CKnifeSFSword)

void CKnifeSFSword::Spawn()
{
	pev->classname = MAKE_STRING("knife_sfsword");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeSFSword::DelayOffAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

	switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 40, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		break;

	}
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_off_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;

	}
	}
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeSFSword::PrimaryAttack(void)
{
	if (m_iWeaponState)
	{
		SendWeaponAnim(SFSWORD_OFF_SLASH, UseDecrement() != FALSE);

		SetThink(&CKnifeSFSword::DelayOffAttack);
		pev->nextthink = gpGlobals->time + 0.1s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4s;

		return;
	}

	switch (m_iSwing)
	{
	case 0:
	{
		SendWeaponAnim(SFSWORD_SLASH1, UseDecrement() != FALSE);

		SetThink(&CKnifeSFSword::DelayMidslash);
		pev->nextthink = gpGlobals->time + 0.1s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.71s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
		break;
	}
	case 1:
	{
		SendWeaponAnim(SFSWORD_STAB, UseDecrement() != FALSE);
		m_iSlash = 0;
		SetThink(&CKnifeSFSword::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.1s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
		break;
	}
	case 2:
	{
		SendWeaponAnim(SFSWORD_SLASH2, UseDecrement() != FALSE);

		SetThink(&CKnifeSFSword::DelayMidslash);
		pev->nextthink = gpGlobals->time + 0.1s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.71s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
		break;
	}
	case 3:
	{
		SendWeaponAnim(SFSWORD_STAB, UseDecrement() != FALSE);
		m_iSlash = 0;
		SetThink(&CKnifeSFSword::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.1s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
		break;
	}
	}

	m_iSwing++;
	m_iSwing %= 4;

}

void CKnifeSFSword::SecondaryAttack(void)
{
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;

	if (m_iWeaponState)
	{
		m_iWeaponState = 0;
		SendWeaponAnim(SFSWORD_ON, UseDecrement() != FALSE);
		m_fMaxSpeed = 250;

		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
	}
	else
	{
		m_iWeaponState = 1;
		SendWeaponAnim(SFSWORD_OFF, UseDecrement() != FALSE);
		m_fMaxSpeed = 251;

		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_1);
	}

	m_pPlayer->ResetMaxSpeed();
}


void CKnifeSFSword::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	switch (KnifeAttack5(GetPrimaryAttackDamage(), 65, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_wall2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	if (++m_iSlash & 1)
	{
		pev->nextthink = gpGlobals->time + 0.57s;
		SetThink(&CKnifeSFSword::DelayPrimaryAttack);
	}
	else
	{
		SetThink(nullptr);
	}
}


void CKnifeSFSword::DelayMidslash()
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	switch (KnifeAttack5(GetMidslashDamage(), 65, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_wall2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif
	SetThink(nullptr);
}

void CKnifeSFSword::WeaponIdle()
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(m_iWeaponState ? SFSWORD_OFF_IDLE : SFSWORD_IDLE, UseDecrement() != FALSE);
}

}
