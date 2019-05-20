#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "player_human_level.h"
#include "gamemode/mods.h"

namespace sv {

PlayerExtraHumanLevel_ZBS::PlayerExtraHumanLevel_ZBS(CBasePlayer *player) : BasePlayerExtra(player)
{
	m_iHealth = 1;
	m_iAttack = 1;
}

float PlayerExtraHumanLevel_ZBS::GetHealthBonus() const
{
	if (m_iHealth >= 40)
		return 800.0f;

	return (m_iHealth - 1) * 20.0f;
}
float PlayerExtraHumanLevel_ZBS::GetAttackBonus() const
{
	if (m_iAttack >= 40)
		return 5.0f;

	return 1.0 + (m_iAttack - 1) * 0.1f;
}

void PlayerExtraHumanLevel_ZBS::LevelUpHealth()
{

	if (m_iHealth >= 40)
		return;

	if (m_pPlayer->m_iAccount < 3000)
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
		BlinkAccount(m_pPlayer, 2);
		return;
	}

	m_pPlayer->AddAccount(-3000);
	m_iHealth++;
	CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/td_heal.wav\n");
	UpdateHUD();

	if (!m_pPlayer->IsAlive())
		return;

	m_pPlayer->pev->health += 20.0f;
}

void PlayerExtraHumanLevel_ZBS::LevelUpAttack()
{
	if (m_iAttack >= 40)
		return;

	if (m_pPlayer->m_iAccount < 3000)
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
		BlinkAccount(m_pPlayer, 2);
		return;
	}

	m_pPlayer->AddAccount(-3000);
	m_iAttack++;
	CLIENT_COMMAND(m_pPlayer->edict(), "spk zombi/td_heal.wav\n");
	UpdateHUD();
}

void PlayerExtraHumanLevel_ZBS::Reset()
{
	m_iHealth = 1;
	m_iAttack = 1;
}

void PlayerExtraHumanLevel_ZBS::UpdateHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZBSLevel, NULL, m_pPlayer->pev);
	WRITE_BYTE(0); // type, reserved.
	WRITE_BYTE(m_iHealth); // HP
	WRITE_BYTE(m_iAttack); // ATK
	WRITE_BYTE(0); // Wall
	MESSAGE_END();
}

}
