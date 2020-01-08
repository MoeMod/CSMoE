/*
zb1_countdown.cpp - CSMoE Gameplay server
Copyright (C) 2019 Moemod Hymei

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
#include "zb1_countdown.h"
#include "gamemode/mod_base.h"

#include "util/u_range.hpp"

namespace sv {

static const char* s_szCountDownSound[11] = {
				"", "one", "two", "three", "four", "five", "six",
				"seven", "eight", "nine", "ten"
};

void CModCountdownHelper::Think()
{
	int iCountDown = static_cast<int>(gpGlobals->time.time_since_epoch() /1s - g_pGameRules->m_fRoundCount.time_since_epoch() /1s);

	if (iCountDown != m_iLastCountDown)
	{
		m_iLastCountDown = iCountDown;
		if (iCountDown > 0 && iCountDown < m_iTotalCounts && !IsFreezePeriod())
		{

			int iCurrentCount = m_iTotalCounts - iCountDown;

			if(m_pDelegate)
				m_pDelegate->OnCountdownChanged(iCurrentCount);
			
			if (iCountDown == 1)
			{
				if (m_pDelegate)
					m_pDelegate->OnCountdownStart();

			}
			else if (iCurrentCount <= 10)
			{
				for (CBasePlayer* player : moe::range::PlayersList())
					CLIENT_COMMAND(player->edict(), "spk %s\n", s_szCountDownSound[iCurrentCount]);

			}
		}
		else if (iCountDown == m_iTotalCounts)
		{
			if (m_pDelegate)
				m_pDelegate->OnCountdownEnd();
		}

	}
}

bool CModCountdownHelper::IsExpired() const
{
	const int iCountDown = static_cast<int>(gpGlobals->time.time_since_epoch() /1s - g_pGameRules->m_fRoundCount.time_since_epoch() /1s);
	return iCountDown > m_iTotalCounts;
}

bool CModCountdownHelper::IsFreezePeriod() const
{
	return g_pGameRules->IsFreezePeriod();
}

}
