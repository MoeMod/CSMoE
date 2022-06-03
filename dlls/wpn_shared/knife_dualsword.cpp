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
#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"


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
	static constexpr float ArmorRatioModifier = 1.7f;
	static constexpr const char *AnimExtension = "katana";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

private:
	int m_iAttackRecord; // iAttackRecord
	int m_iSkillRecord; // iSkillRecord
	int m_iPrimaryAttackRecord; // iPrimaryAttackRecord
	int m_iThinkType; // iThinkType
	time_point_t m_flNextTimeCanHolster; // flNextTimeCanHolster
	int m_iLastAnim; // iLastAnim
	int m_iSecondaryAttackCount; // iSecondaryAttackCount



public:
	BOOL CanHolster() override
	{
		return gpGlobals->time > m_flNextTimeCanHolster;
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
		PRECACHE_SOUND("weapons/dualsword_slash_1.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_2.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_3.wav");
		PRECACHE_SOUND("weapons/dualsword_slash_4.wav");
		PRECACHE_SOUND("weapons/dualsword_hit1.wav");
		PRECACHE_SOUND("weapons/dualsword_hit2.wav");
		PRECACHE_SOUND("weapons/dualsword_hit3.wav");
		PRECACHE_SOUND("weapons/dualsword_skill_start.wav");
		PRECACHE_SOUND("weapons/dualsword_skill_end.wav");
		PRECACHE_SOUND("weapons/dualsword_fly1.wav");
		PRECACHE_SOUND("weapons/dualsword_fly2.wav");
		PRECACHE_SOUND("weapons/dualsword_fly3.wav");
		PRECACHE_SOUND("weapons/dualsword_fly4.wav");
		PRECACHE_SOUND("weapons/dualsword_fly5.wav");
		
		PRECACHE_MODEL("models/p_dualsword_a.mdl");
		PRECACHE_MODEL("models/p_dualsword_b.mdl");
		
		//m_iModelLeaf1 = PRECACHE_MODEL("sprites/leaf01_dualsword.spr");
		//m_iModelLeaf2 = PRECACHE_MODEL("sprites/leaf02_dualsword.spr");
		//m_iModelPetal1 = PRECACHE_MODEL("sprites/petal01_dualsword.spr");
		//m_iModelPetal2 = PRECACHE_MODEL("sprites/petal02_dualsword.spr");
		//m_iModelSkill = PRECACHE_MODEL("models/dualsword_skill.mdl");
		//PRECACHE_MODEL("models/dualswordfx.mdl");
	}

	int m_iModelLeaf1;
	int m_iModelLeaf2;
	int m_iModelPetal1;
	int m_iModelPetal2;
	int m_iModelSkill;

	BOOL Deploy() override
	{
		m_iPrimaryAttackRecord = 1;
		m_iThinkType = 0;
		m_flNextTimeCanHolster = invalid_time_point;
		m_iSecondaryAttackCount = 0;
		m_iSkillRecord %= 2;
		DestroyEffect();

		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;

		BOOL result;

		if (m_iLastAnim <= 5 || m_iLastAnim >= 14 || m_iLastAnim == 6)
		{
			result = DefaultDeploy(V_Model, P_Model_B, 6, AnimExtension, UseDecrement() != FALSE, 0.01s, 0.25s);
			
		}
		else if (m_iLastAnim >= 7 || m_iLastAnim <= 11)
		{
			result = DefaultDeploy(V_Model, P_Model, 11, AnimExtension, UseDecrement() != FALSE, 0.01s, 0.25s);
		}

#if 0
		if(m_iAttackRecord)
			result = DefaultDeploy(V_Model, P_Model, 11, AnimExtension, UseDecrement() != FALSE, 0.1s, 0.25s);
		else
			result = DefaultDeploy(V_Model, P_Model_B, 6, AnimExtension, UseDecrement() != FALSE, 0.1s, 0.25s);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = 0.01s;

		return result;
	}

	void Holster(int skiplocal) override
	{
		m_iSkillRecord %= 2;
		DestroyEffect();
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0s;
		return Base::Holster(skiplocal);
	}

	void SpawnEffect();
	void DestroyEffect();

	void Spawn() override;
	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack, int iType);
	bool AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, bool IsPrimaryAttack);
	void ItemPostFrame() override;
	void ActPrimaryAttack(int iType);
	void DelaySecondaryAttack();
