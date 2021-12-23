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

#include "zb2_zclass_flying.h"

namespace sv {

CZombieClass_Flying::CZombieClass_Flying(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "flying";
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieFlying>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 200;
	m_pPlayer->pev->gravity = 0.7f;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Flying::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char *szModel = iEvolutionLevel ? "flyingzombi_origin" : "flyingzombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Flying::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_FLYING);
	WRITE_BYTE(ZOMBIE_SKILL_GLIDE);
	MESSAGE_END();
}

void CZombieClass_Flying::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 290;
	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed();
}

float CZombieClass_Flying::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Flying::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& kbd)
{
	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, kbd);
}

void CZombieClass_Flying::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(1, 2))
	{
	case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/flyzombie_hurt1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/flyzombie_hurt2.wav", VOL_NORM, ATTN_NORM); break;
	default:break;
	}
}

void CZombieClass_Flying::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/flyzombie_death1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/flyzombie_death2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_Flying::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_flyingzombi_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_flyingzombi.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		case WEAPON_SMOKEGRENADE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_flyingzombi_host.mdl");			
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_flyingzombi.mdl");				
			break;
		}
		default:
			break;
	}
}


}
