/*
z4b_oppovivo.cpp
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

class CKnifeOppoVivo: public LinkWeaponTemplate<CKnifeOppoVivo,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_oppovivo";
	static constexpr const char *V_Model = "models/z4b/v_oppovivo.mdl";
	static constexpr const char *P_Model = "models/z4b/p_oppovivo.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 1.0f };

public:

	enum skullaxe_e
	{
		ANIM_IDLE1 = 0,
		ANIM_NONE1,
		ANIM_NONE2,
		ANIM_DRAW,
		ANIM_STAB1,
		ANIM_STAB2,
		ANIM_SLASH_MISS,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_DRAW_ATTACK,
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/sfsword_hit1.wav");
		PRECACHE_SOUND("weapons/sfsword_hit2.wav");
		PRECACHE_SOUND("weapons/sfsword_on.wav");
		PRECACHE_SOUND("weapons/sfsword_stab.wav");
		PRECACHE_SOUND("weapons/sfsword_wall1.wav");
		PRECACHE_SOUND("weapons/sfsword_midslash1.wav");
		PRECACHE_SOUND("weapons/sfsword_midslash2.wav");
		PRECACHE_SOUND("weapons/sfsword_midslash3.wav");

		m_iSprEffect = PRECACHE_MODEL("sprites/z4b/plasmabomb.spr");
		m_iSprEffect2 = PRECACHE_MODEL("sprites/z4b/m4a1mw_hit.spr");
	}

	BOOL Deploy() override
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_on.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;

		if(gpGlobals->time > m_flLastFire + 800ms)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_flLastFire = gpGlobals->time;
			SetThink(&CKnifeOppoVivo::DelayDeployAttack);
			pev->nextthink = gpGlobals->time + 0.1s;
			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW_ATTACK, AnimExtension, UseDecrement() != FALSE);
		}
		return Base::Deploy();
	}

	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

private:
	void MakeExplosion(const Vector &pos);
	void MakeExplosion2(const Vector &pos);
	int m_iSprEffect;
	int m_iSprEffect2;

public:
	void DelayDeployAttack();
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
	float GetDeployAttackDamage() const
	{
		float flDamage = 25;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 47;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5;
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
			flDamage *= 9.5;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(z4b_oppovivo, CKnifeOppoVivo)

void CKnifeOppoVivo::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_iSwing = 0;
	SendWeaponAnim(RANDOM_LONG(0, 1)? ANIM_SLASH1:ANIM_SLASH2, UseDecrement() != FALSE);
	SetThink(&CKnifeOppoVivo::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.2s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeOppoVivo::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(RANDOM_LONG(0, 1)? ANIM_STAB1:ANIM_STAB2, UseDecrement() != FALSE);

	SetThink(&CKnifeOppoVivo::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.4s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_iSwing = 0;
}

void CKnifeOppoVivo::DelayDeployAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 90;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetDeployAttackDamage(), 90, 45, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
		case HIT_NONE:
		{

			break;
		}
		case HIT_PLAYER:
		{
			if(m_iSwing & 1)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			fDidHit = TRUE;
			break;
		}
	}
	if(tr.flFraction < 1)
	{
		MakeExplosion(tr.vecEndPos);
		MakeExplosion2(tr.vecEndPos);
	}
#endif
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

void CKnifeOppoVivo::DelayPrimaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 90;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 90, 45, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		if(m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_midslash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_midslash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		if(m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		break;
	}
	}
	if(tr.flFraction < 1)
	{
		MakeExplosion(tr.vecEndPos);
		MakeExplosion2(tr.vecEndPos);
	}
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	if(++m_iSwing & 1)
	{
		pev->nextthink = gpGlobals->time + 0.3s;
		SetThink(&CKnifeOppoVivo::DelayPrimaryAttack);
	}
	else
	{
		SetThink(nullptr);
	}

	//return fDidHit;
}

void CKnifeOppoVivo::DelaySecondaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 103;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 103, 90, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, RANDOM_LONG(0,1) ? "weapons/sfsword_hit1.wav" : "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
		fDidHit = TRUE;
		break;
	}
	}
	if(tr.flFraction < 1)
	{
		MakeExplosion(tr.vecEndPos);
		MakeExplosion2(tr.vecEndPos);
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

void CKnifeOppoVivo::MakeExplosion(const Vector &pos)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pos.x + RANDOM_FLOAT(-10.0, 10.0));
	WRITE_COORD(pos.y + RANDOM_FLOAT(-10.0, 10.0));
	WRITE_COORD(pos.z + RANDOM_FLOAT(-10.0, 10.0));
	WRITE_SHORT(m_iSprEffect);
	WRITE_BYTE(3);
	WRITE_BYTE(15);
	WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();
#endif
}

void CKnifeOppoVivo::MakeExplosion2(const Vector &pos)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pos.x + RANDOM_FLOAT(-10.0, 10.0));
	WRITE_COORD(pos.y + RANDOM_FLOAT(-10.0, 10.0));
	WRITE_COORD(pos.z + RANDOM_FLOAT(-10.0, 10.0));
	WRITE_SHORT(m_iSprEffect2);
	WRITE_BYTE(3);
	WRITE_BYTE(15);
	WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();
#endif
}

}
