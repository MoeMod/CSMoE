
#ifndef ZSH_BUILDING_H
#define ZSH_BUILDING_H
#ifdef _WIN32
#pragma once
#endif

#include "gamemode/mod_zsh.h"
#include "gamemode/zsh/zsh_consts.h"

namespace sv
{
	class CZSHBuilding;

	struct BaseBuildingExtra
	{
	public:
		BaseBuildingExtra(CZSHBuilding* p) : m_pBuilding(p) {}

	public:
		CZSHBuilding* const m_pBuilding;
	};

	class IBaseBuildingClass : public BaseBuildingExtra
	{
	public:
		IBaseBuildingClass(CZSHBuilding* p) : BaseBuildingExtra(p) {}
		virtual ~IBaseBuildingClass() = 0;

		virtual void OnCreate() = 0;
		virtual void OnSettle() = 0;
		virtual void OnCompleted() = 0;
		virtual void OnThink() = 0;
		virtual void OnTouch(CBaseEntity* other) = 0;
		virtual void OnKilled(entvars_t* pKiller, int iGib) = 0;

		virtual const char* GetName() const = 0;
		virtual const float GetHealth() const = 0;
		virtual const int GetTotalProgress() const = 0;
		virtual const zsh::ZSHBuildingCost_s GetCost() const = 0;
		virtual const Vector& GetMins() const = 0;
		virtual const Vector& GetMaxs() const = 0;
		virtual bool IsTeamMate(CBaseEntity* that) const = 0;
	};

	inline IBaseBuildingClass::~IBaseBuildingClass() {}

	class CBaseBuildingClass : public IBaseBuildingClass
	{
	public:
		explicit CBaseBuildingClass(CZSHBuilding* p) : IBaseBuildingClass(p) {}

		/* 创建 */
		void OnCreate() override {}
		/* 开始建造 */
		void OnSettle() override {}
		/* 建造完成 */
		void OnCompleted() override {}
		void OnThink() override {}
		void OnTouch(CBaseEntity* other) override {}
		void OnKilled(entvars_t* pKiller, int iGib) override {}

		const char* GetName() const override { return ""; }
		const float GetHealth() const override { return 0.0f; }
		const int GetTotalProgress() const override { return 0; }
		const zsh::ZSHBuildingCost_s GetCost() const override { return { 0, 0, 0 }; }
		const Vector& GetMins() const override { return m_vecMins; }
		const Vector& GetMaxs() const override { return m_vecMaxs; }
		bool IsTeamMate(CBaseEntity* that) const override { return that->pev->team == TEAM_CT; }
	public:
		Vector m_vecMins;
		Vector m_vecMaxs;
	};

	class CZSHBuilding : public CBaseMonster
	{
	public:
		void Spawn() override;
		void Precache() override;
		int ObjectCaps() override { return (CBaseMonster::ObjectCaps() | FCAP_MUST_SPAWN); }
		int Classify() override { return CLASS_MACHINE; }
		void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
		int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
		void BecomeDead(void) override;
		void Killed(entvars_t* pevAttacker, int iGib) override;
		int BloodColor() override { return DONT_BLEED; }
		void GibMonster() override;

		// RAII support to prevent memory leak.
		CZSHBuilding();

	public:
		void EXPORT OnThink();
		void EXPORT OnTouch(CBaseEntity* pOther);
		void Create(CBasePlayer* pPlayer, zsh::ZSHBuildingClass iClass, std::shared_ptr<CBaseBuildingClass> pClass);
		bool Settle();
		void Complete();
		/* 是否已放置 */
		bool isSettled() const { return m_iTotalProgress > 0 && m_iProgress >= 0; }
		/* 是否已完成 */
		bool isCompleted() const { return m_iProgress == -1; }

	public:
		zsh::ZSHBuildingClass m_iClass;
		int m_iProgress;
		int m_iTotalProgress;
		EntityHandle<CBasePlayer> m_hBuilder;
		std::shared_ptr<IBaseBuildingClass> m_pClass;
	private:
		int m_iMetalGibs;
	};

	class CBuilding_Generator_Diesel : public CBaseBuildingClass
	{
	public:
		explicit CBuilding_Generator_Diesel(CZSHBuilding* p) : CBaseBuildingClass(p) {}

		void OnCreate() override;
		void OnCompleted() override;
		void OnThink() override;

		const char* GetName() const override { return "#CSO_zsht_Build_Menu_30"; }
		const float GetHealth() const override { return 1000.0f; }
		const int GetTotalProgress() const override { return 104; }
		const zsh::ZSHBuildingCost_s GetCost() const override { return { 5, 5, 0 }; }
	};

	class CBuilding_Wall01 : public CBaseBuildingClass
	{
	public:
		explicit CBuilding_Wall01(CZSHBuilding* p) : CBaseBuildingClass(p) {}

		void OnCreate() override;
		void OnCompleted() override;
		void OnThink() override;

		const char* GetName() const override { return "#CSO_zsht_Build_Menu_16"; }
		const float GetHealth() const override { return 1000.0f; }
		const int GetTotalProgress() const override { return 104; }
		const zsh::ZSHBuildingCost_s GetCost() const override { return { 5, 5, 0 }; }
	};

	class CBuilding_WareHouse01 : public CBaseBuildingClass
	{
	public:
		explicit CBuilding_WareHouse01(CZSHBuilding* p) : CBaseBuildingClass(p) {}

		void OnCreate() override;
		void OnCompleted() override;
		void OnThink() override;

		const char* GetName() const override { return "#CSO_zsht_Build_Menu_8"; }
		const float GetHealth() const override { return 1000.0f; }
		const int GetTotalProgress() const override { return 104; }
		const zsh::ZSHBuildingCost_s GetCost() const override { return { 5, 5, 0 }; }
	};

}

#endif