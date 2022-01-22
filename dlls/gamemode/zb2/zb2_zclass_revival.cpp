#include <memory>

/*
zb2_zclass_tank.cpp - CSMoE Gameplay server : Zombie Mod 2
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

#include "player/player_zombie.h"

#include "zb2_zclass_revival.h"

namespace sv {

CZombieClass_Revival::CZombieClass_Revival(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "revival";
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieRevival>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 200;
	m_pPlayer->pev->gravity = 0.8f;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Revival::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char* szModel = iEvolutionLevel ? "revival_zombi_origin" : "revival_zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Revival::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, {}, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_REVIVAL);
	WRITE_BYTE(ZOMBIE_SKILL_REVIVAL);
	MESSAGE_END();
}

void CZombieClass_Revival::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 290;
	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed();
}

float CZombieClass_Revival::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	flDamage *= 0.5;
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Revival::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& kbd)
{
	// knife knockback

	/*if (attacker->m_pActiveItem && attacker->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
	{
		//return CBaseZombieClass_ZB2::ApplyKnockback(attacker, { 700, 1600, 1300, 400, 1.0f });
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, attacker->m_pActiveItem->GetKnockBackData());
		
	}*/

	m_iRevivalSkillStatus = m_pZombieSkill->GetStatus();
	if (m_iRevivalSkillStatus == SKILL_STATUS_USING)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f,
			});

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.07f,
			kbd.flNotOnGround * 0.28f,
			kbd.flFlying * 0.25f,
			kbd.flDucking * 0.07f,
			kbd.flVelocityModifier * 0.97f,
	});
}

void CZombieClass_Revival::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_revival_hurt.wav", VOL_NORM, ATTN_NORM);
}

void CZombieClass_Revival::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_revival_death1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_revival_death2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_Revival::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombirevival_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombirevival.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_revival_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_revival.mdl");
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_revival_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_revival.mdl");
			break;
		}
		default:
			break;
	}
}


}
