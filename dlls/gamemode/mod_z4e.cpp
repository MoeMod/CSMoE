/*
mod_z4e.cpp - CSMoE Gameplay server : Project Z4E
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"

#include "mod_z4e.h"

#include <memory>

namespace sv {

    void CMod_Z4E::InstallPlayerModStrategy(CBasePlayer *player) {
        player->m_pModStrategy = std::make_unique<CPlayerModStrategy_Z4E>(player, this);
    }

    void CMod_Z4E::CheckWinConditions()
    {
        // If a winner has already been determined and game of started.. then get the heck out of here
        if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
            return;

        if (!FInfectionStarted() || m_bRoundTerminating)
            return;

        moe::range::PlayersList list;
        auto iAliveHuman = std::count_if(list.begin(), list.end(), [](CBasePlayer *player) { return player->m_iTeam == TEAM_CT && !player->m_bIsZombie && player->IsAlive() && !player->IsDormant(); });
        auto iAliveZombie = std::count_if(list.begin(), list.end(), [](CBasePlayer *player) {
            auto strategy = dynamic_cast<CPlayerModStrategy_ZB3*>(player->m_pModStrategy.get());
            return ((player->m_iTeam == TEAM_TERRORIST && player->m_bIsZombie) || (strategy && strategy->IsPickAsZombie())) && !player->IsDormant();
        });

        if (!iAliveHuman)
            ZombieWin();
        else if (!iAliveZombie)
            HumanWin();
    }

    void CMod_Z4E::CheckTimeoutWin()
    {
        // 僵尸逃跑模式没有时间上限
        //CMod_Zombi::CheckTimeoutWin();
    }

    bool CMod_Z4E::CustomRandomSpawn(CBasePlayer *player)
    {
        // TODO : 复活到5秒前的位置
        return IBaseMod::CustomRandomSpawn(player);
    }

    int CMod_Z4E::ShouldCollide(CBaseEntity *pTouched, CBaseEntity *pOther)
    {
        // players can penetrate with each other
        if(pTouched->IsPlayer() && pOther->IsPlayer())
            return false;
        return IBaseMod::ShouldCollide(pTouched, pOther);
    }

    bool CPlayerModStrategy_Z4E::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& data)
    {
        // 击退要狠狠地削
        KnockbackData knockback2 = {
                data.flOnGround / 3,
                data.flNotOnGround / 3,
                data.flFlying / 3,
                data.flDucking / 3,
                sqrt(data.flVelocityModifier)
        };
        return CPlayerModStrategy_ZBZ::ApplyKnockback(attacker, knockback2);
    }

    void CPlayerModStrategy_Z4E::OnThink()
    {
        CPlayerModStrategy_ZBZ::OnThink();
        for(CBaseEntity *pEntity : moe::range::PlayersList())
        {
            pEntity->pev->groupinfo |=  (1 << m_pPlayer->entindex());
        }
    }

    void CPlayerModStrategy_Z4E::OnPostThink()
    {
        CPlayerModStrategy_Default::OnPostThink();
        for(CBaseEntity *pEntity : moe::range::PlayersList())
        {
            pEntity->pev->groupinfo &=  ~(1 << m_pPlayer->entindex());
        }
    }

    void CPlayerModStrategy_Z4E::BecomeHuman()
    {
        CPlayerModStrategy_ZBZ::BecomeHuman();
        // 人类的血量要减回100左右，否则地图上面某些坑爹点摔不死
        m_pPlayer->pev->health = m_pPlayer->pev->max_health = m_pPlayer->pev->max_health / 10;
    }

    void CMod_Z4E::PickHero()
    {
        // z4e里面屏蔽掉英雄
    }

    ModConfigData_t CMod_Z4E::QueryModConfig(const char *key, ModConfigData_t defaultValue, CBaseEntity *invoker)
    {
        if(!strcmp(key, "SprintSkillCooldownTime"))
            return 120s;
        if(!strcmp(key, "HeadShotSkillCooldownTime"))
            return 60s;
        if(!strcmp(key, "Knife2xSkillCooldownTime"))
            return 120s;
        if (!strcmp(key, "EnableForstBomb"))
            return 1;
        if (!strcmp(key, "EnableKnockBackBomb"))
            return 1;
        return IBaseMod::QueryModConfig(key, defaultValue, invoker);
    }

    bool CPlayerModStrategy_Z4E::IsSkillAvailable(ZombieZSkillSkillId skill)
    {
        if(!CPlayerModStrategy_ZBZ::IsSkillAvailable(skill))
            return false;

        if(skill == Skill_HeroicPresence)
            return false;

        return true;
    }


}
