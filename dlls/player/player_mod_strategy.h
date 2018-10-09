#ifndef PLAYER_MOD_STRATEGY_H
#define PLAYER_MOD_STRATEGY_H
#ifdef _WIN32
#pragma once
#endif

class CBasePlayer;

// Strategy Pattern
// impliments part of player to deduce unnecessary references...
class IBasePlayerModStrategy
{
public:
	virtual void GiveAmmo(int iCount, char *szName, int iMax) = 0;

};

class CPlayerModStrategy_Default : public IBasePlayerModStrategy
{

public:
	CBasePlayer *m_pPlayer;
};

#endif