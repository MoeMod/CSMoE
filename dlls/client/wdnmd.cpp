
#include "extdll.h"
#include "cbase.h"
#include "util.h"
#include "gamerules.h"
#include "player/player_mod_strategy.h"
#include "util/u_range.hpp"

#include "wdnmd.h"
#include "immenu.h"

namespace sv {
    namespace wdnmdmenu {
        
    char g_szSounds[24][24];
    time_point_t g_flSoundCD[33];
    int g_iSoundTimes[33];

    constexpr const char* g_szSoundsName[] = {
            "A1高闪来一个好吗,秋梨膏",
            "Nice,我艹",
            "哈哈哈哈哈",
            "牛逼牛逼,我艹",
            "手感来了",
            "套路是死的,人是活的",
            "完了",
            "我艹你,哈哈哈哈哈哈哈",
            "我们家来啦",
            "我起了,一枪秒了有什么好说的",
            "真输不了,真的输不了",
            "又是他!姚志鹏",
            "我艹你妈",
            "我艹了都",
            "我不打了,我退了,我崩溃了",
            "尴尬",
            "唉,我艹,尼玛",
            "白给少年又来了",
            "今天是上分的好日子",
            "你们这群菜逼",
            "杀不到人就吊他",
            "今天是上坟的好日子",
            "稳得很,来嘛来嘛",
            "WGNMD",
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

    void WdnmdSoundPrecache()
    {
        for (int i = 0; i < 24; i++)
        {
            sprintf(g_szSounds[i], "radio/qz/c%d.wav", i + 1);

            PRECACHE_SOUND(g_szSounds[i]);
        }
    }

    void WdnmdResetTime()
    {
        for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
        {
            g_iSoundTimes[iIndex] = 0;
            g_flSoundCD[iIndex] = invalid_time_point;
        }
    }

    void WdnmdResetTime(int idx)
    {
        g_iSoundTimes[idx] = 0;
        g_flSoundCD[idx] = invalid_time_point;
    }

    void ShowWdnmdMenu(CBasePlayer *playerid, int page)
    {
        ImMenu(playerid, [round = g_pGameRules->m_iTotalRoundsPlayed, page](auto context, CBasePlayer * playerid){         
            if (round != g_pGameRules->m_iTotalRoundsPlayed)
                return;
            context.begin("wdnmd语音菜单");
            if (page == -1)
            {
                for (int i = 0; i < 24; i++)
                {
                    if (context.item(g_szSoundsName[i], g_szSoundsName[i]))
                    {
                        int id = playerid->entindex();
                        if (g_flSoundCD[id] > gpGlobals->time)
                        {
                            PrintToChat(playerid, "就算是茄子本人也要喘口气啊wdnmd!");
                            return;
                        }

                        if (10 <= g_iSoundTimes[id])
                        {
                            PrintToChat(playerid, "wdnmd本局语音已达上限!");
                            return;
                        }

                        std::string playername = GetPlayerName(playerid);

                        for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
                        {
                            CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);

                            if (!entity)
                                continue;

                            CBasePlayer* player = static_cast<CBasePlayer*>(entity);

                            if (player->m_bBlockWdnmd)
                                continue;

                            CLIENT_COMMAND(player->edict(), "spk %s\n", g_szSounds[i]);
                        }

                        g_flSoundCD[id] = gpGlobals->time + 3.0s;
                        g_iSoundTimes[id]++;

                        PrintToChatAll("^7[^5" + playername + "^7]" + "^2:" + g_szSoundsName[i]);
                    }
                }
            }
            else
            {
                for (int i = page; i < page + 7; i++)
                {
                    if (context.item(g_szSoundsName[i], g_szSoundsName[i]))
                    {
                        int id = playerid->entindex();
                        if (g_flSoundCD[id] > gpGlobals->time)
                        {
                            PrintToChat(playerid, "就算是茄子本人也要喘口气啊wdnmd!");
                            return;
                        }

                        if (10 <= g_iSoundTimes[id])
                        {
                            PrintToChat(playerid, "wdnmd本局语音已达上限!");
                            return;
                        }

                        std::string playername = GetPlayerName(playerid);

                        for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
                        {
                            CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);

                            if (!entity)
                                continue;

                            CBasePlayer* player = static_cast<CBasePlayer*>(entity);

                            if (player->m_bBlockWdnmd)
                                continue;

                            CLIENT_COMMAND(player->edict(), "spk %s\n", g_szSounds[i]);
                        }

                        g_flSoundCD[id] = gpGlobals->time + 3.0s;
                        g_iSoundTimes[id]++;

                        PrintToChatAll("^7[^5" + playername + "^7]" + "^2:" + g_szSoundsName[i]);
                    }
                }
            }
           
            context.end(playerid);
        });
    }
    }
}