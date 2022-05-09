/*
mod_z4e.h - CSMoE Gameplay server : Project Z4E
Copyright (C) 2022 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include "mod_zbz.h"

namespace sv {
    class CMod_Z4E : public CMod_ZombieZ
    {
    public:
        void InstallPlayerModStrategy(CBasePlayer *) override;
        void StartForceRespawn() override {} // no force respawn
        bool isForceRespawn() const override { return true; }
        void CheckWinConditions() override;
        void CheckTimeoutWin() override;
        bool CustomRandomSpawn(CBasePlayer *player) override;
        int ShouldCollide(CBaseEntity *pTouched, CBaseEntity *pOther) override;
        void PickHero() override;
        ModConfigData_t QueryModConfig(const char *key, ModConfigData_t defaultValue, CBaseEntity *invoker) override;
    };

    class CPlayerModStrategy_Z4E : public CPlayerModStrategy_ZBZ
    {
    public:
        using CPlayerModStrategy_ZBZ::CPlayerModStrategy_ZBZ;
        void OnThink() override;
        void OnPostThink() override;
        bool ApplyKnockback(CBasePlayer* attacker, const KnockbackData& data) override;
        void BecomeHuman() override;
        bool IsSkillAvailable(ZombieZSkillSkillId skill) override;
    };
}