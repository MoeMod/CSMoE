#ifndef PLAYER_ACCOUNT_H
#define PLAYER_ACCOUNT_H
#ifdef _WIN32
#pragma once
#endif

class CPlayerAccount
{
	CPlayerAccount() : m_iAmount(0), m_iCached(0) {}

	int get() const
	{
		return m_iAmount;
	}

	CPlayerAccount &operator+=(int delta)
	{
		m_iAmount += delta;
		m_iCached += delta;
		return *this;
	}

	CPlayerAccount &operator-=(int delta)
	{
		return *this += (-delta);
	}

	CPlayerAccount &operator=(int amount)
	{
		int iDelta = amount - m_iAmount;
		return *this += iDelta;
	}

	operator int() const { return get(); }

private:
	int m_iAmount;
	int m_iCached;
};

#endif