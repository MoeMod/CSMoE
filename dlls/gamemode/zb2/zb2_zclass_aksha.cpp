#include <memory>

/*
zb2_zclass_aksha.cpp - CSMoE Gameplay server : Zombie Mod 2
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

#include "zb2_zclass_aksha.h"

namespace sv {

	CZombieClass_Aksha::CZombieClass_Aksha(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_ZombieClass = "aksha";
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieDefense>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieStrengthRecovery>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 4000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 1000;
	m_iAnim = 10;
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Aksha::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	const char *szModel = iEvolutionLevel ? "akshazombi_origin" : "akshazombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Aksha::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_AKSHA);
	WRITE_BYTE(ZOMBIE_SKILL_CHARGE);
	WRITE_BYTE(ZOMBIE_SKILL_STRENGTHRECOVERY);
	MESSAGE_END();
}

void CZombieClass_Aksha::ResetMaxSpeed() const
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		m_pPlayer->pev->maxspeed = 290;
		m_pPlayer->pev->gravity = 0.85f;
	}
	else if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->maxspeed = 319;
		m_pPlayer->pev->gravity = 0.75f;
	}
	else
	{
		m_pPlayer->pev->maxspeed = 333;
		m_pPlayer->pev->gravity = 0.7f;
	}

	if (m_tStunGravityTime > gpGlobals->time)
	{
		m_pPlayer->pev->gravity = max(m_flStunGravity, 1.0);
	}

	if (m_tStunSpeedTime > gpGlobals->time)
	{
		m_pPlayer->pev->maxspeed = min(m_flStunSpeed, 100.0);
	}

	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed();
}

float CZombieClass_Aksha::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	if (m_pZombieSkill->GetStatus() == SKILL_STATUS_USING)
	{
		flDamage *= 0.1;
		m_pPlayer->m_flDamageUnderTake += flDamage;
	}
	
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Aksha::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	// knife knockback

	if (m_pZombieSkill->GetStatus() == SKILL_STATUS_USING)
	{
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem;
		if (pWeapon)
		{
			if (m_iAnim <= 13)
			{
				pWeapon->SendWeaponAnim(m_iAnim, TRUE);
				pWeapon->m_flTimeWeaponIdle = 2.5s;
				m_iAnim++;
			}
			else
			{
				m_iAnim = 10;
			}
		}
		
		CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		0.0f,
			});

		m_pPlayer->pev->velocity = g_vecZero;
		return TRUE;
	}

	if(m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.9f,
			kbd.flNotOnGround * 0.9f,
			kbd.flFlying * 0.9f,
			kbd.flDucking * 0.9f,
			kbd.flVelocityModifier,
			});

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.6f,
			kbd.flNotOnGround * 0.7f,
			kbd.flFlying * 0.7f,
			kbd.flDucking * 0.6f,
			kbd.flVelocityModifier,
	});
}

void CZombieClass_Aksha::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/akshazombie_hurt1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/akshazombie_hurt2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombieClass_Aksha::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/akshazombie_death1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/akshazombie_death2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_Aksha::OnWeaponDeploy(CBasePlayerItem *item)
{
	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombiaksha_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombiaksha.mdl");
			break;
		}
		case WEAPON_HEGRENADE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_aksha_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_aksha.mdl");
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_aksha_host.mdl");
			else
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_aksha.mdl");
			break;
		}
		default:
			break;
	}
}


}
