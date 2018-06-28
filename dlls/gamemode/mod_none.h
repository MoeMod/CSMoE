
#ifndef MOD_NONE_H
#define MOD_NONE_H
#ifdef _WIN32
#pragma once
#endif

#include "bte_mod.h"

class CMod_None : public IBaseMod
{
public:
	CMod_None();

public: // CHalfLifeMultiplay
	void UpdateGameMode(CBasePlayer *pPlayer) override;

public:
	bool IsZBMode() override { return false; }

protected:
	std::vector<Vector> m_mapBombZones;
};

#endif
