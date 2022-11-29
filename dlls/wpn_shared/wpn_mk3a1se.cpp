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
#include "wpn_mk3a1.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_mk3a1se, CMK3A1SE)

	void CMK3A1SE::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_MK3A1SE;
		SET_MODEL(ENT(pev), "models/w_mk3a1se.mdl");

		m_iDefaultAmmo = 15;

		FallInit();
	}

	void CMK3A1SE::Precache(void)
	{
		PRECACHE_MODEL("models/v_mk3a1se.mdl");
		PRECACHE_MODEL("models/w_mk3a1se.mdl");
		PRECACHE_MODEL("models/p_mk3a1se.mdl");

		PRECACHE_MODEL("sprites/ef_mk3a1seburn.spr");


		m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

		PRECACHE_SOUND("weapons/mk3a1se-1.wav");

		m_usFireMK3A1 = PRECACHE_EVENT(1, "events/mk3a1.sc");
	}

	int CMK3A1SE::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "12gauge";
		p->iMaxAmmo1 = 40;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 15;
		p->iSlot = 0;
		p->iPosition = 12;
		p->iId = m_iId = WEAPON_MK3A1SE;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	BOOL CMK3A1SE::Deploy(void)
	{
		return DefaultDeploy("models/v_mk3a1se.mdl", "models/p_mk3a1se.mdl", ANIM_DRAW, "m249", UseDecrement() != FALSE);
	}

	void CMK3A1SE::PrimaryAttack(void)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;

#ifndef CLIENT_DLL
			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
			}
#endif
			return;
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(6, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.088, 0.088, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());

		if (CBaseEntity* pEntity = gMultiDamage.pEntity)
		{
			if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY) && (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE))
			{
				float flBurnAmount = GetBuffDamage();

				pEntity->m_pBuffAttacker = m_pPlayer->pev;
				pEntity->m_iHealthDecreaseCount = 5;
				pEntity->m_flHealthDecreaseAmount = flBurnAmount;
				pEntity->m_flHealthDecreaseInterval = 1.0s;

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(pEntity->entindex());
				WRITE_BYTE(21);
				MESSAGE_END();
				
			}
		}
#endif
		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireMK3A1, 0, g_vecZero, g_vecZero, 0.0, 0.088, 7, m_vVecAiming.x * 100, m_iClip != 0, TRUE);
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.33s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.33s;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
		else
			m_flTimeWeaponIdle = 0.75s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 11);
		else
			m_pPlayer->pev->punchangle.x -= 3;
	}

	void CMK3A1SE::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(15, ANIM_RELOAD, 2.3s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		}
	}

	int CMK3A1SE::GetDamage() const
	{
		int iDamage = 24;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			iDamage = 32;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			iDamage = 92;
#endif
		return iDamage;
	}

	float CMK3A1SE::GetBuffDamage() const
	{
		float iDamage = 5.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			iDamage = 20.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			iDamage = 20.0f;
#endif
		return iDamage;
	}
}
