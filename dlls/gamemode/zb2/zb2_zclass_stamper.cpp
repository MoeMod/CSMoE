#include <memory>

/*
zb2_zclass_stamper.cpp - CSMoE Gameplay server : Zombie Mod 2
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

#include "zb2_zclass_stamper.h"

namespace sv {

CZombieClass_Stamper::CZombieClass_Stamper(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "stamper";
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombiePile>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieArmorRecovery>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 800;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 200;
	m_pPlayer->pev->gravity = 0.8f;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Stamper::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char* szModel = iEvolutionLevel ? "stamper_zombi_origin" : "stamper_zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Stamper::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_STAMPER);
	WRITE_BYTE(ZOMBIE_SKILL_STAMPER);
	WRITE_BYTE(ZOMBIE_SKILL_ARMORRECOVERY);
	MESSAGE_END();
}

void CZombieClass_Stamper::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 270;
}

float CZombieClass_Stamper::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
		flDamage *= 1.2;
	else
		flDamage *= 1.1;
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Stamper::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	// knife knockback
	//if (attacker->m_pActiveItem && attacker->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
	//	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, { 550, 750, 400, 300, 0.85f });

	if(m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.35f,
			kbd.flNotOnGround * 0.75f,
			kbd.flFlying * 0.75f,
			kbd.flDucking * 0.35f,
			kbd.flVelocityModifier * 0.95f,
		});
	}

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.25f,
			kbd.flNotOnGround * 0.65f,
			kbd.flFlying * 0.65f,
			kbd.flDucking * 0.25f,
			kbd.flVelocityModifier * 0.95f,
	});
}

void CZombieClass_Stamper::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_stamper_1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_stamper_2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_Stamper::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_stamper_1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_stamper_2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_Stamper::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombiundertaker.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		case WEAPON_SMOKEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_undertaker.mdl");
			break;
		}
		default:
			break;
	}
}

}
