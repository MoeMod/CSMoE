#include <memory>

/*
zb2_zclass_deimos2.cpp - CSMoE Gameplay server : Zombie Mod 2
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

#include "zb2_zclass_spider.h"

namespace sv {

CZombieClass_Spider::CZombieClass_Spider(CBasePlayer* player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "spider";
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieWebShooter>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieWebBomb>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 20000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 9000;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Spider::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char *szModel = "zombi_spider";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Spider::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_SPIDER);
	WRITE_BYTE(ZOMBIE_SKILL_WEBSHOOTER);
	WRITE_BYTE(ZOMBIE_SKILL_WEBBOMB);
	MESSAGE_END();
}

void CZombieClass_Spider::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 310;
	m_pPlayer->m_flDefaultGravity = m_pPlayer->pev->gravity = 0.55f;

	if (m_tStunGravityTime > gpGlobals->time)
	{
		m_pPlayer->pev->gravity = max(m_flStunGravity, 1.0);
	}

	if (m_tStunSpeedTime > gpGlobals->time)
	{
		m_pPlayer->pev->maxspeed = min(m_flStunSpeed, 100.0);
	}
}

float CZombieClass_Spider::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	flDamage *= 0.9;
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Spider::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 1.2f,
			kbd.flNotOnGround * 1.8f,
			kbd.flFlying * 1.8f,
			kbd.flDucking * 1.2f,
			kbd.flVelocityModifier,
	});
}

void CZombieClass_Spider::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/spider_hurt1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/spider_hurt2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_Spider::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/spider_death1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/spider_death2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_Spider::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombispider.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_spider.mdl");
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_spider.mdl");
			break;
		}
		default:
			break;
	}
}


}
