/*
knife_zombi.cpp
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
#include "wpn_knife.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

class CKnife_Zombi : public CKnife
{
public:
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return KNIFE_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	void WeaponIdle() override;

public:
	int Stab(int fFirst);
	int Swing(int fFirst);
};

LINK_ENTITY_TO_CLASS(knife_zombi, CKnife_Zombi)

enum knife_e
{
	KNIFE_IDLE,
	KNIFE_ATTACK1HIT,
	KNIFE_ATTACK2HIT,
	KNIFE_DRAW,
	KNIFE_STABHIT,
	KNIFE_STABMISS,
	KNIFE_MIDATTACK1HIT,
	KNIFE_MIDATTACK2HIT
};

void CKnife_Zombi::Precache(void)
{
	PRECACHE_MODEL("models/v_knife_zombi.mdl");
	PRECACHE_MODEL("models/w_knife.mdl");

	PRECACHE_SOUND("zombi/zombi_attack_1.wav");
	PRECACHE_SOUND("zombi/zombi_attack_2.wav");
	PRECACHE_SOUND("zombi/zombi_attack_3.wav");


	PRECACHE_SOUND("zombi/zombi_swing_1.wav");
	PRECACHE_SOUND("zombi/zombi_swing_2.wav");
	PRECACHE_SOUND("zombi/zombi_swing_3.wav");

	PRECACHE_SOUND("zombi/zombi_wall_1.wav");
	PRECACHE_SOUND("zombi/zombi_wall_2.wav");
	PRECACHE_SOUND("zombi/zombi_wall_3.wav");

	m_usKnife = PRECACHE_EVENT(1, "events/knife.sc");
}

int CKnife_Zombi::GetItemInfo(ItemInfo *p)
{
	CKnife::GetItemInfo(p);
	p->pszName = "weapon_knife";
	p->iWeight = 5;

	return 1;
}

BOOL CKnife_Zombi::Deploy(void)
{
	m_fMaxSpeed = 250;
	m_iSwing = 0;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;

	return DefaultDeploy("models/v_knife_zombi.mdl", "", KNIFE_DRAW, "knife", UseDecrement() != FALSE);
}

void CKnife_Zombi::PrimaryAttack(void)
{
	Swing(TRUE);
}

void CKnife_Zombi::SecondaryAttack(void)
{
	Stab(TRUE);
	pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.35s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnife_Zombi::WeaponIdle(void)
{
	ResetEmptySound();
 	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_bShieldDrawn != true)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(KNIFE_IDLE, UseDecrement() != FALSE);
	}
}

int CKnife_Zombi::Swing(int fFirst)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 25;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction >= 1)
	{
		if (fFirst)
		{
			switch ((m_iSwing++) % 2)
			{
			case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != FALSE); break;
			case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != FALSE); break;
			}

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_swing_1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 1: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_swing_2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 2: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_swing_3.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			}

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		}
	}
	else
	{
		fDidHit = TRUE;

		switch ((m_iSwing++) % 2)
		{
		case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != FALSE); break;
		case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != FALSE); break;
		}

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		SetPlayerShieldAnim();

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		ClearMultiDamage();
		if (pEntity)
		{
			pEntity->TraceAttack(m_pPlayer->pev, 75, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
		}
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		float flVol = 1;
#ifndef CLIENT_DLL
		int fHitWorld = TRUE;
#endif
		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				switch (RANDOM_LONG(0, 1))
				{
					case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/zombi_attack_1.wav", VOL_NORM, ATTN_NORM); break;
					case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/zombi_attack_2.wav", VOL_NORM, ATTN_NORM); break;
				}

				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if (!pEntity->IsAlive())
					return TRUE;

				flVol = 0.1;
#ifndef CLIENT_DLL
				fHitWorld = FALSE;
#endif
			}
		}

#ifndef CLIENT_DLL
		if (fHitWorld)
		{
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 1: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 2: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_3.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			}

		}
#endif

		m_trHit = tr;
		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		SetThink(&CKnife_Zombi::Smack);
		pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.2s;
		SetPlayerShieldAnim();
	}

	return fDidHit;
}

int CKnife_Zombi::Stab(int fFirst)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 45;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction >= 1)
	{
		if (fFirst)
		{
			SendWeaponAnim(KNIFE_STABMISS, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;

			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_swing_1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 1: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_swing_2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 2: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_swing_3.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			}

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		}
	}
	else
	{
		fDidHit = TRUE;

#ifndef CLIENT_DLL
		SendWeaponAnim(KNIFE_STABHIT, UseDecrement() != FALSE);
#endif
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		float flDamage = 200.0;

		if (pEntity && pEntity->IsPlayer())
		{
			Vector2D vec2LOS;
			Vector vecForward = gpGlobals->v_forward;

			UTIL_MakeVectors(m_pPlayer->pev->angles);

			vec2LOS = vecForward.Make2D();
			vec2LOS = vec2LOS.Normalize();

			if (DotProduct(vec2LOS, gpGlobals->v_forward.Make2D()) > 0.8)
				flDamage *= 3.0;
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		ClearMultiDamage();
		if (pEntity)
			pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		float flVol = 1;
#ifndef CLIENT_DLL
		int fHitWorld = TRUE;
#endif
		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/zombi_attack_3.wav", VOL_NORM, ATTN_NORM);
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if (!pEntity->IsAlive())
					return TRUE;

				flVol = 0.1;
#ifndef CLIENT_DLL
				fHitWorld = FALSE;
#endif
			}
		}

#ifndef CLIENT_DLL
		if (fHitWorld)
		{
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 1: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			case 2: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_wall_3.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3)); break;
			}
		}
#endif

		m_trHit = tr;
		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		SetThink(&CKnife_Zombi::Smack);
		pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.2s;
	}

	return fDidHit;
}

}
