#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "player_human_level.h"

float CBasePlayer::HumanLevel_GetHealthBonus()
{
	if (m_iHumanLevel.m_iHealth >= 40)
		return 800.0f;

	return m_iHumanLevel.m_iHealth * 20.0f;
}
float CBasePlayer::HumanLevel_GetAttackBonus()
{
	if (m_iHumanLevel.m_iAttack >= 40)
		return 5.0f;

	return 1.0 + m_iHumanLevel.m_iAttack * 0.1f;
}

void CBasePlayer::HumanLevel_LevelUpHealth()
{
	if (m_iHumanLevel.m_iHealth >= 40)
		return;

	m_iHumanLevel.m_iHealth++;
	CLIENT_COMMAND(edict(), "spk zombi/td_heal.wav\n");

	if (!IsAlive())
		return;

	pev->health += (m_iHumanLevel.m_iHealth >= 40) ? 40.0f : 20.0f;
}

void CBasePlayer::HumanLevel_LevelUpAttack()
{
	if (m_iHumanLevel.m_iAttack >= 40)
		return;

	m_iHumanLevel.m_iAttack++;
	CLIENT_COMMAND(edict(), "spk zombi/td_heal.wav\n");
}


void CBasePlayer::HumanLevel_Reset()
{
	m_iHumanLevel.m_iHealth= 0;
	m_iHumanLevel.m_iAttack = 0;
}