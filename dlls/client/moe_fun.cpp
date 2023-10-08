
#include "extdll.h"
#include "cbase.h"
#include "util.h"
#include "gamerules.h"
#include "player/player_mod_strategy.h"
#include "util/u_range.hpp"
#include "player/player_const.h"
#include "gamemode/zb2//zb2_const.h"
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

    struct ZombieSkinMenu
    {
        ZombieClassType ClassType;
        const char* SkinName;
    };

    std::map<ZombieSkin, ZombieSkinMenu> g_ZombieSkinName =
    {
        { SKIN_CHNNORMAL, { ZOMBIE_CLASS_TANK ,"国服(普通僵尸装饰)" } },
        { SKIN_CHNSPEED, { ZOMBIE_CLASS_SPEED ,"国服(暗影芭比装饰)" } },
        { SKIN_CHNPC, { ZOMBIE_CLASS_PC ,"国服(迷雾鬼影装饰)" } },
        { SKIN_COSSPEED1, { ZOMBIE_CLASS_SPEED , "血腥玛丽(暗影芭比装饰)" } },
        { SKIN_SANTASKINLIGHT, { ZOMBIE_CLASS_SPEED ,"圣诞派对(暗影芭比装饰)" } },
        { SKIN_SANTASKINHEAVY, { ZOMBIE_CLASS_HEAVY ,"圣诞派对(憎恶屠夫装饰)"} },
        { SKIN_CHEFSKINHEAVY, { ZOMBIE_CLASS_HEAVY ,"特级厨师(憎恶屠夫装饰)"} },
        { SKIN_NURSESKINRESI, { ZOMBIE_CLASS_RESIDENT ,"死亡天使(痛苦女王装饰)"} },
        { SKIN_TWSKINHEAVY, { ZOMBIE_CLASS_HEAVY ,"电音三太子(憎恶屠夫装饰)"} },
        { SKIN_DEVILSKINLIGHT, { ZOMBIE_CLASS_SPEED ,"黑夜魔女(暗影芭比装饰)" } },
        { SKIN_STONEDEIMOS2, { ZOMBIE_CLASS_DEIMOS2 ,"晶石魔像(恶魔猎手装饰)" } },
        { SKIN_MUMMYSTAMPER, { ZOMBIE_CLASS_STAMPER ,"木乃伊(送葬者装饰)" } },
        { SKIN_STUDENTLIGHT, { ZOMBIE_CLASS_SPEED ,"黑暗萝莉(暗影芭比装饰)" } },
        { SKIN_STONEBOOMER, { ZOMBIE_CLASS_BOOMER ,"晶石巨像(腐化暴君装饰)" } },
        { SKIN_Y19S2BLOTERSKIN, { ZOMBIE_CLASS_PASS ,"稻草人(爆弹狂魔装饰)" } },
        { SKIN_Y19S2LIGHTSKIN, { ZOMBIE_CLASS_SPEED ,"西部牛仔(暗影芭比装饰)" } },
        { SKIN_Y19S4PSYCHO, { ZOMBIE_CLASS_PC ,"海盗船长(迷雾鬼影装饰)" } },
        { SKIN_Y19S4RUSTYWING, { ZOMBIE_CLASS_FLYING ,"堕落天使(断翼恶灵装饰)" } },
        { SKIN_Y20S1DOCTORSKIN, { ZOMBIE_CLASS_HEAL ,"祸乱朝堂(巫蛊术尸装饰)" } },
        { SKIN_Y20S1WITCHSKIN, { ZOMBIE_CLASS_WITCH ,"村野民妇(嗜血女妖装饰)" } },
        { SKIN_Y20S2RESISKIN, { ZOMBIE_CLASS_RESIDENT ,"南方女巫(痛苦女王装饰)" } },
        { SKIN_Y20S2BOOTSKIN, { ZOMBIE_CLASS_BOOSTER ,"西方女巫(追猎傀儡装饰)" } },
        { SKIN_Y20S3SPEEDSKIN, { ZOMBIE_CLASS_SPEED ,"人工智能(暗影芭比装饰)" } },
        { SKIN_Y20S3STAMPERSKIN, { ZOMBIE_CLASS_STAMPER ,"街头派对(送葬者装饰)" } },
        { SKIN_Y21S1AKSHA, { ZOMBIE_CLASS_AKSHA ,"冰霜巨魔(赤炎恶鬼装饰)" } },
        { SKIN_Y21S1FLYINGZOMBIE, { ZOMBIE_CLASS_FLYING ,"冰雪妖姬(断翼恶灵装饰)" } },
        { SKIN_Y21S2DEIMOSSKIN, { ZOMBIE_CLASS_DEIMOS ,"半兽人(恶魔之子装饰)" } },
        { SKIN_Y21S2WITCHSKIN, { ZOMBIE_CLASS_WITCH ,"黑暗精灵(嗜血女妖装饰)" } },
        { SKIN_Y21S3DEIMOS2SKIN, { ZOMBIE_CLASS_DEIMOS2 ,"拘束装甲(恶魔猎手装饰)" } },
        { SKIN_Y21S3BOOSTERZOMBISKIN, { ZOMBIE_CLASS_BOOSTER ,"自动人偶(追猎傀儡装饰)" } },
        { SKIN_Y21S4BLOTERSKIN, { ZOMBIE_CLASS_PASS ,"双脊龙(爆弹狂魔装饰)" } },
        { SKIN_Y21S4BOOMERSKIN, { ZOMBIE_CLASS_BOOMER ,"盾甲龙(腐化禁卫装饰)" } },
        { SKIN_Y23S1NORMAL, { ZOMBIE_CLASS_TANK ,"机械殖装(普通僵尸装饰)" } },
        { SKIN_Y23S2WITCHSKIN, { ZOMBIE_CLASS_WITCH ,"红心王后(嗜血女妖装饰)" } },
        { SKIN_Y23S3NORMAL, { ZOMBIE_CLASS_TANK ,"沙漠守卫(普通僵尸装饰)" } },
        
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

    void ShowZombieSkinMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("僵尸皮肤管理");

            g_ZombieSkinName.size();

            for (auto it = g_ZombieSkinName.begin(); it != g_ZombieSkinName.end(); it++) {

                char szMenuSkinName[64];

                if (adminid->m_iZombieClassSkin[it->second.ClassType] == it->first)
                    Q_snprintf(szMenuSkinName, sizeof(szMenuSkinName), "%s[使用中]", it->second.SkinName);
                else
                    Q_snprintf(szMenuSkinName, sizeof(szMenuSkinName), "%s", it->second.SkinName);

                if (context.item(it->second.SkinName, szMenuSkinName))
                {
                    if (adminid->m_iZombieClassSkin[it->second.ClassType] == it->first)
                    {
                        adminid->m_iZombieClassSkin[it->second.ClassType] = 0;

                        PrintToChat(adminid, "已清除皮肤");
                    }
                    else
                    {
                        adminid->m_iZombieClassSkin[it->second.ClassType] = it->first;

                        PrintToChat(adminid, "已应用皮肤");
                    }
                }
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

            if (context.item("zombieskinmenu", "僵尸皮肤管理"))
                ShowZombieSkinMenu(playerid);

            context.end(playerid);
        });
    }
    }
}