#endif
	void DelayPrimaryAttack();

	float GetPrimaryAttackDamage(int iType) const
	{
		/*
		float flDamage = iType ? 50 : 770;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 4.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;*/
		float flDamage = iType ? 50 : 770;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 3.3f;
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
			flDamage *= 2.5f;	//4.5
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(knife_dualsword, CKnifeDualsword)

void CKnifeDualsword::Spawn()
{
	pev->classname = MAKE_STRING("knife_dualsword");
	m_iId = WEAPON_KNIFE;
	m_iAttackRecord = 0;
	m_iLastAnim = 7;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeDualsword::WeaponIdle()
{
	if (m_flTimeWeaponIdle > 0.0s)
		return;

	ResetEmptySound();

	m_iSkillRecord %= 2;

	m_iPrimaryAttackRecord = 1;
	m_iAttackRecord = 0;

	if (m_iLastAnim <= 5 || m_iLastAnim >= 14 || m_iLastAnim == 6)
	{
		SendWeaponAnim(0);
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 11, 0, TRUE, FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
	}
	else if (m_iLastAnim >= 7 || m_iLastAnim <= 11)
	{
		SendWeaponAnim(7);
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 11, 0, FALSE, FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0s;
	}

#if 0
	if (m_iAttackRecord % 2)
	{
		SendWeaponAnim(7);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
	}
	else
	{
		SendWeaponAnim(0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0s;
	}
#endif
}

#ifndef CLIENT_DLL
void CKnifeDualsword::ItemPostFrame()
{
	//new iBteWpn = WeaponIndex(iEnt);
	//new id = get_pdata_cbase(iEnt, m_pPlayer, 4);

	if (gpGlobals->time < pev->nextthink)
		return Base::ItemPostFrame();

	auto iAttackRecord = m_iAttackRecord;  // 648
	switch (m_iThinkType)
	{
	case 1:
	{
		DelaySecondaryAttack();
		break;
	}
	case 2:
	{
		// TEMPENTITY HERE, AFTER SECONDARYATTACK
		if (m_iSkillRecord == 11)
		{
			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, id, m_usFire[iBteWpn][0], 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 2, EV_DUALSWORD_STAB_END, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_slash_4_2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_iThinkType = 7;
			pev->nextthink = gpGlobals->time + (0.65s - 0.53s);
		}
		else
		{
			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, id, m_usFire[iBteWpn][0], 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 1, EV_DUALSWORD_STAB_END, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_slash_4_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_iThinkType = 3;
			pev->nextthink = gpGlobals->time + 0.77s;
		}
		break;
	}
	case 3:
	{
		SendWeaponAnim(5);
		m_iLastAnim = 5;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

		// ADDED
		m_iThinkType = 0;
		break;
	}
	case 4:
	{
		DelayPrimaryAttack();
		break;
	}
	case 5:
	{
		ActPrimaryAttack(m_iPrimaryAttackRecord);

		int int_Skill = -1;
		switch (m_iSkillRecord)
		{
		case 0:
		{
			if (m_iPrimaryAttackRecord)
				int_Skill = 1;
			break;
		}
		case 5:
		{
			if (m_iPrimaryAttackRecord)
				int_Skill = 6;
			break;
		}
		case 6:
		{
			if (!m_iPrimaryAttackRecord)
				int_Skill = 7;
			break;
		}
		}

		m_iSkillRecord = (int_Skill == -1) ? 0 : int_Skill;
		m_iPrimaryAttackRecord = 1 - m_iPrimaryAttackRecord;
		break;
	}
	case 6:
	{
		SendWeaponAnim(10);
		m_iLastAnim = 10;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		// ADDED
		m_iThinkType = 0;
		break;
	}
	case 7:
	{
		SendWeaponAnim(14);
		m_iLastAnim = 14;

		m_iThinkType = 8;
		pev->nextthink = gpGlobals->time + 1.0s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s; // ADDED
		break;
	}
	case 8:
	{
		SendWeaponAnim(15);
		m_iLastAnim = 15;

		m_iThinkType = 9;
		pev->nextthink = gpGlobals->time + 12.48s; // ???
		break;
	}
	case 9:
	{
		SendWeaponAnim(6);
		m_iLastAnim = 6;

		m_iSkillRecord = 1;

		// NOT SURE
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;

		// ADDED
		m_iThinkType = 0;
		break;
	}
	}

	return Base::ItemPostFrame();
}
#endif

void CKnifeDualsword::PrimaryAttack(void)
{
	if (m_iSkillRecord == 1)
	{
		SendWeaponAnim(12);	//switch to blue
		m_iLastAnim = 12;

		m_iSkillRecord = 0;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
		
		m_iThinkType = 4;
		pev->nextthink = gpGlobals->time + 0.0s;
	}
	else if (m_iSkillRecord != 3 || m_iThinkType != 1)
	{
		DelayPrimaryAttack();		//m_iThinkType = 5;
	}
	else
	{
		m_iThinkType = 4;
		//pev->nextthink = gpGlobals->time + 0.0s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
	}
}

void CKnifeDualsword::SecondaryAttack(void)
{
	m_iSecondaryAttackCount = 0;
	m_iPrimaryAttackRecord = 1;
	if (!m_iSkillRecord)
	{
		SendWeaponAnim(13);
		m_iLastAnim = 13;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;

		m_iThinkType = 1;
		pev->nextthink = gpGlobals->time + 0.0s;
		return;
	}


	auto int_check = -1;
	switch (m_iSkillRecord)
	{
	case 1:int_check = 1; break;
	case 7:int_check = 1; break;
	}
	if (int_check == -1)
		m_iSkillRecord = 0;
#ifndef CLIENT_DLL
	DelaySecondaryAttack();
#endif
}

#ifndef CLIENT_DLL
bool CKnifeDualsword::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, bool IsPrimaryAttack)
{
	bool bResult = false;
	if (pEntity->IsBSPModel())
	{
		UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, ignore_monsters, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0)
		{
			if (ptr->pHit)
			{
				CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

				if (pHit && pHit == pEntity)
					bResult = true;
			}
		}
	}
	else
	{
		UTIL_TraceHull(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, head_hull, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0 && ptr->pHit != NULL && CBaseEntity::Instance(ptr->pHit) == pEntity)
		{
			UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, pAttacker->edict(), ptr);

			bResult = true;
		}
		else
		{
			Vector vecEnd = pEntity->BodyTarget(vecSrc);

			if (DotProduct(vecAngleCheckDir, (vecEnd.Make2D() - vecSrc.Make2D()).Normalize()) > flMinimumCosine)
			{
				edict_t* peAttacker = pAttacker->edict();

				for (int i = 5; i > 0; i--)
				{
					UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, peAttacker, ptr);

					if (!ptr->pHit || !GET_PRIVATE(ptr->pHit))
					{
						break;
					}

					CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

					if (ptr->flFraction < 1.0 || pHit == pEntity)
					{
						bResult = true;

						break;
					}

					if (pHit->IsBSPModel())
					{
						break;
					}

					vecSrc = ptr->vecEndPos + vecDir * 5.0;
				}
			}
		}
	}

	if (!bResult)
		return bResult;

	if (pEntity->IsPlayer())
	{
		if(IsPrimaryAttack)
		{
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, g_vecZero, 0.0, 0.0, 1, 0, TRUE, TRUE);
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, g_vecZero, 0.0, 0.0, 1, 0, TRUE, FALSE);
		}
		else
		{
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, g_vecZero, 0.0, 0.0, 1, 0, FALSE, TRUE);
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, g_vecZero, 0.0, 0.0, 1, 0, FALSE, FALSE);
		}
	}
	
	return bResult;
}

