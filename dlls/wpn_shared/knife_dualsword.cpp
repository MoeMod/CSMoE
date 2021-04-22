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
#include "util/u_range.hpp"
#endif

#include <random>

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeDualsword: public LinkWeaponTemplate<CKnifeDualsword,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_dualsword";
	static constexpr const char *V_Model = "models/v_dualsword.mdl";
	static constexpr const char *P_Model = "models/p_dualsword_a.mdl";
	static constexpr const char *P_Model_B = "models/p_dualsword_b.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "katana";
	KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 0.6f };

private:
	int m_iuser1; // iAttackRecord
	int m_iuser3; // iSkillRecord
	int m_iuser4; // iPrimaryAttackRecord
	int m_sequence; // iThinkType
	time_point_t m_fuser1; // flNextTimeCanHolster
	int m_weaponanim; // iLastAnim
	int m_waterlevel; // iSecondaryAttackCount



public:

	BOOL CanHolster() override
	{
		return gpGlobals->time > m_fuser1;
	}

	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/dualsword_slash_4_1.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_4_2.wav");
		PRECACHE_SOUND("weapons/dualsword_stab1.wav");
		PRECACHE_SOUND("weapons/dualsword_stab2.wav");
		PRECACHE_SOUND("weapons/dualsword_stab1_hit.wav");
		PRECACHE_SOUND("weapons/dualsword_stab2_hit.wav");
		PRECACHE_SOUND("weapons/mastercombat_wall.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_1.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_2.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_3.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_4.wav");
		PRECACHE_SOUND("weapons/dualsword_hit1.wav");
		PRECACHE_SOUND("weapons/dualsword_hit2.wav");
		PRECACHE_SOUND("weapons/dualsword_hit3.wav");
		PRECACHE_SOUND("weapons/mastercombat_wall.wav");
		PRECACHE_SOUND("weapons/dualsword_skill_start.wav");
		PRECACHE_SOUND("weapons/dualsword_skill_end.wav");
		PRECACHE_SOUND("weapons/dualsword_fly1.wav");
		PRECACHE_SOUND("weapons/dualsword_fly2.wav");
		PRECACHE_SOUND("weapons/dualsword_fly3.wav");
		PRECACHE_SOUND("weapons/dualsword_fly4.wav");
		PRECACHE_SOUND("weapons/dualsword_fly5.wav");
		
		PRECACHE_MODEL("models/p_dualsword_a.mdl");
		PRECACHE_MODEL("models/p_dualsword_b.mdl");
		
		m_iModelLeaf1 = PRECACHE_MODEL("sprites/leaf01_dualsword.spr");
		m_iModelLeaf2 = PRECACHE_MODEL("sprites/leaf02_dualsword.spr");
		m_iModelPetal1 = PRECACHE_MODEL("sprites/petal01_dualsword.spr");
		m_iModelPetal2 = PRECACHE_MODEL("sprites/petal02_dualsword.spr");
		m_iModelSkill = PRECACHE_MODEL("models/dualsword_skill.mdl");
		
	}

	int m_iModelLeaf1;
	int m_iModelLeaf2;
	int m_iModelPetal1;
	int m_iModelPetal2;
	int m_iModelSkill;

	BOOL Deploy() override
	{
		m_iuser4 = 1;
		m_iuser1 = 0;
		m_sequence = 0;
		m_fuser1 = invalid_time_point;
		m_waterlevel = 0;
		m_iuser3 %= 2;
		DestroyEffect();

		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;

		BOOL result;
		if(m_iuser1)
			result = DefaultDeploy(V_Model, P_Model, 11, AnimExtension, UseDecrement() != FALSE);
		else
			result = DefaultDeploy(V_Model, P_Model_B, 6, AnimExtension, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = 0.2s;

		return result;
	}

	void Holster(int skiplocal) override
	{
		m_iuser3 %= 2;
		DestroyEffect();
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0s;
		return Base::Holster(skiplocal);
	}

	void SpawnEffect();
	void DestroyEffect();

	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

	
#ifndef CLIENT_DLL
	void ItemPostFrame() override;
	void ActPrimaryAttack(int iType);
	void DelaySecondaryAttack();
#endif
	void DelayPrimaryAttack();

	float GetPrimaryAttackDamage(int iType) const
	{
		float flDamage = iType ? 50 : 770;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 4.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage(int iType) const
	{
		float flDamage = 50;
		switch (iType)
		{
		case 0: flDamage = 50.0; break;
		case 1: flDamage = 70.0; break;
		case 2: flDamage = 90.0; break;
		case 3: flDamage = 405.0; break;
		}

#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 4.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_dualsword, CKnifeDualsword)

void CKnifeDualsword::WeaponIdle()
{
	if (m_flTimeWeaponIdle > 0.0s)
		return;

	ResetEmptySound();

	m_iuser3 %= 2;

	m_iuser4 = 0;
	m_iuser1 = 0;

	if (m_iuser1 % 2)
	{
		SendWeaponAnim(7);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
	}
	else
	{
		SendWeaponAnim(0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0s;
	}
}

#ifndef CLIENT_DLL
void CKnifeDualsword::ItemPostFrame()
{
	//new iBteWpn = WeaponIndex(iEnt);
	//new id = get_pdata_cbase(iEnt, m_pPlayer, 4);

	if (gpGlobals->time < pev->nextthink)
		return Base::ItemPostFrame();

	auto iAttackRecord = m_iuser1;  // 648
	switch (m_sequence)
	{
	case 1:
	{
		DelaySecondaryAttack();
		break;
	}
	case 2:
	{
		// TEMPENTITY HERE, AFTER SECONDARYATTACK
		if (m_iuser3 == 11)
		{
			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, id, m_usFire[iBteWpn][0], 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 2, EV_DUALSWORD_STAB_END, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_slash_4_2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_sequence = 7;
			pev->nextthink = gpGlobals->time + (0.65s - 0.53s);
		}
		else
		{
			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, id, m_usFire[iBteWpn][0], 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 1, EV_DUALSWORD_STAB_END, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_slash_4_1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_sequence = 3;
			pev->nextthink = gpGlobals->time + 0.77s;
		}
		break;
	}
	case 3:
	{
		SendWeaponAnim(5);
		m_weaponanim = 5;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

		// ADDED
		m_sequence = 0;
		break;
	}
	case 4:
	{
		DelayPrimaryAttack();
		break;
	}
	case 5:
	{
		ActPrimaryAttack(m_iuser4);

		int int_Skill = -1;
		switch (m_iuser3)
		{
		case 0:
		{
			if (m_iuser4)
				int_Skill = 1;
			break;
		}
		case 5:
		{
			if (m_iuser4)
				int_Skill = 6;
			break;
		}
		case 6:
		{
			if (!m_iuser4)
				int_Skill = 7;
			break;
		}
		}

		m_iuser3 = (int_Skill == -1) ? 0 : int_Skill;
		m_iuser4 = 1 - m_iuser4;
		break;
	}
	case 6:
	{
		SendWeaponAnim(10);
		m_weaponanim = 10;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		// ADDED
		m_sequence = 0;
		break;
	}
	case 7:
	{
		SendWeaponAnim(14);
		m_weaponanim = 14;

		m_sequence = 8;
		pev->nextthink = gpGlobals->time + 1.0s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s; // ADDED
		break;
	}
	case 8:
	{
		SendWeaponAnim(15);
		m_weaponanim = 15;

		m_sequence = 9;
		pev->nextthink = gpGlobals->time + 12.48s; // ???
		break;
	}
	case 9:
	{
		SendWeaponAnim(6);
		m_weaponanim = 6;

		m_iuser3 = 1;

		// NOT SURE
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;

		// ADDED
		m_sequence = 0;
		break;
	}
	}

	return Base::ItemPostFrame();
}
#endif

void CKnifeDualsword::PrimaryAttack(void)
{
	if (m_iuser3 == 1)
	{
		SendWeaponAnim(12);
		m_weaponanim = 12;

		m_iuser3 = 0;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
		
		m_sequence = 4;
		pev->nextthink = gpGlobals->time + 0.0s;
	}
	else if (m_iuser3 != 3 || m_sequence != 1)
	{
		DelayPrimaryAttack();
	}
	else
	{
		m_sequence = 4;
		//pev->nextthink = gpGlobals->time + 0.0s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
	}
}

void CKnifeDualsword::SecondaryAttack(void)
{
	m_waterlevel = 0;
	m_iuser4 = 1;
	if (!m_iuser3)
	{
		SendWeaponAnim(13);
		m_weaponanim = 13;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;

		m_sequence = 1;
		pev->nextthink = gpGlobals->time + 0.0s;
		return;
	}


	auto int_check = -1;
	switch (m_iuser3)
	{
	case 1:int_check = 1; break;
	case 7:int_check = 1; break;
	}
	if (int_check == -1)
		m_iuser3 = 0;
#ifndef CLIENT_DLL
	DelaySecondaryAttack();
#endif
}

void CKnifeDualsword::DelayPrimaryAttack()
{
	if (m_iuser4 == 0)
	{
		SendWeaponAnim(9);
		m_weaponanim = 9;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab2.wav", VOL_NORM, ATTN_NORM, 0, 94);
	}
	else
	{
		SendWeaponAnim(8);
		m_weaponanim = 8;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab1.wav", VOL_NORM, ATTN_NORM, 0, 94);
	}

	m_sequence = 5;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
	pev->nextthink = gpGlobals->time + 0.25s;
}

#ifndef CLIENT_DLL

void CKnifeDualsword::ActPrimaryAttack(int iType)
{
	const auto flDamage = GetPrimaryAttackDamage(iType);
	const float flRadius = iType ? 130 : 140;
	const float flAngle = iType ? 90 : 180;

	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	const Vector vecSrc = m_pPlayer->GetGunPosition();
	const Vector vecDir = gpGlobals->v_forward;
	/*{
		TraceResult tr;
		Vector vecEnd = vecSrc + vecDir.Normalize() * flAngle;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			const Vector vecOrigin = tr.vecEndPos + Vector(0, 0, 10);
			const Vector vecVelocity = tr.vecPlaneNormal * 1;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_SPRITE_SPRAY);
			WRITE_COORD(vecOrigin[0]);
			WRITE_COORD(vecOrigin[1]);
			WRITE_COORD(vecOrigin[2]);
			WRITE_COORD(vecVelocity[0]);
			WRITE_COORD(vecVelocity[1]);
			WRITE_COORD(vecVelocity[2]);
			WRITE_SHORT(iType ? m_iModelLeaf1 : m_iModelLeaf2);
			WRITE_BYTE(10); // count
			WRITE_BYTE(80); // speed
			WRITE_BYTE(150); // noise
			MESSAGE_END();
		}
	}*/

	switch (KnifeAttack4(vecSrc, vecDir, flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev,TRUE))
	{
	case HIT_NONE:
	{
		
		break;
	}
	case HIT_PLAYER:
	{
		if (iType)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab2_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		break;
	}
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + (iType ? 0.15s : 0.65s);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + (m_iuser3 ? 0.15s : 0.65s);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
	m_fuser1 = gpGlobals->time + (iType ? 0.15s : 0.65s);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	pev->nextthink = gpGlobals->time + (iType ? 0.42s : 0.65s);
	m_sequence = 6;
}

void CKnifeDualsword::DelaySecondaryAttack()
{
	int v8 = m_waterlevel % 4;
	SendWeaponAnim(v8 + 1);
	m_weaponanim = v8 + 1;

	if (m_iuser3)
	{
		if (v8 == m_iuser3 - 1)
			m_iuser3 = (v8 + 2);
		else if (v8 == m_iuser3 - 7)
			m_iuser3 = (v8 + 8);
		else
			m_iuser3 = (0);
	}

	m_waterlevel++;

	const auto flDamage = GetSecondaryAttackDamage(v8);
	const float flRadius = 120;
	const float flAngle = 330;

	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	const Vector vecSrc = m_pPlayer->GetGunPosition();
	const Vector vecDir = gpGlobals->v_forward;
	/*{
		TraceResult tr;
		Vector vecEnd = vecSrc + vecDir.Normalize() * flAngle;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			const Vector vecOrigin = tr.vecEndPos + Vector(0, 0, 10);
			const Vector vecVelocity = tr.vecPlaneNormal * 1;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_SPRITE_SPRAY);
			WRITE_COORD(vecOrigin[0]);
			WRITE_COORD(vecOrigin[1]);
			WRITE_COORD(vecOrigin[2]);
			WRITE_COORD(vecVelocity[0]);
			WRITE_COORD(vecVelocity[1]);
			WRITE_COORD(vecVelocity[2]);
			WRITE_SHORT((v8 % 2) ? m_iModelPetal1 : m_iModelPetal2);
			WRITE_BYTE(10); // count
			WRITE_BYTE(80); // speed
			WRITE_BYTE(150); // noise
			MESSAGE_END();
		}
	}*/

	char buffer[64];
	sprintf(buffer, "weapons/dualsword_slash_%d.wav", v8 + 1);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, buffer, VOL_NORM, ATTN_NORM, 0, 94);

	switch (KnifeAttack4(vecSrc, gpGlobals->v_forward, flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev, FALSE))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		if (v8 + 1 < 4)
		{
			sprintf(buffer, "weapons/dualsword_hit%d.wav", v8 + 1);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, buffer, VOL_NORM, ATTN_NORM, 0, 94);
		}
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		if (v8 + 1 < 4)
		{
			sprintf(buffer, "weapons/dualsword_hit%d.wav", v8 + 1);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, buffer, VOL_NORM, ATTN_NORM, 0, 94);
		}
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
		fDidHit = TRUE;
		break;
	}
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	if (m_waterlevel < 4)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;

		pev->nextthink = gpGlobals->time + 0.15s;
		m_sequence = 1;
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.65s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;

		m_fuser1 = gpGlobals->time + 0.65s;
		pev->nextthink = gpGlobals->time + 0.53s;
		m_sequence = 2;

		// (pev_iuser3, this+548) : iSkillRecord
		if (m_iuser3 == 4)
		{
			m_iuser1 = 0;
			return;
		}

		if (m_iuser3 == 11)
		{
			//set_pev(iEnt, pev_iuser1, 7);
			SpawnEffect();
			return;
		}

		if (m_iuser3 == 4 || m_iuser1 != 2)
		{
			m_iuser1 = 0;
		}
		else
		{
			m_iuser1 = 3;
		}
	}
}

