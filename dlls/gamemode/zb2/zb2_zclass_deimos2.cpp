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

#include "zb2_zclass_deimos2.h"

namespace sv {

CZombieClass_Deimos2::CZombieClass_Deimos2(CBasePlayer *player, ZombieLevel iEvolutionLevel) : CBaseZombieClass_ZB2(player, iEvolutionLevel)
{
	m_pPlayer->m_iZombieClass = ZOMBIE_CLASS_DEIMOS2;
	m_pZombieSkill = std::make_unique<CZombieSkill_ZombieCrazy2>(m_pPlayer);
	m_pZombieSkill2 = std::make_unique<CZombieSkill_ZombieArmorRecovery>(m_pPlayer);
	Evolution(iEvolutionLevel);

	// set default property
	m_pPlayer->ResetMaxSpeed();
}

void CZombieClass_Deimos2::Evolution(ZombieLevel iEvolutionLevel)
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
		const char* szModel = iEvolutionLevel ? "deimos2_zombi_origin" : "deimos_zombi_host";
		SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", szModel);
		Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	}

	m_pPlayer->SetNewPlayerModel(szModelPath);
}

void CZombieClass_Deimos2::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_DEIMOS2);
	WRITE_BYTE(ZOMBIE_SKILL_CRAZY2);
	WRITE_BYTE(ZOMBIE_SKILL_ARMORRECOVERY);
	MESSAGE_END();
}

void CZombieClass_Deimos2::ResetMaxSpeed() const
{
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
	{
		m_pPlayer->pev->maxspeed = 275;
		m_pPlayer->pev->gravity = 0.85f;
	}
	else if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->maxspeed = 308;
		m_pPlayer->pev->gravity = 0.75f;
	}
	else
	{
		m_pPlayer->pev->maxspeed = 322;
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

float CZombieClass_Deimos2::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	if (m_pZombieSkill->GetStatus() != SKILL_STATUS_USING)
		flDamage *= 0.7;

	return CBaseZombieClass_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

bool CZombieClass_Deimos2::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	// knife knockback
	m_iCrazySkillStatus = m_pZombieSkill->GetStatus();
	if (m_iCrazySkillStatus == SKILL_STATUS_USING)
	{
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
			});
		/*if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
			return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
				kbd.flOnGround * 0.12f * 0.6f,
				kbd.flNotOnGround * 0.35f * 0.6f,
				kbd.flFlying * 0.35f * 0.6f,
				kbd.flDucking * 0.12f * 0.6f,
				Q_max(kbd.flVelocityModifier, 0.8f),
				});

		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
				kbd.flOnGround * 0.08f * 0.55f,
				kbd.flNotOnGround * 0.3f * 0.55f,
				kbd.flFlying * 0.3f * 0.55f,
				kbd.flDucking * 0.08f * 0.55f,
				Q_max(kbd.flVelocityModifier, 0.8f),
			});*/
	}
			//return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			//kbd.flOnGround * 0.05f,
			//kbd.flNotOnGround * 0.05f,
			//kbd.flFlying * 0.05f,
			//kbd.flDucking * 0.05f,
			//Q_max(kbd.flVelocityModifier, 0.8f),
			//});

	/*if (attacker->m_pActiveItem && attacker->m_pActiveItem->iItemSlot() == KNIFE_SLOT)
	{
		//return CBaseZombieClass_ZB2::ApplyKnockback(attacker, { 700, 1600, 1300, 400, 1.0f });
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, attacker->m_pActiveItem->GetKnockBackData());
	}*/

	if(m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
		return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.12f,
			kbd.flNotOnGround * 0.35f,
			kbd.flFlying * 0.35f,
			kbd.flDucking * 0.12f,
			kbd.flVelocityModifier * 0.95f,
			});

	return CBaseZombieClass_ZB2::ApplyKnockback(attacker, {
			kbd.flOnGround * 0.08f,
			kbd.flNotOnGround * 0.3f,
			kbd.flFlying * 0.3f,
			kbd.flDucking * 0.08f,
			kbd.flVelocityModifier * 0.95f,
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
	ZombieSkin skin = ZombieSkin(m_pPlayer->m_iZombieClassSkin[m_pPlayer->m_iZombieClass]);

	switch(item->m_iId)
	{
		case WEAPON_KNIFE:
		{
			if (g_ZombieSkinModelName.count(skin))
			{
				const char* szModel = g_ZombieSkinModelName[skin];
				char szModelPath[128];
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				{
					Q_snprintf(szModelPath, sizeof(szModelPath), "models/v_knife_%s_host.mdl", szModel, szModel);

					m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
				}
				else
				{
					Q_snprintf(szModelPath, sizeof(szModelPath), "models/v_knife_%s.mdl", szModel, szModel);

					m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
				}		
			}
			else
			{
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombideimos_host.mdl");
				else
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_knife_zombideimos2.mdl");
			}
			
			break;
		}
		case WEAPON_HEGRENADE:
		{
			if (g_ZombieSkinModelName.count(skin))
			{
				const char* szModel = g_ZombieSkinModelName[skin];
				char szModelPath[128];
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				{
					Q_snprintf(szModelPath, sizeof(szModelPath), "models/v_zombibomb_%s_host.mdl", szModel, szModel);

					m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
				}
				else
				{
					Q_snprintf(szModelPath, sizeof(szModelPath), "models/v_zombibomb_%s.mdl", szModel, szModel);

					m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
				}
			}
			else
			{
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb-deimos_host.mdl");
				else
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_zombibomb-deimos2.mdl");
			}
			break;
		}
		case WEAPON_SMOKEGRENADE:
		{
			if (g_ZombieSkinModelName.count(skin))
			{
				const char* szModel = g_ZombieSkinModelName[skin];
				char szModelPath[128];
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
				{
					Q_snprintf(szModelPath, sizeof(szModelPath), "models/@frogvari_v_zombibomb_%s_host.mdl", szModel, szModel);

					m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
				}
				else
				{
					Q_snprintf(szModelPath, sizeof(szModelPath), "models/@frogvari_v_zombibomb_%s.mdl", szModel, szModel);

					m_pPlayer->pev->viewmodel = MAKE_STRING(szModelPath);
				}
			}
			else
			{
				if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST)
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb-deimos_host.mdl");
				else
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/@frogvari_v_zombibomb-deimos2.mdl");
			}
			break;
		}
		default:
			break;
	}
}


}
