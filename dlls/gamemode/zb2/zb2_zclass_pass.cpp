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

#include "zb2_zclass_pass.h"

namespace sv {

CZombieClass_Pass::CZombieClass_Pass(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_iZombieClass = ZOMBIE_CLASS_PASS;
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombiePass>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieCrazy>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Pass::Evolution(ZombieLevel iEvolutionLevel)
{
	CBaseZombieClass_ZB2::Evolution(iEvolutionLevel);

	ZombieSkin skin = ZombieSkin(m_pPlayer->m_iZombieClassSkin[m_pPlayer->m_iZombieClass]);

	char szModelPath[128];

	if (g_ZombieSkinModelName.count(skin))
	{
		char szModel[32];
		if (iEvolutionLevel)
		{
			Q_snprintf(szModel, sizeof(szModel), "%s_origin", g_ZombieSkinModelName[skin]);
		}
		else
		{
			Q_snprintf(szModel, sizeof(szModel), "%s_host", g_ZombieSkinModelName[skin]);
		}

		SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);
		Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	}
	else
	{
		const char* szModel = iEvolutionLevel ? "pass_zombi_origin" : "pass_zombi_host";
		SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);
		Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	}

	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Pass::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_PASS);
	WRITE_BYTE(ZOMBIE_SKILL_PASS);
	WRITE_BYTE(ZOMBIE_SKILL_CRAZY);
	MESSAGE_END();
}

void CZombieClass_Pass::ResetMaxSpeed() const
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		m_pPlayer->pev->maxspeed = 290;
		m_pPlayer->pev->gravity = 0.75f;
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

	m_pPlayer->m_flDefaultGravity = m_pPlayer->pev->gravity;

	if (m_pZombieSkill2)
		m_pZombieSkill2->ResetMaxSpeed();

	if (m_tStunGravityTime > gpGlobals->time)
	{
		m_pPlayer->pev->gravity = max(m_flStunGravity, 1.0);
	}

	if (m_tStunSpeedTime > gpGlobals->time)
	{
		m_pPlayer->pev->maxspeed = min(m_flStunSpeed, 100.0);
	}
}

float CZombieClass_Pass::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Pass::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& kbd)
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, kbd);

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.7f,
			kbd.flNotOnGround * 0.75f,
			kbd.flFlying * 0.75f,
			kbd.flDucking * 0.7f,
			kbd.flVelocityModifier,
		});
}

void CZombieClass_Pass::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(1, 2))
	{
	case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/passzombie_hurt1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/passzombie_hurt2.wav", VOL_NORM, ATTN_NORM); break;
	default:break;
	}
}

void CZombieClass_Pass::DeathSound_Zombie()
{
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/passzombie_death1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/passzombie_death2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}


void CZombieClass_Pass::OnWeaponDeploy(CBasePlayerItem *item)
{
	ZombieSkin skin = ZombieSkin(m_pPlayer->m_iZombieClassSkin[m_pPlayer->m_iZombieClass]);

	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			if (g_ZombieSkinModelName.count(skin))
			{
				const char* szModel = g_ZombieSkinModelName[skin];
				char szModelPath[128];
				Q_snprintf(szModelPath, sizeof(szModelPath), "models/v_knife_%s.mdl", szModel, szModel);

				m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
			}
			else
			{
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_pass_zombi_host.mdl");
				else
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_pass_zombi.mdl");
			}			
			break;
		}
		case WEAPON_HEGRENADE:
		{
			if (g_ZombieSkinModelName.count(skin))
			{
				const char* szModel = g_ZombieSkinModelName[skin];
				char szModelPath[128];
				Q_snprintf(szModelPath, sizeof(szModelPath), "models/v_zombibomb_%s.mdl", szModel, szModel);

				m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
			}
			else
			{
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_pass_zombi_host.mdl");
				else
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb_pass_zombi.mdl");
			}		
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			if (g_ZombieSkinModelName.count(skin))
			{
				const char* szModel = g_ZombieSkinModelName[skin];
				char szModelPath[128];
				Q_snprintf(szModelPath, sizeof(szModelPath), "models/@frogvari_v_zombibomb_%s.mdl", szModel, szModel);

				m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
			}
			else
			{
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_pass_zombi_host.mdl");
				else
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb_pass_zombi.mdl");
			}				
			break;
		}
		default:
			break;
	}
}


}