class CDualSwordSpecialEffect : public CBaseEntity
{
private:
	int m_iEffectType; // this+93, iuser1
	time_point_t m_flNextEffect; // this+97,fuser1
	time_point_t m_flLastAttack1;
	time_point_t m_flLastAttack2;
	time_point_t m_flLastAttack3;

	int m_iAnim;
	
	int m_iModelSkillFx1;
	int m_iModelSkillFx2;

public:
	void Precache() override
	{
		m_iModelSkillFx1 = PRECACHE_MODEL("models/dualsword_skillfx1.mdl");
		m_iModelSkillFx2 = PRECACHE_MODEL("models/dualsword_skillfx2.mdl");
	}

	void Spawn() override
	{
		Precache();

		pev->classname = MAKE_STRING("d_dualsword");
		m_iEffectType = 0;
		m_flNextEffect = gpGlobals->time + 0.65s;
		pev->nextthink = gpGlobals->time + 0.017s;
		SetThink(&CDualSwordSpecialEffect::EffectThink);
	}

	void EXPORT EffectThink()
	{
		pev->nextthink = gpGlobals->time + 0.017s;

		CBasePlayer *pPlayer = static_ent_cast<CBasePlayer *>(pev->owner);
		if (!pPlayer->IsAlive() || pPlayer->m_bIsZombie)
		{
			SUB_Remove();
		}

		switch (m_iEffectType)
		{
		case 0:
		{
			if (gpGlobals->time > m_flNextEffect)
			{
				// TEMPENTITY HERE (90 15 10)
				// "weapons/dualsword_skill_start.wav"
				//PLAYBACK_EVENT_FULL(FEV_GLOBAL, id, m_usFire[iBteWpn][0], 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, EV_DUALSWORD_SKILL_START, FALSE, FALSE);
				EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_skill_start.wav", VOL_NORM, ATTN_NORM, 0, 94);
				m_iEffectType = 1; // this+93(iEffectType)
				m_flNextEffect = gpGlobals->time + 1.49s;
			}
			break;
		}
		case 1:
		{
			if (gpGlobals->time > m_flNextEffect)
			{
				m_iEffectType = 2; // this+93(iEffectType)
				m_flNextEffect = gpGlobals->time + 10.0s;
			}
			break;
		}
		case 2:
		{
			Attack();
			break;
		}
		case 3:
		{
			if (gpGlobals->time > m_flNextEffect)
			{
				// TEMPENTITY HERE (90 -20 15 10)
				// "weapons/dualsword_skill_end.wav"
				//PLAYBACK_EVENT_FULL(FEV_GLOBAL, id, m_usFire[iBteWpn][0], 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, EV_DUALSWORD_SKILL_END, FALSE, FALSE);
				EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_skill_loop_end.wav", VOL_NORM, ATTN_NORM, 0, 94);
				m_iEffectType = 4; // this+93(iEffectType)
				m_flNextEffect = gpGlobals->time + 1.4s; // this+97
			}
			break;
		}
		case 4:
		{
			if (gpGlobals->time > m_flNextEffect)
			{
				EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_skill_end.wav", VOL_NORM, ATTN_NORM, 0, 94);
				SetThink(nullptr);
				SUB_Remove();
			}
			break;
		}
		}
	}

