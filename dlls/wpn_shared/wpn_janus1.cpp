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
	class CJanus1Rocket : public CGrenade
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
			return WEAPON_JANUS11;
		}

		int m_iType;

	private:
		float GetRadius();
		float GetDamage();

		
		int m_iTrail;
	};


	LINK_ENTITY_TO_CLASS(janus1_rocket, CJanus1Rocket)

	void CJanus1Rocket::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_BOUNCE;
		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/shell_svdex.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CJanus1Rocket::RocketTouch);
		SetThink(&CJanus1Rocket::IgniteThink);

		pev->angles.x -= 30.0;

		UTIL_MakeVectors(pev->angles);
		pev->angles.x = -(pev->angles.x + 30.0);

		pev->velocity = gpGlobals->v_forward * 300;
		pev->gravity = 0.5;
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->dmg = GetDamage();

	}

	void CJanus1Rocket::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

		PRECACHE_SOUND("weapons/janus1_exp.wav");

		PRECACHE_MODEL("models/shell_svdex.mdl");
	}

	void CJanus1Rocket::IgniteThink()
	{
		pev->movetype = MOVETYPE_BOUNCE;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(40);
		WRITE_BYTE(5);
		WRITE_BYTE(224);
		WRITE_BYTE(224);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		MESSAGE_END();

		SetThink(&CJanus1Rocket::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CJanus1Rocket::FollowThink()
	{
		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CJanus1Rocket::RocketTouch(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		ExplodeTouch3(pOther, GetRadius(), EXPTYPE_M79ROCKET, false);
	}

	float CJanus1Rocket::GetRadius()
	{
		float flRadius = 1.0f;

		switch (m_iType)
		{
		case 0:
		{
			//janus1 mode a
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		case 1:
		{
			//janus1 mode b
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		}

		return flRadius * 39.37f;
	}

	float CJanus1Rocket::GetDamage()
	{
		float flDamage;

		switch (m_iType)
		{
		case 0:
		{
			//janus1 mode a
			flDamage = 60.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 200.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 120.0f;
			break;
		}
		case 1:
		{
			//janus1 mode b
			flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 400.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 160.0f;
			break;
		}
		}
		return flDamage;
	}
#endif

LINK_ENTITY_TO_CLASS(weapon_janus1, CJANUS1)

enum janus1_e
{
	JANUS1_IDLE1,
	JANUS1_DRAW,
	JANUS1_SHOOT1_1,
	JANUS1_SHOOT1_2,
	JANUS1_SHOOT1_SIGNAL,
	JANUS1_CHANGE1,
	JANUS1_IDLE2,
	JANUS1_DRAW2,
	JANUS1_SHOOT2_1,
	JANUS1_SHOOT2_2,
	JANUS1_CHANGE2,
	JANUS1_SIGNAL,
	JANUS1_DRAW_SIGNAL,
	JANUS1_SHOOT1_2_SIGNAL,
};

#define WPNSTATE_JANUS1_SIGNAL 1
#define WPNSTATE_JANUS1_USING 2

void CJANUS1::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_janus1");

	Precache();
	m_iId = WEAPON_JANUS1;
	SET_MODEL(ENT(pev), "models/w_janus1.mdl");

	m_iShotsRequired = 2;
	m_iDefaultAmmo = 5;
	FallInit();
}

void CJANUS1::Precache(void)
{
	m_usFireM79 = PRECACHE_EVENT(1, "events/janus1.sc");

	PRECACHE_SOUND("weapons/janus1-1.wav");
	PRECACHE_SOUND("weapons/janus1-2.wav");
	PRECACHE_SOUND("weapons/change1_ready.wav");

	PRECACHE_MODEL("models/p_janus1.mdl");
	PRECACHE_MODEL("models/v_janus1.mdl");
	PRECACHE_MODEL("models/w_janus1.mdl");
}

int CJANUS1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "40mmGrenade";
	p->iMaxAmmo1 = 5;
	p->pszAmmo2 = NULL;
	p->pszAmmo2 = "InfiniteAmmo";
	p->iMaxAmmo2 = 255;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_JANUS1;
	p->iFlags = 0;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

int CJANUS1::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo)
	{
		if (m_pPlayer)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		return TRUE;
	}
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CJANUS1::Deploy(void)
{
	m_fMaxSpeed = 250;

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair(false);
#endif

	int iAnim;
	if (m_iWeaponState == WPNSTATE_JANUS1_USING)
		iAnim = JANUS1_DRAW2;
	else if (m_iWeaponState == WPNSTATE_JANUS1_SIGNAL)
		iAnim = JANUS1_DRAW_SIGNAL;
	else
		iAnim = JANUS1_DRAW;

	return DefaultDeploy("models/v_janus1.mdl", "models/p_janus1.mdl", iAnim, "onehanded", UseDecrement() != FALSE);
}

void CJANUS1::PrimaryAttack(void)
{
	if (m_iWeaponState == WPNSTATE_JANUS1_USING)
		Janus1Fire(true);
	else
		Janus1Fire(false);
}

void CJANUS1::Janus1Fire(bool bSignal)
{
	if (!bSignal)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
			return;
		}

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector vecVelocity = gpGlobals->v_forward * 800 + Vector(0, 0, 100);

	CJanus1Rocket* pRocket = (CJanus1Rocket*)CBaseEntity::Create("janus1_rocket", m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8, m_pPlayer->pev->v_angle, m_pPlayer->edict());

	if (pRocket)
	{
		pRocket->m_iType = bSignal ? 1 : 0;
		pRocket->pev->velocity = vecVelocity;
		pRocket->pev->owner = m_pPlayer->edict();
		pRocket->m_iTeam = m_pPlayer->m_iTeam;
	}

	if (bSignal)
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/janus1-2.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);
	else
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/janus1-1.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/janus1-2.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);
	}
