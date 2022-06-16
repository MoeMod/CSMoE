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

class CKnifeHolyFist: public LinkWeaponTemplate<CKnifeHolyFist,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_holyfist";
	static constexpr const char *V_Model = "models/z4b/v_holyfist.mdl";
	static constexpr const char *P_Model = "models/z4b/p_holyfist.mdl";
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

		PRECACHE_SOUND("CK_WPN/WEAPON/holyfist/AsceticHero_ShieldExplosion.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/holyfist/AsceticHero_ShieldStart.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/holyfist/Ascetic_STAB.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/holyfist/Ascetic_Attack_1.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/holyfist/Ascetic_Attack_3.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/holyfist/Ascetic_Select.wav");

		PRECACHE_MODEL("sprites/terminator_ring.spr"); 
		m_iSkillGlitchEffect = PRECACHE_MODEL("sprites/glitch.spr");
		m_iSkillRingEffect = PRECACHE_MODEL("sprites/glitch_ring.spr");
		m_iSkillShockWaveEffect = PRECACHE_MODEL("sprites/graphic.spr");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		pev->iuser1 = 0;
		pev->iuser2 = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		ResetRunIdle();

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
	time_point_t m_tNextThink;
	time_point_t m_flTimeNextCharge;
	time_point_t m_flTimeHostler;
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
public:
#ifndef CLIENT_DLL
	void Explode();
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
			flDamage = 330 + (g_pGameRules->m_iTotalRoundsPlayed * 50);
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
			flDamage = 4000 + (g_pGameRules->m_iTotalRoundsPlayed * 100);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(z4b_holyfist, CKnifeHolyFist)


void CKnifeHolyFist::Spawn()
{
	pev->classname = MAKE_STRING("z4b_holyfist");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_flTimeHostler = gpGlobals->time;
	m_iDefaultAmmo = 100;
	m_iClip = WEAPON_NOCLIP;
	m_flTimeNextCharge = invalid_time_point;
	FallInit();
}

int CKnifeHolyFist::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "holyfistammo";
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

int CKnifeHolyFist::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iDefaultAmmo = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CKnifeHolyFist::Holster(int skiplocal)
{
	SetThink(nullptr);

	m_iHolsterAmmo = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	m_flTimeHostler = gpGlobals->time;
	return CBasePlayerWeapon::Holster(skiplocal);
}
#ifndef CLIENT_DLL
void CKnifeHolyFist::Explode()
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/holyfist/AsceticHero_ShieldExplosion.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	KnifeAttack(GetSkillAttackDamage(), 220, 360, DMG_NEVERGIB | DMG_BULLET | DMG_NOMORALE, this->pev, m_pPlayer, true);

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_TEMPSPRITE);
	WRITE_COORD(m_pPlayer->pev->origin.x);
	WRITE_COORD(m_pPlayer->pev->origin.y);
	WRITE_COORD(m_pPlayer->pev->origin.z + 105);
	WRITE_SHORT(m_iSkillGlitchEffect);
	WRITE_BYTE(8);
	WRITE_BYTE(255);
	WRITE_BYTE(12);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_TEMPSPRITE);
	WRITE_COORD(m_pPlayer->pev->origin.x);
	WRITE_COORD(m_pPlayer->pev->origin.y);
	WRITE_COORD(m_pPlayer->pev->origin.z);
	WRITE_SHORT(m_iSkillRingEffect);
	WRITE_BYTE(7);
	WRITE_BYTE(255);
	WRITE_BYTE(12);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, m_pPlayer->pev);
	WRITE_BYTE(16);
	WRITE_SHORT(m_pPlayer->entindex());
	WRITE_BYTE(15);
	MESSAGE_END();
}
#endif

