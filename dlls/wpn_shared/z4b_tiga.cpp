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

class CKnifeTIGA: public LinkWeaponTemplate<CKnifeTIGA,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_tiga";
	static constexpr const char *V_Model = "models/z4b/v_tiga.mdl";
	static constexpr const char *P_Model = "models/z4b/p_tiga.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 0.7f };

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
		ANIM_SKILL,
		ANIM_SKILL_CHANGE,
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHero_Attack_1.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHero_Attack_2.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHero_Attack_3.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHero_HIT.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHero_STAB.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHero_SWING.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHeroSkill.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/tiga/MasterHeroSkill_Hit.wav");
		m_iSkillEffect = PRECACHE_MODEL("sprites/ef_tiga.spr");
	}

	BOOL Deploy() override
	{
#if 0
		SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", "masterhunter");
		m_pPlayer->SetNewPlayerModel("models/player/masterhunter/masterhunter.mdl");
#endif
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		pev->iuser1 = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;

		if (m_flTimeHostler != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeHostler) / 0.125s;
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
	void DelaySkillAttack();
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsSkillAttack = false);
#endif
private:
	int m_iThinkType;
	int m_iSkillEffect;
	int m_iHolsterAmmo;
	time_point_t m_tNextThink;
	time_point_t m_flTimeNextCharge;
	time_point_t m_flTimeHostler;
public:

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
			flDamage = 1605 + (g_pGameRules->m_iTotalRoundsPlayed * 100);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	float GetSkillAttackDamage() const
	{
		float flDamage = 80;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1140 + (g_pGameRules->m_iTotalRoundsPlayed * 100);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(z4b_tiga, CKnifeTIGA)


void CKnifeTIGA::Spawn()
{
	m_iKnifeID = KNIFE_TIGA;
	pev->classname = MAKE_STRING("z4b_tiga");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_flTimeHostler = gpGlobals->time;
	m_iDefaultAmmo = 100;
	m_iClip = WEAPON_NOCLIP;
	m_flTimeNextCharge = invalid_time_point;
	FallInit();
}

int CKnifeTIGA::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "tigaammo";
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

int CKnifeTIGA::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iDefaultAmmo = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CKnifeTIGA::Holster(int skiplocal)
{
	SetThink(nullptr);

	m_iHolsterAmmo = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	m_flTimeHostler = gpGlobals->time;
	return CBasePlayerWeapon::Holster(skiplocal);
}


void CKnifeTIGA::ItemPostFrame()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 100)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
			m_flTimeNextCharge = gpGlobals->time + 0.125s;
		}
	}

	if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time)
	{
		switch (m_iThinkType)
		{
		case 1:
			DelaySkillAttack();
			m_tNextThink = gpGlobals->time + 0.25s;
			m_iThinkType = 2;
			break;
		case 2:
			DelaySkillAttack();
			m_tNextThink = gpGlobals->time + 0.27s;
			m_iThinkType = 3;
			break;
		case 3:
			DelaySkillAttack();
			m_tNextThink = gpGlobals->time + 0.34s;
			m_iThinkType = 4;
			break;
		case 4:
			DelaySkillAttack();
			m_tNextThink = gpGlobals->time + 0.1s;
			m_iThinkType = 5;
			break;
		case 5:
			m_tNextThink = invalid_time_point;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			m_iThinkType = 0;
			break;
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeTIGA::Reload()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

		m_iThinkType = 1;
		m_tNextThink = gpGlobals->time + 0.7s;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/tiga/MasterHeroSkill.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		SendWeaponAnim(ANIM_SKILL, UseDecrement() != FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

#ifndef CLIENT_DLL
		strcpy(m_pPlayer->m_szCustomSequence, "knife_skill");
		m_pPlayer->pev->frame = 0;
		m_pPlayer->ResetSequenceInfo();
		m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 2.0s;
#endif
	}
}
void CKnifeTIGA::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
	{
		strcpy(m_pPlayer->m_szCustomSequence, "crouch_shoot_Lknife");
	}
	else
		strcpy(m_pPlayer->m_szCustomSequence, "ref_shoot_Lknife");
	m_pPlayer->pev->frame = 0;
	m_pPlayer->ResetSequenceInfo();
	m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.87s;
#endif
	m_iSwing = 0;
	SendWeaponAnim(RANDOM_LONG(0, 1) ? ANIM_STAB1 : ANIM_STAB2, UseDecrement() != FALSE);

	pev->iuser1 %= 2;
	SetThink(&CKnifeTIGA::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.09s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;
}

void CKnifeTIGA::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(pev->iuser1++ ? ANIM_SLASH1 : ANIM_SLASH2, UseDecrement() != FALSE);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/tiga/MasterHero_Attack_3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	DelayPrimaryAttack();
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
	m_iSwing = 0;
}

void CKnifeTIGA::DelaySkillAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSkillAttackDamage(), 135, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/tiga/MasterHeroSkill_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/tiga/MasterHero_SWING.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif
}

void CKnifeTIGA::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetPrimaryAttackDamage(), 85, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/tiga/MasterHero_HIT.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/tiga/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/tiga/MasterHero_SWING.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	if(++m_iSwing & 1)
	{
		pev->nextthink = gpGlobals->time + 0.11s;
		SetThink(&CKnifeTIGA::DelayPrimaryAttack);
	}
	else
	{
		SetThink(nullptr);
	}

	//return fDidHit;
}

void CKnifeTIGA::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 115, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/tiga/MasterHero_HIT.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/tiga/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/tiga/MasterHero_SWING.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	SetThink(nullptr);
}

#ifndef CLIENT_DLL	
hit_result_t CKnifeTIGA::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsSkillAttack)
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

		if (IsSkillAttack)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z - 16);
			WRITE_SHORT(m_iSkillEffect);
			WRITE_BYTE(8);
			WRITE_BYTE(255);
			WRITE_BYTE(10);
			MESSAGE_END();
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

void CKnifeTIGA::WeaponIdle(void)
{
	ResetEmptySound();
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	}

}

}
