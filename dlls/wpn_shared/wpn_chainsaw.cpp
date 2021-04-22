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
#include "weapons/KnifeAttack.h"
#include "wpn_chainsaw.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum CChainsaw::chainsaw_e
{
	ANIM_IDLE,
    ANIM_DRAW,
    ANIM_DRAW_EMPTY,
    ANIM_ATTACK_BEGIN,
    ANIM_ATTACK_LOOP,
    ANIM_ATTACK_END,
    ANIM_RELOAD,
    ANIM_SLASH1,
    ANIM_SLASH2,
    ANIM_SLASH3,
    ANIM_SLASH4,
    ANIM_IDLE_EMPTY
};

LINK_ENTITY_TO_CLASS(weapon_chainsaw, CChainsaw)

void CChainsaw::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_chainsaw");

	Precache();
	m_iId = WEAPON_SCOUT;
	SET_MODEL(ENT(pev), "models/w_chainsaw.mdl");

	m_iDefaultAmmo = CHAINSAW_MAX_CLIP;
	FallInit();
}

void CChainsaw::Precache(void)
{
	PRECACHE_MODEL("models/v_chainsaw.mdl");
	PRECACHE_MODEL("models/w_chainsaw.mdl");
	PRECACHE_MODEL("models/p_chainsaw.mdl");

	PRECACHE_SOUND("weapons/chainsaw_attack1_end.wav");
	PRECACHE_SOUND("weapons/chainsaw_attack1_start.wav");
	PRECACHE_SOUND("weapons/chainsaw_attack1_loop.wav");
	PRECACHE_SOUND("weapons/chainsaw_draw.wav");
	PRECACHE_SOUND("weapons/chainsaw_draw1.wav");
	PRECACHE_SOUND("weapons/chainsaw_hit1.wav");
	PRECACHE_SOUND("weapons/chainsaw_hit2.wav");
	PRECACHE_SOUND("weapons/chainsaw_hit3.wav");
	PRECACHE_SOUND("weapons/chainsaw_hit4.wav");
	PRECACHE_SOUND("weapons/chainsaw_idle.wav");
	PRECACHE_SOUND("weapons/chainsaw_reload.wav");
	PRECACHE_SOUND("weapons/chainsaw_slash1.wav");
	PRECACHE_SOUND("weapons/chainsaw_slash2.wav");
	PRECACHE_SOUND("weapons/chainsaw_slash3.wav");
	PRECACHE_SOUND("weapons/chainsaw_slash4.wav");

	m_usFireChainsaw = PRECACHE_EVENT(1, "events/chainsaw.sc");
}

int CChainsaw::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "chainsawoil";
	p->iMaxAmmo1 = CHAINSAW_MAX_CLIP;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = CHAINSAW_MAX_CLIP	;
	p->iSlot = 0;
	p->iPosition = 9;
	p->iId = m_iId = WEAPON_SCOUT;
	p->iFlags = 0;
	p->iWeight = SCOUT_WEIGHT;

	return 1;
}

BOOL CChainsaw::Deploy(void)
{
	if (DefaultDeploy("models/v_chainsaw.mdl", "models/p_chainsaw.mdl", m_iClip ? ANIM_DRAW : ANIM_DRAW_EMPTY, "rifle", UseDecrement() != FALSE))
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.2s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.1s;
		pev->iuser1 = 0;
		pev->iuser2 = 0;
		pev->iuser3 = 0;
		return TRUE;
	}

	return FALSE;
}

void CChainsaw::SecondaryAttack(void)
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
	int iAnim = RANDOM_LONG(0, 1);
	hit_result_t iCallBack = KnifeAttack1(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), CHAINSAW_DISTANCE_B, CHAINSAW_SLASH_ANGLE, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev, iAnim == 1);
	m_pPlayer->SetAnimation(PLAYER_ATTACK2);
	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireChainsaw, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, CHAINSAW_DISTANCE_B, float(iAnim), iAnim, 2, iCallBack == HIT_PLAYER, m_iClip > 0);
	pev->iuser3 = iAnim;
#endif
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.15s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
	
	pev->iuser1 = 0;

}

void CChainsaw::PrimaryAttack(void)
{
	return;
}