void CKnifeHolyFist::ItemPostFrame()
{
#ifndef CLIENT_DLL
	if ((m_pPlayer->m_tHealthBuffTime != invalid_time_point && m_pPlayer->m_tHealthBuffTime < gpGlobals->time) || (!m_pPlayer->m_flBuffHealthAmount && pev->iuser1))
	{
		m_pPlayer->m_iBuff &= ~BUFF_HPBUFF;
		m_pPlayer->m_tHealthBuffTime = invalid_time_point;
		m_pPlayer->m_flBuffHealthAmount = 0;
		pev->iuser1 = 0;
		pev->iuser2 = 0;
		Explode();
		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(6); // TYPE
		WRITE_SHORT(int(m_pPlayer->m_flBuffHealthAmount));
		MESSAGE_END();
	}

	if (m_pPlayer->m_flBuffHealthAmount <= 500 && pev->iuser2 && pev->iuser1)
	{
		pev->iuser2 = 0;
		float time = float((gpGlobals->time - m_pPlayer->m_tHealthBuffTime) / 1s);
		if (time <= 1.0)
			time = 1.0;
		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(5);
		WRITE_BYTE(39);
		WRITE_SHORT(-1);
		WRITE_BYTE(1);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(5);
		WRITE_BYTE(39);
		WRITE_SHORT(time);
		WRITE_BYTE(1);
		MESSAGE_END();
	}
#endif

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
			DelayPrimaryAttack();
			m_tNextThink = gpGlobals->time + 0.12s;
			m_iThinkType = 2;
			break;
		case 2:
			DelayPrimaryAttack();
			m_tNextThink = gpGlobals->time + 0.23s;
			m_iThinkType = 3;
			break;
		case 3:
			DelayPrimaryAttack();
			m_tNextThink = gpGlobals->time + 0.21s;
			m_iThinkType = 4;
			break;
		case 4:
			DelayPrimaryAttack();
			m_tNextThink = invalid_time_point;
			m_iThinkType = 0;
			break;
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeHolyFist::Reload()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/holyfist/AsceticHero_ShieldStart.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		
#ifndef CLIENT_DLL
		m_pPlayer->m_iBuff |= BUFF_HPBUFF;
		m_pPlayer->m_tHealthBuffTime = gpGlobals->time + 5.0s;
		m_pPlayer->m_flBuffHealthAmount = 1500;
		pev->iuser1 = 1;
		pev->iuser2 = 1;
		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(6); // TYPE
		WRITE_SHORT(int(m_pPlayer->m_flBuffHealthAmount));
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(5);
		WRITE_BYTE(39);
		WRITE_SHORT(5);
		WRITE_BYTE(0);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(16);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_BYTE(15);
		MESSAGE_END();
#endif
	}
}

void CKnifeHolyFist::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/holyfist/Ascetic_Attack_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	m_iThinkType = 1;
	m_tNextThink = gpGlobals->time + 0.13s;

	SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.65s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
	ResetRunIdle();
}

void CKnifeHolyFist::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
	{
		strcpy(m_pPlayer->m_szCustomSequence, "crouch_shoot_shieldknife");
	}
	else
		strcpy(m_pPlayer->m_szCustomSequence, "ref_shoot_shieldknife");
	m_pPlayer->pev->frame = 0;
	m_pPlayer->ResetSequenceInfo();
	m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1.8s;
#endif

	SendWeaponAnim(RANDOM_LONG(0, 1)? ANIM_STAB1:ANIM_STAB2, UseDecrement() != FALSE);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/holyfist/Ascetic_Attack_3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	SetThink(&CKnifeHolyFist::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.8s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.8s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
	ResetRunIdle();
}

void CKnifeHolyFist::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetPrimaryAttackDamage(), 65, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/holyfist/Ascetic_STAB.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/holyfist/Ascetic_STAB.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif
}

void CKnifeHolyFist::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 115, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/holyfist/Ascetic_STAB.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/holyfist/Ascetic_STAB.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	SetThink(nullptr);
}

#ifndef CLIENT_DLL	
hit_result_t CKnifeHolyFist::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsSkillAttack)
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

void CKnifeHolyFist::WeaponIdle(void)
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
