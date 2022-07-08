#ifndef PLAYER_ACCOUNT_H
#define PLAYER_ACCOUNT_H
#ifdef _WIN32
#pragma once
#endif

#ifndef CLIENT_DLL
namespace sv {
class CBasePlayer;

struct CPlayerAccount
{
	int Get() const
	{
		return m_iAmount;
	}

	CPlayerAccount &operator+=(int delta);

	CPlayerAccount &operator-=(int delta)
	{
		return *this += (-delta);
	}

	CPlayerAccount &operator=(int amount)
	{
		int iDelta = amount - m_iAmount;
		return *this += iDelta;
	}

	operator int() const { return Get(); }

	void UpdateHUD(CBasePlayer *player, bool bTrackChange = false) const;
	void Reset() { m_iAmount = m_iLastAmount = 0; }

	int m_iAmount = 0;
	mutable int m_iLastAmount = 0;
};
}
#else
using CPlayerAccount = int;
#endif

#endif