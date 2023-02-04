
#ifndef ZSH_BUILDING_TURRET_H
#define ZSH_BUILDING_TURRET_H
#ifdef _WIN32
#pragma once
#endif

#include "extdll.h"
#include "util.h"
#include "cbase.h"

namespace sv
{
    
class CSentryGun : public CBaseMonster
{
public:
    static CSentryGun* Create(const Vector& vecOrigin, const Vector& vecAngles, int iTeam);

public:
    void Spawn(void);
    void Precache(void);
    void Killed(entvars_t* pevAttacker, int iGib) override;

    float GetHealth() { return 1000.0f; }

    int ObjectCaps() override { return (CBaseMonster::ObjectCaps() | FCAP_MUST_SPAWN); }
    int Classify() override { return CLASS_MACHINE; }

    void EXPORT Think();
    void SetAnim(int iAnim);
    void SetLife(duration_t flTime);
    bool MoveTurret();
    virtual CBaseEntity *BestVisibleEnemy();

    virtual void Search(); // 寻找目标
    virtual void Active(); // 有目标可以攻击
    virtual void Retire(); // 没有目标可以攻击
    
    Vector m_vecCurAngles; // 当前角度
    Vector m_vecGoalAngles; // 目标角度
    float m_flTurnRate; // 转速
    float m_flVerticalTurnRate;
    int m_iRange; // 寻敌范围
    time_point_t m_flFireRate;
    time_point_t m_flRemoveTime;

    EHANDLE m_hEnemy;
    CBasePlayer* m_pOwner;

};

}
#endif