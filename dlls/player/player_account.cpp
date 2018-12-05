#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"

#include "gamemode/mods.h"

#include <algorithm>

void CBasePlayer::AddAccount(int amount, bool bTrackChange)
{
	m_iAccount += amount;
}

CPlayerAccount &CPlayerAccount::operator+=(int delta)
{
	const int iMax = g_pModRunning->MaxMoney();

	m_iLastAmount = m_iAmount;
	m_iAmount = std::min(std::max(0, m_iLastAmount + delta), iMax);

	return *this;
}

void CPlayerAccount::UpdateHUD(CBasePlayer *player, bool bTrackChange)
{
	if (!bTrackChange || m_iAmount != m_iLastAmount)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMoney, NULL, player->pev);
		WRITE_LONG(m_iAmount);
		WRITE_BYTE(bTrackChange);
		MESSAGE_END();

		m_iLastAmount = m_iAmount;
	}
}