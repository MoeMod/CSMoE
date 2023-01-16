
#ifndef ZSH_BUILDING_TURRET_H
#define ZSH_BUILDING_TURRET_H
#ifdef _WIN32
#pragma once
#endif

#include "zsh_building.h"

namespace sv
{
    
class CBuilding_Turret : public CBaseBuildingClass
{
public:
    explicit CBuilding_Turret(CZSHBuilding* p) : CBaseBuildingClass(p) {}

    void OnCreate() override;
    void OnSettle() override;
    void OnCompleted() override;
    void OnThink() override;

    const char* GetName() const override { return "#CSO_zsht_Build_Menu_24"; }
    const float GetHealth() const override { return 1000.0f; }
    const int GetTotalProgress() const override { return 104; }
    const zsh::ZSHBuildingCost_s GetCost() const override { return { 5, 5, 0 }; }

protected:
    void SetAnim(int iAnim);
    bool MoveTurret();
    virtual CBaseEntity *BestVisibleEnemy();

    virtual void Search(); // 寻找目标
    virtual void Active(); // 有目标可以攻击
    virtual void Retire(); // 没有目标可以攻击
    
protected:
    Vector m_vecCurAngles; // 当前角度
    Vector m_vecGoalAngles; // 目标角度
    float m_flTurnRate; // 转速
    int m_iRange; // 寻敌范围
    time_point_t m_flFireRate;
};

}
#endif