	void Attack()
	{
		if (gpGlobals->time < m_flNextEffect)
		{
			if (gpGlobals->time >= m_flLastAttack1 + 0.15s)
			{
				Attack1();
				m_flLastAttack1 = gpGlobals->time;
			}
			if (gpGlobals->time >= m_flLastAttack2 + 0.05s)
			{
				Attack2();
				m_flLastAttack2 = gpGlobals->time;
			}
			if (gpGlobals->time >= m_flLastAttack3 + 0.15s)
			{
				Attack3();
				m_flLastAttack3 = gpGlobals->time;
			}
		}
		else
		{
			m_flNextEffect = gpGlobals->time + 0.59s; // this+97
			m_iEffectType = 3; // this+93(iEffectType)
		}
	}

	void Attack1() 
	{
		const float flRadius = 200;

		static std::random_device rd;
		auto random_float = [](float a, float b) { return std::uniform_real_distribution<float>(a, b)(rd); };
		auto random_num = [](int a, int b) { return std::uniform_int_distribution<int>(a, b)(rd); };

		float fFloat = flRadius * random_float(0.1f, 0.85f);
		float fFloat2 = flRadius * random_float(0.1f, 0.85f);

		const Vector vecOrigin = pev->owner->v.origin;
		Vector vecOrigin3[2] = { vecOrigin, vecOrigin };

		vecOrigin3[0][2] += random_float(-5.0, 65.0);
		vecOrigin3[1][2] += random_float(-5.0, 65.0);

		m_iAnim = (m_iAnim + 1) % 4;
		switch (m_iAnim)
		{
		case 0:
		{
			vecOrigin3[0][0] += fFloat * random_float(0.5, 1.0);
			vecOrigin3[0][1] += fFloat2 * random_float(0.5, 1.0);

			vecOrigin3[1][0] -= fFloat * random_float(0.5, 1.0);
			vecOrigin3[1][1] -= fFloat2 * random_float(0.5, 1.0);
		}
		case 1:
		{
			vecOrigin3[0][0] += fFloat * random_float(0.5, 1.0);
			vecOrigin3[0][1] -= fFloat2 * random_float(0.5, 1.0);

			vecOrigin3[1][0] -= fFloat * random_float(0.5, 1.0);
			vecOrigin3[1][1] += fFloat2 * random_float(0.5, 1.0);
		}
		case 2:
		{
			vecOrigin3[0][0] -= fFloat * random_float(0.5, 1.0);
			vecOrigin3[0][1] += fFloat2 * random_float(0.5, 1.0);

			vecOrigin3[1][0] += fFloat * random_float(0.5, 1.0);
			vecOrigin3[1][1] -= fFloat2 * random_float(0.5, 1.0);
		}
		case 3:
		{
			vecOrigin3[0][0] -= fFloat * random_float(0.5, 1.0);
			vecOrigin3[0][1] -= fFloat2 * random_float(0.5, 1.0);

			vecOrigin3[1][0] += fFloat * random_float(0.5, 1.0);
			vecOrigin3[1][1] += fFloat2 * random_float(0.5, 1.0);
		}
		}

		{
			const Vector vecNewOrigin = vecOrigin3[0];
			const Vector vecNewVelocity = (vecOrigin3[1] - vecOrigin3[0]).Normalize();

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
			WRITE_BYTE(TE_PROJECTILE);
			WRITE_COORD(vecNewOrigin[0]);
			WRITE_COORD(vecNewOrigin[1]);
			WRITE_COORD(vecNewOrigin[2]);
			WRITE_COORD(vecNewVelocity[0]);
			WRITE_COORD(vecNewVelocity[1]);
			WRITE_COORD(vecNewVelocity[2]);
			WRITE_SHORT(m_iModelSkillFx1);
			WRITE_BYTE(1); // (life in 1's)
			WRITE_BYTE(ENTINDEX(pev->owner)); // (owner)
			MESSAGE_END();
		}

		{
			const Vector vecNewOrigin = vecOrigin3[0];
			const Vector vecNewVelocity = (vecOrigin3[1] - vecOrigin3[0]).Normalize();

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
			WRITE_BYTE(TE_PROJECTILE);
			WRITE_COORD(vecNewOrigin[0]);
			WRITE_COORD(vecNewOrigin[1]);
			WRITE_COORD(vecNewOrigin[2]);
			WRITE_COORD(vecNewVelocity[0] * 1000);
			WRITE_COORD(vecNewVelocity[1] * 1000);
			WRITE_COORD(vecNewVelocity[2] * 1000);
			WRITE_SHORT(m_iModelSkillFx2);
			WRITE_BYTE(1); // (life in 1's)
			WRITE_BYTE(ENTINDEX(pev->owner)); // (owner)
			MESSAGE_END();
		}
	}

