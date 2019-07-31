/*
zb2_skill_human.cpp - CSMoE Gameplay server : Zombie Mod 2
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
#include "zb2_zclass.h"

namespace sv {

constexpr bool ENABLE_SPRINT = true;
constexpr bool ENABLE_HEADSHOT = true;
constexpr bool ENABLE_KNIFE2X = true;

void HumanSkill_Precache()
{
	PRECACHE_SOUND("zombi/speedup.wav");
	PRECACHE_SOUND("zombi/human_breath_male.wav");
	PRECACHE_SOUND("zombi/speedup_heartbeat.wav");
}

class SprintSkill : protected CZombieSkill_Base
{
public:
	explicit SprintSkill(CBasePlayer * player) : CZombieSkill_Base(player), m_flTimeZombieSkillEffect(invalid_time_point), m_bDebuffStatus(false) {}

	void Think() override
	{
		CZombieSkill_Base::Think();

		if (m_iZombieSkillStatus == SKILL_STATUS_USING && gpGlobals->time > m_flTimeZombieSkillEffect)
		{
			OnCrazyEffect();
		}
	}

	void Activate() override
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "The 'Sprint' skill can only be used once per round."); // #CSO_CantSprintUsed
			return;
		}

		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_flTimeZombieSkillEnd = gpGlobals->time + 10.0f;
		m_flTimeZombieSkillNext = invalid_time_point;
		m_flTimeZombieSkillEffect = gpGlobals->time + 1.0f;
		m_bDebuffStatus = false;

		m_pPlayer->ResetMaxSpeed();

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/speedup.wav", VOL_NORM, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_SPRINT);
		WRITE_SHORT(10);
		WRITE_SHORT(-1);
		MESSAGE_END();
	}

	void ResetMaxSpeed() override
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
		{
			if (m_bDebuffStatus)
			{
				m_pPlayer->pev->maxspeed = 100;
			}
			else
			{
				m_pPlayer->pev->maxspeed = 350;
			}
		}
	}

	using CZombieSkill_Base::GetStatus;

protected:
	void OnSkillEnd() override
	{
		if (m_bDebuffStatus)
		{
			m_iZombieSkillStatus = SKILL_STATUS_USED;
			m_bDebuffStatus = false;
		}
		else
		{
			m_iZombieSkillStatus = SKILL_STATUS_USING;
			m_bDebuffStatus = true;
			m_flTimeZombieSkillEnd = gpGlobals->time + 5.0f;
		}
		m_pPlayer->ResetMaxSpeed();
	}

	void OnCrazyEffect()
	{
		m_flTimeZombieSkillEffect = gpGlobals->time + 1.0f;

		if (m_bDebuffStatus)
			CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/human_breath_male.wav\n");
		else
			CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/speedup_heartbeat.wav\n");
	}

private:
	EngineClock::time_point m_flTimeZombieSkillEffect;
	bool m_bDebuffStatus;
};

class HeadShotSkill : protected CZombieSkill_Base
{
public:
	explicit HeadShotSkill(CBasePlayer * player) : CZombieSkill_Base(player) {}

	void Activate() override
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "The 'HeadShot' skill can only be used once per round."); // #CSO_CantSprintUsed
			return;
		}
		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_flTimeZombieSkillEnd = gpGlobals->time + 4.5f;
		m_flTimeZombieSkillNext = invalid_time_point;

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/speedup.wav", VOL_NORM, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_HEADSHOT);
		WRITE_SHORT(5);
		WRITE_SHORT(-1);
		MESSAGE_END();
	}

	void OnSkillEnd() override
	{
		m_iZombieSkillStatus = SKILL_STATUS_USED;
	}

	using CZombieSkill_Base::GetStatus;
	using CZombieSkill_Base::Think;
};

class Knife2xSkill : protected CZombieSkill_Base
{
public:
	explicit Knife2xSkill(CBasePlayer * player) : CZombieSkill_Base(player) {}

	void Activate() override
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "The 'Knife2x' skill can only be used once per round."); // #CSO_CantSprintUsed
			return;
		}
		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_flTimeZombieSkillEnd = gpGlobals->time + 10.f;
		m_flTimeZombieSkillNext = invalid_time_point;

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/speedup.wav", VOL_NORM, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_KNIFE2X);
		WRITE_SHORT(10);
		WRITE_SHORT(-1);
		MESSAGE_END();
	}

	void OnSkillEnd() override
	{
		m_iZombieSkillStatus = SKILL_STATUS_USED;
	}

	using CZombieSkill_Base::GetStatus;
	using CZombieSkill_Base::Think;
};

class CHuman_ZB2::impl_t : public BasePlayerExtra
{
public:
	explicit impl_t(CBasePlayer *player) : BasePlayerExtra(player), spd(player), hs(player), k2x(player) {}
	SprintSkill spd;
	HeadShotSkill hs;
	Knife2xSkill k2x;
};

CHuman_ZB2::CHuman_ZB2(CBasePlayer * player) : CHuman_ZB1(player), pimpl(std::unique_ptr<impl_t>(new impl_t(player)))
{

}

void CHuman_ZB2::ActivateSkill(ZombieSkillSlot which)
{
	if (which == SKILL_SLOT_1 && ENABLE_SPRINT)
		pimpl->spd.Activate();
	else if (which == SKILL_SLOT_2 && ENABLE_HEADSHOT)
		pimpl->hs.Activate();
	else if (which == SKILL_SLOT_3 && ENABLE_KNIFE2X)
		pimpl->k2x.Activate();
}

void CHuman_ZB2::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_HUMAN);
	WRITE_BYTE(ENABLE_SPRINT ? ZOMBIE_SKILL_SPRINT : ZOMBIE_SKILL_EMPTY);
	WRITE_BYTE(ENABLE_HEADSHOT ? ZOMBIE_SKILL_HEADSHOT : ZOMBIE_SKILL_EMPTY);
	WRITE_BYTE(ENABLE_KNIFE2X ? ZOMBIE_SKILL_KNIFE2X : ZOMBIE_SKILL_EMPTY);
	MESSAGE_END();
}

void CHuman_ZB2::Think()
{
	if(ENABLE_SPRINT)
		pimpl->spd.Think();

	if(ENABLE_HEADSHOT)
		pimpl->hs.Think();

	if(ENABLE_KNIFE2X)
		pimpl->k2x.Think();
}

void CHuman_ZB2::ResetMaxSpeed() const
{
	if (ENABLE_KNIFE2X)
		pimpl->spd.ResetMaxSpeed();
}

ZombieSkillStatus CHuman_ZB2::GetSkillStatus(ZombieSkillSlot which) const
{
	if (which == SKILL_SLOT_1 && ENABLE_SPRINT)
		return pimpl->spd.GetStatus();
	if (which == SKILL_SLOT_2 && ENABLE_HEADSHOT)
		return pimpl->hs.GetStatus();
	if (which == SKILL_SLOT_3 && ENABLE_KNIFE2X)
		return pimpl->k2x.GetStatus();
	return SKILL_STATUS_USED;
}

CHuman_ZB2::~CHuman_ZB2() = default;

}
