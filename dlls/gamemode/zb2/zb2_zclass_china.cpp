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

#include "zb2_zclass_china.h"

namespace sv {

CZombieClass_China::CZombieClass_China(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "china";
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieStiffen>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieStrengthRecovery>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set china property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 200;
	m_pPlayer->pev->gravity = 0.7f;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_China::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char* szModel = iEvolutionLevel ? "china_zombi_origin" : "china_zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_China::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_CHINA);
	WRITE_BYTE(ZOMBIE_SKILL_STIFFEN);
	WRITE_BYTE(ZOMBIE_SKILL_STRENGTHRECOVERY);
	MESSAGE_END();
}

void CZombieClass_China::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 285;
}

float CZombieClass_China::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	if (m_pPlayer->m_iZombieLevel != ZOMBIE_LEVEL_HOST)
		flDamage *= 0.9;
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_China::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& kbd)
{
	// knife knockback

	/*if (attacker->m_pActiveItem && attacker->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
	{
		//return CBaseZombieClass_ZB2::ApplyKnockback(attacker, { 700, 1600, 1300, 400, 1.0f });
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, attacker->m_pActiveItem->GetKnockBackData());
		
	}*/

	if(m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, kbd);

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.7f,
			kbd.flNotOnGround * 0.75f,
			kbd.flFlying * 0.75f,
			kbd.flDucking * 0.7f,
			kbd.flVelocityModifier,
	});
}

void CZombieClass_China::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_chinese_hurt.wav", VOL_NORM, ATTN_NORM);
}

void CZombieClass_China::DeathSound_Zombie()
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_chinese_death.wav", VOL_NORM, ATTN_NORM);
}


void CZombieClass_China::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombichina.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		case WEAPON_SMOKEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_china.mdl");
			break;
		}
		china:
			break;
	}
}


}
