/*
z4b_stormgiant.cpp
Copyright (C) 2019 Moemod Hymei

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

class CKnifeStormgiantX: public LinkWeaponTemplate<CKnifeStormgiantX,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_stormgiantx";
	static constexpr const char *V_Model = "models/z4b/v_stormgiantx.mdl";
	static constexpr const char *P_Model = "models/z4b/p_stormgiantx.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "skullaxe";
	static constexpr const auto &DefaultDeployTime = 0s;
	KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 1.0f };

public:

	enum stormgiant_e
	{
		ANIM_IDLE1,
		ANIM_DRAW_ATTACK,
		ANIM_DRAW_ATTACK_HIT,
		ANIM_DRAW,
		ANIM_MIDSLASH1,
		ANIM_MIDSLASH1_HIT,
		ANIM_MIDSLASH1_FAIL,
		ANIM_MIDSLASH2,
		ANIM_MIDSLASH2_HIT,
		ANIM_MIDSLASH2_FAIL
	};

	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/stormgiant_hit1.wav");
		PRECACHE_SOUND("weapons/stormgiant_hit2.wav");
		PRECACHE_SOUND("weapons/stormgiant_draw.wav");
		PRECACHE_SOUND("weapons/stormgiant_draw_attack.wav");
		PRECACHE_SOUND("weapons/stormgiant_attack_hit2.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash1.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash1_fail.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash2.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash2_fail.wav");
		PRECACHE_SOUND("weapons/stormgiant_stab_miss.wav");
		PRECACHE_SOUND("weapons/stormgiant_stone1.wav");
		PRECACHE_SOUND("weapons/stormgiant_stone2.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash1_hit.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash2_hit.wav");
		PRECACHE_SOUND("weapons/stormgiant_idle.wav");
		PRECACHE_SOUND("weapons/stormgiant_metal1.wav");
		PRECACHE_SOUND("weapons/stormgiant_metal2.wav");
		PRECACHE_SOUND("weapons/stormgiant_wood1.wav");
		PRECACHE_SOUND("weapons/stormgiant_wood2.wav");

		m_iSprEffect = PRECACHE_MODEL("sprites/ef_coilmg.spr");
	}

	BOOL Deploy() override
	{
		//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/skullaxe_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 1;
		m_pPlayer->m_bShieldDrawn = false;

		SetThink(&CKnifeStormgiantX::DeployAttack);
		pev->nextthink = gpGlobals->time + 0.23s;

		BOOL ret = Base::Deploy();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.23s;

		return ret;
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
		float flDamage = 133;
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
		float flDamage = 208;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetDeployAttackDamage() const
	{
		float flDamage = 45;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetDeployExtraAttackDamage() const
	{
		float flDamage = 90;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	void MaterialSound(TraceResult *ptr, Vector vecStart, Vector vecEnd);
	void CreateHitFX(const Vector &pos);

	void DeployAttack();
	void DeployComboAttackCheck();

private:
	int m_iSprEffect;
};

LINK_ENTITY_TO_CLASS(z4b_stormgiantx, CKnifeStormgiantX)

void CKnifeStormgiantX::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_MIDSLASH1, UseDecrement() != FALSE);

	SetThink(&CKnifeStormgiantX::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 1.033s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeStormgiantX::SecondaryAttack(void)
{
	if(m_iSwing)
	{
		m_iSwing = 2;
		return;
	}
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_MIDSLASH2, UseDecrement() != FALSE);

	SetThink(&CKnifeStormgiantX::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 1.033s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
}

void CKnifeStormgiantX::DelayPrimaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 110;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 110, 360, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		SendWeaponAnim(ANIM_MIDSLASH1_FAIL, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash1_fail.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		SendWeaponAnim(ANIM_MIDSLASH1_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		SendWeaponAnim(ANIM_MIDSLASH1_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		MaterialSound(&tr, vecSrc, vecEnd);
		break;
	}
	}
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

void CKnifeStormgiantX::DelaySecondaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 110;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 105, 180, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		SendWeaponAnim(ANIM_MIDSLASH2_FAIL, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash2_fail.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		SendWeaponAnim(ANIM_MIDSLASH2_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		SendWeaponAnim(ANIM_MIDSLASH2_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash2_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
		fDidHit = TRUE;
		MaterialSound(&tr, vecSrc, vecEnd);
		break;
	}
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

void CKnifeStormgiantX::DeployAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 110;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetDeployAttackDamage(), 105, 180, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
		case HIT_NONE:
		{
			SendWeaponAnim(ANIM_DRAW_ATTACK, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash1_fail.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			SendWeaponAnim(ANIM_DRAW_ATTACK_HIT, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			SendWeaponAnim(ANIM_DRAW_ATTACK_HIT, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
			fDidHit = TRUE;
			MaterialSound(&tr, vecSrc, vecEnd);
			break;
		}
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	SetThink(&CKnifeStormgiantX::DeployComboAttackCheck);
	pev->nextthink = gpGlobals->time + 0.3s;

	//return fDidHit;
}

void CKnifeStormgiantX::DeployComboAttackCheck()
{
	if(m_iSwing != 2)
	{
		SendWeaponAnim(ANIM_DRAW, UseDecrement() != FALSE);
		m_iSwing = 0;
		m_flNextSecondaryAttack = m_flNextPrimaryAttack; // OK
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
		return;
	}
	m_iSwing = 0;
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 110;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 110, 110, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
		case HIT_NONE:
		{
			SendWeaponAnim(ANIM_MIDSLASH2_FAIL, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash2_fail.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			SendWeaponAnim(ANIM_MIDSLASH2_HIT, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			SendWeaponAnim(ANIM_MIDSLASH2_HIT, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/stormgiant_midslash2_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
			fDidHit = TRUE;
			MaterialSound(&tr, vecSrc, vecEnd);
			break;
		}
	}
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.17s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
	SetThink(nullptr);

	// TODO knockback
	//return fDidHit;
}

void CKnifeStormgiantX::MaterialSound(TraceResult *ptr, Vector vecStart, Vector vecEnd)
{
#ifndef CLIENT_DLL
	const char *rgsz[2];

	switch (UTIL_TextureHit(ptr, vecStart, vecEnd))
	{
		case CHAR_TEX_WOOD:
			rgsz[0] = "weapons/stormgiant_wood1.wav";
			rgsz[1] = "weapons/stormgiant_wood2.wav";

			break;

		case CHAR_TEX_GRATE:
		case CHAR_TEX_METAL:
		case CHAR_TEX_COMPUTER:
			rgsz[0] = "weapons/stormgiant_metal1.wav";
			rgsz[1] = "weapons/stormgiant_metal2.wav";

			break;

		default:
			rgsz[0] = "weapons/stormgiant_stone1.wav";
			rgsz[1] = "weapons/stormgiant_stone2.wav";

			break;
	}

	UTIL_EmitAmbientSound(ENT(0), ptr->vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
	CreateHitFX(ptr->vecEndPos);
#endif
}

void CKnifeStormgiantX::CreateHitFX(const Vector &pos)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pos.x);
	WRITE_COORD(pos.y);
	WRITE_COORD(pos.z);
	WRITE_SHORT(m_iSprEffect);
	WRITE_BYTE(3);
	WRITE_BYTE(15);
	WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();
#endif
}


}
