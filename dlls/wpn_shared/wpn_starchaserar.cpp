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
#include "wpn_starchaserar.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum starchaserar_e
{
	STARCHASERAR_IDLE1,
	STARCHASERAR_RELOAD,
	STARCHASERAR_DRAW,
	STARCHASERAR_SHOOT1,
	STARCHASERAR_SHOOT2,
	STARCHASERAR_SHOOT3
};

LINK_ENTITY_TO_CLASS(weapon_starchaserar, CStarchaserar)

void CStarchaserar::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_starchaserar");

	Precache();
	m_iId = WEAPON_AUG;
	SET_MODEL(ENT(pev), "models/w_starchaserar.mdl");

	m_iDefaultAmmo = GALIL_DEFAULT_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CStarchaserar::Precache(void)
{
	PRECACHE_MODEL("models/v_starchaserar.mdl");
	PRECACHE_MODEL("models/w_starchaserar.mdl");
	PRECACHE_MODEL("models/p_starchaserar.mdl");
	PRECACHE_MODEL("sprites/ef_starchasersr_explosion.spr");
	PRECACHE_MODEL("sprites/ef_starchasersr_line.spr");

	PRECACHE_SOUND("weapons/starchaserar_boltpull.wav");
	PRECACHE_SOUND("weapons/starchaserar_clipout.wav");
	PRECACHE_SOUND("weapons/starchaserar_clipin.wav");
	PRECACHE_SOUND("weapons/starchaserar-1.wav");
	PRECACHE_SOUND("weapons/starchaserar-2.wav");
	PRECACHE_SOUND("weapons/starchasersr_exp.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireStarchaserar = PRECACHE_EVENT(1, "events/starchaserar.sc");
}

int CStarchaserar::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = 105;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GALIL_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 14;
	p->iId = m_iId = WEAPON_AUG;
	p->iFlags = 0;
	p->iWeight = AUG_WEIGHT;

	return 1;
}

BOOL CStarchaserar::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	return DefaultDeploy("models/v_starchaserar.mdl", "models/p_starchaserar.mdl", STARCHASERAR_DRAW, "carbine", UseDecrement() != FALSE);
}

void CStarchaserar::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV != 90)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 55;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CStarchaserar::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		StarchaserarFire(0.035 + (0.4) * m_flAccuracy, 0.0825s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		StarchaserarFire(0.035 + (0.07) * m_flAccuracy, 0.0825s, FALSE);
	else if (m_pPlayer->pev->fov == 90)
		StarchaserarFire((0.02) * m_flAccuracy, 0.0825s, FALSE);
	else
		StarchaserarFire((0.02) * m_flAccuracy, 0.135s, FALSE);
}

#ifndef CLIENT_DLL
void CStarchaserar::RadiusDamage(Vector vecAiming, float flDamage)
{
	float flRadius = 85.0f;

	if (g_pModRunning->DamageTrack() == DT_ZBS)
		flRadius = 140.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flRadius = 125.0f;
	
	const Vector vecSrc = vecAiming;
	entvars_t * const pevAttacker = VARS(pev->owner);
	entvars_t * const pevInflictor = this->pev;
	int bitsDamageType = DMG_BULLET;

	TraceResult tr;
	//const float falloff = flRadius ? flDamage / flRadius : 1;
	const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

	CBaseEntity *pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			if (bInWater && !pEntity->pev->waterlevel)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->pev == pevAttacker)
				continue;

			Vector vecSpot = pEntity->BodyTarget(vecSrc);
			UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
			{
				if (tr.fStartSolid)
				{
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0;
				}
				/*float flAdjustedDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;
				flAdjustedDamage = Q_max(0, flAdjustedDamage);*/

				if (tr.flFraction == 1.0f)
				{
					pEntity->TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
				}
				else
				{
					tr.iHitgroup = HITGROUP_CHEST;
					ClearMultiDamage();
					pEntity->TraceAttack(pevInflictor, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);
				}

				/*CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
				if (pVictim->m_bIsZombie) // Zombie Knockback...
				{
				ApplyKnockbackData(pVictim, vecSpot - vecSrc, GetKnockBackData());
				}*/
			}
		}
	}

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(vecAiming[0]);
	WRITE_COORD(vecAiming[1]);
	WRITE_COORD(vecAiming[2]);
	WRITE_SHORT(MODEL_INDEX("sprites/ef_starchasersr_explosion.spr"));
	WRITE_BYTE(6);
	WRITE_BYTE(25);
	WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();

	EMIT_SOUND_DYN(this->edict(), CHAN_STATIC, "weapons/starchasersr_exp.wav", VOL_NORM, ATTN_NORM, 0, 94);

}
#endif	

