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

#include "zb2_zclass_meatwall.h"

namespace sv {

CZombieClass_MeatWall::CZombieClass_MeatWall(CBasePlayer* player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "meatwall";
	m_pPlayer->m_iBuff |= BUFF_IGNORE_KNOCKBACK;
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieDogShoot>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieRush>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 20000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 9000;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_MeatWall::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char *szModel = "zombi_meatwall";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_MeatWall::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_MEATWALL);
	WRITE_BYTE(ZOMBIE_SKILL_DOGSHOOT);
	WRITE_BYTE(ZOMBIE_SKILL_RUSH);
	MESSAGE_END();
}

void CZombieClass_MeatWall::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 170.0;

	m_pPlayer->m_flDefaultGravity = m_pPlayer->pev->gravity = 0.7f;

	if (m_pPlayer->pev->velocity.Length2D() < 170.0f)
		m_pPlayer->pev->maxspeed = 225.0f;

	if (m_pZombieSkill2)
		m_pZombieSkill2->ResetMaxSpeed();
}

float CZombieClass_MeatWall::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	flDamage *= 0.5;
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_MeatWall::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
			});
}

void CZombieClass_MeatWall::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/meatwallzombie_hurt1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/meatwallzombie_hurt2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_MeatWall::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/meatwallzombie_death1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/meatwallzombie_death2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_MeatWall::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombimeatwall.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_meatwall.mdl");
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_meatwall.mdl");
			break;
		}
		default:
			break;
	}
}


}