hit_result_t CKnifeDualsword::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack, int iType)
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

		if (!AngularCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr, IsPrimaryAttack))
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

		if (pEntity->IsPlayer())
		{
			CBasePlayer* player = (CBasePlayer*)pEntity;

			if (player->m_bIsZombie)
			{	
				if (IsPrimaryAttack)
				{	
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);

					if(iType)
						ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.8f });
					else
						ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.5f });

					if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.0s);
							strategy->SetStunGravityTime(1.0s);
						}
					}
				}
			}
		}
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

void CKnifeDualsword::DelayPrimaryAttack()
{
	if (m_iPrimaryAttackRecord == 0)
	{
		SendWeaponAnim(9);
		m_iLastAnim = 9;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
	else
	{
		SendWeaponAnim(8);
		m_iLastAnim = 8;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

	m_iThinkType = 5;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
	pev->nextthink = gpGlobals->time + 0.25s;
}

#ifndef CLIENT_DLL
void CKnifeDualsword::ActPrimaryAttack(int iType)
{
	const auto flDamage = GetPrimaryAttackDamage(iType);
	const float flRadius = iType ? 130 : 140;
	const float flAngle = iType ? 90 : 180;

	
	/*{
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

	switch (KnifeAttack(flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true, iType))
	{
	case HIT_NONE:
	{
		
		break;
	}
	case HIT_PLAYER:
	{
		if (iType)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_stab2_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		TraceResult tr;

		const Vector vecSrc = m_pPlayer->GetGunPosition();
		const Vector vecDir = gpGlobals->v_forward;

		Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, g_vecZero, 0.0, 0.0, 1, 0, TRUE, TRUE);
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, g_vecZero, 0.0, 0.0, 1, 0, TRUE, FALSE);

		}

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + (iType ? 0.15s : 0.65s);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + (m_iSkillRecord ? 0.15s : 0.65s);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
	m_flNextTimeCanHolster = gpGlobals->time + (iType ? 0.15s : 0.65s);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	pev->nextthink = gpGlobals->time + (iType ? 0.42s : 0.65s);
	m_iThinkType = 6;
}

void CKnifeDualsword::DelaySecondaryAttack()
{
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	int v8 = m_iSecondaryAttackCount % 4;
	SendWeaponAnim(v8 + 1);
	PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 5, v8, FALSE, FALSE);
	m_iLastAnim = v8 + 1;

	if (m_iSkillRecord)
	{
		if (v8 == m_iSkillRecord - 1)
			m_iSkillRecord = (v8 + 2);
		else if (v8 == m_iSkillRecord - 7)
			m_iSkillRecord = (v8 + 8);
		else
			m_iSkillRecord = (0);
	}

	m_iSecondaryAttackCount++;

	const auto flDamage = GetSecondaryAttackDamage(v8);
	const float flRadius = 120;
	const float flAngle = 330;

	
	/*{
		
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
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, buffer, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	switch (KnifeAttack(flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false, 0))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		int random_sound;
		/*random_sound = v8 + 1;
		if (random_sound < 4)
		{
			sprintf(buffer, "weapons/dualsword_hit%d.wav", random_sound);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, buffer, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		else
		{*/
		random_sound = RANDOM_FLOAT(1, 3);
		sprintf(buffer, "weapons/dualsword_hit%d.wav", random_sound);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, buffer, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		TraceResult tr;

		const Vector vecSrc = m_pPlayer->GetGunPosition();
		const Vector vecDir = gpGlobals->v_forward;

		Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, g_vecZero, 0.0, 0.0, 1, 0, FALSE, TRUE);
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, g_vecZero, 0.0, 0.0, 1, 0, FALSE, FALSE);

		}
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
		break;
	}
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	if (m_iSecondaryAttackCount < 4)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.0s;
		m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;

		pev->nextthink = gpGlobals->time + 0.15s;
		m_iThinkType = 1;
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.65s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;

		m_flNextTimeCanHolster = gpGlobals->time + 0.65s;
		pev->nextthink = gpGlobals->time + 0.53s;
		m_iThinkType = 2;

		// (pev_iuser3, this+548) : iSkillRecord
		if (m_iSkillRecord == 4)
		{
			m_iAttackRecord = 0;
			return;
		}

		if (m_iSkillRecord == 11)
		{
			//set_pev(iEnt, pev_iuser1, 7);
			SpawnEffect();
			return;
		}

		if (m_iSkillRecord == 4 || m_iAttackRecord != 2)
		{
			m_iAttackRecord = 0;
		}
		else
		{
			m_iAttackRecord = 3;
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
	
	//int m_iModelSkillFx1;
	//int m_iModelSkillFx2;

public:
	float GetArmorRatioModifier() { return ::sv::GetArmorRatioModifier(WEAPON_KNIFE); }
	int GetWeaponsId(void) { return WEAPON_KNIFE; }
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 0.9f };
	void Precache() override
	{
		//m_iModelSkillFx1 = PRECACHE_MODEL("models/dualsword_skillfx1.mdl");
		//m_iModelSkillFx2 = PRECACHE_MODEL("models/dualsword_skillfx2.mdl");
	}

	void Spawn() override
	{
		Precache();

		pev->classname = MAKE_STRING("dualsword_specialeffect");
		m_iEffectType = 0;
		m_flNextEffect = gpGlobals->time + 0.65s;
		pev->nextthink = gpGlobals->time + 0.017s;
		SetThink(&CDualSwordSpecialEffect::EffectThink);
	}

