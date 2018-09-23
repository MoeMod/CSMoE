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

	return (m_iHumanLevel.m_iHealth - 1) * 20.0f;
}
float CBasePlayer::HumanLevel_GetAttackBonus()
{
	if (m_iHumanLevel.m_iAttack >= 40)
		return 5.0f;

	return 1.0 + (m_iHumanLevel.m_iAttack - 1) * 0.1f;
}

void CBasePlayer::HumanLevel_LevelUpHealth()
{
	if (m_iHumanLevel.m_iHealth >= 40)
		return;

	if (m_iAccount < 3000)
	{
		ClientPrint(pev, HUD_PRINTCENTER, "#Not_Enough_Money");
		BlinkAccount(this, 2);
		return;
	}

	AddAccount(-3000);
	m_iHumanLevel.m_iHealth++;
	CLIENT_COMMAND(edict(), "spk zombi/td_heal.wav\n");
	HumanLevel_UpdateHUD();

	if (!IsAlive())
		return;

	pev->health += 20.0f;
}

void CBasePlayer::HumanLevel_LevelUpAttack()
{
	if (m_iHumanLevel.m_iAttack >= 40)
		return;

	if (m_iAccount < 3000)
	{
		ClientPrint(pev, HUD_PRINTCENTER, "#Not_Enough_Money");
		BlinkAccount(this, 2);
		return;
	}

	AddAccount(-3000);
	m_iHumanLevel.m_iAttack++;
	CLIENT_COMMAND(edict(), "spk zombi/td_heal.wav\n");
	HumanLevel_UpdateHUD();
}

void CBasePlayer::HumanLevel_Reset()
{
	m_iHumanLevel.m_iHealth = 1;
	m_iHumanLevel.m_iAttack = 1;
}

void CBasePlayer::HumanLevel_UpdateHUD()
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZBSLevel, NULL, this->pev);
	WRITE_BYTE(0); // type, reserved.
	WRITE_BYTE(m_iHumanLevel.m_iHealth); // HP
	WRITE_BYTE(m_iHumanLevel.m_iAttack); // ATK
	WRITE_BYTE(0); // Wall
	MESSAGE_END();
}