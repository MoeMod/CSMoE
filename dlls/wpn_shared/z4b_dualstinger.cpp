/*
z4b_oppovivo.cpp
Copyright (C) 2019 Moemod Hymei

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

class CKnifeDualStinger: public LinkWeaponTemplate<CKnifeDualStinger,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_dualstinger";
	static constexpr const char *V_Model = "models/z4b/v_dualstinger.mdl";
	static constexpr const char *P_Model = "models/z4b/p_dualstinger.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 0.8f };

public:

	enum skullaxe_e
	{
		ANIM_IDLE1 = 0,
		ANIM_NONE,
		ANIM_WSSB,
		ANIM_DRAW,
		ANIM_STAB1,
		ANIM_STAB2,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_RUN,
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/DevilHunter_Attack_1.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/DevilHunter_Attack_2.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/DevilHunter_Attack_3.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/DevilHunter_Hit.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/DevilHunter_Select.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/DevilHunter_Stab.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/DevilHunter_Swing.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/DualStinger/HumanBossAppear.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		pev->iuser1 = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		ResetRunIdle();

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/DualStinger/DevilHunter_Select.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		if (m_flTimeHostler != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeHostler) / 0.1s;
		}
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 100;

		return Base::Deploy();
	}

	int GetItemInfo(ItemInfo* p) override;
	void Spawn() override;
	void Reload() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	BOOL CanDrop() override { return false; }
	void Holster(int skiplocal) override;
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsSkillAttack = false);
#endif
private:
	int m_iThinkType;
	int m_iSkillGlitchEffect;
	int m_iSkillShockWaveEffect;
	int m_iSkillRingEffect;	
	int m_iHolsterAmmo;
	Vector m_vecDashVelocity;
	time_point_t m_tNextThink;
	time_point_t m_flTimeNextCharge;
	
	time_point_t m_flTimeHostler;
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
public:
#ifndef CLIENT_DLL
	time_point_t m_flDashTime;
#endif
	void ResetRunIdle()
	{
		m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
	}

	void DelayPrimaryAttack();
	void DelaySecondaryAttack();

	float GetPrimaryAttackDamage() const
	{
		float flDamage = 47;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 230 + (g_pGameRules->m_iTotalRoundsPlayed * 50);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 120;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 360 + (g_pGameRules->m_iTotalRoundsPlayed * 100);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

};

LINK_ENTITY_TO_CLASS(z4b_dualstinger, CKnifeDualStinger)


void CKnifeDualStinger::Spawn()
{
	pev->classname = MAKE_STRING("z4b_dualstinger");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_flTimeHostler = gpGlobals->time;
	m_iDefaultAmmo = 100;
	m_iClip = WEAPON_NOCLIP;
#ifndef CLIENT_DLL
	m_flDashTime = invalid_time_point;
#endif
	m_flTimeNextCharge = invalid_time_point;
	FallInit();
}

int CKnifeDualStinger::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "dualstingerammo";
	p->iMaxAmmo1 = 100;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 100;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

int CKnifeDualStinger::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iDefaultAmmo = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CKnifeDualStinger::Holster(int skiplocal)
{
	SetThink(nullptr);

	m_iHolsterAmmo = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	m_flTimeHostler = gpGlobals->time;
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CKnifeDualStinger::ItemPostFrame()
{
#ifndef CLIENT_DLL
	if (m_flDashTime != invalid_time_point && m_flDashTime > gpGlobals->time)
	{
		m_pPlayer->pev->velocity = m_vecDashVelocity;
	}
#endif
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 100)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
			m_flTimeNextCharge = gpGlobals->time + 0.1s;
		}
	}

	if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time)
	{
		switch (m_iThinkType)
		{
		case 1:
			DelaySecondaryAttack();
			m_tNextThink = gpGlobals->time + 0.28s;
			m_iThinkType = 2;
			break;
		case 2:
			DelaySecondaryAttack();
			m_tNextThink = gpGlobals->time + 0.27s;
			m_iThinkType = 3;
			break;
		case 3:
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			DelaySecondaryAttack();
			m_tNextThink = gpGlobals->time + 0.26s;
			m_iThinkType = 4;
			break;
		case 4:
			DelaySecondaryAttack();
			m_tNextThink = invalid_time_point;
			m_iThinkType = 0;
			break;
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeDualStinger::Reload()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
		
		Vector vecSrc = m_pPlayer->GetGunPosition();

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		Vector vecDir = gpGlobals->v_forward;
		Vector vecEnd = vecSrc + vecDir * 300;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);
		vecSrc = tr.vecEndPos;

		Vector vecVelocity = vecSrc - m_pPlayer->pev->origin;
		vecVelocity *= 600 / vecVelocity.Length();
				
		m_vecDashVelocity = vecVelocity;
		

#ifndef CLIENT_DLL
		m_flDashTime = gpGlobals->time + 0.2s;

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(2);
		WRITE_BYTE(1);
		WRITE_SHORT(1);
		WRITE_SHORT(-1);
		MESSAGE_END();
#endif
	}
}

void CKnifeDualStinger::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	m_iThinkType = 1;
	m_tNextThink = gpGlobals->time + 0.37s;
	m_iSwing = 0;
	SendWeaponAnim(ANIM_STAB1, UseDecrement() != FALSE);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/DualStinger/DevilHunter_Attack_3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.8s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
	ResetRunIdle();
}

void CKnifeDualStinger::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	SendWeaponAnim(pev->iuser1++ ? ANIM_SLASH1 : ANIM_SLASH2, UseDecrement() != FALSE);
	if (pev->iuser1)
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/DualStinger/DevilHunter_Attack_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/DualStinger/DevilHunter_Attack_2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	pev->iuser1 %= 2;
	SetThink(&CKnifeDualStinger::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.17s;
	m_iSwing = 0;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	ResetRunIdle();
}

void CKnifeDualStinger::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetPrimaryAttackDamage(), 65, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualStinger/DevilHunter_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualStinger/DevilHunter_Swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	if (++m_iSwing & 1)
	{
		pev->nextthink = gpGlobals->time + 0.27s;
		SetThink(&CKnifeDualStinger::DelayPrimaryAttack);
	}
	else
	{
		SetThink(nullptr);
	}
}

void CKnifeDualStinger::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 95, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualStinger/DevilHunter_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualStinger/DevilHunter_Stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	SetThink(nullptr);
}

#ifndef CLIENT_DLL	
hit_result_t CKnifeDualStinger::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsSkillAttack)
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

		pAttackPlayer->m_iWeaponVolume = 128;

		ClearMultiDamage();
		if (!IsSkillAttack)
		{			
			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - pAttackPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}
			tr.iHitgroup = HITGROUP_HEAD;
		}
		else
			tr.iHitgroup = HITGROUP_CHEST;
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

void CKnifeDualStinger::WeaponIdle(void)
{
	ResetEmptySound();
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
		{
			if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

				m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.55s;
			}

			m_flNextIdle = UTIL_WeaponTimeBase();
		}
		else
		{
			if (m_flNextIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

				m_flNextIdle = UTIL_WeaponTimeBase() + 10.0s;
			}

			m_flNextRunIdle = UTIL_WeaponTimeBase();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
	}

}

}
