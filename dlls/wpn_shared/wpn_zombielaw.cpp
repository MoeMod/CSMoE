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
#include "wpn_zombielaw.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	void CZombieLawRocket::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLYMISSILE;
		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/s_grenade.mdl");
		UTIL_SetSize(pev, g_vecZero, g_vecZero);

		pev->classname = MAKE_STRING("zombielaw");
		SetThink(&CZombieLawRocket::IgniteThink);
		SetTouch(&CZombieLawRocket::RocketTouch);

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CZombieLawRocket::Precache(void)
	{
		PRECACHE_MODEL("models/s_grenade.mdl");
	}

	void CZombieLawRocket::FollowThink(void)
	{
		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CZombieLawRocket::IgniteThink(void)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(g_sModelIndexSmokeBeam);
		WRITE_BYTE(40);
		WRITE_BYTE(5);
		WRITE_BYTE(224);
		WRITE_BYTE(224);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		MESSAGE_END();

		m_flIgniteTime = gpGlobals->time;

		SetThink(&CZombieLawRocket::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CZombieLawRocket::RocketTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pOther->edict() == pev->owner)
			return;

		float flExplodeDamage = GetExplodeDamage();
		float flRadius = GetRadius();
		entvars_t* pevOwner = VARS(pev->owner);

		if (!pOther->IsDormant())
		{
			if (pOther->pev->takedamage != DAMAGE_NO)
			{
				pOther->TakeDamage(pev, pevOwner, flExplodeDamage, DMG_NEVERGIB | DMG_EXPLOSION);
			}
		}

		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->pev == pevOwner)
				continue;

			if (pEntity == pOther)
				continue;

			int falloff = flExplodeDamage / flRadius;

			float flCurrentDamage = max(0, flExplodeDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

			pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_NEVERGIB | DMG_EXPLOSION);
		}

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(MODEL_INDEX("sprites/eexplo.spr"));
		WRITE_BYTE(25);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
	}

	LINK_ENTITY_TO_CLASS(zombielaw_rocket, CZombieLawRocket);
#endif

LINK_ENTITY_TO_CLASS(weapon_zombielaw, CZombieLaw)

void CZombieLaw::Spawn(void)
{
	Precache();
	m_iId = WEAPON_ZOMBIELAW;
	m_iClip = WEAPON_NOCLIP;
	SET_MODEL(ENT(pev), "models/w_zombielaw.mdl");
	m_iDefaultAmmo = 3;

	FallInit();
}

void CZombieLaw::Precache(void)
{
	PRECACHE_MODEL("models/v_zombielaw.mdl");
	PRECACHE_MODEL("models/w_zombielaw.mdl");
	PRECACHE_MODEL("models/p_zombielaw.mdl");

	m_usFirezombielaw = PRECACHE_EVENT(1, "events/zombielaw.sc");
}

int CZombieLaw::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ZombieLawGrenade";
	p->iMaxAmmo1 = 3;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 3;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_ZOMBIELAW;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CZombieLaw::Deploy(void)
{
#if 0
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", "csmoe_nemesis");
	m_pPlayer->SetNewPlayerModel("models/player/csmoe_nemesis/csmoe_nemesis.mdl");
#endif
	return DefaultDeploy("models/v_zombielaw.mdl", "models/p_zombielaw.mdl", ZOMBIELAW_DRAW, "at4", UseDecrement() != FALSE, 3.1s, 3.5s);
}

int CZombieLaw::ExtractAmmo(CBasePlayerWeapon* pWeapon)
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

void CZombieLaw::PrimaryAttack(void)
{
	m_bDelayFire = true;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}
	--m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

	Vector vecDir;

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecVelocity = gpGlobals->v_forward;
	Vector VecAngle = m_pPlayer->pev->v_angle;
	VecAngle[0] = 360.0 - VecAngle[0];

	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
	CZombieLawRocket* pEnt = static_cast<CZombieLawRocket*>(CBaseEntity::Create("zombielaw_rocket", vecSrc, VecAngle, m_pPlayer->edict()));
	if (pEnt)
	{
		pEnt->pev->velocity = vecVelocity * 1000.0;
	}
#endif

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	SendWeaponAnim(ZOMBIELAW_SHOOT, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;
#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
	m_flNextTimeCanHolster = gpGlobals->time + 1.2s;

	pev->punchangle[0] -= RANDOM_FLOAT(0.55, 1.0);
	pev->punchangle[1] -= RANDOM_FLOAT(-0.65, 0.95);
}

void CZombieLaw::Reload(void)
{
	return;
}

void CZombieLaw::Holster(int skiplocal)
{
	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->RemoveWeapon(m_iId);
		DestroyItem();
	}
}


BOOL CZombieLaw::CanHolster(void)
{
	return gpGlobals->time > m_flNextTimeCanHolster;
}

void CZombieLaw::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (pev->iuser1)
	{
		RetireWeapon();
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_flNextTimeCanHolster = gpGlobals->time + 1.7s;
		SendWeaponAnim(ZOMBIELAW_DISCARD, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
		pev->iuser1 = 1;	//mark remove
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ZOMBIELAW_IDLE, UseDecrement() != FALSE);
	}
}

}
