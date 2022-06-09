//
// Created by OrangePeel on 2022/6/6.
//

#pragma once

#include "mod_zbs.h"
#include "zsh/zsh_consts.h"
#include "player/player_mod_strategy.h"
#include "usercmd.h"

#include <memory>

namespace sv {

	class CClientFog;
	class CZSHBuilding;

	class CMod_ZombieShelter : public CMod_ZombieScenario
	{
	public:
		CMod_ZombieShelter();
		~CMod_ZombieShelter();

	public:
		void UpdateGameMode(CBasePlayer *pPlayer) override;
		void CheckMapConditions() override;
		void RestartRound() override;
		void Think() override;
		void InstallPlayerModStrategy(CBasePlayer* player) override;

	public:
		void AddResource(zsh::ResourceType_e type, int amount);
		int GetMaxResource(zsh::ResourceType_e type);
		void SetMaxResource(zsh::ResourceType_e type, int amount);
	public:
		struct round_t;
		std::unique_ptr<round_t> round;
		CClientFog* m_pFogEntity;
	};

	class CPlayerModStrategy_ZSH : public CPlayerModStrategy_Default
	{
	public:
		CPlayerModStrategy_ZSH(CBasePlayer* player, CMod_ZombieShelter* mp);
		bool CanPlayerBuy(bool display) override;
		void ClientPutInServer() override;
		void OnSpawn() override;
		bool ClientCommand(const char* pcmd) override;
		void OnInitHUD() override;
		void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) override;
		void OnThink() override;
		void GiveDefaultItems() override;
		void CmdStart(struct usercmd_s* cmd, unsigned int random_seed) override;
		//int ComputeMaxAmmo(const char* szAmmoClassName, int iOriginalMax) override { return 600; }

	public:
		CZSHBuilding* CreateBuilding(const zsh::ZSHBuildingClass iBuilding);
		void AddResource(zsh::ResourceType_e type, int amount);
	public:
		CMod_ZombieShelter* const m_pModZSH;
		/* 资源 */
		int m_iResource[zsh::RT_MAX];
		/* 最大资源 */
		int m_iMaxResource[zsh::RT_MAX];
		/* 资源折扣 */
		int g_iReduceCost[zsh::RT_MAX];
		/* 正在建造的建筑 */
		EntityHandle <CZSHBuilding> m_hBuilding;
		/* 建造增加进度 */
		int m_iBuildProgress;
		/* 修复建筑增加生命值 */
		int m_iFixHealth;
		/* 当前建筑朝向 */
		int m_iRotate;
		/* 收集资源数量 */
		int m_iCollection;
		/* 技能点 */
		int m_iSkillPoint;
	protected:
		EventListener m_listenerAdjustDamage;
		EventListener m_listenerMonsterKilled;
		time_point_t m_flSpawnTips;
		time_point_t m_flSpawnTime;
	};

}