#endif

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM79, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_iWeaponState, int(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0, bSignal);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + (bSignal ? 0.5s : 2.8s);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.8s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;

	pev->iuser1++;

#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);

}

void CJANUS1::SecondaryAttack(void)
{
	if (m_iWeaponState == WPNSTATE_JANUS1_SIGNAL)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 255;
		m_iWeaponState = WPNSTATE_JANUS1_USING;
		SendWeaponAnim(JANUS1_CHANGE1, UseDecrement() != FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

		m_flNextReset = gpGlobals->time + 8.0s;
	}
}

void CJANUS1::ItemPostFrame(void)
{
	if (m_flNextReset != invalid_time_point && m_flNextReset < gpGlobals->time)
	{
		if (m_iWeaponState == WPNSTATE_JANUS1_SIGNAL)
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0s;
			m_flNextReset = invalid_time_point;
		}
		if (m_iWeaponState == WPNSTATE_JANUS1_USING)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
			SendWeaponAnim(JANUS1_CHANGE2, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
			m_flNextReset = invalid_time_point;
		}
		pev->iuser1 = 0;
		m_iWeaponState = 0;
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CJANUS1::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (!m_iWeaponState)
	{
		if (pev->iuser1 >= m_iShotsRequired)
		{
			m_iWeaponState = WPNSTATE_JANUS1_SIGNAL;
			m_flNextReset = gpGlobals->time + 9.0s;

			EMIT_SOUND_DYN(edict(), CHAN_ITEM, "weapons/change1_ready.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
		}
	}

	int iAnim;
	if (m_iWeaponState == WPNSTATE_JANUS1_USING)
		iAnim = JANUS1_IDLE2;
	else if (m_iWeaponState == WPNSTATE_JANUS1_SIGNAL)
		iAnim = JANUS1_SIGNAL;
	else
		iAnim = JANUS1_IDLE1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(iAnim, UseDecrement() != FALSE);
}


}
