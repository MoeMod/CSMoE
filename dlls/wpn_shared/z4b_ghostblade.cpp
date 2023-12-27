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

class CKnifeGhostBlade: public LinkWeaponTemplate<CKnifeGhostBlade,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_ghostblade";
	static constexpr const char *V_Model = "models/z4b/v_ghostblade.mdl";
	static constexpr const char *P_Model = "models/z4b/p_ghostblade.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 0.7f };
	time_point_t m_flTimeHostler;

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
		ANIM_CHANGE_B,
		ANIM_IDLE_B,
		ANIM_STAB_B,
		ANIM_DEFENSE_START,
		ANIM_DEFENSE_LOOP,
		ANIM_DEFENSE_HIT,
		ANIM_RUN_B,
		ANIM_CHANGE_A,
		ANIM_SKILL,
		ANIM_SKILL_LOOP,
		ANIM_SKILL_END,
		ANIM_DASH_F,
		ANIM_DASH_B,
		ANIM_DASH_L,
		ANIM_DASH_R,
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("CK_WPN/WEAPON/ghostblade/ghostblade_Attack_3.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/ghostblade/ghostblade_Attack_2.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/ghostblade/ghostblade_Attack_1.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
	
		m_pPlayer->m_bShieldDrawn = false;
		ResetRunIdle();

		if (m_flTimeHostler != invalid_time_point)
		{
			m_iClip += (gpGlobals->time - m_flTimeHostler) / 0.125s;
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHostler) / 0.125s;
		}
		if (m_iClip >= 100)
			m_iClip = 100;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;

		return Base::Deploy();
	}

	void ItemPostFrame() override;
	void Reload() override;
	int GetItemInfo(ItemInfo* p) override;
	void Holster(int skiplocal) override;
	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }
	void WeaponIdle() override;
	void WeaponCallBack(int iType) override;
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer);
#endif

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}
private:
	int m_iDefenseState;
	time_point_t m_flTimeBuff;
	time_point_t m_flTimeHolster;
	time_point_t m_flTimeNextCharge;
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
public:
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
			flDamage = 280 + (g_pGameRules->m_iTotalRoundsPlayed * 50);
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
			flDamage = 1105 + (g_pGameRules->m_iTotalRoundsPlayed * 100);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(z4b_ghostblade, CKnifeGhostBlade)

int CKnifeGhostBlade::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ghostbladeammo";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = "ghostbladeammo2";
	p->iMaxAmmo2 = 100;
	p->iMaxClip = 100;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

void CKnifeGhostBlade::Spawn()
{
	m_iKnifeID = KNIFE_GHOSTBLADE;
	pev->classname = MAKE_STRING("z4b_ghostblade");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iDefaultAmmo = 100;
	m_flTimeHostler = gpGlobals->time;
	m_flTimeNextCharge = invalid_time_point;
	FallInit();
}

void CKnifeGhostBlade::Holster(int skiplocal)
{
	SetThink(nullptr);

	m_flTimeHostler = gpGlobals->time;
	return CBasePlayerWeapon::Holster(skiplocal);
}


void CKnifeGhostBlade::ItemPostFrame()
{
	if (m_flTimeNextCharge <= gpGlobals->time)
	{
		if (m_iClip < 100)
		{
			m_iClip++;
		}

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
		}

		m_flTimeNextCharge = gpGlobals->time + 0.125s;
	}

	return CBasePlayerWeapon::ItemPostFrame();
}


void CKnifeGhostBlade::Reload()
{
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase() || m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
		return;

	if (m_iWeaponState)
	{
		m_iDefenseState = m_iWeaponState = 0;
		SendWeaponAnim(ANIM_CHANGE_A, UseDecrement() != FALSE);
	}
		
	else
	{
		m_iWeaponState = 1;
		SendWeaponAnim(ANIM_CHANGE_B, UseDecrement() != FALSE);
	}
	
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.33s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33s;
	m_iSwing = 0;
	ResetRunIdle();
}

