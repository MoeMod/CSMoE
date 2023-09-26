#include <memory>

/*
zb2_zclass_zombie_licker.cpp - CSMoE Gameplay server : Zombie Mod 2
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

#include "zp_zclass_licker.h"

namespace sv {


CZombieClass_Licker::CZombieClass_Licker(CBasePlayer* player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_iZombieClass = ZOMBIE_CLASS_LICKER;
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieHide>(m_pPlayer);

	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 1500;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 500;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Licker::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char* szModel = iEvolutionLevel ? "zombie_licker1" : "zombie_licker2";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Licker::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_LICKER);
	WRITE_BYTE(ZOMBIE_SKILL_HIDE);
	WRITE_BYTE(ZOMBIE_SKILL_EMPTY);
	MESSAGE_END();
}

void CZombieClass_Licker::ResetMaxSpeed() const
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		m_pPlayer->pev->maxspeed = 280;
		m_pPlayer->pev->gravity = 0.75f;
	}
	else if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->maxspeed = 285;
		m_pPlayer->pev->gravity = 0.7f;
	}
	else
	{
		m_pPlayer->pev->maxspeed = 280;
		m_pPlayer->pev->gravity = 0.7f;
	}

	m_pPlayer->m_flDefaultGravity = m_pPlayer->pev->gravity;

	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed();

	if (m_tStunGravityTime > gpGlobals->time)
	{
		m_pPlayer->pev->gravity = max(m_flStunGravity, 0.8);
	}

	if (m_tStunSpeedTime > gpGlobals->time)
	{
		m_pPlayer->pev->maxspeed = min(m_flStunSpeed, 100.0);
	}
}

float CZombieClass_Licker::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Licker::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& kbd)
{
	// knife knockback

	/*if (attacker->m_pActiveItem && attacker->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
	{
		//return CBaseZombieClass_ZB2::ApplyKnockback(attacker, { 700, 1600, 1300, 400, 1.0f });
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, attacker->m_pActiveItem->GetKnockBackData());
		
	}*/
/*
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
*/
	if(m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 1.2f,
			kbd.flNotOnGround * 1.2f,
			kbd.flFlying * 0.5f,
			kbd.flDucking * 0.4f,
			kbd.flVelocityModifier,
	});

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 1.0f,
			kbd.flNotOnGround * 1.0f,
			kbd.flFlying * 0.4f,
			kbd.flDucking * 0.3f,
			kbd.flVelocityModifier,
	});
}

void CZombieClass_Licker::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 5))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_pain1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_pain2.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_pain3.wav", VOL_NORM, ATTN_NORM); break;
		case 3: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_pain4.wav", VOL_NORM, ATTN_NORM); break;
		case 4: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_pain5.wav", VOL_NORM, ATTN_NORM); break;
		case 5: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_pain6.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_Licker::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 5))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_die1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_die2.wav", VOL_NORM, ATTN_NORM); break;
		case 3: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_die3.wav", VOL_NORM, ATTN_NORM); break;
		case 4: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_die4.wav", VOL_NORM, ATTN_NORM); break;
		case 5: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombie_die5.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_Licker::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/zombie_plague/v_zombie_licker.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb.mdl");
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb.mdl");
			break;
		}
		default:
			break;
	}
}


}
