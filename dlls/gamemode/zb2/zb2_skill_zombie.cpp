/*
zb2_skill_zombie.cpp - CSMoE Gameplay server : Zombie Mod 2
Copyright (C) 2019 Moemod Yanase

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

#include "gamemode/zb2/zb2_const.h"
#include "gamemode/zb2/zb2_zclass.h"
#include "gamemode/zb2/zb2_skill.h"

void ZombieSkill_Precache()
{
	PRECACHE_SOUND("zombi/zombi_pressure.wav");
	PRECACHE_SOUND("zombi/zombi_pre_idle_1.wav");
	PRECACHE_SOUND("zombi/zombi_pre_idle_2.wav");
}

void CBaseZombieClass_ZB2::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	MESSAGE_END();
}

void CZombieClass_Default::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_TANK);
	WRITE_BYTE(ZOMBIE_SKILL_CRAZY);
	MESSAGE_END();
}

void CBaseZombieClass_ZB2::ActivateSkill(ZombieSkillSlot which)
{
	if (m_pZombieSkill && which == SKILL_SLOT_1)
		m_pZombieSkill->Activate();
}

void CBaseZombieClass_ZB2::Think()
{
	if (m_pZombieSkill)
		m_pZombieSkill->Think();
	return CZombie_ZB1::Think();
}

void CBaseZombieClass_ZB2::ResetMaxSpeed() const
{
	CZombie_ZB1::ResetMaxSpeed();
	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed(); // cans replace result of CZombie_ZB1::ResetMaxSpeed()
}

bool CBaseZombieClass_ZB2::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	// TODO : knockback adjustment by skill
	return CZombie_ZB1::ApplyKnockback(attacker, kbd);
}

float CBaseZombieClass_ZB2::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	if (m_pZombieSkill)
		flDamage *= m_pZombieSkill->GetDamageRatio();
	return CZombie_ZB1::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

ZombieSkillStatus CBaseZombieClass_ZB2::GetSkillStatus(ZombieSkillSlot which) const
{
	return m_pZombieSkill && which == SKILL_SLOT_1 ? m_pZombieSkill->GetStatus() : SKILL_STATUS_USED;
}

float CBaseZombieClass_ZB2::HealthRecoveryAmount() const
{
	return m_pZombieSkill->GetStatus() == SKILL_STATUS_USING ? 0 : m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 200 : 500;
}

CZombieClass_Default::CZombieClass_Default(CBasePlayer *player, ZombieLevel lv) : CBaseZombieClass_ZB2(player, lv)
{
	m_pZombieSkill.reset(new CZombieSkill_ZombieCrazy(m_pPlayer));
}


void CZombieSkill_Base::Think()
{
	if (m_iZombieSkillStatus == SKILL_STATUS_USING && gpGlobals->time > m_flTimeZombieSkillEnd)
	{
		OnSkillEnd();
	}

	if (m_iZombieSkillStatus == SKILL_STATUS_FREEZING && gpGlobals->time > m_flTimeZombieSkillNext)
	{
		m_iZombieSkillStatus = SKILL_STATUS_READY;
		OnSkillReady();
	}

}

CZombieSkill_Base::CZombieSkill_Base(CBasePlayer *player) : BasePlayerExtra(player), m_iZombieSkillStatus(SKILL_STATUS_READY)
{

}

CZombieSkill_ZombieCrazy::CZombieSkill_ZombieCrazy(CBasePlayer *player) : CZombieSkill_Base(player)
{

}

void CZombieSkill_ZombieCrazy::Think()
{
	CZombieSkill_Base::Think();

	if (m_iZombieSkillStatus == SKILL_STATUS_USING && gpGlobals->time > m_flTimeZombieSkillEffect)
	{
		OnCrazyEffect();
	}
}

void CZombieSkill_ZombieCrazy::Activate()
{
	if (m_iZombieSkillStatus != SKILL_STATUS_READY)
	{
		switch (m_iZombieSkillStatus)
		{
		case SKILL_STATUS_USING:
		case SKILL_STATUS_FREEZING:
			char buf[16];
			sprintf(buf, "%d", static_cast<int>(m_flTimeZombieSkillNext - gpGlobals->time));
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER,
				"The 'Berserk' skill can't be used because the skill is in cooldown. [Remaining Cooldown Time: %s1 sec.]",
				buf
			); // #CSO_WaitCoolTimeNormal

			break;
		case SKILL_STATUS_USED:
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "The 'Sprint' skill can only be used once per round."); // #CSO_CantSprintUsed
			break;
		default:
			break;
		}

		return;
	}

	if (m_pPlayer->pev->health <= 500.0f)
		return;

	m_iZombieSkillStatus = SKILL_STATUS_USING;
	m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
	m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();
	m_flTimeZombieSkillEffect = gpGlobals->time + 3.0f;

	m_pPlayer->pev->renderfx = kRenderFxGlowShell;
	m_pPlayer->pev->rendercolor = { 255,0,0 };
	m_pPlayer->pev->renderamt = 1;
	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 105;
	m_pPlayer->pev->health -= 500.0f;
	m_pPlayer->ResetMaxSpeed();

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pressure.wav", VOL_NORM, ATTN_NORM);

	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
	WRITE_BYTE(ZOMBIE_SKILL_CRAZY);
	WRITE_SHORT(GetDurationTime());
	WRITE_SHORT(GetCooldownTime());
	MESSAGE_END();
}

void CZombieSkill_ZombieCrazy::ResetMaxSpeed()
{
	if (m_iZombieSkillStatus == SKILL_STATUS_USING)
		m_pPlayer->pev->maxspeed = 390;
}

void CZombieSkill_ZombieCrazy::OnSkillEnd()
{
	m_iZombieSkillStatus = SKILL_STATUS_FREEZING;

	m_pPlayer->pev->renderfx = kRenderFxNone;
	m_pPlayer->pev->rendercolor = { 255,255,255 };
	m_pPlayer->pev->renderamt = 16;
	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieSkill_ZombieCrazy::OnCrazyEffect()
{
	m_flTimeZombieSkillEffect = gpGlobals->time + 3.0f;

	if (RANDOM_LONG(0, 1))
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pre_idle_1.wav", VOL_NORM, ATTN_NORM);
	else
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pre_idle_2.wav", VOL_NORM, ATTN_NORM);
}

float CZombieSkill_ZombieCrazy::GetDurationTime() const
{
	return m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 3.0f : 10.0f;
}

float CZombieSkill_ZombieCrazy::GetCooldownTime() const
{
	return m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 10.0f : 10.0f;
}

float CZombieSkill_ZombieCrazy::GetDamageRatio() const
{
	if (m_iZombieSkillStatus == SKILL_STATUS_USING)
		return 1.6f;
	return 1.0f;
}

