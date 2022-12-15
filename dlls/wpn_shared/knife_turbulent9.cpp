/*
knife_skullaxe.cpp
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

class CKnifeTurbulent9: public LinkWeaponTemplate<CKnifeTurbulent9,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_turbulent9";
	static constexpr const char *V_Model = "models/v_turbulent9.mdl";
	static constexpr const char *P_Model = "models/p_turbulent9.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "dragontail";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

#define TURBULENT9_HIT_COUNT 6
#define TURBULENT9_HIT_INTERVAL 0.2s

	time_point_t m_flTimeCheckCharge;
	time_point_t m_flAttackLoop;
	int m_iChargeState;
	int m_iHitCount;

public:
	enum
	{
		ANIM_IDLE,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_DRAW,
		ANIM_SLASH3_READY,
		ANIM_SLASH3_IDLE,
		ANIM_SLASH3_HIT_START,
		ANIM_SLASH3_HIT_IDLE,
		ANIM_SLASH3_HIT_END,
	};

	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/turbulent9_idle.wav");
		PRECACHE_SOUND("weapons/turbulent9_hit1.wav");
		PRECACHE_SOUND("weapons/turbulent9_hit2.wav");
		PRECACHE_SOUND("weapons/turbulent9_wood1.wav");
		PRECACHE_SOUND("weapons/turbulent9_wood2.wav");
		PRECACHE_SOUND("weapons/turbulent9_stone1.wav");
		PRECACHE_SOUND("weapons/turbulent9_stone2.wav");
		PRECACHE_SOUND("weapons/turbulent9_metal1.wav");
		PRECACHE_SOUND("weapons/turbulent9_metal2.wav");
		PRECACHE_SOUND("weapons/turbulent9_slash3_ready.wav");
		PRECACHE_SOUND("weapons/turbulent9_slash3_idle.wav");
		PRECACHE_SOUND("weapons/turbulent9_slash3_hit_idle.wav");
		PRECACHE_SOUND("weapons/turbulent9_slash1.wav");
		PRECACHE_SOUND("weapons/turbulent9_slash2.wav");
		PRECACHE_SOUND("weapons/turbulent9_stab_miss.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState = 0;
		pev->iuser1 = 0;
		m_flAttackLoop = invalid_time_point;
		m_flTimeCheckCharge = invalid_time_point;
		m_iChargeState = 0;

		return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "dragontail", UseDecrement() != FALSE, 1.5s);
	}

	void Spawn() override;
	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }
	void ItemPostFrame() override;
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
	void OnAttack2Released();
	void StopIdleSound(bool IsSlash);
	void MaterialSound(float flRadius);
public:
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 30;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 540.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1800.0f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 30;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 540.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1800.0f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_turbulent9, CKnifeTurbulent9)

void CKnifeTurbulent9::Spawn()
{
	pev->classname = MAKE_STRING("knife_turbulent9");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	m_flTimeCheckCharge = m_flAttackLoop = invalid_time_point;
	FallInit();
}

void CKnifeTurbulent9::PrimaryAttack(void)
{
	m_flAttackLoop = invalid_time_point;
	m_flTimeCheckCharge = invalid_time_point;
	m_iChargeState = 0;

	SendWeaponAnim(pev->iuser1 ? ANIM_SLASH2 : ANIM_SLASH1, UseDecrement() != FALSE);

	SetThink(&CKnifeTurbulent9::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.2s;

	m_iSwing = 0;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
}

void CKnifeTurbulent9::SecondaryAttack(void)
{
	if (m_iChargeState)
	{
		if (gpGlobals->time > m_flTimeCheckCharge)
		{
			if (m_iChargeState < 2)
			{
				SendWeaponAnim(ANIM_SLASH3_IDLE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent9_slash3_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				m_iChargeState = 2;
				m_flTimeCheckCharge = gpGlobals->time + 1.0s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
			}
			else
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent9_slash3_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				m_flTimeCheckCharge = gpGlobals->time + 1.0s;
			}

			
		}
	}
	else
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent9_slash3_ready.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		SendWeaponAnim(ANIM_SLASH3_READY);

		m_flAttackLoop = m_flTimeCheckCharge = gpGlobals->time + 0.6s;
		m_iChargeState = 1;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 9999.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	}
}

void CKnifeTurbulent9::DelayPrimaryAttack()
{
	if (pev->iuser1)
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent9_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

#ifndef CLIENT_DLL
	switch (KnifeAttack5(GetPrimaryAttackDamage(), 60, 80, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer)) 
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/turbulent9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/turbulent9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		break;
	}
	case HIT_WALL:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;

		MaterialSound(79);

		break;
	}
	}
#endif

	if (++m_iSwing & 1)
	{
		pev->iuser1++;
		pev->iuser1 %= 2;
		pev->nextthink = gpGlobals->time + 0.13s;
		SetThink(&CKnifeTurbulent9::DelayPrimaryAttack);
	}
	else
	{
		SetThink(nullptr);
	}

	m_iSwing++;
	m_iSwing %= 2;
}

void CKnifeTurbulent9::OnAttack2Released()
{
	SendWeaponAnim(ANIM_SLASH3_HIT_START);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.13s;

	m_iChargeState = 3;

	m_iHitCount = TURBULENT9_HIT_COUNT;

	m_flAttackLoop = gpGlobals->time + 0.1s;
}

void CKnifeTurbulent9::MaterialSound(float flRadius)
{
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecStart = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecStart + gpGlobals->v_forward * flRadius;
	TraceResult ptr;
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &ptr);

	const char* rgsz[2];

	switch (UTIL_TextureHit(&ptr, vecStart, vecEnd))
	{
	case CHAR_TEX_WOOD:
		rgsz[0] = "weapons/turbulent9_wood1.wav";
		rgsz[1] = "weapons/turbulent9_wood2.wav";

		break;

	case CHAR_TEX_GRATE:
	case CHAR_TEX_METAL:
	case CHAR_TEX_COMPUTER:
		rgsz[0] = "weapons/turbulent9_metal1.wav";
		rgsz[1] = "weapons/turbulent9_metal2.wav";

		break;

	default:
		rgsz[0] = "weapons/turbulent9_stone1.wav";
		rgsz[1] = "weapons/turbulent9_stone12.wav";

		break;
	}

	UTIL_EmitAmbientSound(ENT(0), ptr.vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
#endif
}


void CKnifeTurbulent9::DelaySecondaryAttack()
{
	SetThink(nullptr);
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	switch (KnifeAttack5(GetSecondaryAttackDamage(), 79, 80, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer)) 
	{
	case HIT_NONE:
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.4s;
		break;
	}
	case HIT_PLAYER:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/turbulent9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/turbulent9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_WALL:
	{
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;

		MaterialSound(79);
		
		break;
	}
	}
#endif
}

void CKnifeTurbulent9::ItemPostFrame()
{
	if (m_flAttackLoop != invalid_time_point && m_flAttackLoop < gpGlobals->time)
	{
		switch (m_iChargeState)
		{
		case 5:
		{
			SendWeaponAnim(ANIM_SLASH3_HIT_END);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.4s;


			m_flAttackLoop = invalid_time_point;
			m_iChargeState = 0;
			break;
		}
		case 4:
		{
			m_flAttackLoop = gpGlobals->time + TURBULENT9_HIT_INTERVAL;

			DelaySecondaryAttack();

			m_iHitCount--;

			if (m_iHitCount <= 0)
			{
				m_iChargeState = 5;
			}

			break;
		}
		case 3:
		{
			m_iChargeState = 4;

			SendWeaponAnim(ANIM_SLASH3_HIT_IDLE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;

			m_flAttackLoop = gpGlobals->time + 0.1s;
			break;
		}
		default:
			break;
		}

	}

	CBasePlayerWeapon::ItemPostFrame();
}


void CKnifeTurbulent9::StopIdleSound(bool IsSlash)
{
#ifndef CLIENT_DLL
	int player = m_pPlayer->entindex();
	if (IsSlash)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(79);
		WRITE_SHORT(player);
		WRITE_SHORT(CHAN_ITEM);
		WRITE_STRING("weapons/turbulent9_slash3_idle.wav");
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(79);
		WRITE_SHORT(player);
		WRITE_SHORT(CHAN_ITEM);
		WRITE_STRING("weapons/turbulent9_idle.wav");
		MESSAGE_END();
	}
#endif
}


void CKnifeTurbulent9::WeaponIdle()
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flAttackLoop != invalid_time_point && m_flAttackLoop < gpGlobals->time)
	{
		if (m_iChargeState > 0 && m_iChargeState < 3 && !(m_pPlayer->pev->button & IN_ATTACK2))
		{
			m_flAttackLoop = invalid_time_point;
			OnAttack2Released();
		}
	}


	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
}

}
