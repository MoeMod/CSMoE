
#include "extdll.h"
#include "cbase.h"
#include "util.h"
#include "gamerules.h"
#include "player/player_mod_strategy.h"
#include "util/u_range.hpp"
#include "player/player_const.h"
#include "moe_fun.h"
#include "wdnmd.h"
#include "immenu.h"

namespace sv {
    namespace moefunmenu {
        
    std::map<HandTattooName, const char*> g_TattoName =
    {
        { TATTOO_NONE, "清除纹身" },
        { TATTOO_CHAOS, "火花纹身" },
        { TATTOO_DRAGON, "青龙纹身" },
        { TATTOO_TIGER, "猛虎纹身" },
        { TATTOO_BAMBOU, "翠竹纹身" },
        { TATTOO_CSOWC, "CSOWC 纪念纹身" },
        { TATTOO_TTCUSTOM, "蚩尤纹身" },
        { TATTOO_DIONE, "狄欧斯纹身" },
        { TATTOO_GLUTTONY, "狂暴饕餮纹身" },
        { TATTOO_MADMECHANIC, "破面重甲纹身" },
        { TATTOO_PHOBOS, "异形斗兽纹身" },
        { TATTOO_POISONFIRE, "鬼火纹身" },
        { TATTOO_RUSTYSCORPION, "铁甲巨蝎纹身" },
        { TATTOO_SNAKE, "巨蛇纹身" },
        { TATTOO_SNOW, "冰晶纹身" },
        { TATTOO_ZAVISTNEID, "恸哭双子纹身" },
        { TATTOO_LETTERCHN1, "纹身设计师活动获奖作品1(文字纹身)" },
        { TATTOO_LETTERCHN2, "纹身设计师活动获奖作品2(文字纹身)" },
        { TATTOO_LETTERCHN3, "纹身设计师活动获奖作品3(文字纹身)" },
        { TATTOO_TIGER22, "黑虎纹身" },
    };

