
#ifndef MOD_ZB2_H
#define MOD_ZB2_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_zb1.h"

class CSupplyBox;

class CMod_ZombieMod2 : public CMod_Zombi
{
public:
	CMod_ZombieMod2();

public:
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void Think() override;

protected:
	void MakeSupplyboxThink();
	void RemoveAllSupplybox();
	CSupplyBox *CreateSupplybox();
	int SupplyboxCount();

protected:
	float m_flTimeNextMakeSupplybox;
};

#endif
