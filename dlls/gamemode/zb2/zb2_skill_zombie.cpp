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
#include "zb2_zclass.h"
#include "player/player_zombie.h"
#include "entity_state.h"

namespace sv {

	void ZombieSkill_Precache()
	{
		PRECACHE_SOUND("zombi/zombi_pressure.wav");
		PRECACHE_SOUND("zombi/zombi_pre_idle_1.wav");
		PRECACHE_SOUND("zombi/zombi_pre_idle_2.wav");
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

	CZombieSkill_Base::CZombieSkill_Base(CBasePlayer* player) : BasePlayerExtra(player), m_iZombieSkillStatus(SKILL_STATUS_READY)
	{

	}

	CZombieSkill_ZombieCrazy::CZombieSkill_ZombieCrazy(CBasePlayer* player) : CZombieSkill_Base(player)
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
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));
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
		m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;

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
		WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
		WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
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
		m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pre_idle_1.wav", VOL_NORM, ATTN_NORM);
		else
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pre_idle_2.wav", VOL_NORM, ATTN_NORM);
	}

	duration_t CZombieSkill_ZombieCrazy::GetDurationTime() const
	{
		return m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 3.0s : 10.0s;
	}

	duration_t CZombieSkill_ZombieCrazy::GetCooldownTime() const
	{
		return m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 15.0s : 10.0s;
	}

	float CZombieSkill_ZombieCrazy::GetDamageRatio() const
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			return 1.6f;
		return 1.0f;
	}

	//Speed Zombie
	CZombieSkill_ZombieHide::CZombieSkill_ZombieHide(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombieHide::Think()
	{
		CZombieSkill_Base::Think();
		float flInvisibleAlpha = m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 100 : 50;

		if (m_iZombieSkillStatus == SKILL_STATUS_USING && gpGlobals->time > m_flTimeZombieSkillEffect)
		{
			OnHideEffect();
		}
		//Fade start

		/*if (m_iZombieSkillStatus == SKILL_STATUS_USING)
		{
			float flAlpha = flInvisibleAlpha;

			if (m_flInvisiable - gpGlobals->time > 0.0s)
			{
				flAlpha += (255.0 - flInvisibleAlpha) * ((m_flInvisiable - gpGlobals->time) / 0.5s);
			}

			if (m_flTimeZombieSkillEnd > gpGlobals->time && m_flTimeZombieSkillEnd - gpGlobals->time < 0.5s)
			{
				flAlpha += (255.0 - flInvisibleAlpha) * ((0.5s - (m_flTimeZombieSkillEnd - gpGlobals->time)) / 0.5s);
			}
			m_pPlayer->pev->rendermode = kRenderTransAlpha;
			m_pPlayer->pev->renderamt = flAlpha;

		}*/

	}

	int CZombieSkill_ZombieHide::AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
	{
		float flInvisibleAlpha = m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 100 : 50;
		if (player)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(ent);
			auto id = pEntity->entindex();
			if (id >= 1 && id <= 32 && m_iZombieSkillStatus == SKILL_STATUS_USING)
			{
				float flAlpha = flInvisibleAlpha;

				if (m_flInvisiable - gpGlobals->time > 0.0s)
				{
					flAlpha += (255.0 - flInvisibleAlpha) * ((m_flInvisiable - gpGlobals->time) / 0.5s);
				}

				if (m_flTimeZombieSkillEnd > gpGlobals->time && m_flTimeZombieSkillEnd - gpGlobals->time < 0.5s)
				{
					flAlpha += (255.0 - flInvisibleAlpha) * ((0.5s - (m_flTimeZombieSkillEnd - gpGlobals->time)) / 0.5s);
				}

				state->rendermode = kRenderTransAlpha;
				state->renderamt = flAlpha;

			}
		}

		return 1;

	}

	void CZombieSkill_ZombieHide::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER,
					"The 'Hide' skill can't be used because the skill is in cooldown. [Remaining Cooldown Time: %s1 sec.]",
					buf
				); // #CSO_WaitCoolTimeNormal

				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "The 'Hide' skill can only be used once per round."); // #CSO_CantSprintUsed
				break;
			default:
				break;
			}

			return;
		}

		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
		m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();
		m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;
		m_flInvisiable = gpGlobals->time + 0.5s;

		m_pPlayer->pev->rendermode = kRenderTransAlpha;
		m_pPlayer->pev->gravity = m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 0.8 : 0.64;
		m_pPlayer->pev->renderamt = 255.0;
		m_pPlayer->pev->skin = 1;

		m_pPlayer->ResetMaxSpeed();

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pressure_female.wav", VOL_NORM, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_HIDE);
		WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
		WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
		MESSAGE_END();
	}

	void CZombieSkill_ZombieHide::ResetMaxSpeed()
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			m_pPlayer->pev->maxspeed = m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 180 : 250;
	}

	void CZombieSkill_ZombieHide::OnSkillEnd()
	{
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;

		m_pPlayer->pev->gravity = 0.64;
		m_pPlayer->pev->renderamt = 255.0;
		m_pPlayer->pev->skin = 0;
		m_pPlayer->ResetMaxSpeed();
	}

	void CZombieSkill_ZombieHide::OnHideEffect()
	{
		m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pressure_female.wav", VOL_NORM, ATTN_NORM);

	}

	duration_t CZombieSkill_ZombieHide::GetDurationTime() const
	{
		return m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 10s : 20s;
	}

	duration_t CZombieSkill_ZombieHide::GetCooldownTime() const
	{
		return 35s;
	}

	float CZombieSkill_ZombieHide::GetDamageRatio() const
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			return m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 1.1 : 1.0;
		return 1.0f;
	}

	//Deimos2 Zombie
	CZombieSkill_ZombieCrazy2::CZombieSkill_ZombieCrazy2(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombieCrazy2::Think()
	{
		CZombieSkill_Base::Think();

		/*if (m_iZombieSkillStatus == SKILL_STATUS_USING && gpGlobals->time > m_flTimeZombieSkillEffect)
		{
			OnCrazy2Effect();
		}*/
	}

	void CZombieSkill_ZombieCrazy2::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER,
					"The [Charge] skill can't be used because the skill is in cooldown. [Remaining Cooldown Time: %s1 sec.]",
					buf
				); // #CSO_WaitCoolTimeNormal

				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "You can no longer use [Bloody Blade]. It can only be used once per round."); // #CSO_CantSprintUsed
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
		m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;
		switch (m_pPlayer->m_iZombieLevel)
		{
			case ZOMBIE_LEVEL_HOST: m_pPlayer->pev->gravity = 0.69; break;
			case ZOMBIE_LEVEL_ORIGIN: m_pPlayer->pev->gravity = 0.7; break;
			case ZOMBIE_LEVEL_ORIGIN_LV2: m_pPlayer->pev->gravity = 0.71; break;
		}

		m_pPlayer->pev->renderfx = kRenderFxGlowShell;
		m_pPlayer->pev->rendercolor = { 255,0,0 };
		m_pPlayer->pev->renderamt = 1;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 105;
		m_pPlayer->pev->health -= 500.0f;
		m_pPlayer->ResetMaxSpeed();

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pressure.wav", VOL_NORM, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_CRAZY2);
		WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
		WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
		MESSAGE_END();
	}

	void CZombieSkill_ZombieCrazy2::ResetMaxSpeed()
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			m_pPlayer->pev->maxspeed = 300;
	}

	void CZombieSkill_ZombieCrazy2::OnSkillEnd()
	{
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;

		m_pPlayer->pev->renderfx = kRenderFxNone;
		m_pPlayer->pev->rendercolor = { 255,255,255 };
		m_pPlayer->pev->renderamt = 16;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
		m_pPlayer->ResetMaxSpeed();
	}

	void CZombieSkill_ZombieCrazy2::OnCrazy2Effect()
	{
		//m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pre_idle_1.wav", VOL_NORM, ATTN_NORM);
		else
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_pre_idle_2.wav", VOL_NORM, ATTN_NORM);
	}

	duration_t CZombieSkill_ZombieCrazy2::GetDurationTime() const
	{
		switch (m_pPlayer->m_iZombieLevel)
		{
		case ZOMBIE_LEVEL_HOST: return 2.5s; break;
		case ZOMBIE_LEVEL_ORIGIN: return 3.5s; break;
		case ZOMBIE_LEVEL_ORIGIN_LV2: return 4.5s; break;
		}
	}

	duration_t CZombieSkill_ZombieCrazy2::GetCooldownTime() const
	{
		switch (m_pPlayer->m_iZombieLevel)
		{
			case ZOMBIE_LEVEL_HOST: return 15.0s; break;
			case ZOMBIE_LEVEL_ORIGIN: return 14.0s; break;
			case ZOMBIE_LEVEL_ORIGIN_LV2: return 13.0s; break;
		}
	}

	float CZombieSkill_ZombieCrazy2::GetDamageRatio() const
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			return 0.85f;
		return 1.0f;
	}

}

