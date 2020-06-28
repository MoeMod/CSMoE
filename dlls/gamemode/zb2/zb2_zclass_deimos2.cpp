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

#include "zb2_zclass_deimos2.h"

namespace sv {

CZombieClass_Deimos2::CZombieClass_Deimos2(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieCrazy2>(m_pPlayer);

	const char *szModel = iEvolutionLevel ? "deimos2_zombi_origin" : "deimos_zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 200;
	m_pPlayer->pev->gravity = 0.72f;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Deimos2::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_DEIMOS2);
	WRITE_BYTE(ZOMBIE_SKILL_CRAZY2);
	MESSAGE_END();
}

void CZombieClass_Deimos2::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 300;
	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed();
}

float CZombieClass_Deimos2::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	flDamage *= 0.9;
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Deimos2::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	// knife knockback
	m_iCrazySkillStatus = m_pZombieSkill->GetStatus();
	if (m_iCrazySkillStatus == SKILL_STATUS_USING)
	{
		switch (m_pPlayer->m_iZombieLevel)
		{
		case ZOMBIE_LEVEL_HOST: return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.4f,
			kbd.flNotOnGround * 0.4f,
			kbd.flFlying * 0.4f,
			kbd.flDucking * 0.4f,
			kbd.flVelocityModifier * float(0.01),
			});break;
		case ZOMBIE_LEVEL_ORIGIN: return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.2f,
			kbd.flNotOnGround * 0.2f,
			kbd.flFlying * 0.2f,
			kbd.flDucking * 0.2f,
			kbd.flVelocityModifier * float(0.01),
			}); break;
		case ZOMBIE_LEVEL_ORIGIN_LV2: return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.2f,
			kbd.flNotOnGround * 0.2f,
			kbd.flFlying * 0.2f,
			kbd.flDucking * 0.2f,
			kbd.flVelocityModifier * float(0.01),
			}); break;
		}
	}

	if(attacker->m_pActiveItem && attacker->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, { 700, 1600, 1300, 400, 1.0f });

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

void CZombieClass_Deimos2::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_01.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_02.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_Deimos2::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_Deimos2::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombideimos_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombideimos2.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb-deimos_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb-deimos2.mdl");
			break;
		}
		default:
			break;
	}
}


}
