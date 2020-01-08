
#ifndef MOD_NONE_H
#define MOD_NONE_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"

#include <vector>
#include <utility>

namespace sv {

class CMod_None : public CBaseModDefault
{
public:
	CMod_None() {}

public: // CHalfLifeMultiplay
	BOOL IsTeamplay(void) override { return TRUE; }
	void CheckMapConditions() override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;

protected:
	std::vector<std::pair<CBaseEntity *, Vector>> m_mapBombZones;
};

}

#endif
