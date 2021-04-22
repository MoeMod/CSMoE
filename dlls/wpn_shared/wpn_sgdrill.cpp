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
#include "wpn_sgdrill.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum CSgdrill::sgdrill_e
{
	SGDRILL_IDLE,
	SGDRILL_FIRE,
	SGDRILL_SLASH,
	SGDRILL_RELOAD,
	SGDRILL_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_sgdrill, CSgdrill)

void CSgdrill::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	SET_MODEL(ENT(pev), "models/w_sgdrill.mdl");

	m_iDefaultAmmo = SGDRILL_DEFAULT_GIVE;

	FallInit();
}

void CSgdrill::Precache(void)
{
	PRECACHE_MODEL("models/v_sgdrill.mdl");
	PRECACHE_MODEL("models/w_sgdrill.mdl");
	PRECACHE_MODEL("models/p_sgdrill.mdl");
	PRECACHE_MODEL("models/p_sgdrill_slash.mdl");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shell_sgdrill.mdl");

	PRECACHE_SOUND("weapons/sgdrill-1.wav");
	PRECACHE_SOUND("weapons/sgdrill_clipin.wav");
	PRECACHE_SOUND("weapons/sgdrill_slash.wav");
	PRECACHE_SOUND("weapons/sgdrill_clipout.wav");
	PRECACHE_SOUND("weapons/sgdrill_idle.wav");
	PRECACHE_SOUND("weapons/sgdrill_draw.wav");

	m_usFireSgdrill = PRECACHE_EVENT(1, "events/sgdrill.sc");
}

int CSgdrill::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SgdrillAmmo";
	p->iMaxAmmo1 = 105;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SGDRILL_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CSgdrill::Deploy(void)
{
	return DefaultDeploy("models/v_sgdrill.mdl", "models/p_sgdrill.mdl", SGDRILL_DRAW, "m249", UseDecrement() != FALSE);
}

int CSgdrill::GetPrimaryAttackDamage() const
	{
		int iDamage = 22;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			iDamage = 42;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			iDamage = 42;
#endif
		return iDamage;
	}

float CSgdrill::GetSecondaryAttackDamage() const
	{
		float flDamage = 115.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 6.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 6.0f;
#endif
		return flDamage;
	}

void CSgdrill::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

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
	m_pPlayer->FireBullets(8, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.03, 0.03, 0.0), 4096, BULLET_PLAYER_BUCKSHOT, 0, GetPrimaryAttackDamage());
#endif
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSgdrill, 0, (float *)&g_vecZero, (float *)&g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, FALSE);


#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.32s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.32s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, 3.0, 4.0);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5.0, 6.0);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 2.0);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2.0, 3.0);
		
}

void CSgdrill::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(SGDRILL_MAX_CLIP, SGDRILL_RELOAD, 2.9s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}

void CSgdrill::SecondaryAttack(void)
{
	#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK2);
	m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgdrill_slash.mdl");
#endif
	SendWeaponAnim(SGDRILL_SLASH, UseDecrement() != FALSE);
	SetThink(&CSgdrill::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.6s;
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sgdrill_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.3s;
	m_flNextResetModel = gpGlobals->time + 1.2s;
}

void CSgdrill::DelaySecondaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
	KnifeAttack(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 155, 120, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);
#endif
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;
	SetThink(nullptr);
}
void CSgdrill::ItemPostFrame()
{
	if (m_flNextResetModel <= gpGlobals->time)
	{
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgdrill.mdl");
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CSgdrill::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(SGDRILL_IDLE, UseDecrement() != FALSE);
	
}
#ifndef CLIENT_DLL
BOOL CSgdrill::KnifeAttack(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
	entvars_t *pevInflictor, entvars_t *pevAttacker)
{
	TraceResult tr;
	BOOL result = 0;

	vecSrc.z += 1;

	if (!pevAttacker)
		pevAttacker = pevInflictor;

	Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pevAttacker), &tr);

	if (tr.flFraction >= 1) {
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pevAttacker), &tr);

		if (tr.flFraction < 1) {
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel()) {
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(pevAttacker));
			}

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction < 1) {
		CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
		if (pHit && pHit->IsBSPModel() && pHit->pev->takedamage != DAMAGE_NO) {
			const float flAdjustedDamage = flDamage;
			ClearMultiDamage();
			pHit->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
			ApplyMultiDamage(pevInflictor, pevAttacker);
		}

		float flVol = 1;
		BOOL fHitWorld = TRUE;
		if (pHit && pHit->Classify() != CLASS_NONE && pHit->Classify() != CLASS_MACHINE) {
			flVol = 0.1f;
			fHitWorld = FALSE;
		}

		if (fHitWorld) {
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			result = TRUE;
		}
	}

	CBaseEntity *pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != nullptr) {
		if (pEntity->pev->takedamage != DAMAGE_NO) {
			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->pev == pevAttacker)
				continue;

			Vector vecSpot = pEntity->BodyTarget(vecSrc);
			vecSpot.z = vecEnd.z;
			UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

			if (AngleBetweenVectors(tr.vecEndPos - vecSrc, vecDir) > flAngleDegrees)
				continue;

			if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict()) {
				if (tr.fStartSolid) {
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0;
				}

				if (tr.flFraction == 1.0f) {
					pEntity->TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
				}

				Vector vecRealDir = (tr.vecEndPos - vecSrc).Normalize();

				ClearMultiDamage();
				pEntity->TraceAttack(pevInflictor, flDamage, vecRealDir, &tr, bitsDamageType);
				ApplyMultiDamage(pevInflictor, pevAttacker);

				CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
				if (pVictim->m_bIsZombie) // Zombie Knockback...
				{
					ApplyKnockbackData(pVictim, vecSpot - vecSrc, { 700, 1600, 1300, 400, 1.0f });
				}
				result = TRUE;

			}
		}
	}

	return result;
}
#endif


void CSgdrill::FindHullIntersection(const Vector &vecSrc, TraceResult &tr, const float *pflMins, const float *pfkMaxs, edict_t *pEntity)
{
	TraceResult trTemp;
	float flDistance = 1000000;
	const float *pflMinMaxs[2] = { pflMins, pfkMaxs };
	Vector vecHullEnd = tr.vecEndPos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	TRACE_LINE(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &trTemp);

	if (trTemp.flFraction < 1)
	{
		tr = trTemp;
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				Vector vecEnd;
				vecEnd.x = vecHullEnd.x + pflMinMaxs[i][0];
				vecEnd.y = vecHullEnd.y + pflMinMaxs[j][1];
				vecEnd.z = vecHullEnd.z + pflMinMaxs[k][2];

				TRACE_LINE(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &trTemp);

				if (trTemp.flFraction < 1)
				{
					float flThisDistance = (trTemp.vecEndPos - vecSrc).Length();

					if (flThisDistance < flDistance)
					{
						tr = trTemp;
						flDistance = flThisDistance;
					}
				}
			}
		}
	}
}


void CSgdrill::Holster(int skiplocal)
{
	SetThink(nullptr);
	return CBasePlayerWeapon::Holster(skiplocal);
}

}