#ifndef CLIENT_DLL
	void RadiusAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
	{
		CBaseEntity* pEntity = NULL;
		BOOL fHit = FALSE;

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

			pAttackPlayer->m_iWeaponVolume = 128;

			ClearMultiDamage();

			tr.iHitgroup = HITGROUP_CHEST;
			pEntity->TraceAttack(pAttackPlayer->pev, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

			fHit = TRUE;

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie && player->m_ZombieClass)
				{
					CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(pEntity);
					if (player && player->m_bIsZombie)
					{
						Vector vecDir = (player->pev->origin - pAttackPlayer->pev->origin);
						vecDir.z = 0;

						ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.9f });
					}
				}
			}
		}

		if (!fHit)
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
		}
	}
#endif

	void EXPORT EffectThink()
	{
		pev->nextthink = gpGlobals->time + 0.017s;

		CBasePlayer *pPlayer = static_ent_cast<CBasePlayer *>(pev->owner);
		if (!pPlayer->IsAlive() || pPlayer->m_bIsZombie)
		{
			SUB_Remove();
		}

		if(pPlayer->m_pActiveItem != NULL && pPlayer->m_pActiveItem->m_iId != WEAPON_KNIFE && g_pModRunning->DamageTrack() != DT_ZBS)
			SUB_Remove();

		switch (m_iEffectType)
		{
		case 0:
		{
			if (gpGlobals->time > m_flNextEffect)
			{
				// TEMPENTITY HERE (90 15 10)
				// "weapons/dualsword_skill_start.wav"
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pPlayer->pev->origin, g_vecZero, 0.0, 0.0, 10, pPlayer->entindex(), FALSE, 0);
				EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_skill_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
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
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pPlayer->pev->origin, g_vecZero, 0.0, 0.0, 10, pPlayer->entindex(), FALSE, 1);
				EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_skill_loop_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				m_iEffectType = 4; // this+93(iEffectType)
				m_flNextEffect = gpGlobals->time + 1.4s; // this+97
			}
			break;
		}
		case 4:
		{
			if (gpGlobals->time > m_flNextEffect)
			{
				EMIT_SOUND_DYN(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/dualsword_skill_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
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
			/*Vector vecNewOrigin = vecOrigin3[0];
			Vector vecNewVelocity = (vecOrigin3[1] - vecOrigin3[0]).Normalize();

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
			MESSAGE_END();*/

			Vector vecAngle2;
			vecAngle2[0] = RANDOM_FLOAT(15.0, 30.0);
			vecAngle2[1] = RANDOM_FLOAT(0.0, 180.0);

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, vecOrigin3[0], vecAngle2, 0.0, 0.0, 10, 1, TRUE, 1);
		}

		{
			/*Vector vecNewOrigin = vecOrigin3[0];
			Vector vecNewVelocity = (vecOrigin3[1] - vecOrigin3[0]).Normalize();

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
			MESSAGE_END();*/

			float speed = 1000.0;
			Vector vecVelocity = (vecOrigin3[1] - vecOrigin3[0]) * (speed / (vecOrigin3[1] - vecOrigin3[0]).Length());

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, vecOrigin3[0], vecVelocity, 0.0, 0.0, 10, 2, TRUE, 1);
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
			flDamage = 240;	//380 original
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 380;
#endif
		const float flRadius = 200;
		const float flAngle = 360;

		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(pPlayer->pev->v_angle);
		Vector vecSrc = pPlayer->GetGunPosition();

		RadiusAttack(flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, pPlayer->pev, pPlayer);
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

	m_iAttackRecord = 0;
	m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
}

void CKnifeDualsword::DestroyEffect()
{
	CBaseEntity* pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "dualsword_specialeffect")) != nullptr)
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
	m_iAttackRecord = 0;
	m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.992s;
}
void CKnifeDualsword::DestroyEffect() {}
#endif

}
