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
#include "weapons/RadiusDamage.hpp"
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
		
		m_iSteamModelIndex = PRECACHE_MODEL("sprites/spsmg_steam.spr");

		PRECACHE_SOUND("weapons/spknife_steam.wav");
		PRECACHE_SOUND("weapons/spknife_slashb3_1.wav");
		PRECACHE_SOUND("weapons/spknife_slashb3_2.wav");
		PRECACHE_SOUND("weapons/spknife_slasha1_1.wav");
		PRECACHE_SOUND("weapons/spknife_slasha1_2.wav");
		PRECACHE_SOUND("weapons/spknife_slasha2_1.wav");
		PRECACHE_SOUND("weapons/spknife_slasha2_2.wav");
	
		PRECACHE_SOUND("weapons/skullaxe_wall.wav");
		PRECACHE_SOUND("weapons/balrog9_hit1.wav");
		PRECACHE_SOUND("weapons/balrog9_hit2.wav");
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
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer);
#endif
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 33.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 330.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 430.0f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
	
		float flDamage = 55.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 550.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 665.0f;
#endif
		return flDamage;
	}

	float GetBuffDamage() const
	{
		float flDamage = 40.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 400.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 280.0f;
#endif
		return flDamage;
	}

	KnockbackData GetKnockBackData() override
	{
		if (pev->iuser1)
			return { 5000.0f, 500.0f, 300.0f, 200.0f, 1.0f };
		else
			return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	}

	void ItemPostFrame() override;

	int m_iSteamModelIndex;
	int m_iAmmo;
	time_point_t m_flTimeNextCharge;

};

LINK_ENTITY_TO_CLASS(knife_spknife, CKnifeSPKnife)

void CKnifeSPKnife::Spawn()
{
	m_iKnifeID = KNIFE_SPKNIFE;
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

	if (m_iSwing & 1)
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_slasha2_2.wav", VOL_NORM, ATTN_NORM, 0, 94);
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_slasha2_1.wav", VOL_NORM, ATTN_NORM, 0, 94);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.56s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	pev->iuser1 = 0;
	switch (sv::KnifeAttack(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, GetPrimaryAttackDamage(), 80, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
	
		break;
	}
	case HIT_PLAYER:
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.37s;
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		if (m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/balrog9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/balrog9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);	
		break;
	}
	case HIT_WALL:
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.37s;
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	}
#endif

	m_iSwing++;
	m_iSwing %= 2;

}

void CKnifeSPKnife::SecondaryAttack(void)
{
	int iAnim;
	if (m_iAmmo < 30)
	{
		m_iWeaponState = 0;
		iAnim = m_iSwing ? ANIM_SLASHA1_2 : ANIM_SLASHA1_1;

		if (m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_slasha1_2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_slasha1_1.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	}
	else
	{
		m_iWeaponState = 1;
		iAnim = RANDOM_LONG(0, 1) ? ANIM_SLASHB3_2 : ANIM_SLASHB3_1;

		if (iAnim % 2)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_slashb3_1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spknife_slashb3_2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.52s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		m_iAmmo -= 30;

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(44);
		WRITE_BYTE(m_iAmmo);
		MESSAGE_END();
#endif
		if (m_iAmmo < 30)
			iAnim += 4;
	}

	SendWeaponAnim(iAnim, UseDecrement() != FALSE);
	
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	pev->iuser1 = 1;
	switch (KnifeAttack(GetSecondaryAttackDamage(), 80, 50, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		if (m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/balrog9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/balrog9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}

	if (m_iWeaponState)
	{
		struct
		{
			float RadiusDamageRadius;
			int RadiusDamageBitsDamageType;
			bool RadiusDamageCanHeadshot;
			float RadiusDamageAmount;
			float RadiusDamageHasFallOff;
		} Data;

		Vector vecSrc = m_pPlayer->GetGunPosition();

		Data.RadiusDamageRadius = 175;
		Data.RadiusDamageBitsDamageType = DMG_BULLET;
		Data.RadiusDamageCanHeadshot = false;
		Data.RadiusDamageAmount = GetBuffDamage();
		Data.RadiusDamageHasFallOff = false;

		std::vector<CBaseEntity*> out;
		RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
		for (CBaseEntity* ent : out)
		{
			CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
			if (player && player->m_bIsZombie)
				player->pev->velocity.z = 400.0f;
		}
	}
#endif
	m_iWeaponState = 0;

}

#ifndef CLIENT_DLL
hit_result_t CKnifeSPKnife::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
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

		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
			pPlayer->pev->velocity.z = 400;

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

			if (m_iAmmo == 30 && m_flTimeWeaponIdle <= 0.0s)
			{
				//event
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, FALSE);

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.01s;
			}

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
