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
#include "deimostail.h"
#include "zombiebomb.h"
#include "teleportentity.h"
#include "zb2_zclass.h"
#include "player/player_zombie.h"
#include "entity_state.h"

namespace sv {

	void ZombieSkill_Precache()
	{
		PRECACHE_SOUND("zombi/zombi_pressure.wav");
		PRECACHE_SOUND("zombi/zombi_pre_idle_1.wav");
		PRECACHE_SOUND("zombi/zombi_pre_idle_2.wav");	
		PRECACHE_SOUND("zombi/zombi_smoke.wav");
		PRECACHE_SOUND("zombi/teleport_skill1.wav");
		PRECACHE_SOUND("zombi/teleport_skill2.wav");
		PRECACHE_SOUND("zombi/teleport_skill2_1.wav");
		PRECACHE_SOUND("zombi/zombi_teleport_gate.wav");
		PRECACHE_SOUND("zombi/zombi_chinese_jump.wav");
		PRECACHE_SOUND("zombi/passzombie_pullout.wav");
		PRECACHE_SOUND("zombi/passzombie_skill.wav");
		PRECACHE_SOUND("zombi/passzombie_slash2.wav");
		PRECACHE_SOUND("zombi/flyzombie_fly_start.wav");
		PRECACHE_SOUND("zombi/flyzombie_fly_idle.wav");
		PRECACHE_SOUND("zombi/flyzombie_downhill_start.wav");
		
		PRECACHE_MODEL("sprites/zombihealer.spr");
		PRECACHE_MODEL("sprites/zombiheal_head.spr");
		PRECACHE_MODEL("sprites/ef_teleportzombie.spr");
		PRECACHE_MODEL("sprites/ef_teleportzombieout.spr");

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

		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
		m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();
		m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;

		m_pPlayer->pev->renderfx = kRenderFxGlowShell;
		m_pPlayer->pev->rendercolor = { 255,0,0 };
		m_pPlayer->pev->renderamt = 1;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 105;
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

		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
		m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();
		m_flTimeZombieSkillEffect = gpGlobals->time + 3.0s;
		switch (m_pPlayer->m_iZombieLevel)
		{
		case ZOMBIE_LEVEL_HOST: m_pPlayer->pev->gravity = 0.71; break;
		case ZOMBIE_LEVEL_ORIGIN: m_pPlayer->pev->gravity = 0.7; break;
		case ZOMBIE_LEVEL_ORIGIN_LV2: m_pPlayer->pev->gravity = 0.69; break;
		}

		m_pPlayer->pev->renderfx = kRenderFxGlowShell;
		m_pPlayer->pev->rendercolor = { 255,0,0 };
		m_pPlayer->pev->renderamt = 1;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 105;
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
		if (m_pPlayer->IsBot())
			return 5.0s;

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


	//Heal Zombie From CS:AE
	CZombieSkill_ZombieHeal::CZombieSkill_ZombieHeal(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombieHeal::Think()
	{
		CZombieSkill_Base::Think();
	}

	void CZombieSkill_ZombieHeal::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));

				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_WaitSkillCoolTime", buf);
				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "You can no longer use [Bloody Blade]. It can only be used once per round."); // #CSO_CantSprintUsed
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

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "zombi/zombi_heal.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);


		float flOldHealth = m_pPlayer->pev->health;
		m_pPlayer->pev->health += m_pPlayer->pev->max_health * (m_pPlayer->m_iZombieLevel ? 0.3 : 0.1);
		m_pPlayer->pev->health = min(m_pPlayer->pev->health, m_pPlayer->pev->max_health);
		
		MESSAGE_BEGIN(MSG_ALL, gmsgHeadIcon, NULL, m_pPlayer->pev);
		WRITE_BYTE(2);	//type
		WRITE_SHORT(ENTINDEX(m_pPlayer->edict()));	//entity
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, gmsgHeadIcon, NULL, m_pPlayer->pev);
		WRITE_BYTE(3);	//type
		WRITE_SHORT(ENTINDEX(m_pPlayer->edict()));	//entity
		MESSAGE_END();

		if (flOldHealth < m_pPlayer->pev->health)
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "zombi/TD_Heal.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);

		}
		else
		{
			m_pPlayer->pev->armorvalue += 250;
			if (m_pPlayer->pev->armorvalue > 1000)
				m_pPlayer->pev->armorvalue = 1000;
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "zombi/td_buff.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
		}

		UTIL_MakeVectors(m_pPlayer->pev->angles);

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* player = (CBasePlayer*)UTIL_PlayerByIndex(i);

			if (player == NULL)
				continue;

			if (FNullEnt(player->pev))
				continue;

			if (player->IsDormant())
				continue;

			if (player->m_iTeam != m_pPlayer->m_iTeam)
				continue;

			if (player == m_pPlayer)
				continue;

			Vector vecDelta = player->pev->origin - m_pPlayer->pev->origin;

			if (vecDelta.IsLengthGreaterThan(500.0f))
				continue;

			if (DotProduct(vecDelta.Normalize(), gpGlobals->v_forward) > 0.68)
			{
				flOldHealth = player->pev->health;
				player->pev->health += player->pev->max_health * (player->m_iZombieLevel ? 0.3 : 0.1);
				player->pev->health = min(player->pev->health, player->pev->max_health);

				if (flOldHealth < player->pev->health)
				{
					if (player->m_ZombieClass != NULL)
					{
						if (!stricmp(player->m_ZombieClass, "speed") || !stricmp(player->m_ZombieClass, "booster"))
							EMIT_SOUND_DYN(ENT(player->pev), CHAN_AUTO, "zombi/zombi_heal_female.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
						else if (!stricmp(m_pPlayer->m_ZombieClass, "teleport"))
							EMIT_SOUND_DYN(ENT(player->pev), CHAN_AUTO, "zombi/zombi_heal_teleport.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);						
						else
							EMIT_SOUND_DYN(ENT(player->pev), CHAN_AUTO, "zombi/zombi_heal.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
					}
					else
						EMIT_SOUND_DYN(ENT(player->pev), CHAN_AUTO, "zombi/zombi_heal.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					player->pev->armorvalue += 250;
					if (player->pev->armorvalue > 1000)
						player->pev->armorvalue = 1000;
					EMIT_SOUND_DYN(ENT(player->pev), CHAN_AUTO, "zombi/td_buff.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
				}

				MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, player->pev);
				WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
				WRITE_BYTE(ZOMBIE_SKILL_HEAL);
				WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
				WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
				MESSAGE_END();
				
				MESSAGE_BEGIN(MSG_ALL, gmsgHeadIcon, NULL, player->pev);
				WRITE_BYTE(2);	//type
				WRITE_SHORT(ENTINDEX(player->edict()));	//entity
				MESSAGE_END();

				MESSAGE_BEGIN(MSG_ALL, gmsgHeadIcon, NULL, player->pev);
				WRITE_BYTE(3);	//type
				WRITE_SHORT(ENTINDEX(player->edict()));	//entity
				MESSAGE_END();
			}
		}

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_HEAL);
		WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
		WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
		MESSAGE_END();

	}

	void CZombieSkill_ZombieHeal::ResetMaxSpeed()
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			m_pPlayer->pev->maxspeed = 300;
	}

	void CZombieSkill_ZombieHeal::OnSkillEnd()
	{
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;

	}

	duration_t CZombieSkill_ZombieHeal::GetDurationTime() const
	{
		return 1.0s;
	}

	duration_t CZombieSkill_ZombieHeal::GetCooldownTime() const
	{
		switch (m_pPlayer->m_iZombieLevel)
		{
		case ZOMBIE_LEVEL_HOST: return 10.0s; break;
		case ZOMBIE_LEVEL_ORIGIN: return 7.0s; break;
		case ZOMBIE_LEVEL_ORIGIN_LV2: return 7.0s; break;
		}
	}

	float CZombieSkill_ZombieHeal::GetDamageRatio() const
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			return 0.85f;
		return 1.0f;
	}

	//Deimos Zombie From CS:AE
	CZombieSkill_ZombieDeimos::CZombieSkill_ZombieDeimos(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombieDeimos::Think()
	{
		if (m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill < gpGlobals->time)
			FireZombieSkill();
		CZombieSkill_Base::Think();
	}

	void CZombieSkill_ZombieDeimos::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));

				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_WaitSkillCoolTime", buf);
				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "You can no longer use [Bloody Blade]. It can only be used once per round."); // #CSO_CantSprintUsed
				break;
			default:
				break;
			}

			return;
		}

		if (m_pPlayer->m_pActiveItem && m_pPlayer->m_pActiveItem->m_iId == WEAPON_KNIFE)
		{
			m_iZombieSkillStatus = SKILL_STATUS_USING;
			m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
			m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();

			CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
			if (pWeapon)
			{
				pWeapon->SendWeaponAnim(2, TRUE);
				pWeapon->m_flTimeWeaponIdle = 1.0s;
			}
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 94;
			m_pPlayer->m_flNextAttack = 1.5s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "zombi/deimos_skill_start.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

			int anim = m_pPlayer->LookupSequence("skill");
			m_pPlayer->pev->sequence = anim;
			m_pPlayer->pev->frame = 0;


			m_flTimeTriggerZombieSkill = gpGlobals->time + 0.5s;

			MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
			WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
			WRITE_BYTE(ZOMBIE_SKILL_SHOCK);
			WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
			WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
			MESSAGE_END();
		}
	}

	void CZombieSkill_ZombieDeimos::OnSkillEnd()
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;
	}

	duration_t CZombieSkill_ZombieDeimos::GetDurationTime() const
	{
		return 1.0s;
	}

	void CZombieSkill_ZombieDeimos::FireZombieSkill()
	{
		CDeimosTail* pTail = (CDeimosTail*)CBaseEntity::Create("deimostail", m_pPlayer->GetGunPosition(), m_pPlayer->pev->angles, m_pPlayer->edict());

		if (!pTail)
			return;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		if (m_pPlayer->IsBot())
		{
			pTail->Initialize(2.0s);
			pTail->pev->velocity = gpGlobals->v_forward * 1800;
		}
		else
		{
			pTail->Initialize(m_pPlayer->m_iZombieLevel ? 1.0s : 0.5s);
			pTail->pev->velocity = gpGlobals->v_forward * 1200;
		}
		m_flTimeTriggerZombieSkill = invalid_time_point;
	}

	duration_t CZombieSkill_ZombieDeimos::GetCooldownTime() const
	{
		switch (m_pPlayer->m_iZombieLevel)
		{
		case ZOMBIE_LEVEL_HOST: return 15.0s; break;
		case ZOMBIE_LEVEL_ORIGIN: return 7.0s; break;
		case ZOMBIE_LEVEL_ORIGIN_LV2: return 7.0s; break;
		}
	}

	//PC Zombie
	CZombieSkill_ZombiePC::CZombieSkill_ZombiePC(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombiePC::Think()
	{
		if (m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill < gpGlobals->time)
		{
			PLAYBACK_EVENT_FULL(0, NULL, PRECACHE_EVENT(1, "events/createsmoke.sc"), 0, m_pPlayer->pev->origin - Vector(0, 0, 30), (float*)&g_vecZero, 0, 0, 1, 1, TRUE, FALSE);
			m_flTimeTriggerZombieSkill = invalid_time_point;
			m_flTimeNextSmoke = gpGlobals->time + 1.5s;
		}
		if (m_flTimeNextSmoke != invalid_time_point && m_flTimeNextSmoke < gpGlobals->time)
		{
			PLAYBACK_EVENT_FULL(0, NULL, PRECACHE_EVENT(1, "events/createsmoke.sc"), 0, m_pPlayer->pev->origin - Vector(0, 0, 30), (float*)&g_vecZero, 0, 0, 1, 1, TRUE, FALSE);
			m_flTimeNextSmoke = invalid_time_point;
		}

		CZombieSkill_Base::Think();
	}

	void CZombieSkill_ZombiePC::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));

				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_WaitSkillCoolTime", buf);
				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "You can no longer use [Bloody Blade]. It can only be used once per round."); // #CSO_CantSprintUsed
				break;
			default:
				break;
			}

			return;
		}

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "zombi/zombi_smoke.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);

		PLAYBACK_EVENT_FULL(0, NULL, PRECACHE_EVENT(1, "events/createsmoke.sc"), 0, m_pPlayer->pev->origin - Vector(0, 0, 30), (float*)&g_vecZero, 0, 0, 1, 1, TRUE, FALSE);
		
		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_pPlayer->ResetMaxSpeed();

		m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
		m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_SMOKE);
		WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
		WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
		MESSAGE_END();

		m_flTimeTriggerZombieSkill = gpGlobals->time + 1.5s;
	}

	void CZombieSkill_ZombiePC::ResetMaxSpeed()
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING)
			m_pPlayer->pev->maxspeed = 1;
	}

	void CZombieSkill_ZombiePC::OnSkillEnd()
	{
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;
		m_pPlayer->ResetMaxSpeed();
	}

	duration_t CZombieSkill_ZombiePC::GetDurationTime() const
	{
		return 1.0s;
	}

	duration_t CZombieSkill_ZombiePC::GetCooldownTime() const
	{
		return 20.0s;
	}
	
	//Pass Zombie
	CZombieSkill_ZombiePass::CZombieSkill_ZombiePass(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombiePass::Think()
	{
		if (m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill < gpGlobals->time)
			FireZombieSkill();
		CZombieSkill_Base::Think();
	}

	void CZombieSkill_ZombiePass::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));

				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_WaitSkillCoolTime", buf);
				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "You can no longer use [Bloody Blade]. It can only be used once per round."); // #CSO_CantSprintUsed
				break;
			default:
				break;
			}

			return;
		}

		if(m_pPlayer->IsBot())
			CZombibomb* pBomb = (CZombibomb*)CBaseEntity::Create("zombiebomb", m_pPlayer->pev->origin, m_pPlayer->pev->angles, m_pPlayer->edict());
		else
		{
			CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[GRENADE_SLOT];
			if (pWeapon != NULL && !Q_strcmp(STRING(pWeapon->pev->classname), "weapon_zombibomb"))
			{
				CBasePlayerWeapon* pWpn = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
				if (pWpn)
				{
					if (pWpn != NULL && !Q_strcmp(STRING(pWpn->pev->classname), "weapon_zombibomb"))
						m_pPlayer->DropPlayerItem(STRING(pWpn->pev->classname));
						m_flTimeTriggerZombieSkill = gpGlobals->time + 0.5s;
				}
				return;		
			}
			else
			{
				m_pPlayer->GiveNamedItem("weapon_zombibomb");
				m_pPlayer->SwitchWeapon(m_pPlayer->m_rgpPlayerItems[GRENADE_SLOT]);
				CBasePlayerWeapon* pWpn = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
				if (pWpn)
				{
					pWpn->SendWeaponAnim(5, TRUE);
					pWpn->m_flTimeWeaponIdle = 1.0s;
				}
				m_pPlayer->m_flNextAttack = 2.0s;
			}
				
		}
			
		m_iZombieSkillStatus = SKILL_STATUS_USING;
		m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
		m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();
		
		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_TRAP);
		WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
		WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
		MESSAGE_END();
	}

	void CZombieSkill_ZombiePass::ResetMaxSpeed()
	{

	}

	void CZombieSkill_ZombiePass::OnSkillEnd()
	{
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;
	}

	duration_t CZombieSkill_ZombiePass::GetDurationTime() const
	{
		return 1.0s;
	}

	duration_t CZombieSkill_ZombiePass::GetCooldownTime() const
	{
		return 10.0s;
	}

	void CZombieSkill_ZombiePass::FireZombieSkill()
	{
		m_flTimeTriggerZombieSkill = invalid_time_point;
	}
	//teleport Zombie
	CZombieSkill_ZombieTeleport::CZombieSkill_ZombieTeleport(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombieTeleport::Think()
	{
		if (m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill < gpGlobals->time)
			FireZombieSkill();
		if (m_pPlayer->IsBot())
		{
			if (m_flTimeBotUseSkill != invalid_time_point && m_flTimeBotUseSkill < gpGlobals->time)
			{
				if(m_bHasFireTeleportEntity)
					FireZombieSkill();
			}
		}	
		CZombieSkill_Base::Think();
	}

	void CZombieSkill_ZombieTeleport::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));

				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_WaitSkillCoolTime", buf);
				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "You can no longer use [Bloody Blade]. It can only be used once per round."); // #CSO_CantSprintUsed
				break;
			default:
				break;
			}

			return;
		}

		if (m_bHasFireTeleportEntity)
		{
			if (m_pPlayer->m_pActiveItem && m_pPlayer->m_pActiveItem->m_iId == WEAPON_KNIFE)
			{
				CBaseEntity* pEntity = NULL;
				CTeleportEntity* pEnt;
				while ((pEntity = UTIL_FindEntityByClassname(pEntity, "teleportentity")) != NULL)
				{
					if (pEntity->pev->owner && pEntity->pev->owner == m_pPlayer->edict())
					{
						pEnt = static_cast<CTeleportEntity*>(pEntity);
						if (pEnt)
						{
							pEnt->Play_EfTeleport();
							EMIT_SOUND_DYN(pEnt->edict(), CHAN_ITEM, "zombi/teleport_skill2_1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
						}
					}
				}
				//preact teleport
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
				if (pWeapon)
				{
					pWeapon->SendWeaponAnim(8, TRUE);
					pWeapon->m_flTimeWeaponIdle = 1.0s;
				}

				int anim = m_pPlayer->LookupSequence("skill2");
				m_pPlayer->pev->sequence = anim;
				m_pPlayer->pev->frame = 0;			

				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_EXPLOSION);
				WRITE_COORD(m_pPlayer->pev->origin[0]);
				WRITE_COORD(m_pPlayer->pev->origin[1]);
				WRITE_COORD(m_pPlayer->pev->origin[2]);
				WRITE_SHORT(MODEL_INDEX("sprites/ef_teleportzombie.spr"));
				WRITE_BYTE(5);
				WRITE_BYTE(30);
				WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
				MESSAGE_END();
				
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "zombi/teleport_skill2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

				m_iZombieSkillStatus = SKILL_STATUS_USING;
				m_pPlayer->m_flNextAttack = 2.0s;
				m_flTimeTriggerZombieSkill = gpGlobals->time + 2.0s;
				m_flTimeBotUseSkill = gpGlobals->time + 2.0s;		//should be random
				m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
				m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();

				MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
				WRITE_BYTE(ZB2_MESSAGE_SKILL_ICON_CHANGE);
				WRITE_BYTE(ZOMBIE_SKILL_TELEPORT);
				MESSAGE_END();

				MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
				WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
				WRITE_BYTE(ZOMBIE_SKILL_TELEPORT);
				WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
				WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
				MESSAGE_END();

				m_pPlayer->ResetMaxSpeed();
				return;
			}
		}
		else
		{
			if (m_pPlayer->m_pActiveItem && m_pPlayer->m_pActiveItem->m_iId == WEAPON_KNIFE)
			{
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
				if (pWeapon)
				{
					pWeapon->SendWeaponAnim(2, TRUE);
					pWeapon->m_flTimeWeaponIdle = 1.0s;

					int anim = m_pPlayer->LookupSequence("skill1");
					m_pPlayer->pev->sequence = anim;
					m_pPlayer->pev->frame = 0;

					m_pPlayer->m_flNextAttack = 1.5s;

					MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
					WRITE_BYTE(ZB2_MESSAGE_SKILL_ICON_CHANGE);
					WRITE_BYTE(ZOMBIE_SKILL_TELEPORT2);
					MESSAGE_END();
					
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "zombi/teleport_skill2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

					m_flTimeTriggerZombieSkill = gpGlobals->time + 1.0s;
				}
			}
		}
	}

	Vector CZombieSkill_ZombieTeleport::CalcPositon()
	{
		float dis = 9999.0;
		int target = 0;
		Vector vecOrigin = m_pPlayer->pev->origin;
		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);

			if (!entity)
				continue;

			CBasePlayer* player = static_cast<CBasePlayer*>(entity);

			if (FNullEnt(player->pev))
				continue;

			if (player == m_pPlayer)
				continue;

			if (player->IsDormant())
				continue;

			if (g_pGameRules->PlayerRelationship(m_pPlayer, player) == GR_TEAMMATE)
				continue;

			Vector vecPlayerOrigin = player->pev->origin;

			float fDelta = (vecPlayerOrigin - m_pPlayer->pev->origin).Length();
			if (fDelta < dis)
			{
				target = iIndex;
				dis = fDelta;
			}
		}
		if (target)
		{
			CBaseEntity* entity = UTIL_PlayerByIndex(target);

			if (!entity)
				return vecOrigin;

			CBasePlayer* player = static_cast<CBasePlayer*>(entity);
			vecOrigin = player->pev->origin;
		}

		return vecOrigin;
	}

	void CZombieSkill_ZombieTeleport::FireZombieSkill()
	{
		if (!m_bHasFireTeleportEntity)
		{
			if (m_pPlayer->IsBot())
			{
				UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
				Vector vecOrigin = CalcPositon();
				CTeleportEntity* pEnt = (CTeleportEntity*)CBaseEntity::Create("teleportentity", vecOrigin, m_pPlayer->pev->angles, m_pPlayer->edict());

				if (!pEnt)
					return;

				m_flTimeTriggerZombieSkill = invalid_time_point;
				m_bHasFireTeleportEntity = 1;
			}
			else
			{
				CTeleportEntity* pEnt = (CTeleportEntity*)CBaseEntity::Create("teleportentity", m_pPlayer->GetGunPosition(), m_pPlayer->pev->angles, m_pPlayer->edict());

				if (!pEnt)
					return;
				pEnt->entindex();
				UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
				Vector vecVelocity;
				vecVelocity = gpGlobals->v_forward * 1200;
				pEnt->pev->velocity = vecVelocity;

				m_flTimeTriggerZombieSkill = invalid_time_point;
				m_bHasFireTeleportEntity = 1;
			}
		}
		else
		{
			CBaseEntity* pEntity = NULL;
			CTeleportEntity* pEnt;
			while ((pEntity = UTIL_FindEntityByClassname(pEntity, "teleportentity")) != NULL)
			{
				if (pEntity->pev->owner && pEntity->pev->owner == m_pPlayer->edict())
				{
					pEnt = static_cast<CTeleportEntity*>(pEntity);

					Vector vecEntOrigin = pEnt->pev->origin;
					if (m_pPlayer->pev->flags & FL_DUCKING)
						vecEntOrigin[2] += 18.0;
					else
						vecEntOrigin[2] += 36.0;
					m_pPlayer->pev->origin = vecEntOrigin;
					m_flTimeBotUseSkill = invalid_time_point;
					m_flTimeTriggerZombieSkill = invalid_time_point;
					CBaseEntity* pEntity = NULL;
					while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEntOrigin, 30)) != NULL)
					{
						if (pEntity->pev->takedamage != DAMAGE_NO)
						{
							if (pEntity->IsPlayer())
							{
								CBasePlayer* player = static_cast<CBasePlayer*>(pEntity);

								if (player == m_pPlayer)
									continue;

								if (player->IsAlive())
									player->Killed(player->pev, 1);
								break;
							}
						}
					}
					pEnt->Kill();
					m_bHasFireTeleportEntity = 0;
				}
			}
		}
	}

	void CZombieSkill_ZombieTeleport::ResetMaxSpeed()
	{
		if (m_iZombieSkillStatus == SKILL_STATUS_USING && m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill > gpGlobals->time)
			m_pPlayer->pev->maxspeed = 1;
	}

	void CZombieSkill_ZombieTeleport::OnSkillEnd()
	{
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;
		m_pPlayer->ResetMaxSpeed();
	}

	duration_t CZombieSkill_ZombieTeleport::GetDurationTime() const
	{
		return 2.0s;
	}

	duration_t CZombieSkill_ZombieTeleport::GetCooldownTime() const
	{
		return 10.0s;
	}

	//teleport Zombie
	CZombieSkill_ZombieBooster::CZombieSkill_ZombieBooster(CBasePlayer* player) : CZombieSkill_Base(player)
	{

	}

	void CZombieSkill_ZombieBooster::Think()
	{
		if (m_pPlayer->IsBot())
		{
			CZombieSkill_Base::Think();
		}
		else
		{		
			if (m_flPercent < 0.3)
				m_iZombieSkillStatus = SKILL_STATUS_USED;

			if (m_flTimeNextBoost != invalid_time_point && m_flTimeNextBoost > gpGlobals->time)
				return;

			if (m_flTimeChargeCool > gpGlobals->time)
				return;
			
			m_flTimeNextBoost = invalid_time_point;
			m_flTimeChargeCool = gpGlobals->time + 0.2s;		//avoid boost too fast
			m_vecVelocity = m_pPlayer->pev->velocity;
			float flSpeed = m_vecVelocity.Length();
			if (flSpeed < 375.0f || m_flBoostCharge >= m_flMaxpower)
			{
				//ALERT(at_console, "Your Speed is %f , and now power is %f and percents is %f\n", flSpeed, m_flBoostCharge, m_flPercent);
				return;
			}
			
			if (flSpeed > 475.0f)
				flSpeed = 475.0f;
			m_flBoostCharge += flSpeed - 375.0f;
			if (m_flBoostCharge >= m_flMaxpower)
				m_flBoostCharge = m_flMaxpower;

			m_flPercent = m_flBoostCharge / m_flMaxpower;
			if (m_flPercent >= 0.3)
			{
				m_iZombieSkillStatus = SKILL_STATUS_READY;
				MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
				WRITE_BYTE(ZB2_MESSAGE_SKILL_ICON_CHANGE);
				WRITE_BYTE(ZOMBIE_SKILL_BOOSTER);
				MESSAGE_END();

				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "Tip: Can boost");
			}
			if (m_flPercent = 1.0)
			{
				//MESSAGE blink
			}
				//ALERT(at_console, "Your Speed is %f , and now power is %f and percents is %f\n", flSpeed, m_flBoostCharge, m_flPercent);
		}
		
	}

	void CZombieSkill_ZombieBooster::Activate()
	{
		if (m_iZombieSkillStatus != SKILL_STATUS_READY)
		{
			switch (m_iZombieSkillStatus)
			{
			case SKILL_STATUS_USING:
			case SKILL_STATUS_FREEZING:
				char buf[16];
				sprintf(buf, "%d", static_cast<int>((m_flTimeZombieSkillNext - gpGlobals->time) / 1s));

				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_WaitSkillCoolTime", buf);
				break;
			case SKILL_STATUS_USED:
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_BoosterZombie_Fail"); // #CSO_CantSprintUsed
				break;
			default:
				break;
			}

			return;
		}

		if (m_pPlayer->IsBot())
		{
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			m_flBoostCharge = RANDOM_FLOAT(400, 600);
			m_pPlayer->pev->velocity[0] += m_flBoostCharge * gpGlobals->v_forward[0];
			m_pPlayer->pev->velocity[1] += m_flBoostCharge * gpGlobals->v_forward[1];
			m_pPlayer->pev->velocity[2] += m_flBoostCharge;
			m_iZombieSkillStatus = SKILL_STATUS_USING;
			m_flTimeZombieSkillEnd = gpGlobals->time + GetDurationTime();
			m_flTimeZombieSkillNext = gpGlobals->time + GetCooldownTime();
			
		}
		else
		{
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			if (m_pPlayer->pev->velocity[2])
				m_pPlayer->pev->velocity += m_flBoostCharge * gpGlobals->v_forward;
			else
			{
				m_pPlayer->pev->velocity[0] += m_flBoostCharge * gpGlobals->v_forward[0];
				m_pPlayer->pev->velocity[1] += m_flBoostCharge * gpGlobals->v_forward[1];
			}
			m_flBoostCharge = 0.0f;
			m_flTimeNextBoost = gpGlobals->time + 1.0s;
			m_iZombieSkillStatus = SKILL_STATUS_USED;
		}
		int anim = m_pPlayer->LookupSequence("skill");
		m_pPlayer->pev->sequence = anim;
		m_pPlayer->pev->frame = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
		WRITE_BYTE(ZOMBIE_SKILL_BOOSTER);
		WRITE_SHORT(static_cast<int>(GetDurationTime() / 1s));
		WRITE_SHORT(static_cast<int>(GetCooldownTime() / 1s));
		MESSAGE_END();

	}

	void CZombieSkill_ZombieBooster::ResetMaxSpeed()
	{
		
	}

	void CZombieSkill_ZombieBooster::OnSkillEnd()
	{
		m_iZombieSkillStatus = SKILL_STATUS_FREEZING;
	}

	duration_t CZombieSkill_ZombieBooster::GetDurationTime() const
	{
		return 1.0s;
	}

	duration_t CZombieSkill_ZombieBooster::GetCooldownTime() const
	{
		if (m_pPlayer->IsBot())
			return 20.0s;
		else
			return 0.0s;
	}
	
}