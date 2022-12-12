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

class CKnifeKatanaD: public LinkWeaponTemplate<CKnifeKatanaD,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_katanad";
	static constexpr const char *V_Model = "models/v_katanad.mdl";
	static constexpr const char *P_Model = "models/p_katanad.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "katanad";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	int m_iSlash;

public:
	enum
	{
		ANIM_IDLE,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_DRAW,
		ANIM_STAB,
		ANIM_STAB_MISS,
		ANIM_MIDSLASH1,
		ANIM_MIDSLASH2,
		ANIM_MIDSLASH3,
	};

	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/mastercombat_wall.wav");
		PRECACHE_SOUND("weapons/katanad_hit1.wav");
		PRECACHE_SOUND("weapons/katanad_hit2.wav");
		PRECACHE_SOUND("weapons/katanad_slash1.wav");
		PRECACHE_SOUND("weapons/katanad_slash2.wav");
		PRECACHE_SOUND("weapons/katanad_slash3.wav");
		PRECACHE_SOUND("weapons/katanad_stab.wav");
		PRECACHE_SOUND("weapons/katanad_stab_miss.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iSlash = 0;
		m_iWeaponState = 0;

		return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "katanad", UseDecrement() != FALSE, 1.5s);
	}

	void Holster(int skiplocal) override
	{
		SetThink(NULL);
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void Spawn() override;
	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();

public:
	float GetPrimaryAttackDamage() const
	{
		float flDamage = m_iSwing % 2 ? 41 : 43;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = m_iSwing % 2 ? 82 : 86;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = m_iSwing % 2 ? 164 : 171;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 100.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 560.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 400.0f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_katanad, CKnifeKatanaD)

void CKnifeKatanaD::Spawn()
{
	pev->classname = MAKE_STRING("knife_katanad");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeKatanaD::PrimaryAttack(void)
{
	SendWeaponAnim(m_iSwing ? ANIM_SLASH2 : ANIM_SLASH1, UseDecrement() != FALSE);

	switch (m_iSwing)
	{
	case 0:
	{
		SendWeaponAnim(ANIM_MIDSLASH2, UseDecrement() != FALSE); break;
	}
	case 1:
	{
		SendWeaponAnim(ANIM_MIDSLASH3, UseDecrement() != FALSE); break;
	}
	case 2:
	{
		SendWeaponAnim(ANIM_MIDSLASH2, UseDecrement() != FALSE); break;
	}
	case 3:
	{
		SendWeaponAnim(ANIM_MIDSLASH3, UseDecrement() != FALSE); break;
	}
	case 4:
	{
		SendWeaponAnim(ANIM_MIDSLASH1, UseDecrement() != FALSE); break;
	}

	}

	float flDamage = GetPrimaryAttackDamage();

	if (m_iSwing != 4)
	{
		if(m_iSwing % 2)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_slash3.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		SetThink(&CKnifeKatanaD::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.46s;
	}
	else
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

		flDamage *= 2.0f;
	}
		
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	switch (KnifeAttack5(flDamage, 1.6 * 39.37, 150, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
	
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	m_iSwing++;
	m_iSwing %= 5;
}

void CKnifeKatanaD::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

	SetThink(&CKnifeKatanaD::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.46s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.7s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;
}

void CKnifeKatanaD::DelayPrimaryAttack()
{
	SetThink(nullptr);
#ifndef CLIENT_DLL
	switch (KnifeAttack5(GetPrimaryAttackDamage(), 1.6 * 39.37, 150, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
	{
	case HIT_NONE:
	{
		if (m_iSwing % 2)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_slash3.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/katanad_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
}

void CKnifeKatanaD::DelaySecondaryAttack()
{
	SetThink(nullptr);
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	switch (KnifeAttack(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, GetSecondaryAttackDamage(), 55, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/katanad_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		m_pPlayer->pev->punchangle.x += RANDOM_FLOAT(-0.02, 0.02) * 100;
		m_pPlayer->pev->punchangle.y += RANDOM_FLOAT(-0.02, 0.02) * 100;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/katanad_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		m_pPlayer->pev->punchangle.x += RANDOM_FLOAT(-0.02, 0.02) * 100;
		m_pPlayer->pev->punchangle.y += RANDOM_FLOAT(-0.02, 0.02) * 100;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
}

void CKnifeKatanaD::WeaponIdle()
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
}

}