    void PrintToChat(CBasePlayer* target, const std::string& what)
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgSayText, {}, target->edict());
        WRITE_BYTE(0);
        WRITE_STRING(what.c_str());
        MESSAGE_END();
    }

    void PrintToChatAll(const std::string& what)
    {
        MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
        WRITE_BYTE(0);
        WRITE_STRING(what.c_str());
        MESSAGE_END();
    }

    std::string GetPlayerName(CBasePlayer* player)
    {
        return STRING(player->pev->netname);
    }

    Vector vecSize[70] =
    {
    {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}, {0.0, 1.0, 0.0}, {0.0, -1.0, 0.0}, {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}, {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0}, {1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, -1.0},
    {0.0, 0.0, 2.0}, {0.0, 0.0, -2.0}, {0.0, 2.0, 0.0}, {0.0, -2.0, 0.0}, {2.0, 0.0, 0.0}, {-2.0, 0.0, 0.0}, {-2.0, 2.0, 2.0}, {2.0, 2.0, 2.0}, {2.0, -2.0, 2.0}, {2.0, 2.0, -2.0}, {-2.0, -2.0, 2.0}, {2.0, -2.0, -2.0}, {-2.0, 2.0, -2.0}, {-2.0, -2.0, -2.0},
    {0.0, 0.0, 3.0}, {0.0, 0.0, -3.0}, {0.0, 3.0, 0.0}, {0.0, -3.0, 0.0}, {3.0, 0.0, 0.0}, {-3.0, 0.0, 0.0}, {-3.0, 3.0, 3.0}, {3.0, 3.0, 3.0}, {3.0, -3.0, 3.0}, {3.0, 3.0, -3.0}, {-3.0, -3.0, 3.0}, {3.0, -3.0, -3.0}, {-3.0, 3.0, -3.0}, {-3.0, -3.0, -3.0},
    {0.0, 0.0, 4.0}, {0.0, 0.0, -4.0}, {0.0, 4.0, 0.0}, {0.0, -4.0, 0.0}, {4.0, 0.0, 0.0}, {-4.0, 0.0, 0.0}, {-4.0, 4.0, 4.0}, {4.0, 4.0, 4.0}, {4.0, -4.0, 4.0}, {4.0, 4.0, -4.0}, {-4.0, -4.0, 4.0}, {4.0, -4.0, -4.0}, {-4.0, 4.0, -4.0}, {-4.0, -4.0, -4.0},
    {0.0, 0.0, 5.0}, {0.0, 0.0, -5.0}, {0.0, 5.0, 0.0}, {0.0, -5.0, 0.0}, {5.0, 0.0, 0.0}, {-5.0, 0.0, 0.0}, {-5.0, 5.0, 5.0}, {5.0, 5.0, 5.0}, {5.0, -5.0, 5.0}, {5.0, 5.0, -5.0}, {-5.0, -5.0, 5.0}, {5.0, -5.0, -5.0}, {-5.0, 5.0, -5.0}, {-5.0, -5.0, -5.0}
    };

    bool Is_Hull_Vacant(CBasePlayer* playerid, Vector Origin, int Hull)
    {
        TraceResult tr;
        UTIL_TraceHull(Origin, Origin, dont_ignore_monsters, Hull, playerid->edict(), &tr);

        if (!tr.fStartSolid || !tr.fAllSolid) //if (!get_tr2(tr, TR_StartSolid) || !get_tr2(tr, TR_AllSolid)) get_tr2(tr, TR_InOpen))
            return true;

        return false;
    }

    void CheckStuck(CBasePlayer* playerid)
    {
        int hull;
        hull = playerid->pev->flags & FL_DUCKING ? head_hull : human_hull;

        if (!Is_Hull_Vacant(playerid, playerid->pev->origin, hull) && !(playerid->pev->solid & SOLID_NOT))
        {
            //if (!is_hull_vacant(origin, hull, player) && !get_user_noclip(player) && !(pev(player, pev_solid) & SOLID_NOT))

            Vector min, vec, vecOrigin;

            min = playerid->pev->mins;
            vecOrigin = playerid->pev->origin;
            vec[2] = vecOrigin[2];

            for (int i = 0; i < (sizeof(vecSize) / sizeof(Vector)); ++i)
            {
                vec[0] = vecOrigin[0] - min[0] * vecSize[i][0];
                vec[1] = vecOrigin[1] - min[1] * vecSize[i][1];
                vec[2] = vecOrigin[2] - min[2] * vecSize[i][2];

                if (Is_Hull_Vacant(playerid, vec, hull))
                {
                    playerid->pev->origin = vec;
                    PrintToChat(playerid, "解除卡住");
                    break;
                }
            }
        }
        else
            PrintToChat(playerid, "你没有被卡住");
    }

    void ShowTattoMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("纹身管理");
            for (int i = TATTOO_NONE; i < NUM_HANDTATTOO_TYPES; i++)
            {   
                if (context.item(std::to_string(i), g_TattoName[HandTattooName(i)]))
                {
                    MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, adminid->pev);
                    WRITE_BYTE(46);
                    WRITE_BYTE(i);
                    MESSAGE_END();

                    if (i == TATTOO_NONE)
                        PrintToChat(adminid, "已清除纹身");
                    else
                        PrintToChat(adminid, "已应用纹身");
                }

            }
            context.end(adminid);
            });
    }

    void ShowKillConfirmMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("确认自杀吗");
           
            if (context.item("Yes", "是"))
            {
                return ClientKill(adminid->edict());
            }

            context.end(adminid);
            });
    }
 
    void ShowMoeFunMenu(CBasePlayer *playerid)
    {
        ImMenu(playerid, [](auto context, CBasePlayer * playerid){         
   
            context.begin("Moe_Fun");

            if (context.item("CheckStuck", "解除卡住"))
                CheckStuck(playerid);

            if (context.item("suicide", "自杀"))
                ShowKillConfirmMenu(playerid);

            if (context.item("wdnmd", "wdnmd语音菜单"))
                wdnmdmenu::ShowWdnmdMenu(playerid);

            if (context.item("tattomenu", "纹身管理"))
                ShowTattoMenu(playerid);

            context.end(playerid);
        });
    }
    }
}