	void Attack2()
	{
		std::random_device rd;
		std::size_t N = std::uniform_int_distribution<std::size_t>(0, 4)(rd);
		static constexpr const char *sound[] = {
			"weapons/dualsword_fly1.wav",
			"weapons/dualsword_fly2.wav",
			"weapons/dualsword_fly3.wav",
			"weapons/dualsword_fly4.wav",
			"weapons/dualsword_fly5.wav",
		};
		EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, sound[N], VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

	void Attack3()
	{
		CBasePlayer *pPlayer = static_ent_cast<CBasePlayer *>(pev->owner);

		float flDamage = 120;

#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 380;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 380;
#endif
		const float flRadius = 200;
		const float flAngle = 360;

		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(pPlayer->pev->v_angle);
		Vector vecSrc = pPlayer->GetGunPosition();

		KnifeAttack3(vecSrc, gpGlobals->v_forward, flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, pPlayer->pev, pPlayer->pev);
	}

};

void CKnifeDualsword::SpawnEffect()
{
	auto pEffect = CreateClassPtr<CDualSwordSpecialEffect>();
	pEffect->pev->owner = m_pPlayer->edict();

	if (pEffect)
	{
		pEffect->Spawn();
	}

	//client_print(id, print_chat, "DualSword_SpawnEffect");

	m_iuser1 = 0;
	m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
}

void CKnifeDualsword::DestroyEffect()
{
	for (CBaseEntity *pEntity : moe::range::EntityList<moe::Enumer_ClassName<CBaseEntity>>("d_dualsword"))
	{
		if (pEntity->pev->owner == m_pPlayer->edict())
		{
			pEntity->SUB_Remove();
		}
	}
}

#else
void CKnifeDualsword::SpawnEffect()
{
	m_iuser1 = 0;
	m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
}
void CKnifeDualsword::DestroyEffect() {}
#endif

}