Vector CStarchaserar::Get_ShootPosition(CBaseEntity *pevAttacker, Vector Start)
{
	Vector end, vecforward, GunFire;

	end = pevAttacker->pev->v_angle;
	UTIL_MakeVectors(end);
	GunFire[0] = Start[0] + gpGlobals->v_forward[0] * 160.0 + gpGlobals->v_right[0] * 40.0 + gpGlobals->v_up[0] * -40.0;
	GunFire[1] = Start[1] + gpGlobals->v_forward[1] * 160.0 + gpGlobals->v_right[1] * 40.0 + gpGlobals->v_up[1] * -40.0;
	GunFire[2] = Start[2] + gpGlobals->v_forward[2] * 160.0 + gpGlobals->v_right[2] * 40.0 + gpGlobals->v_up[2] * -40.0;
	end = gpGlobals->v_forward;

	end = end * 8192.0;
	end = Start + end;


	TraceResult tr;
	UTIL_TraceLine(Start, end, dont_ignore_monsters, pevAttacker->edict(), &tr);
	end = tr.vecEndPos;

#ifndef CLIENT_DLL
	vecforward = (end - GunFire) / 17.5;
	if ((GunFire - end).Length() > 125.0)
	{

		int iCount = round((end - GunFire).Length() / 17.5);
		int iCount2 = iCount;
		while (iCount)
		{
			float fPrecent = float(iCount / iCount2);
			GunFire += vecforward;
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(GunFire[0]);
			WRITE_COORD(GunFire[1]);
			WRITE_COORD(GunFire[2]);
			WRITE_SHORT(MODEL_INDEX("sprites/ef_starchasersr_line.spr"));
			WRITE_BYTE(1);
			WRITE_BYTE(15 + round(15.0 * fPrecent));
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
			MESSAGE_END();
			iCount--;
		}
	}
#endif
	return end;
}

void CStarchaserar::StarchaserarFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215.0) + 0.3;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

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
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.96, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	if (gpGlobals->time - tWorldTime < 1.0s)
	{
		tDelta += gpGlobals->time - tWorldTime;
	}

	if (tNextAttack > 1.0s || (gpGlobals->time - tWorldTime > 1.0s) || tDelta > 1.0s)	//可以多射一次
	{
		tNextAttack = 0.0s;
		tDelta = 0.0s;
		CBaseEntity *pevAttacker = this->m_pPlayer;
		auto vecShootPosition = Get_ShootPosition(pevAttacker, vecSrc);
#ifndef CLIENT_DLL
		RadiusDamage(vecShootPosition, g_pModRunning->DamageTrack() == DT_NONE ? 30 : 300);
#endif
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_STATIC, "weapons/starchaserar-2.wav", VOL_NORM, ATTN_NORM, 0, 94);
	}

	tWorldTime = gpGlobals->time;

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireStarchaserar, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.0, 0.45, 0.275, 0.05, 4.0, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.25, 0.45, 0.22, 0.18, 5.5, 4.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.575, 0.325, 0.2, 0.011, 3.25, 2.0, 8);
	else
		KickBack(0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8);
}

void CStarchaserar::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(AUG_MAX_CLIP, STARCHASERAR_RELOAD, 3.3s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CStarchaserar::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(STARCHASERAR_IDLE1, UseDecrement() != FALSE);
}

float CStarchaserar::GetDamage() const
{
	float flDamage = 32.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 70.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 70.0f;
#endif
	return flDamage;
}
}
