#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "client.h"
#include "player_zombie_skill.h"

void CBasePlayer::ZombieSkill_Check()
{
	if (!m_iZombieSkill)
		return;

	if (m_flTimeNextZombieSkillThink > gpGlobals->time)
		return;

	if (m_iZombieSkill == 1)
	{
		ZombieSkill_End();
		return;
	}

	if (m_iZombieSkill == 2)
	{
		m_iZombieSkill = 0;
		ClientPrint(pev, HUD_PRINTCENTER, "Skill Recovered.");
	}
}

void CBasePlayer::ZombieSkill_Start()
{
	if (m_iZombieSkill)
		return;

	m_iZombieSkill = 1;
	m_flTimeNextZombieSkillThink = gpGlobals->time + ZOMBIECRAZY_DURATION;
	ResetMaxSpeed();
	ClientPrint(pev, HUD_PRINTCENTER, "Skill Start.");
	//EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_hurt_01.wav", VOL_NORM, ATTN_NORM);
}

void CBasePlayer::ZombieSkill_End()
{
	ClientPrint(pev, HUD_PRINTCENTER, "Skill End.");
	m_iZombieSkill = 2;
	m_flTimeNextZombieSkillThink = gpGlobals->time + ZOMBIECRAZY_COOLDOWN;
}
