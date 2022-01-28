//
// Created by 小白白 on 2021/12/12.
//

#pragma once

#include "mod_zb1.h"

#include <memory>

namespace sv {

	class CMod_ZombieIV;
	class CZ4CountdownDelegate : public CZB1CountdownDelegate {
	public:
		CZ4CountdownDelegate(CMod_ZombieIV *mod);

		void OnCountdownStart() override;
		void OnCountdownChanged(int iCurrentCount) override;
	};

	class CMod_ZombieIV : public CMod_Zombi
	{
	public:
		CMod_ZombieIV();
		~CMod_ZombieIV();

	public:
		void UpdateGameMode(CBasePlayer *pPlayer) override;

		void RestartRound() override;
		void Think() override;
		void HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker) override;
		void InstallPlayerModStrategy(CBasePlayer *player) override;
		HitBoxGroup GetAdjustedTraceAttackHitgroup(CBaseEntity *victim, entvars_t * pevAttacker, float flDamage, const Vector & vecDir, TraceResult * ptr, int bitsDamageType) override;
		int IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled);

		void InfectionSound() override;
		void EmitHomoSound(CBasePlayer *player, CBasePlayer *attacker) override;
		void HumanWin() override;
		void ZombieWin() override;
		BOOL FAllowFlashlight() override { return true; }
		void CheckWinConditions() override;
		BOOL ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char* szRejectReason) override;

	public:
		struct round_t;
		std::unique_ptr<round_t> round;
		CZ4CountdownDelegate m_CountDownDelegate;
	};
}
