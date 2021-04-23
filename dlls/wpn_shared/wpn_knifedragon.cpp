/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_knifedragon.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(knife_knifedragon, CKnifeDragon)

void CKnifeDragon::Spawn(void)
{
	Precache();
	m_iId = WEAPON_KNIFE;
	SET_MODEL(ENT(pev), "models/w_knife.mdl");

	m_iClip = WEAPON_NOCLIP;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CKnifeDragon::Precache(void)
{

#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_knife.mdl");
#endif
	PRECACHE_MODEL("models/p_knifedragon.mdl");
	PRECACHE_MODEL("models/v_knifedragon.mdl");

	PRECACHE_SOUND("weapons/knifedragon_draw.wav");
	PRECACHE_SOUND("weapons/knifedragon_hit1.wav");
	PRECACHE_SOUND("weapons/knifedragon_slash.wav");
	PRECACHE_SOUND("weapons/knifedragon_stab.wav");
	PRECACHE_SOUND("weapons/knifedragon_wall.wav");

	m_usKnife = PRECACHE_EVENT(1, "events/knifedragon.sc");
}

int CKnifeDragon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

BOOL CKnifeDragon::Deploy(void)
{
	
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knifedragon_draw.wav", 0.3, 2.4);

	m_fMaxSpeed = 250;
	m_iSwing = 0;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;
#ifdef ENABLE_SHIELD
	if (m_pPlayer->HasShield() != false)
		return DefaultDeploy("models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_SHIELD_DRAW, "shieldknife", UseDecrement() != FALSE);
	else
#endif
	{
		return DefaultDeploy("models/v_knifedragon.mdl", "models/p_knifedragon.mdl", KNIFE_DRAW, "knife", UseDecrement() != FALSE);
	}
}

void CKnifeDragon::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;
}

void CKnifeDragon::WeaponAnimation(int iAnimation)
{
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usKnife, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, iAnimation, 2, 3, 4);
}


void CKnifeDragon::PrimaryAttack(void)
{
	Swing(TRUE);
}

void CKnifeDragon::SetPlayerShieldAnim(void)
{
	if (m_pPlayer->HasShield() == true)
	{
		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			strcpy(m_pPlayer->m_szAnimExtention, "shield");
		else
			strcpy(m_pPlayer->m_szAnimExtention, "shieldknife");
	}
}

void CKnifeDragon::ResetPlayerShieldAnim(void)
{
	if (m_pPlayer->HasShield() == true)
	{
		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			strcpy(m_pPlayer->m_szAnimExtention, "shieldknife");
	}
}

bool CKnifeDragon::ShieldSecondaryFire(int up_anim, int down_anim)
{
	if (m_pPlayer->HasShield() == false)
		return false;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(down_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shieldknife");
		m_fMaxSpeed = 250;
		m_pPlayer->m_bShieldDrawn = false;
	}
	else
	{
		m_iWeaponState |= WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(up_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shielded");
		m_fMaxSpeed = 180;
		m_pPlayer->m_bShieldDrawn = true;
	}

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_SHIELD_DRAWN) == 0);
	m_pPlayer->ResetMaxSpeed();
#endif
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
	return true;
}

void CKnifeDragon::SecondaryAttack(void)
{
	if (ShieldSecondaryFire(KNIFE_SHIELD_UP, KNIFE_SHIELD_DOWN) == true)
		return;

	Stab(TRUE);
	pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.35s;
}

void CKnifeDragon::Smack(void)
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR, false, m_pPlayer->pev, false);
}

void CKnifeDragon::SwingAgain(void)
{
	Swing(FALSE);
}

void CKnifeDragon::WeaponIdle(void)
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

int CKnifeDragon::Swing(int fFirst)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

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
			if (m_pPlayer->HasShield() == false)
			{
				switch ((m_iSwing++) % 2)
				{
					case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != FALSE); break;
					case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != FALSE); break;
				}

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
			}
			else
			{
				SendWeaponAnim(KNIFE_SHIELD_ATTACKHIT, UseDecrement() != FALSE);

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
			}

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knifedragon_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		}
	}
	else
	{
		fDidHit = TRUE;

		if (m_pPlayer->HasShield() == false)
		{
			switch ((m_iSwing++) % 2)
			{
				case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != FALSE); break;
				case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != FALSE); break;
			}

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		}
		else
		{
			SendWeaponAnim(KNIFE_SHIELD_ATTACKHIT, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		SetPlayerShieldAnim();

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		ClearMultiDamage();
		if (pEntity)
		{
			float flDamage = 15;
			if (m_flNextPrimaryAttack + 0.4s < UTIL_WeaponTimeBase())
				flDamage = 20;

#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage *= 9.5f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage *= 5.5f;
#endif

			pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
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

				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knifedragon_hit1.wav", VOL_NORM, ATTN_NORM); 

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
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knifedragon_wall.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		}
#endif

		m_trHit = tr;
		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		SetThink(&CKnifeDragon::Smack);
		pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.2s;
		SetPlayerShieldAnim();
	}

	return fDidHit;
}

int CKnifeDragon::Stab(int fFirst)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;
	m_flLastFire = gpGlobals->time;
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

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knifedragon_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
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
		float flDamage = 65.0;

		if (pEntity && pEntity->IsPlayer())
		{
			Vector2D vec2LOS;
			Vector vecForward = gpGlobals->v_forward;

			// ???
			UTIL_MakeVectors(pEntity->pev->angles);

			vec2LOS = vecForward.Make2D();
			vec2LOS = vec2LOS.Normalize();

			if (DotProduct(vec2LOS, gpGlobals->v_forward.Make2D()) > 0.8)
				flDamage *= 3.0;
		}

#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if(g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif

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
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knifedragon_stab.wav", VOL_NORM, ATTN_NORM);
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
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knifedragon_wall.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		}
#endif

		m_trHit = tr;
		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		SetThink(&CKnifeDragon::Smack);
		pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.2s;
	}

	return fDidHit;
}

}