void CKnifeGhostBlade::PrimaryAttack(void)
{
	if (m_iWeaponState)
	{
		SendWeaponAnim(ANIM_STAB_B, UseDecrement() != FALSE);

		SetThink(&CKnifeGhostBlade::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.17s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
		ResetRunIdle();
	}
	else
	{
		SendWeaponAnim(m_iSwing++ ? ANIM_SLASH1 : ANIM_SLASH2, UseDecrement() != FALSE);
		if (m_iSwing)
		{
#ifndef CLIENT_DLL
			strcpy(m_pPlayer->m_szCustomSequence, "combo1_aim_knife");
			m_pPlayer->ResetSequenceInfo();
			m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.52s;
#endif

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/ghostblade/ghostblade_Attack_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		else
		{
#ifndef CLIENT_DLL
			strcpy(m_pPlayer->m_szCustomSequence, "combo2_aim_knife");
			m_pPlayer->ResetSequenceInfo();
			m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.52s;
#endif

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/ghostblade/ghostblade_Attack_2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}

		m_iSwing %= 2;
		SetThink(&CKnifeGhostBlade::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.17s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
		ResetRunIdle();
	}

}

void CKnifeGhostBlade::WeaponCallBack(int iType)
{
	int bitsDamageType = iType;
	if (bitsDamageType & DMG_BULLET)
	{
		float flDamage = 75.0f;

		if (m_pPlayer->pev->health - flDamage >= 300)
		{
			m_pPlayer->pev->health += flDamage;
		}
	}

	SendWeaponAnim(ANIM_DEFENSE_HIT, UseDecrement() != FALSE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
}

void CKnifeGhostBlade::SecondaryAttack(void)
{
	if (m_iWeaponState)
	{
		if (m_iDefenseState == 1)
		{
			m_iDefenseState = m_iWeaponState = 0;
			m_pPlayer->m_iBuff &= ~BUFF_HOLYSWORD_AVOID_INFECT;

			SendWeaponAnim(ANIM_CHANGE_A, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.33s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33s;
		}
		else
		{
			SendWeaponAnim(ANIM_DEFENSE_START, UseDecrement() != FALSE);

			m_pPlayer->m_iBuff |= BUFF_HOLYSWORD_AVOID_INFECT;
			m_iDefenseState = 1;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
			ResetRunIdle();
		}
	
	}
	else
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		SendWeaponAnim(RANDOM_LONG(0, 1) ? ANIM_STAB1 : ANIM_STAB2, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/ghostblade/Hero_ATTACK_3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		SetThink(&CKnifeGhostBlade::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.56s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
		m_iSwing = 0;
		ResetRunIdle();
	}
}

void CKnifeGhostBlade::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetPrimaryAttackDamage(), 85, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualKukri/Hero_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualKukri/Hero_SWING.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif
}

void CKnifeGhostBlade::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 135, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualKukri/Hero_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualKukri/Hero_SWING.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	SetThink(nullptr);
}

#ifndef CLIENT_DLL	
hit_result_t CKnifeGhostBlade::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
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

		tr.iHitgroup = HITGROUP_HEAD;
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

void CKnifeGhostBlade::WeaponIdle(void)
{
	ResetEmptySound();
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iDefenseState)
		{
			SendWeaponAnim(ANIM_DEFENSE_LOOP, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() +20.0s;
			return;
		}

		if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
		{
			if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
			{
				if (m_iWeaponState == 1)
				{
					SendWeaponAnim(ANIM_RUN_B, UseDecrement() != FALSE);

					m_flNextRunIdle = UTIL_WeaponTimeBase() + 1.3s;
				}
				else
				{
					SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

					m_flNextRunIdle = UTIL_WeaponTimeBase() + 1.0s;
				}
			}

			m_flNextIdle = UTIL_WeaponTimeBase();
		}
		else
		{
			if (m_flNextIdle <= UTIL_WeaponTimeBase())
			{
				if (m_iWeaponState == 1)
				{
					SendWeaponAnim(ANIM_IDLE_B, UseDecrement() != FALSE);

					m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
				}
				else
				{
					SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

					m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
				}			
			}

			m_flNextRunIdle = UTIL_WeaponTimeBase();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
	}

}

}
