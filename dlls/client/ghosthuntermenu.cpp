
#include "extdll.h"
#include "cbase.h"
#include "util.h"
#include "gamerules.h"
#include "weapons_moe_buy.h"
#include "player/player_mod_strategy.h"
#include "util/u_range.hpp"

#include "ghosthuntermenu.h"
#include "immenu.h"

namespace sv {
    namespace ghosthuntermenu {
        
    void ShowGhostHunterMenu(CBasePlayer *adminid)
    {
        ImMenu(adminid, [round = g_pGameRules->m_iTotalRoundsPlayed](auto context, CBasePlayer *adminid){
            if (round != g_pGameRules->m_iTotalRoundsPlayed)
                return;
            context.begin("请选择要使用的英雄");
            if (context.item("MasterHunter", "终极猎手"))
            {
                adminid->RemoveAllItems(FALSE);
                adminid->m_bHasPrimary = false;

                SET_CLIENT_KEY_VALUE(adminid->entindex(), GET_INFO_BUFFER(adminid->edict()), "model", "masterhunter");
                adminid->SetNewPlayerModel("models/player/masterhunter/masterhunter.mdl");
                adminid->GiveNamedItem("z4b_dualslayer");

                EMIT_SOUND(adminid->edict(), CHAN_AUTO, "zombi/ghosthunterappear.wav", VOL_NORM, ATTN_NORM);

                UTIL_ClientPrintAll(HUD_PRINTCENTER, "终极猎手 %s 出现了", STRING(adminid->pev->netname));

                MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, adminid->edict());
                WRITE_BYTE(7);
                WRITE_BYTE(1);
                WRITE_BYTE(2);
                MESSAGE_END();
            }
            if (context.item("TimeHunter", "时空猎手"))
            {
                adminid->RemoveAllItems(FALSE);
                adminid->m_bHasPrimary = false;

                SET_CLIENT_KEY_VALUE(adminid->entindex(), GET_INFO_BUFFER(adminid->edict()), "model", "timehunter");
                adminid->SetNewPlayerModel("models/player/timehunter/timehunter.mdl");
                adminid->GiveNamedItem("z4b_dualstinger");

                EMIT_SOUND(adminid->edict(), CHAN_AUTO, "CK_WPN/WEAPON/DualStinger/HumanBossAppear.wav", VOL_NORM, ATTN_NORM);

                UTIL_ClientPrintAll(HUD_PRINTCENTER, "时空猎手 %s 出现了", STRING(adminid->pev->netname));

                MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, adminid->edict());
                WRITE_BYTE(7);
                WRITE_BYTE(2);
                WRITE_BYTE(2);
                MESSAGE_END();
            }
            if (context.item("AsceticHero", "圣拳猎手"))
            {
                adminid->RemoveAllItems(FALSE);
                adminid->m_bHasPrimary = false;

                SET_CLIENT_KEY_VALUE(adminid->entindex(), GET_INFO_BUFFER(adminid->edict()), "model", "ascetichero");
                adminid->SetNewPlayerModel("models/player/ascetichero/ascetichero.mdl");
                adminid->GiveNamedItem("z4b_holyfist");

                EMIT_SOUND(adminid->edict(), CHAN_AUTO, "zombi/ghosthunterappear.wav", VOL_NORM, ATTN_NORM);

                UTIL_ClientPrintAll(HUD_PRINTCENTER, "圣拳猎手 %s 出现了", STRING(adminid->pev->netname));

                MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, adminid->edict());
                WRITE_BYTE(7);
                WRITE_BYTE(3);
                WRITE_BYTE(2);
                MESSAGE_END();
            }
            if (context.item("HealHunter", "救赎猎手"))
            {
                adminid->RemoveAllItems(FALSE);
                adminid->m_bHasPrimary = false;
                adminid->m_bHolsterDisabled = false;
                SET_CLIENT_KEY_VALUE(adminid->entindex(), GET_INFO_BUFFER(adminid->edict()), "model", "healhunter");
                adminid->SetNewPlayerModel("models/player/healhunter/healhunter.mdl");
                adminid->GiveNamedItem("z4b_heartstimulater");

                EMIT_SOUND(adminid->edict(), CHAN_AUTO, "CK_WPN/WEAPON/HeartStimulater/Fist_Select.wav", VOL_NORM, ATTN_NORM);

                UTIL_ClientPrintAll(HUD_PRINTCENTER, "救赎猎手 %s 出现了", STRING(adminid->pev->netname));

                MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, adminid->edict());
                WRITE_BYTE(7);
                WRITE_BYTE(5);
                WRITE_BYTE(2);
                MESSAGE_END();
            }
            if (context.item("LastHero", "动力机甲"))
            {
                adminid->RemoveAllItems(FALSE);
                adminid->m_bHasPrimary = false;
                adminid->m_bHolsterDisabled = true;
                SET_CLIENT_KEY_VALUE(adminid->entindex(), GET_INFO_BUFFER(adminid->edict()), "model", "lasthero");
                adminid->SetNewPlayerModel("models/player/lasthero/lasthero.mdl");
                adminid->GiveNamedItem("weapon_herochainsaw");

                UTIL_ClientPrintAll(HUD_PRINTCENTER, "动力机甲 %s 已投放完毕", STRING(adminid->pev->netname));
            }
            context.end(adminid);
        });
    }
    }
}