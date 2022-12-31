/*
knife_ironfan.cpp
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

class CKnifeIronFan: public LinkWeaponTemplate<CKnifeIronFan,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_ironfan";
	static constexpr const char *V_Model = "models/v_ironfan.mdl";
	static constexpr const char *P_Model = "models/p_ironfan.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "tomahawk";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

	enum ironfan_e
	{
		ANIM_IDLE = 0,
		ANIM_SLASH1_1,
		ANIM_SLASH1_2,
		ANIM_SLASH2_1,
		ANIM_SLASH2_2,
		ANIM_SLASH3_1,
		ANIM_SLASH3_2,
		ANIM_DRAW,
	};

	void Precache() override
	{
		Base::Precache();


		PRECACHE_SOUND("weapons/ironfan_slash2_1.wav");
		PRECACHE_SOUND("weapons/ironfan_slash3_1_2.wav");
		PRECACHE_SOUND("weapons/ironfan_slash1_1_2.wav");

		PRECACHE_SOUND("weapons/tomahawk_wall.wav");
		PRECACHE_SOUND("weapons/tomahawk_slash1_hit.wav");
		PRECACHE_SOUND("weapons/tomahawk_slash2_hit.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState = 0;
		m_iCheck = 0;

		m_iLastAnim = ANIM_DRAW;
		return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE);

	}

	void Holster(int skiplocal) override;
	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void WeaponIdle() override;
	void ItemPostFrame() override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

	void SkillAttack();
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
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

	float GetSkillDamage() const
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
		if (m_iWeaponState == 2)
			return { 5000.0f, 500.0f, 300.0f, 200.0f, 1.0f };
		else
			return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	}

	int m_iLastAnim;
	int m_iCheck;

};

LINK_ENTITY_TO_CLASS(knife_ironfan, CKnifeIronFan)

void CKnifeIronFan::Spawn()
{
	pev->classname = MAKE_STRING("knife_ironfan");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	m_iCheck = 0;
	FallInit();
}

void CKnifeIronFan::Holster(int skiplocal)
{
	SetThink(nullptr);
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CKnifeIronFan::DelaySecondaryAttack(void)
{
	SetThink(nullptr);

	SendWeaponAnim(ANIM_SLASH3_2, UseDecrement() != FALSE);
	m_iLastAnim = ANIM_SLASH3_2;

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ironfan_slash3_1_2.wav", VOL_NORM, ATTN_NORM, 0, 94);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;

	m_iWeaponState = 2;
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 116, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_slash2_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_WALL:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	}
#endif
}

void CKnifeIronFan::DelayPrimaryAttack(void)
{
	SetThink(nullptr);

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ironfan_slash1_1_2.wav", VOL_NORM, ATTN_NORM, 0, 94);

	m_iWeaponState = 0;
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSkillDamage(), 71, 49, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_NONE:
	{

		break;
	}
	case HIT_PLAYER:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_slash2_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_WALL:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	}
#endif
}

void CKnifeIronFan::SkillAttack()
{
	Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	SendWeaponAnim(ANIM_SLASH2_1, UseDecrement() != FALSE);
	m_iLastAnim = ANIM_SLASH2_1;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ironfan_slash2_1.wav", VOL_NORM, ATTN_NORM, 0, 94);
	
	m_iWeaponState = 1;
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSkillDamage(), 82, 67, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_slash2_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_WALL:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	}
#endif
}

void CKnifeIronFan::PrimaryAttack(void)
{
	switch (m_iCheck)
	{
	case 0:
	{
		m_iCheck = 1;
		break;
	}
	case 1:
	{
		SkillAttack();
		m_iCheck = 2;
		return;
	}
	default:
	{
		m_iCheck = 0;
		break;
	}
	}

	Q_strcpy(m_pPlayer->m_szAnimExtention, "skullaxe");

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	SendWeaponAnim(ANIM_SLASH1_1, UseDecrement() != FALSE);
	m_iLastAnim = ANIM_SLASH1_1;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

	SetThink(&CKnifeIronFan::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 1.12s;

}

void CKnifeIronFan::SecondaryAttack(void)
{
	if (m_iCheck == 2)
	{
		Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		DelaySecondaryAttack();
		m_iCheck = 0;
		return;
	}
	else
		m_iCheck = 0;

	Q_strcpy(m_pPlayer->m_szAnimExtention, "skullaxe");

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	SendWeaponAnim(ANIM_SLASH3_1, UseDecrement() != FALSE);
	m_iLastAnim = ANIM_SLASH3_1;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.6s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	SetThink(&CKnifeIronFan::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 1.2s;

}

#ifndef CLIENT_DLL
hit_result_t CKnifeIronFan::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
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

		if (m_iWeaponState == 1)
		{
			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
			if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
				pPlayer->pev->velocity.z = 400;

		}

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

void CKnifeIronFan::ItemPostFrame(void)
{
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iLastAnim == ANIM_SLASH1_1)
		{
			m_iCheck = 0;

			Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");

			m_iLastAnim = ANIM_SLASH1_2;
			SendWeaponAnim(ANIM_SLASH1_2, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.83s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.83s;
		}
		else if (m_iLastAnim == ANIM_SLASH2_1)
		{
			m_iCheck = 0;

			Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");

			m_iLastAnim = ANIM_SLASH2_2;
			SendWeaponAnim(ANIM_SLASH2_2, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
		}
	}
	
	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeIronFan::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_iCheck = 0;

	Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");

	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	m_iLastAnim = ANIM_IDLE;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
}


}
