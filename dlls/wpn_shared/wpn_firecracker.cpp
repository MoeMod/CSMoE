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
#include "wpn_m79.h"


#ifndef CLIENT_DLL
#include "soundent.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CFireCrackerRocket : public CGrenade
	{
	public:
		void Spawn(void);
		void Precache(void);

	public:
		void EXPORT FollowThink(void);
		void EXPORT IgniteThink(void);
		void EXPORT RocketTouch(CBaseEntity* pOther);

	public:
		void Explode(CBaseEntity* pOther);

	public:
		int GetWeaponsId(void)
		{
			return WEAPON_FIRECRACKER;
		}

		float GetRadius()
		{
			float flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			return flRadius * 39.37f;
		}

		float GetExplodeDamage()
		{
			float flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 500.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 700.0f;
			return flDamage;
		}

		float GetKickRate()
		{
			float flKickRate = 0.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flKickRate = 0.5f;
			return flKickRate;
		}

		bool m_bBounce;
		int m_iBounce;

	private:
		Vector m_vecLastPos;
		int m_iTrail;		
	};

	LINK_ENTITY_TO_CLASS(firecracker_rocket, CFireCrackerRocket)

	void CFireCrackerRocket::Spawn(void)
	{
		Precache();

		pev->classname = MAKE_STRING("firecracker_rocket");
		pev->movetype = MOVETYPE_BOUNCE;
		pev->solid = SOLID_BBOX;
		pev->gravity = 0.4;
		pev->friction = 0.3;
		pev->dmg = GetExplodeDamage();
		pev->dmgtime = gpGlobals->time + 1.5s;
		pev->angles.x -= 30.0;

		UTIL_MakeVectors(pev->angles);
		pev->angles.x = -(pev->angles.x + 30.0);

		SET_MODEL(edict(), "models/shell_svdex.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		SetThink(&CFireCrackerRocket::IgniteThink);
		SetTouch(&CGrenade::ExplodeTouch);

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CFireCrackerRocket::Explode(CBaseEntity* pOther)
	{
		TraceResult tr;
		Vector vecParticleVel;

		UTIL_TraceLine(pev->origin, pev->origin + (pev->origin - m_vecLastPos).Normalize() * 5.0, ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0)
		{
			if (tr.vecPlaneNormal.z < 0)
			{
				vecParticleVel = tr.vecPlaneNormal * 100;
			}
			else if (tr.vecPlaneNormal.z < 0.8)
			{
				vecParticleVel = tr.vecPlaneNormal * 50;
			}
			else
			{
				vecParticleVel = tr.vecPlaneNormal * 200;
			}
		}
		else
		{
			vecParticleVel = Vector(0, 0, 200);
		}

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(g_sModelIndexFireball2);
		WRITE_BYTE(5);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		EMIT_SOUND_DYN(edict(), CHAN_VOICE, "weapons/firecracker_explode.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		ExplodeTouchWithKickRate(pOther, GetRadius(), EXPTYPE_FIRECRACKERROCKET, GetKickRate(), false);
	}

	void CFireCrackerRocket::FollowThink(void)
	{
		pev->angles = UTIL_VecToAngles(pev->velocity);

		if (gpGlobals->time > pev->dmgtime)
			Explode(this);

		m_vecLastPos = pev->origin;
	}

	void CFireCrackerRocket::IgniteThink(void)
	{
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, "weapons/firecracker-wick.wav", VOL_NORM, 0.5, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(4);
		WRITE_BYTE(2);
		WRITE_BYTE(224);
		WRITE_BYTE(90);
		WRITE_BYTE(90);
		WRITE_BYTE(200);
		MESSAGE_END();

		SetThink(&CFireCrackerRocket::FollowThink);
		m_vecLastPos = pev->origin;
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CFireCrackerRocket::Precache(void)
	{
		PRECACHE_MODEL("models/shell_svdex.mdl");
		PRECACHE_MODEL("sprites/spark1.spr");
		PRECACHE_MODEL("sprites/spark2.spr");
		PRECACHE_MODEL("sprites/spark3.spr");

		PRECACHE_SOUND("weapons/firecracker_bounce1.wav");
		PRECACHE_SOUND("weapons/firecracker_bounce2.wav");
		PRECACHE_SOUND("weapons/firecracker_bounce3.wav");
		PRECACHE_SOUND("weapons/firecracker_explode.wav");
		PRECACHE_SOUND("weapons/firecracker-wick.wav");

		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	}


	void CFireCrackerRocket::RocketTouch(CBaseEntity* pOther)
	{
		if (m_bBounce && !pOther->IsPlayer())
		{
			char szSound[64];

			sprintf(szSound, "weapons/firecracker_bounce%d.wav", RANDOM_LONG(1, 3));
			EMIT_SOUND_DYN(edict(), CHAN_VOICE, szSound, VOL_NORM, 0.5, 0, PITCH_NORM);

			m_iBounce++;

			if (m_iBounce < 5)
				return;

			Explode(pOther);
		}
		else
		{
			Explode(pOther);
		}
	}
#endif

LINK_ENTITY_TO_CLASS(weapon_firecracker, CFireCracker)

void CFireCracker::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_firecracker");

	Precache();
	m_iId = WEAPON_FIRECRACKER;
	SET_MODEL(ENT(pev), "models/w_firecracker.mdl");

	m_iDefaultAmmo = 5;
	FallInit();
}

void CFireCracker::Precache(void)
{
	m_usFireM79 = PRECACHE_EVENT(1, "events/m79.sc");

	PRECACHE_MODEL("sprites/muzzleflash12.spr");

	PRECACHE_SOUND("weapons/m79-1.wav");

	PRECACHE_MODEL("models/p_firecracker.mdl");
	PRECACHE_MODEL("models/v_firecracker.mdl");
	PRECACHE_MODEL("models/w_firecracker.mdl");
}

int CFireCracker::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "40mmGrenade";
	p->iMaxAmmo1 = 5;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_FIRECRACKER;
	p->iFlags = 0;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

BOOL CFireCracker::Deploy(void)
{
	m_fMaxSpeed = 250;

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair(false);
#endif

	return DefaultDeploy("models/v_firecracker.mdl", "models/p_firecracker.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
}

void CFireCracker::PrimaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		FireCrackerFire(false);
	else
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	}
}

void CFireCracker::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		FireCrackerFire(true);
	else
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	}
}

void CFireCracker::FireCrackerFire(bool bBounce)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector vecVelocity = gpGlobals->v_forward * 800 + Vector(0, 0, 100);

	CFireCrackerRocket* pRocket = (CFireCrackerRocket*)CBaseEntity::Create("firecracker_rocket", m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8, m_pPlayer->pev->v_angle, m_pPlayer->edict());

	if (pRocket)
	{
		pRocket->pev->velocity = vecVelocity;
		pRocket->pev->owner = m_pPlayer->edict();
		pRocket->m_iTeam = m_pPlayer->m_iTeam;
		pRocket->m_bBounce = bBounce;
		pRocket->m_iBounce = 0;
		pRocket->SetTouch(&CFireCrackerRocket::RocketTouch);
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/m79-1.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);
#endif

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM79, 0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0, FALSE);


	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.8s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.8s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;

#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);

}

}
