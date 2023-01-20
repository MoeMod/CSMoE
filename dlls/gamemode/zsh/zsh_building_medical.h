
#ifndef ZSH_BUILDING_MEDICAL_H
#define ZSH_BUILDING_MEDICAL_H
#ifdef _WIN32
#pragma once
#endif

#include "zsh_building.h"

namespace sv
{
    const duration_t ZSH_Building_Medical_ThinkRate = 1.0s;
    const float ZSH_Building_Medical_RecoverRadius = 85.0f;

    class CBuilding_Medical : public CBaseBuildingClass
    {
    public:
        explicit CBuilding_Medical(CZSHBuilding* p) : CBaseBuildingClass(p) {}

        void OnCreate() override;
        void OnSettle() override;
        void OnCompleted() override;
        void OnThink() override;

        const char* GetName() const override { return "#CSO_zsht_Build_Menu_33"; }
        const float GetHealth() const override { return 800.0f; }
        const int GetTotalProgress() const override { return 65; }
        const zsh::ZSHBuildingCost_s GetCost() const override { return { 10, 10, 20 }; }
        //                                                              Ä¾Í·/Ìú/µç

    protected:

        void OnRecovery();
    };

}
#endif