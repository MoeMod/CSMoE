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

#include "zb2_zclass_boomer.h"

namespace sv {

CZombieClass_Boomer::CZombieClass_Boomer(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "boomer";
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieSelfDestruct>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieArmorRecovery>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set boomer property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 4000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 1000;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Boomer::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char* szModel = iEvolutionLevel ? "boomer_zombi_origin" : "boomer_zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Boomer::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_BOOMER);
	WRITE_BYTE(ZOMBIE_SKILL_SELFDESTRUCT);
	WRITE_BYTE(ZOMBIE_SKILL_ARMORRECOVERY);
	MESSAGE_END();
}

void CZombieClass_Boomer::ResetMaxSpeed() const
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		m_pPlayer->pev->maxspeed = 265;
		m_pPlayer->pev->gravity = 0.9f;
	}
	else if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->maxspeed = 313;
		m_pPlayer->pev->gravity = 0.8f;
	}
	else
	{
		m_pPlayer->pev->maxspeed = 328;
		m_pPlayer->pev->gravity = 0.7f;
	}

	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed();

	if (m_tStunGravityTime > gpGlobals->time)
	{
		m_pPlayer->pev->gravity = max(m_flStunGravity, 1.0);
	}

	if (m_tStunSpeedTime > gpGlobals->time)
	{
		m_pPlayer->pev->maxspeed = min(m_flStunSpeed, 100.0);
	}
}

float CZombieClass_Boomer::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	if (m_pZombieSkill->GetStatus() != SKILL_STATUS_USING)
		flDamage *= 0.6;
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Boomer::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& kbd)
{

	if (m_pZombieSkill->GetStatus() == SKILL_STATUS_USING)
	{
		if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
			return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
				kbd.flOnGround * 0.7f,
				kbd.flNotOnGround * 0.7f,
				kbd.flFlying * 0.7f,
				kbd.flDucking * 0.7f,
				Q_max(kbd.flVelocityModifier, 0.55f),
				});

		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
				kbd.flOnGround * 0.7f * 0.7f,
				kbd.flNotOnGround * 0.75f * 0.7f,
				kbd.flFlying * 0.75f * 0.7f,
				kbd.flDucking * 0.7f * 0.7f,
				Q_max(kbd.flVelocityModifier, 0.55f),
			});
	}

	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
				kbd.flOnGround,
				kbd.flNotOnGround,
				kbd.flFlying,
				kbd.flDucking,
				kbd.flVelocityModifier * 0.99f,
			});
	}

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.7f,
			kbd.flNotOnGround * 0.75f,
			kbd.flFlying * 0.75f,
			kbd.flDucking * 0.7f,
			kbd.flVelocityModifier * 0.99f,
	});
}

void CZombieClass_Boomer::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/boomer_hurt1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/boomer_hurt2.wav", VOL_NORM, ATTN_NORM); break;
		boomer:break;
	}
}

void CZombieClass_Boomer::DeathSound_Zombie()
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/boomer_death.wav", VOL_NORM, ATTN_NORM);
}


void CZombieClass_Boomer::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombiboomer.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_boomer.mdl");
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_boomer.mdl");
			break;
		}
		default:
			break;
	}
}


}
