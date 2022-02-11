#include <memory>

/*
zb2_zclass_resident.cpp - CSMoE Gameplay server : Zombie Mod 2
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

#include "zb2_zclass_witch.h"

namespace sv {

CZombieClass_Witch::CZombieClass_Witch(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "witch";
	
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieHook>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieJumpUpM>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 1000;

	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Witch::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char* szModel = iEvolutionLevel ? "witch_zombi_origin" : "witch_zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Witch::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_WITCH);
	WRITE_BYTE(ZOMBIE_SKILL_HOOK);
	WRITE_BYTE(ZOMBIE_SKILL_JUMPUPM);
	MESSAGE_END();
}

void CZombieClass_Witch::ResetMaxSpeed() const
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		m_pPlayer->pev->maxspeed = 290;
		m_pPlayer->pev->gravity = 0.7f;
	}
	else if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->maxspeed = 319;
		m_pPlayer->pev->gravity = 0.65f;
	}
	else
	{
		m_pPlayer->pev->maxspeed = 333;
		m_pPlayer->pev->gravity = 0.6f;
	}

	if (m_pZombieSkill2)
		m_pZombieSkill2->ResetMaxSpeed();

	if (m_tStunGravityTime > gpGlobals->time)
	{
		m_pPlayer->pev->gravity = 1.0f;
	}

	if (m_tStunSpeedTime > gpGlobals->time)
	{
		m_pPlayer->pev->maxspeed = 100.0f;
	}
}

float CZombieClass_Witch::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	flDamage *= 1.2;

	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Witch::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	// knife knockback

	/*if (attacker->m_pActiveItem && attacker->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
	{
		//return CBaseZombieClass_ZB2::ApplyKnockback(attacker, { 700, 1600, 1300, 400, 1.0f });
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, attacker->m_pActiveItem->GetKnockBackData());
	}*/

	if(m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 1.1f,
			kbd.flNotOnGround * 1.1f,
			kbd.flFlying * 1.1f,
			kbd.flDucking * 1.1f,
			kbd.flVelocityModifier * 0.85f,
			});

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 1.0f,
			kbd.flNotOnGround * 0.9f,
			kbd.flFlying * 0.75f,
			kbd.flDucking * 0.9f,
			kbd.flVelocityModifier * 0.85f,
	});
}

void CZombieClass_Witch::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_banshee_1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_banshee_2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_Witch::DeathSound_Zombie()
{
	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_banshee_1.wav", VOL_NORM, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_banshee_2.wav", VOL_NORM, ATTN_NORM); break;
	default:break;
	}
}


void CZombieClass_Witch::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombiewitch.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_witch.mdl");
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_witch.mdl");
			break;
		}
		default:
			break;
	}
}


}
