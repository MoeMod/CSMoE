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
#include "z4b_m60amethyst.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(z4b_m60amethyst, CM60AMETHYST)

	void CM60AMETHYST::Spawn(void)
	{
		pev->classname = MAKE_STRING("z4b_m60amethyst");

		Precache();
		m_iId = WEAPON_Z4B_M60AMETHYST;
		SET_MODEL(ENT(pev), "models/w_m60_amethyst.mdl");

		m_iDefaultAmmo = 9999;
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;

		FallInit();
	}

	void CM60AMETHYST::Precache(void)
	{
		PRECACHE_MODEL("models/v_m60_amethyst.mdl");
		PRECACHE_MODEL("models/p_m60_amethyst.mdl");
		PRECACHE_MODEL("models/w_m60_amethyst.mdl");
		PRECACHE_MODEL("sprites/muzzleflash5.spr");
		PRECACHE_MODEL("sprites/ame5stack.spr");
		PRECACHE_SOUND("weapons/m60e4-1.wav");


		m_iShell = PRECACHE_MODEL("models/pshell.mdl");
		m_usFireM60AMETHYST = PRECACHE_EVENT(1, "events/m60amethyst.sc");
	}

	int CM60AMETHYST::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "ammo_amethyst";
		p->iMaxAmmo1 = 9999;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 200;
		p->iSlot = 0;
		p->iPosition = 4;
		p->iId = m_iId = WEAPON_Z4B_M60AMETHYST;
		p->iFlags = 0;
		p->iWeight = M249_WEIGHT;

		return 1;
	}

	BOOL CM60AMETHYST::Deploy(void)
	{
		iShellOn = 1;

		return DefaultDeploy("models/v_m60_amethyst.mdl", "models/p_m60_amethyst.mdl", 2, "m249", UseDecrement() != FALSE);
	}

	void CM60AMETHYST::PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			M60AMETHYSTFire(0.065 + (0.75) * m_flAccuracy, 0.09s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			M60AMETHYSTFire(0.045 + (0.095) * m_flAccuracy, 0.09s, FALSE);
		else
			M60AMETHYSTFire((0.04) * m_flAccuracy, 0.09s, FALSE);
	}

	void CM60AMETHYST::M60AMETHYSTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 170.0) + 0.4;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDirShooting = gpGlobals->v_forward;
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		if (m_iRequiredShoots < 10)
		{
			m_iRequiredShoots++;
		}
		else
		{
			m_iRequiredShoots = 0;
			ExpToLineTargets(vecSrc, vecDirShooting);
		}

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM60AMETHYST, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.25, 0.5, 0.375, 0.065, 4.0, 3.0, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.7, 0.3, 0.225, 0.02, 3.5, 2.0, 9);
		else
			KickBack(0.825, 0.35, 0.35, 0.035, 4.0, 3.5, 9);
	}

	void CM60AMETHYST::ExpToLineTargets(Vector vecSrc, Vector vecDir)
	{
#ifndef CLIENT_DLL
		TraceResult tr;
		g_engfuncs.pfnSetGroupMask(0, GROUP_OP_AND);
		m_pPlayer->pev->groupinfo = (1 << 1);

		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);
			if (!entity)
				continue;

			if (entity->edict() == m_pPlayer->edict())
				continue;

			CBasePlayer* TargetPlayer = static_cast<CBasePlayer*>(entity);

			if (g_pGameRules->PlayerRelationship(m_pPlayer, TargetPlayer) == GR_TEAMMATE)
				TargetPlayer->pev->groupinfo = (1 << 2);
		}

		UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		m_pPlayer->pev->groupinfo = 0;

		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);
			if (!entity)
				continue;

			CBasePlayer* TargetPlayer = static_cast<CBasePlayer*>(entity);
			TargetPlayer->pev->groupinfo = 0;
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(tr.vecEndPos[0]);
		WRITE_COORD(tr.vecEndPos[1]);
		WRITE_COORD(tr.vecEndPos[2]);
		WRITE_SHORT(MODEL_INDEX("sprites/ame5stack.spr"));
		WRITE_BYTE(10);
		WRITE_BYTE(0.3f);
		WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		EMIT_SOUND_DYN(this->edict(), CHAN_STATIC, "weapons/broad-2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if (tr.flFraction < 1.0)
		{
			TraceResult CheckHeadShotTR;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr.vecEndPos, 100.0)) != NULL)
			{
				if (pEntity->edict() == m_pPlayer->edict())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (g_pGameRules->PlayerRelationship(m_pPlayer, target) == GR_TEAMMATE)
						continue;
				}

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(tr.vecEndPos, vecEnd, dont_ignore_monsters, pev->owner, &CheckHeadShotTR);

				if (CheckHeadShotTR.flFraction < 1.0 && CheckHeadShotTR.pHit != pEntity->edict())
					continue;

				ClearMultiDamage();
				CheckHeadShotTR.iHitgroup = HITGROUP_CHEST;
				pEntity->TraceAttack(m_pPlayer->pev, GetExpDamage(), (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

				ApplyMultiDamage(pev, m_pPlayer->pev);
			}

		}
#endif
	}

	void CM60AMETHYST::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(200, 1, 5.375s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.2;
			m_bDelayFire = false;
			m_iShotsFired = 0;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]!= 9999)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 9999;
			}
		}
	}

	void CM60AMETHYST::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70s;
		SendWeaponAnim(0, UseDecrement() != FALSE);
	}

	float CM60AMETHYST::GetDamage() const
	{
		float flDamage = 20.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 20.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 36.0f;
#endif
		return flDamage;
	}
	float CM60AMETHYST::GetExpDamage() const
	{
		float flDamage = 20.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 75.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 36.0f;
#endif
		return flDamage;
	}
}
