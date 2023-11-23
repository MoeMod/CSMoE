/*
wpn_cannon.cpp - Black Dragon Cannon from CSO
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_cannon.h"
#include "weapons/KnifeAttack.h"
#include "decals.h"

#ifndef CLIENT_DLL
#include "soundent.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_cannon, CCannon)

static const float s_flDamageEnhanced[][9] =
{
	{ 0.0, 2.0, 3.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0 },							// normal
	{ 0.0, 417.0, 1043.0, 2085.0, 2085.0, 2085.0, 2085.0, 2085.0, 2085.0 },	// zs
	{ 0.0, 249.0, 426.0, 710.0, 710.0, 710.0, 710.0, 710.0, 710.0 },			// zb
	{ 0.0, 100.0, 200.0, 300.0, 300.0, 300.0, 300.0, 300.0, 300.0 }
};

static const float s_flClipEnhanced[9] = { 0, 5, 10, 15 };

#ifndef CLIENT_DLL
class CCannonFireBall : public CBaseAnimating
{
public:
	float GetArmorRatioModifier() { return 1.5; }
	int GetWeaponsId(void) { return WEAPON_CANNON; }
	void Spawn() override
	{
		Precache();

		SetThink(&CCannonFireBall::IgniteThink);
		SetTouch(&CCannonFireBall::FireBallTouch);
		SET_MODEL(this->edict(), "models/cannon6_fireball.mdl");

		pev->framerate = 1.0;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_FLYMISSILE;
		pev->nextthink = gpGlobals->time + 0.1s;

		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/cannon6_fireball.mdl");
		m_iSmokeTrail = PRECACHE_MODEL("sprites/ef_cannon6_fire.spr");
		m_iSprExplo = PRECACHE_MODEL("sprites/ef_cannon6_explotion.spr");
	}

	void EXPORT IgniteThink()
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
		WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
		WRITE_SHORT(m_iSmokeTrail);// short (sprite index)
		WRITE_SHORT(m_iSmokeTrail);// short (sprite index)
		WRITE_SHORT(m_iSmokeTrail);// short (sprite index)
		WRITE_BYTE(255); //r
		WRITE_BYTE(255); //g
		WRITE_BYTE(255); //b
		WRITE_BYTE(0); //randomcolor
		WRITE_BYTE(30); //scale in 0.01's
		WRITE_BYTE(6); //freq(time of one cycle) in 0.01's
		WRITE_BYTE(3); //fadetime in 0.1s
		WRITE_BYTE(2);  //count
		WRITE_BYTE(0);  //random in org
		WRITE_BYTE(0);  //speed (if>0 do slow gravity)
		MESSAGE_END();

		SetThink(&CCannonFireBall::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void EXPORT FireBallTouch(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		Explode(pOther);
	}

	void Explode(CBaseEntity* pOther)
	{
		UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/canon_firebal_exp.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iSprExplo);
		WRITE_BYTE(15);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		TraceResult tr = UTIL_GetGlobalTrace();

		if (RANDOM_FLOAT(0.0, 1.0) < 0.5)
			UTIL_DecalTrace(&tr, DECAL_SCORCH1);
		else
			UTIL_DecalTrace(&tr, DECAL_SCORCH2);

		switch (RANDOM_LONG(0, 2))
		{
		case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
		case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
		case 2: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
		}

		CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pev->owner);

		if (!pPlayer || FNullEnt(pPlayer->edict()) || !pPlayer->IsPlayer() || pPlayer->m_bIsZombie)
		{
			SetTouch(NULL);
			SetThink(NULL);

			UTIL_Remove(this);
			return;
		}

		if (!pOther->IsDormant() && pOther->pev->takedamage != DAMAGE_NO)
		{
			pOther->TakeDamage(pev, pPlayer->pev, m_flTouchDamage, DMG_BULLET | DMG_NEVERGIB);
		}

		CBaseEntity* pEntity = NULL;

		Vector vecEnd;
		int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
		float flRadius = 80.0;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->edict() == pev->owner)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->Classify() == CLASS_NONE)
				continue;

			if (g_pGameRules->PlayerRelationship(pPlayer, pEntity) == GR_TEAMMATE)
				continue;

			vecEnd = pEntity->BodyTarget(pev->origin);

			UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;

			int falloff = RadiusDamageAmount / flRadius;

			float flCurrentDamage = max(0, RadiusDamageAmount - (pev->origin - pEntity->pev->origin).Length() * falloff);

			pEntity->TakeDamage(pev, pPlayer->pev, flCurrentDamage, DMG_BULLET | DMG_NEVERGIB);
		}

		SetThink(nullptr);
		SetTouch(nullptr);
		UTIL_Remove(this);
	}

	void EXPORT FollowThink()
	{
		if (m_flRemoveTime < gpGlobals->time)
			Explode(NULL);

		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int iTeam)
	{
		m_flRemoveTime = gpGlobals->time + 5.0s;
		std::tie(m_flTouchDamage, RadiusDamageAmount, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, iTeam);
		pev->velocity = std::move(vecVelocity);
	}

	KnockbackData GetKnockBackData() override { return { 800.0f, 1000.0f, 1200.0f, 600.0f, 0.8f }; }

	time_point_t m_flRemoveTime;
	int m_iSmokeTrail;
	float m_flTouchDamage;
	float RadiusDamageAmount;
	int m_iSprExplo;

};
LINK_ENTITY_TO_CLASS(cannon_fireball, CCannonFireBall)
#endif

void CCannon::Spawn(void)
{
	Base::Spawn();
	m_iId = WEAPON_CANNON;
	m_iClip = WEAPON_NOCLIP;
	m_iDefaultAmmo = ItemInfoData_t::iMaxAmmo1;
}

void CCannon::Precache(void)
{
	Base::Precache();
	PRECACHE_MODEL("sprites/flame_puff01.spr");
	PRECACHE_MODEL("sprites/flame_burn01.spr");
	PRECACHE_SOUND("weapons/cannon-1.wav");
	PRECACHE_MODEL("sprites/muzzleflash88.spr");


	m_usFire = PRECACHE_EVENT(1, "events/cannon.sc");
}

int CCannon::ExtractAmmo(CBasePlayerWeapon *pWeapon)
{
	if (m_iDefaultAmmo)
	{
		if(m_pPlayer)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		return TRUE;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CCannon::PrimaryAttack(void)
{
	CannonFire(0, 3.5s, FALSE);
}

void CCannon::SecondaryAttack(void)
{
	if (GetTotalEnhancedLevel() < 6)
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	SendWeaponAnim(3, UseDecrement() != FALSE);

	UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/canon_fireball.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
	Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

	CCannonFireBall* pEnt = static_cast<CCannonFireBall*>(CBaseEntity::Create("cannon_fireball", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(vecProjDir * 1800, GetTouchDamage(), GetRadiusDamage(), m_pPlayer->m_iTeam);
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.5s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.5s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.5s;

}

void CCannon::CannonFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
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

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
	Vector vecDir = KnifeAttack2(vecSrc, gpGlobals->v_forward, GetDamage(), 405.0f, 45.0f, DMG_BULLET, this->pev, m_pPlayer->pev);
#else
	Vector vecDir = gpGlobals->v_forward;
#endif

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	// "sprites/black_smoke4.spr"

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
#endif
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.5s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(13.0, 2.25, 1.45, 0.7, 12.0, 10.0, 1);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(13.0, 5.0, 1.85, 0.55, 15.0, 5.7, 2);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(9.0, 2.1, 1.25, 0.5, 15.0, 5.5, 1);
	else
		KickBack(13.0, 3.2, 1.5, 0.5, 15.0, 10.0, 2);
}

float CCannon::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 1150;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 2950;
	else if (g_pModRunning->DamageTrack() == DT_ZSE)
		return 650;
#endif
	return 410;
}


float CCannon::GetTouchDamage()
{
	float flDamage = 25.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 750.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 1182.0f;
#endif
	return flDamage;
}

float CCannon::GetRadiusDamage()
{
	float flDamage = 75.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 2230.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 4563.0f;
#endif
	return flDamage;
}

WeaponBotStrategy CCannon::GetBotStrategy() const
{
	return BOT_STRATEGY_CANNON;
}

#ifndef CLIENT_DLL

int CCannon::GetPriority()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return 0;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return 0;

	return 1;
}

int CCannon::GetPriorityByTarget(CBaseEntity* target)
{
	if (!GetPriority())
		return 0;

	float flDistance = (m_pPlayer->pev->origin - target->pev->origin).Length();
	if (flDistance > 405.0f)
		return 0;

	return 1;
}
#endif
}
