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

class CKnifeCrow9: public LinkWeaponTemplate<CKnifeCrow9,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_crow9";
	static constexpr const char *V_Model = "models/v_crow9.mdl";
	static constexpr const char *P_Model = "models/p_crow9a.mdl";
	static constexpr const char* P_Model_1 = "models/p_crow9b.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "dragontail";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	int m_iSlash;

public:
	enum
	{
		ANIM_IDLE,
		ANIM_SLASHA,
		ANIM_SLASHB,
		ANIM_DRAW,
		ANIM_SLASHC_IN,
		ANIM_SLASHC_1,
		ANIM_SLASHC_2,
	};

	void Precache() override
	{
		Base::Precache();

		PRECACHE_MODEL("models/crow9_wind.mdl");
		PRECACHE_MODEL("sprites/muzzleflash35.spr");
	
		PRECACHE_SOUND("weapons/skullaxe_wall.wav");
		PRECACHE_SOUND("weapons/balrog9_hit1.wav");
		PRECACHE_SOUND("weapons/balrog9_hit2.wav");
		PRECACHE_SOUND("weapons/crow9_slasha_1.wav");
		PRECACHE_SOUND("weapons/crow9_slasha_2.wav");
		PRECACHE_SOUND("weapons/crow9_slashc_in.wav");
		PRECACHE_SOUND("weapons/crow9_slashc_1.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iSlash = 0;
		m_iWeaponState = 0;
		m_flNextCheckButton = invalid_time_point;

		return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "dragontail", UseDecrement() != FALSE, 1.26s);
	}

	void Spawn() override;
	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }
	void ItemPostFrame() override;
	void DelaySecondaryAttack();

	time_point_t m_flNextCheckButton;
	int m_iCheckButton;
public:
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 39.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 351.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 215.0f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 70.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 630.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 315.0f;
#endif
		return flDamage;
	}

	float GetSlashAttackDamage() const
	{
		float flDamage = 110.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 605.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 990.0f;
#endif
		return flDamage;
	}

	KnockbackData GetKnockBackData() override
	{
		if (m_iWeaponState > 1)
			return { 6500.0f, 600.0f, 450.0f, 300.0f, 1.0f };		
		else if (m_iWeaponState > 0)
			return { 5000.0f, 500.0f, 300.0f, 200.0f, 1.0f };
		else
			return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };	
	}
};

LINK_ENTITY_TO_CLASS(knife_crow9, CKnifeCrow9)

void CKnifeCrow9::Spawn()
{
	pev->classname = MAKE_STRING("knife_crow9");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeCrow9::PrimaryAttack(void)
{
	m_iWeaponState = 0;
	SendWeaponAnim(m_iSwing ? ANIM_SLASHB : ANIM_SLASHA, UseDecrement() != FALSE);

	if (m_iSwing)
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/crow9_slasha_2.wav", VOL_NORM, ATTN_NORM, 0, 94);
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/crow9_slasha_1.wav", VOL_NORM, ATTN_NORM, 0, 94);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	switch (KnifeAttack(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, GetPrimaryAttackDamage(), 50, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;

		m_pPlayer->pev->punchangle.x += RANDOM_FLOAT(-0.02, 0.02) * 100;
		m_pPlayer->pev->punchangle.y += RANDOM_FLOAT(-0.02, 0.02) * 100;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;

		m_pPlayer->pev->punchangle.x += RANDOM_FLOAT(-0.02, 0.02) * 100;
		m_pPlayer->pev->punchangle.y += RANDOM_FLOAT(-0.02, 0.02) * 100;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
	m_iSwing++;
	m_iSwing %= 2;
}

void CKnifeCrow9::SecondaryAttack(void)
{
	if (m_flNextCheckButton != invalid_time_point && m_flNextCheckButton > gpGlobals->time)
	{
		DelaySecondaryAttack();
		m_flNextCheckButton = invalid_time_point;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
		return;
	}


	m_iWeaponState = 1;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	switch (KnifeAttack5(GetSecondaryAttackDamage(), 60, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
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
		break;
	}
	case HIT_WALL:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
	SendWeaponAnim(ANIM_SLASHC_IN, UseDecrement() != FALSE);

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/crow9_slashc_in.wav", VOL_NORM, ATTN_NORM, 0, 94);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() +  2.62s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.85s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	m_flNextCheckButton = gpGlobals->time + 1.0s;
	m_iCheckButton = 1;
}

void CKnifeCrow9::ItemPostFrame()
{
	if (m_pPlayer->pev->button & IN_ATTACK2)
	{
		if (m_flNextCheckButton != invalid_time_point && m_flNextCheckButton > gpGlobals->time)
		{
			if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
			{

			}

		}
	}

	if (m_flNextCheckButton != invalid_time_point && m_flNextCheckButton < gpGlobals->time)
	{
		m_flNextCheckButton = invalid_time_point;
		SendWeaponAnim(ANIM_SLASHC_2, UseDecrement() != FALSE);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.6s;
	}


	return CBasePlayerWeapon::ItemPostFrame();
}


void CKnifeCrow9::DelaySecondaryAttack()
{
	m_iWeaponState = 2;

	SendWeaponAnim(ANIM_SLASHC_1, UseDecrement() != FALSE);

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/crow9_slashc_2.wav", VOL_NORM, ATTN_NORM, 0, 94);

#ifndef CLIENT_DLL
	switch (KnifeAttack5(GetSlashAttackDamage(), 120, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
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
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/crow9_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
}

void CKnifeCrow9::WeaponIdle()
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
}

}
