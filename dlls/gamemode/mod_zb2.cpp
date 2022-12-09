/*
mod_zb2.cpp - CSMoE Gameplay server : Zombie Mod 2
Copyright (C) 2018 Moemod Hyakuya

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
#include "game.h"
#include "client.h"
#include "bmodels.h"

#include "mod_zb2.h"
#include "zb2/supplybox.h"
#include "zb2/teleportentity.h"
#include "zb2/zombiebomb.h"
#include "player/csdm_randomspawn.h"
#include "player/player_mod_strategy.h"
#include "player/player_zombie.h"

#include <algorithm>
#include <vector>
#include <gamemode/zb2/zb2_zclass.h>
#include <util/u_range.hpp>

namespace sv {

CMod_ZombieMod2::CMod_ZombieMod2() // precache
{
	UTIL_PrecacheOther("supplybox");

	PRECACHE_SOUND("zombi/zombi_box.wav");
	PRECACHE_SOUND("zombi/zombi_evolution.wav");
	PRECACHE_SOUND("zombi/zombi_evolution_female.wav");

	ZombieSkill_Precache();
	HumanSkill_Precache();
}

void CMod_ZombieMod2::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(MOD_ZB2);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxrounds.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

void CMod_ZombieMod2::Think()
{
	MakeSupplyboxThink();

	return CMod_Zombi::Think();
}

void CMod_ZombieMod2::PlayerSpawn(CBasePlayer *pPlayer)
{
	CMod_Zombi::PlayerSpawn(pPlayer);
}

void CMod_ZombieMod2::PlayerThink(CBasePlayer *pPlayer)
{
	return CMod_Zombi::PlayerThink(pPlayer);
}

void CMod_ZombieMod2::RestartRound()
{
	RemoveAllSupplybox();
	RemoveAllEntity();
	m_flTimeNextMakeSupplybox = gpGlobals->time + RandomDuration(30.0s, 60.0s);
	return CMod_Zombi::RestartRound();
}

BOOL CMod_ZombieMod2::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	return CMod_Zombi::ClientCommand(pPlayer, pcmd);
}

float CMod_ZombieMod2::GetAdjustedEntityDamage(CBaseEntity *victim, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = Base::GetAdjustedEntityDamage(victim, pevInflictor, pevAttacker, flDamage, bitsDamageType);

	CBasePlayer *pAttacker = dynamic_ent_cast<CBasePlayer *>(pevAttacker);
	if (pAttacker)
	{
		m_eventAdjustDamage.dispatch(pAttacker, flDamage);
	}

	return flDamage;
}

HitBoxGroup CMod_ZombieMod2::GetAdjustedTraceAttackHitgroup(CBaseEntity *victim, entvars_t * pevAttacker, float flDamage, const Vector & vecDir, TraceResult * ptr, int bitsDamageType)
{
	HitBoxGroup hbg = CMod_Zombi::GetAdjustedTraceAttackHitgroup(victim, pevAttacker, flDamage, vecDir, ptr, bitsDamageType);

	CBasePlayer *pAttacker = dynamic_ent_cast<CBasePlayer *>(pevAttacker);
	if (pAttacker)
	{
		m_eventAdjustHitgroup.dispatch(pAttacker, hbg);
	}

	return hbg;
}

void CMod_ZombieMod2::MakeSupplyboxThink()
{
	if (!FInfectionStarted())
		return;
	if (gpGlobals->time < m_flTimeNextMakeSupplybox)
		return;
	m_flTimeNextMakeSupplybox = gpGlobals->time + RandomDuration(20.0s, 30.0s);

	//RemoveAllSupplybox();

	int iSupplyboxCount = SupplyboxCount();
	for (int i = 0; i < iSupplyboxCount; ++i)
	{
		CSupplyBox *sb = CreateSupplybox();
		if (!sb) 
			continue;
		sb->m_iSupplyboxIndex = i + 1;

		for(CBasePlayer * player : moe::range::PlayersList())
		{
			if(player->m_bIsZombie)
				continue;

			MESSAGE_BEGIN(MSG_ALL, gmsgHostagePos, nullptr, player->pev);
			WRITE_BYTE(1);
			WRITE_BYTE(sb->m_iSupplyboxIndex);
			WRITE_COORD(sb->pev->origin.x);
			WRITE_COORD(sb->pev->origin.y);
			WRITE_COORD(sb->pev->origin.z);
			MESSAGE_END();
		}
	}

	UTIL_ClientPrintAll(HUD_PRINTCENTER, "[💰]强化武器补给箱已到达！！"); // #CSO_SupportItemSpawned

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CLIENT_COMMAND(entity->edict(), "spk zombi/zombi_box.wav\n");

	}

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_SUPPLYBOX_ARRIVE);
	}
}

int CMod_ZombieMod2::SupplyboxCount()
{
	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);
	int iSupplyboxCount = (NumAliveTerrorist + NumAliveCT + NumDeadTerrorist) / 10 + 3;
	return iSupplyboxCount;
}

void CMod_ZombieMod2::RemoveAllSupplybox()
{
	CBaseEntity *ent = nullptr;
	while ((ent = UTIL_FindEntityByClassname(ent, "supplybox")) != nullptr)
	{
		CSupplyBox *sb = dynamic_ent_cast<CSupplyBox *>(ent);
		sb->pev->effects |= EF_NODRAW;
		sb->pev->flags |= FL_KILLME;
		sb->SendPositionMsg();
		sb->SetThink(&CBaseEntity::SUB_Remove);
	}
}

void CMod_ZombieMod2::RemoveAllEntity()
{
	CBaseEntity* ent = nullptr;
	while ((ent = UTIL_FindEntityByClassname(ent, "teleportentity")) != nullptr)
	{
		ent->pev->effects |= EF_NODRAW;
		ent->pev->flags |= FL_KILLME;
		ent->SetThink(&CBaseEntity::SUB_Remove);
	}

	while ((ent = UTIL_FindEntityByClassname(ent, "zombiebomb")) != nullptr)
	{
		ent->pev->effects |= EF_NODRAW;
		ent->pev->flags |= FL_KILLME;
		ent->SetThink(&CBaseEntity::SUB_Remove);
	}

	while ((ent = UTIL_FindEntityByClassname(ent, "zombietrap")) != nullptr)
	{
		ent->pev->effects |= EF_NODRAW;
		ent->pev->flags |= FL_KILLME;
		ent->SetThink(&CBaseEntity::SUB_Remove);
		ent->SetTouch(NULL);
	}

	while ((ent = UTIL_FindEntityByClassname(ent, "zombiepile")) != nullptr)
	{
		ent->pev->effects |= EF_NODRAW;
		ent->pev->flags |= FL_KILLME;
		ent->SetThink(&CBaseEntity::SUB_Remove);
		ent->SetTouch(NULL);
	}
}

CSupplyBox *CMod_ZombieMod2::CreateSupplybox()
{
	auto supplybox = CreateClassPtr<CSupplyBox>();

	Vector backup_v_angle = supplybox->pev->v_angle;
	CSDM_DoRandomSpawn(supplybox);
	supplybox->pev->v_angle = backup_v_angle;

	supplybox->pev->spawnflags |= SF_NORESPAWN;

	DispatchSpawn(supplybox->edict());
	return supplybox;
}

void CMod_ZombieMod2::HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker)
{
	CMod_Zombi::HumanInfectionByZombie(player, attacker);
	m_eventInfection.dispatch(player, attacker);
}

int CMod_ZombieMod2::AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
{
	CBaseEntity* pEntity = CBaseEntity::Instance(ent);
	if (pEntity && pEntity->IsPlayer())
	{
		CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);	
		pPlayer->m_pModStrategy->AddToFullPack_Post(state, e, ent, host, hostflags, player, pSet);
	}
	return 1;
}

void CMod_ZombieMod2::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZB2(player, this));
}

CPlayerModStrategy_ZB2::CPlayerModStrategy_ZB2(CBasePlayer *player, CMod_ZombieMod2 *mp)
	:	CPlayerModStrategy_ZB1(player, mp),
		m_eventListeners {
			mp->m_eventInfection.subscribe(&CPlayerModStrategy_ZB2::Event_OnInfection, this),
			mp->m_eventAdjustDamage.subscribe([this](CBasePlayer *attacker, float &out){ return this->Event_AdjustHumanDamage(attacker, out); }),
			mp->m_eventAdjustHitgroup.subscribe([this](CBasePlayer *attacker, HitBoxGroup &out) { return this->Event_AdjustHumanHitgroup(attacker, out); })
		} ,
		m_pModZB2(mp)
{}

bool CPlayerModStrategy_ZB2::ClientCommand(const char *pcmd)
{
	if (!Q_stricmp(pcmd, "openselector"))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
		WRITE_BYTE(ZB2_MESSAGE_ZOMBIESELECTOR); // TYPE
		WRITE_COORD(20.0);
		MESSAGE_END();
	}

	if (m_pPlayer->m_bIsZombie)
	{
		//判断字符串是否含有指定目标，若返回NULL则没有
		if (Q_strstr(pcmd, "BTE_Zb_Select_Zombie") != NULL)
		{
			int iCmd = 0;
			char pCmdString[32];

			//(将字符串里的字母和下划线去掉)仅保留数字.
			sscanf(pcmd, "%*[A-Za-z_]%31[0-9]", pCmdString);
			//将（string）“数字”转为（int） 数字
			iCmd = StrToInt(pCmdString);

			int iZombieType = 0;

			//如果选择的是随机  iZombieType = -1 ，否则 iZombieType = 人物序号（参见zb2_const.h）
			if (iCmd <= ((int)ZOMBIE_CLASS_BOOMER))
			{
				iZombieType = (iCmd == int((ZOMBIE_CLASS_BOOMER))) ? -1 : iCmd;
			}
			//如果是随机选择
			if (iZombieType == -1)
			{
				m_pPlayer->m_ZombieClass = "random";
				auto RandomZombie = ZombieClassFactory(m_pPlayer->m_iZombieLevel, m_pPlayer->m_ZombieClass);

				m_pCharacter_ZB2 = RandomZombie;
				m_pCharacter = RandomZombie;
				RandomZombie->InitHUD();
			}
			//如果不是随机，根据用户选定的 ZombieClassType 找到对应的僵尸名
			else
			{
				m_pPlayer->m_ZombieClass = ZombieNameByClass((ZombieClassType)iZombieType);
				auto AbsZombie = ZombieClassFactory(m_pPlayer->m_iZombieLevel, m_pPlayer->m_ZombieClass);

				m_pCharacter_ZB2 = AbsZombie;
				m_pCharacter = AbsZombie;
				AbsZombie->InitHUD();
			}

			float flMaxHealth = m_pPlayer->pev->max_health;

			m_pPlayer->pev->sequence = 0;
			m_pPlayer->m_iCustomSequence = -1;

			//如果使用这个函数，会导致无敌时间叠加
			//BecomeZombie(m_pPlayer->m_iZombieLevel);

			if (m_pPlayer->m_pActiveItem)
			{
				m_pPlayer->SwitchWeapon(m_pPlayer->m_pActiveItem);
			}

			m_pPlayer->pev->max_health = flMaxHealth;
			m_pPlayer->pev->health = flMaxHealth;

			return true;
		}
		if (!Q_stricmp(pcmd, "BTE_ZombieSkill1"))
		{
			if (CanUseZombieSkill())
				m_pCharacter_ZB2->ActivateSkill(SKILL_SLOT_1);
			return true;
		}

		if (!Q_stricmp(pcmd, "BTE_ZombieSkill2"))
		{
			if (CanUseZombieSkill())
				m_pCharacter_ZB2->ActivateSkill(SKILL_SLOT_2);
			return true;
		}
	}
	else
	{
		if (!Q_stricmp(pcmd, "MoE_HumanSkill1"))
		{
			m_pCharacter_ZB2->ActivateSkill(SKILL_SLOT_1);
			return true;
		}
		else if (!Q_stricmp(pcmd, "MoE_HumanSkill2"))
		{
			m_pCharacter_ZB2->ActivateSkill(SKILL_SLOT_2);
			return true;
		}
		else if (!Q_stricmp(pcmd, "MoE_HumanSkill3"))
		{
			m_pCharacter_ZB2->ActivateSkill(SKILL_SLOT_3);
			return true;
		}
		else if (!Q_stricmp(pcmd, "MoE_HumanSkill4"))
		{
			m_pCharacter_ZB2->ActivateSkill(SKILL_SLOT_4);
			return true;
		}
	}

	return false;
}

void CPlayerModStrategy_ZB2::OnSpawn()
{
	UpdatePlayerEvolutionHUD();

	return CPlayerModStrategy_ZB1::OnSpawn();
}

void CPlayerModStrategy_ZB2::OnThink()
{
	Zombie_HealthRecoveryThink();
	return CPlayerModStrategy_ZB1::OnThink();
}

bool CPlayerModStrategy_ZB2::CanUseZombieSkill()
{
	return m_pPlayer->m_bIsZombie && m_pPlayer->m_iZombieLevel != ZOMBIE_LEVEL_HOST;
}

void CPlayerModStrategy_ZB2::Zombie_HealthRecoveryThink()
{
	if (!m_pPlayer->IsAlive() || !m_pPlayer->m_bIsZombie)
		return;

#if 0
	if (m_pPlayer->pev->button & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))	//old rule
	{
		m_flTimeNextZombieHealthRecovery = gpGlobals->time + 3.0s;
		return;
	}
#endif
	// cannot recover during using zombie skill.
	const float flRecoverValue = m_pCharacter_ZB2->HealthRecoveryAmount();
	if (flRecoverValue <= 0.0f)
		return;

	if (gpGlobals->time > m_flTimeNextZombieHealthRecovery)
	{
		if (m_pPlayer->pev->max_health != m_pPlayer->pev->health)
		{

			m_flTimeNextZombieHealthRecovery = gpGlobals->time + 1.0s;
			m_pPlayer->pev->health = std::min(m_pPlayer->pev->max_health, m_pPlayer->pev->health + flRecoverValue);

			// effects
			if (m_pPlayer->m_ZombieClass)
			{
				if (!stricmp(m_pPlayer->m_ZombieClass, "speed") || !stricmp(m_pPlayer->m_ZombieClass, "booster") || !stricmp(m_pPlayer->m_ZombieClass, "flying") || !stricmp(m_pPlayer->m_ZombieClass, "resident") || !stricmp(m_pPlayer->m_ZombieClass, "witch") || !stricmp(m_pPlayer->m_ZombieClass, "spider") || !stricmp(m_pPlayer->m_ZombieClass, "siren"))
					CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/zombi_heal_female.wav\n");
				else if (!stricmp(m_pPlayer->m_ZombieClass, "teleport"))
					CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/zombi_heal_teleport.wav\n");
				else if (!stricmp(m_pPlayer->m_ZombieClass, "revival"))
					CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/zombi_heal_revival.wav\n");
				else
					CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/zombi_heal.wav\n");
			}

			MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
			WRITE_BYTE(ZB2_MESSAGE_HEALTH_RECOVERY);
			MESSAGE_END();
		}
	}
}

void CPlayerModStrategy_ZB2::BecomeZombie(ZombieLevel iEvolutionLevel)
{
	auto sp = ZombieClassFactory(iEvolutionLevel, m_pPlayer->m_ZombieClass);
	m_pCharacter_ZB2 = sp;
	m_pCharacter = sp;
	
	if (enhancedzombie.value)
	{
		switch (iEvolutionLevel)
		{
		case ZOMBIE_LEVEL_HOST:
			m_pPlayer->pev->health = m_pPlayer->pev->max_health = 15000;
			break;
		case ZOMBIE_LEVEL_ORIGIN:
			m_pPlayer->pev->health = m_pPlayer->pev->max_health = 20000;
			break;
		case ZOMBIE_LEVEL_ORIGIN_LV2:
			m_pPlayer->pev->health = m_pPlayer->pev->max_health = 25000;
			break;
		default:
			m_pPlayer->pev->health = m_pPlayer->pev->max_health = 4000;
			break;
		}
		
	}

	// fix crash
	if (m_pPlayer->m_pActiveItem)
	{
		m_pPlayer->SwitchWeapon(m_pPlayer->m_pActiveItem);
	}

	m_pPlayer->m_tLockBuffTime = invalid_time_point;
	m_pPlayer->m_flLockSpeed = 0.0;
	m_pPlayer->m_flLockGravity = 0.0;
	sp->InitHUD();
	sp->ResetMaxSpeed();

	m_iZombieInfections = 0;
	UpdatePlayerEvolutionHUD();

	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_ZOMBIESELECTOR); // TYPE
	WRITE_COORD(20.0);
	MESSAGE_END();
}

void CPlayerModStrategy_ZB2::BecomeHuman()
{
	auto sp = std::make_shared<CHuman_ZB2>(m_pPlayer);
	m_pCharacter_ZB2 = sp;
	m_pCharacter = sp;

	sp->InitHUD();
	sp->ResetMaxSpeed();
}

void CPlayerModStrategy_ZB2::Event_OnInfection(CBasePlayer *victim, CBasePlayer *attacker)
{
	if (m_pPlayer != attacker)
		return;

	if (victim == attacker)
		return;

	m_iZombieInfections++;

	CheckEvolution();

	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, victim->pev);
	WRITE_BYTE(ZB2_MESSAGE_ZOMBIESELECTOR); // TYPE
	WRITE_COORD(20.0);
	MESSAGE_END();
}

void CPlayerModStrategy_ZB2::Event_AdjustHumanDamage(CBasePlayer * attacker, float &flDamage)
{
	if (attacker != m_pPlayer)
		return;

	if (!m_pCharacter_ZB2)
		return;

	if (m_pPlayer->m_iBuff & BUFF_SHOOTINGDOWN)
		flDamage *= 0.5;

	if (!m_pPlayer->m_bIsZombie && m_pCharacter_ZB2->GetSkillStatus(SKILL_SLOT_3) == SKILL_STATUS_USING)
	{
		// using knife 2x?
		if (m_pPlayer->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
			flDamage *= 2.0f;
	}
}

void CPlayerModStrategy_ZB2::Event_AdjustHumanHitgroup(CBasePlayer * attacker, HitBoxGroup & iHitgroup)
{
	if (attacker != m_pPlayer)
		return;

	if (!m_pCharacter_ZB2)
		return;

	if (!m_pPlayer->m_bIsZombie && m_pCharacter_ZB2->GetSkillStatus(SKILL_SLOT_2) == SKILL_STATUS_USING)
	{
		if (m_pPlayer->m_pActiveItem->iItemSlot() != KNIFE_SLOT)
			iHitgroup = HITGROUP_HEAD;
	}
}

void CPlayerModStrategy_ZB2::Pain(int m_LastHitGroup, bool HasArmour)
{
	if (m_pPlayer->m_ZombieClass)
	{
		if(!stricmp(m_pPlayer->m_ZombieClass, "revival") && (m_pPlayer->m_iBuff) & BUFF_IGNORE_KNOCKBACK)
			return CPlayerModStrategy_ZB1::Pain(m_LastHitGroup, HasArmour);
		else
			m_flTimeNextZombieHealthRecovery = gpGlobals->time + 3.0s;
	}
	return CPlayerModStrategy_ZB1::Pain(m_LastHitGroup, HasArmour);
}

void CPlayerModStrategy_ZB2::UpdatePlayerEvolutionHUD()
{
	int iCount = 0;
	if (m_pPlayer->m_bIsZombie && m_pPlayer->m_iZombieLevel != ZOMBIE_LEVEL_ORIGIN_LV2)
	{
		int iInfectionRequired = m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 3 : 5;
		iCount = iInfectionRequired - m_iZombieInfections;
	}

	char buf[16];
	Q_snprintf(buf, ARRAYSIZE(buf), "hostage%d", iCount);

	if (iCount)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgScenarioIcon, nullptr, m_pPlayer->pev);
		WRITE_BYTE(1);
		WRITE_STRING(buf);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgScenarioIcon, nullptr, m_pPlayer->pev);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
}

void CPlayerModStrategy_ZB2::CheckEvolution()
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST && m_iZombieInfections >= 3)
	{
		Evolution(ZOMBIE_LEVEL_ORIGIN);
	}

	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN && m_iZombieInfections >= 5)
	{
		Evolution(ZOMBIE_LEVEL_ORIGIN_LV2);
	}

	UpdatePlayerEvolutionHUD();
}

void CPlayerModStrategy_ZB2::Evolution(ZombieLevel iEvolutionLevel)
{
	m_iZombieInfections = 0;
	if (m_pCharacter_ZB2)
		m_pCharacter_ZB2->Evolution(iEvolutionLevel);

	if (iEvolutionLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 7000.0f;
		m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
		m_pPlayer->pev->armorvalue = 500.0f;
	}
	else if (iEvolutionLevel == ZOMBIE_LEVEL_ORIGIN_LV2)
	{
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 14000.0f;
		m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
		m_pPlayer->pev->armorvalue = 1000.0f;
	}

	if (m_pPlayer->m_ZombieClass)
	{
		if (!stricmp(m_pPlayer->m_ZombieClass, "speed") || !stricmp(m_pPlayer->m_ZombieClass, "booster") || !stricmp(m_pPlayer->m_ZombieClass, "teleport") || !stricmp(m_pPlayer->m_ZombieClass, "flying") || !stricmp(m_pPlayer->m_ZombieClass, "spider") || !stricmp(m_pPlayer->m_ZombieClass, "siren"))
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, "zombi/zombi_evolution_female.wav", VOL_NORM, ATTN_NORM);
		else
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_BODY, "zombi/zombi_evolution.wav", VOL_NORM, ATTN_NORM);
	}
}

BOOL CPlayerModStrategy_ZB2::DeployWeapon(CBasePlayerItem *item)
{
	BOOL ret = CPlayerModStrategy_Default::DeployWeapon(item);
	if (!m_pCharacter_ZB2)
		return ret;

	m_pCharacter_ZB2->OnWeaponDeploy(item);

	return ret;
}

int CPlayerModStrategy_ZB2::AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
{
	if (!m_pCharacter_ZB2)
		return CPlayerModStrategy_ZB1::AddToFullPack_Post(state, e, ent, host, hostflags, player, pSet);

	return m_pCharacter_ZB2->AddToFullPack_Post(state, e, ent, host, hostflags, player, pSet);
}

void CPlayerModStrategy_ZB2::OnTouch(CBaseEntity* other)
{
	if (m_pPlayer->m_bIsZombie && m_pPlayer->m_ZombieClass && !stricmp(m_pPlayer->m_ZombieClass, "meatwall") && m_pCharacter_ZB2->GetSkillStatus(SKILL_SLOT_1) == SKILL_STATUS_USING)
	{
		m_pPlayer->m_iBuff |= BUFF_RUSHENTITY;
	}

	if (m_pPlayer->m_bIsZombie && m_pPlayer->m_ZombieClass && !stricmp(m_pPlayer->m_ZombieClass, "boomer") && m_pCharacter_ZB2->GetSkillStatus(SKILL_SLOT_1) == SKILL_STATUS_USING)
	{
		if (!other->IsPlayer())
			return;
		if (!other->IsAlive())
			return;
		CBasePlayer* p = static_cast<CBasePlayer*>(other);
		if (p->m_bIsZombie)
			return;

		if (m_pPlayer->IsAlive())
		{
			m_pPlayer->m_LastHitGroup = HITGROUP_GENERIC;
			m_pPlayer->Killed(m_pPlayer->pev, 1);
		}
	}
	return CPlayerModStrategy_Default::OnTouch(other);
}

}
