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
#include "player/player_mod_strategy.h"
#endif

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeZombiZ4: public LinkWeaponTemplate<CKnifeZombiZ4,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "knife_zombi_z4";
	static constexpr const char *V_Model = "models/v_knife_zombi.mdl";
	static constexpr const char *P_Model = "";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 1.0f, 1.0f, 1.0f, 1.0f, 0.5f };

public:

	enum
	{
		ANIM_IDLE1,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_DRAW,
		ANIM_STUN_START,
		ANIM_STUN_END,
		ANIM_SKILL_START,
		ANIM_SKILL_END,
		ANIM_SKILL,
		ANIM_DASH_START,
		ANIM_DASH_LOOP,
		ANIM_DASH_END,
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("zombi/zombi_attack_1.wav");
		PRECACHE_SOUND("zombi/zombi_attack_2.wav");
		PRECACHE_SOUND("zombi/zombi_attack_3.wav");

		PRECACHE_SOUND("zombi/zombi_wall_1.wav");
		PRECACHE_SOUND("zombi/zombi_wall_2.wav");
		PRECACHE_SOUND("zombi/zombi_wall_3.wav");

		PRECACHE_SOUND("zombi/z4/all/z4zombi_swing_1.wav");
		PRECACHE_SOUND("zombi/z4/all/z4zombi_swing_2.wav");
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
	bool HasSecondaryAttack() override { return true; }
	BOOL CanDrop() override { return false; }

public:
	float GetPrimaryAttackDamage() const
	{
		return 400;
	}
};

LINK_ENTITY_TO_CLASS(knife_zombi_z4, CKnifeZombiZ4)


void CKnifeZombiZ4::Spawn()
{
	pev->classname = MAKE_STRING("weapon_knife");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeZombiZ4::PrimaryAttack(void)
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
			if(m_iSwing & 1)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/z4/all/z4zombi_swing_1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/z4/all/z4zombi_swing_2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 2))
			{
				case 0: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_attack_1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
				case 1: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_attack_2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
				case 2: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_attack_3.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			}
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			switch (RANDOM_LONG(0, 2))
			{
				case 0: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
				case 1: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
				case 2: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_3.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			}
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			break;
		}
	}
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 400ms;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeZombiZ4::SecondaryAttack()
{
#ifndef CLIENT_DLL
	m_pPlayer->m_pModStrategy->ClientCommand("z4_zombieskill");
#endif
}

}
