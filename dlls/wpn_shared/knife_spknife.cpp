/*
knife_spknife.cpp
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
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeSPKnife: public LinkWeaponTemplate<CKnifeSPKnife,
	TGeneralData,
	TPrecacheEvent,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_spknife";
	static constexpr const char *V_Model = "models/v_spknife.mdl";
	static constexpr const char *P_Model = "models/p_spknife.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";
	static constexpr const char* EventFile = "events/spknife.sc";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "dragontail";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

#define SPKNIFE_MAXAMMO	60
	enum spknife_e
	{
		ANIM_IDLEA = 0,
		ANIM_IDLEB,
		ANIM_SLASHA1_1,
		ANIM_SLASHA1_2,
		ANIM_SLASHB1_1,
		ANIM_SLASHB1_2,
		ANIM_SLASHA2_1,
		ANIM_SLASHA2_2,
		ANIM_SLASHB2_1,
		ANIM_SLASHB2_2,
		ANIM_SLASHB3_1,
		ANIM_SLASHB3_2,
		ANIM_DRAWA,
		ANIM_DRAWB,
		ANIM_SLASHA3_1,
		ANIM_SLASHA3_2,
	};

	void Precache() override
	{
		Base::Precache();
		
		PRECACHE_MODEL("sprites/flame3.spr");

		PRECACHE_SOUND("weapons/spknife_stab1.wav");
		PRECACHE_SOUND("weapons/spknife_stab_miss.wav");
		PRECACHE_SOUND("weapons/spknife_off_slash1.wav");
		PRECACHE_SOUND("weapons/spknife_off_slash2.wav");
		PRECACHE_SOUND("weapons/spknife_idle.wav");
		PRECACHE_SOUND("weapons/tomahawk_wall.wav");
		PRECACHE_SOUND("weapons/tomahawk_slash1_hit.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState = 0;

		return DefaultDeploy(V_Model, P_Model, m_iAmmo < 30 ? ANIM_DRAWA : ANIM_DRAWB, "dragontail", UseDecrement() != FALSE);

	}

	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void WeaponIdle();
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

public:
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsSlashAttack);
#endif
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 65.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 715.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 715.0f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		bool bFireOn = m_iAmmo > 0 ? true : false;

		float flDamage = bFireOn ? 120.0f : 80.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = bFireOn ? 1320.0f : 880.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = bFireOn ? 1320.0f : 880.0f;
#endif
		return flDamage;
	}

	float GetBuffDamage() const
	{
		float flDamage = 5.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 50.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 50.0f;
#endif
		return flDamage;
	}

	void ItemPostFrame() override;

	int m_iAmmo;
	time_point_t m_flTimeNextCharge;

};

LINK_ENTITY_TO_CLASS(knife_spknife, CKnifeSPKnife)

void CKnifeSPKnife::Spawn()
{
	pev->classname = MAKE_STRING("knife_spknife");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	m_flTimeNextCharge = invalid_time_point;
	FallInit();
}

void CKnifeSPKnife::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	int iAnim = m_iSwing ? ANIM_SLASHA2_2 : ANIM_SLASHA2_1;

	if (m_iAmmo >= 30)
		iAnim += 2;


	SendWeaponAnim(iAnim, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	switch (sv::KnifeAttack(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, GetPrimaryAttackDamage(), 70, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		if (m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_off_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_off_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if (!RANDOM_LONG(0, 4))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);

		break;
	}
	case HIT_PLAYER:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if (!RANDOM_LONG(0, 4))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif

	m_iSwing++;
	m_iSwing %= 2;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeSPKnife::SecondaryAttack(void)
{
	if (m_iAmmo < 30)
	{
		int iAnim = RANDOM_LONG(0, 1) ? ANIM_SLASHA1_2 : ANIM_SLASHA1_1;

		if (m_iAmmo >= 30)
			iAnim += 2;

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		return;
	}	

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.52s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	int iAnim = RANDOM_LONG(0, 1) ? ANIM_SLASHB3_2 : ANIM_SLASHB3_1;

	m_iAmmo -= 30;
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
	WRITE_BYTE(44);
	WRITE_BYTE(m_iAmmo);
	MESSAGE_END();
#endif
	if (m_iAmmo < 30)
		iAnim += 4;

	SendWeaponAnim(iAnim, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	switch (KnifeAttack(GetSecondaryAttackDamage(), 90, 100, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, true))
	{
	case HIT_NONE:
	{
		if (m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_off_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_off_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if (!RANDOM_LONG(0, 4))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);

		break;
	}
	case HIT_PLAYER:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if (!RANDOM_LONG(0, 4))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
}

#ifndef CLIENT_DLL
hit_result_t CKnifeSPKnife::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsChargeAttack)
{
	CBaseEntity* pEntity = NULL;
	hit_result_t result = HIT_NONE;
	TraceResult tr;
	UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
	Vector vecEnd;
	Vector vecSrc = pAttackPlayer->GetGunPosition();

	Vector vecForward = gpGlobals->v_forward;
	Vector2D vecForward2D = vecForward.Make2D().Normalize();

	float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity == pAttackPlayer)
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, pAttackPlayer))
			continue;

		if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
			continue;

		float flRatio = 1.0;

		if (pEntity && pEntity->IsPlayer())
		{
			UTIL_MakeVectors(pEntity->pev->angles);

			if (DotProduct((pEntity->pev->origin.Make2D() - pAttackPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
			{
				flRatio = 3.0;
				bitsDamageType |= DMG_BACKATK;
			}
		}

		pAttackPlayer->m_iWeaponVolume = 128;

		ClearMultiDamage();

		pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

		ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

		

		result = HIT_PLAYER;
	}

	if (!result)
	{
		static int calced = 0;
		static float calcRes = 0.0f;

		if (!(calced & 1))
		{
			calced |= 1;
			calcRes = sqrt(512.0);
		}

		vecEnd = vecSrc + vecForward * (flRadius - calcRes);

		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, pAttackPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
			result = HIT_WALL;
	}

	return result;
}
#endif

void CKnifeSPKnife::ItemPostFrame()
{
	if (m_iAmmo < SPKNIFE_MAXAMMO)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_iAmmo++;
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(44);
			WRITE_BYTE(m_iAmmo);
			MESSAGE_END();
#endif
			m_flTimeNextCharge = gpGlobals->time + 0.1s;
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeSPKnife::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(m_iAmmo < 30 ? ANIM_IDLEA : ANIM_IDLEB, UseDecrement() != FALSE);
}


}