void CChainsaw::ItemPostFrame(void)
{
	//iuser1 state:
	//0 start attack
	//1 attack loop
	m_iButton = m_pPlayer->pev->button;

	if(m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
	{
		if ((!m_iClip || !(m_iButton & IN_ATTACK) && !(m_iButton & IN_ATTACK2) && !(m_iButton & IN_RELOAD)) &&  pev->iuser1 == 1)
		{
			SendWeaponAnim(ANIM_ATTACK_END, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.53s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.53s;
			pev->iuser1 = 2;	
			return CBasePlayerWeapon::ItemPostFrame();
		}

		if (m_iButton & IN_ATTACK && !(m_iButton & IN_ATTACK2) && m_iClip)
		{
			switch (pev->iuser1)
			{
				//attack start
				case 0:
				{
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.53s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
					pev->iuser1 = 1;
					PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireChainsaw, 0, (float *)&g_vecZero, (float *)&g_vecZero, CHAINSAW_DISTANCE_A, 0.0, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
					return CBasePlayerWeapon::ItemPostFrame();
				}
				case 1:
				{
					if (!m_iClip)
					{
						SendWeaponAnim(ANIM_ATTACK_END, UseDecrement() != FALSE);
						m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.53s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.53s;
						pev->iuser1 = 2;
						return CBasePlayerWeapon::ItemPostFrame();
					}
					//attack loop
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

					UTIL_MakeVectors(m_pPlayer->pev->v_angle);
					Vector vecSrc = m_pPlayer->GetGunPosition();
				#ifndef CLIENT_DLL
					m_pPlayer->SetAnimation(PLAYER_ATTACK1);
				#endif

				#ifndef CLIENT_DLL
					hit_result_t iCallBack = KnifeAttack(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), CHAINSAW_DISTANCE_A, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);
				#endif


				#ifndef CLIENT_DLL
					PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireChainsaw, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, CHAINSAW_DISTANCE_A, 0.0, 0, 0, iCallBack != HIT_NONE, TRUE);
					/*MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, m_pPlayer->pev);
					WRITE_SHORT((1 << 12) * 5);
					WRITE_SHORT(1);
					WRITE_SHORT((1 << 12) * 5);
					MESSAGE_END();*/
				#endif
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.063s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0s;

				#ifndef CLIENT_DLL
					if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
						m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
				#endif

		//m_pPlayer->pev->velocity[2] = 0.0;
				if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
					KickBack(1.0, 0.4, 0.2, 0.15, 3.0, 2.0, 0);
				else if (m_pPlayer->pev->velocity.Length2D() > 0)
					KickBack(0.3, 0.3, 0.05, 0.02, 2.5, 1.5, 0);
				else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
					KickBack(0.175 ,0.04, 0.03, 0.01, 1.5, 1.0, 0);
				else
					KickBack(0.2, 0.2, 0.035, 0.015, 2.0, 1.25, 0);
					return CBasePlayerWeapon::ItemPostFrame();
				}
				case 2:
				{
					pev->iuser1 = 0;
				}
			}
		}
	}

	if (m_flNextSecondaryAttack <= UTIL_WeaponTimeBase())
	{
		if (m_iButton & IN_ATTACK2)
		{
			//attack2
		}
	}
	
	pev->button &= ~IN_ATTACK2;
	pev->button &= ~IN_ATTACK;
	
	return CBasePlayerWeapon::ItemPostFrame();
}

void CChainsaw::Reload(void)
{
	if (DefaultReload(CHAINSAW_MAX_CLIP, ANIM_RELOAD, 3.0s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}

void CChainsaw::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(ANIM_IDLE_EMPTY, UseDecrement() != FALSE);
	}
}

float CChainsaw::GetPrimaryAttackDamage() const
{
	int flDamage = 38;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 72;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 72;
#endif
	return flDamage;
}

float CChainsaw::GetSecondaryAttackDamage() const
{
	float flDamage = 110.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 500.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 500.0f;
#endif
	return flDamage;
}

#ifndef CLIENT_DLL
hit_result_t CChainsaw::KnifeAttack1(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
	entvars_t *pevInflictor, entvars_t *pevAttacker, BOOL iAnim)
{
	TraceResult tr;
	hit_result_t result = HIT_NONE;

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
			result = HIT_WALL;
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


				
				PLAYBACK_EVENT_FULL(0, ENT(pEntity->pev), m_usFireChainsaw, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 1, iAnim, m_iClip > 0);

	
				CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
				if (pVictim->m_bIsZombie) // Zombie Knockback...
				{
					ApplyKnockbackData(pVictim, vecSpot - vecSrc, { 700, 1600, 1300, 400, 1.0f });
				}
				result = HIT_PLAYER;

			}
		}
	}

	return result;
}
#endif

}
