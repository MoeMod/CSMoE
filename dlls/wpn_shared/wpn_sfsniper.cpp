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
#include "wpn_sfsniper.h"
#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "monsters.h"
#endif

#include <vector>
#include <array>

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_sfsniper, CSFSniper)

void CSFSniper::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_sfsniper");

	Precache();
	m_iId = WEAPON_AWP;
	SET_MODEL(ENT(pev), "models/w_sfsniper.mdl");
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_bSearchingTarget = true;
	m_iClip = -1;
	m_iDefaultAmmo = 35;
	m_bIsZoomed = false;
	FallInit();
}

void CSFSniper::Precache(void)
{
	PRECACHE_MODEL("models/v_sfsniper.mdl");
	PRECACHE_MODEL("models/w_sfsniper.mdl");
	PRECACHE_MODEL("models/p_sfsniper.mdl");

	PRECACHE_MODEL("models/z4b/v_zoom_A.mdl");
	PRECACHE_MODEL("models/z4b/v_zoom_B.mdl");
	PRECACHE_MODEL("sprites/muzzleflash19.spr");
	
	PRECACHE_SOUND("weapons/sfsniper-1.wav");
	PRECACHE_SOUND("weapons/sfsniper_zoom.wav");
	PRECACHE_SOUND("weapons/sfsniper_insight1.wav");
	
	m_usFireSFSniper = PRECACHE_EVENT(1, "events/sfsniper.sc");
}

int CSFSniper::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "railbeam";
	p->iMaxAmmo1 = 35;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_SFSNIPER;
	p->iFlags = 0;
	p->iWeight = AWP_WEIGHT;

	return 1;
}

BOOL CSFSniper::Deploy(void)
{
	m_bIsZoomed = false;

	if (DefaultDeploy("models/v_sfsniper.mdl", "models/p_sfsniper.mdl", ANIM_DRAW, "rifle", UseDecrement() != FALSE, 1.37s, 1.37s))
	{
		if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		{
			m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.37s;
		}

		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.37s;

		return TRUE;
	}

	return FALSE;
}

int CSFSniper::ExtractAmmo(CBasePlayerWeapon *pWeapon)
{
	if (m_iDefaultAmmo)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		return TRUE;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CSFSniper::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		SFSniperFire(0.5, 2.6s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 120)
		SFSniperFire(0.07, 2.6s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 10)
		SFSniperFire(0.03, 2.6s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		SFSniperFire(0.0, 2.6s, FALSE);
	else
		SFSniperFire(0.0, 2.6s, FALSE);
}

void CSFSniper::SFSniperFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{

	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->m_bResumeZoom = true;
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_sfsniper.mdl");
		m_bIsZoomed = true;
	}
	else
	{ // not zoomed, so bullets diverts more
		flSpread += 0.01;
		m_bIsZoomed = false;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_338MAG, GetPrimaryAttackDamage(), 0.99, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSFSniper, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), m_bIsZoomed, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.6s;

	m_pPlayer->pev->punchangle.x -= 2;
}

void CSFSniper::SecondaryAttack(void)
{

	switch (m_pPlayer->m_iFOV)
	{
	case 90:
	{
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 35; 
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/z4b/v_zoom_A.mdl");
		break;
	}
	default: 
	{
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; 
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_sfsniper.mdl");
		break;
	}
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsniper_zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
}

void CSFSniper::ItemPostFrame(void)
{

	if (m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->m_bResumeZoom)
		{
			m_pPlayer->m_iFOV = m_pPlayer->m_iLastZoom;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV;
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/z4b/v_zoom_A.mdl");

			if (m_pPlayer->m_iFOV == m_pPlayer->m_iLastZoom)
			{
				// return the fade level in zoom.
				m_pPlayer->m_bResumeZoom = false;
			}

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
		}
	}
	return CBasePlayerWeapon::ItemPostFrame();

}

void CSFSniper::Reload(void)
{
		return;
}

void CSFSniper::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->pev->fov != 90)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192.0;
		TraceResult tr;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0 && tr.pHit)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

				if (m_pPlayer->m_iTeam != target->m_iTeam)
				{
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/z4b/v_zoom_B.mdl");

					if (m_bSearchingTarget)
					{
						EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sfsniper_insight1.wav", VOL_NORM, ATTN_NORM);

						m_bSearchingTarget = false;
					}

				}
			}
			else
			{
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/z4b/v_zoom_A.mdl");

				m_bSearchingTarget = true;
			}
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.25s;
	}
	else
	{
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	}

}

float CSFSniper::GetPrimaryAttackDamage() const
{
	float flDamage = 138;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 830;		//original 690 +5now
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 1290;
#endif
	return flDamage;
}

}
