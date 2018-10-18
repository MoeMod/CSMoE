#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "client.h"
#include "player_zombie_skill.h"

#include <string>

void CBasePlayer::ZombieSkill_Check()
{
	if (m_iZombieSkillStatus == SKILL_STATUS_READY)
	{

	}

	if (m_iZombieSkillStatus == SKILL_STATUS_USING && gpGlobals->time > m_flTimeZombieSkillEnd)
	{
		ZombieSkill_End();
	}

	if (m_iZombieSkillStatus == SKILL_STATUS_FREEZING && gpGlobals->time > m_flTimeZombieSkillNext)
	{
		m_iZombieSkillStatus = SKILL_STATUS_READY;
	}

}

void CBasePlayer::ZombieSkill_Start()
{
	if (m_iZombieSkillStatus != SKILL_STATUS_READY)
	{
		switch (m_iZombieSkillStatus)
		{
		case SKILL_STATUS_USING:
		case SKILL_STATUS_FREEZING:
			ClientPrint(pev, HUD_PRINTCENTER,
				"The 'Berserk' skill can't be used because the skill is in cooldown. [Remaining Cooldown Time: %s1 sec.]",
				std::to_string(static_cast<int>(m_flTimeZombieSkillNext - gpGlobals->time)).c_str()
			); // #CSO_WaitCoolTimeNormal

			break;
		case SKILL_STATUS_USED:
			ClientPrint(pev, HUD_PRINTCENTER, "The 'Sprint' skill can only be used once per round."); // #CSO_CantSprintUsed
		}

		return;
	}


	m_iZombieSkillStatus = SKILL_STATUS_USING;
	m_flTimeZombieSkillEnd = gpGlobals->time + ZOMBIECRAZY_DURATION;
	m_flTimeZombieSkillNext = gpGlobals->time + ZOMBIECRAZY_COOLDOWN;

	pev->renderfx = kRenderFxGlowShell;
	pev->rendercolor = { 255,0,0 };
	pev->renderamt = 1;
	pev->fov = m_iFOV = 105;
	ResetMaxSpeed();

	//EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_hurt_01.wav", VOL_NORM, ATTN_NORM);
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, this->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_ACTIVATE);
	WRITE_BYTE(ZOMBIE_SKILL_CRAZY);
	WRITE_SHORT(ZOMBIECRAZY_DURATION);
	WRITE_SHORT(ZOMBIECRAZY_COOLDOWN);
	MESSAGE_END();
}

void CBasePlayer::ZombieSkill_End()
{
	m_iZombieSkillStatus = SKILL_STATUS_FREEZING;

	pev->renderfx = kRenderFxNone;
	pev->rendercolor = { 255,255,255 };
	pev->renderamt = 16;
	pev->fov = m_iFOV = 90;
	ResetMaxSpeed();
}

void CBasePlayer::ZombieSkill_Init()
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, NULL, this->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	WRITE_BYTE(ZOMBIE_CLASS_TANK);
	WRITE_BYTE(ZOMBIE_SKILL_CRAZY);
	MESSAGE_END();
}

void CBasePlayer::ZombieSkill_Reset()
{
	m_iZombieSkillStatus = SKILL_STATUS_READY;
}

void ZombieSkill_Precache()
{
	//PRECACHE_SOUND("zombi/zombi_hurt_01.wav");
}