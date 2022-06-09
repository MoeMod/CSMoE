
#include "extdll.h"
#include "cbase.h"
#include "util.h"
#include "gamerules.h"
#include "weapons_moe_buy.h"
#include "player/player_mod_strategy.h"
#include "util/u_range.hpp"

#include "admin.h"
#include "immenu.h"

namespace sv {
    void SV_Start_VoteMaps();
    namespace admin {
        constexpr const char * g_szReasonForPunishPlayer[] = {
                "根据玩家投票要求",
                "发生BUG需要处理",
                "模式或活动的需要",
                "恶意堵路",
                "基建恶意拆迁/堵门",
                "越狱/匪镇乱杀",
                "越狱/逃跑不听指挥",
                "越狱/逃跑乱用道具",
                "恶意抢狱长/指挥",
                "浪费游戏资源",
                "长时间不在位置(挂机)",
                "恶意坑队友(闪光弹/震荡弹)",
                "恶意卡屏(烟雾弹/手电筒)",
                "使用禁用道具/途径",
                "卡神(PING非常高)",
                "未知来源的玩家(假人)",
                "使用透视外挂",
                "使用自瞄外挂",
                "使用加速/瞬移外挂",
                "使用连跳类辅助脚本",
                "使用刷分挂机脚本",
                "地图BUG干扰游戏(卡天卡门)",
                "利用BUG刷任务/道具",
                "利用BUG炸服/卡服",
                "发送无关广告/拉人",
                "辱骂服务器及管理员",
                "祖安玩家",
                "聊天灌水/麦克风噪音",
                "搞黄色",
                "恶意发起投票换图/踢人",
                "对于管理员有过分要求",
                "白嫖道具/活动",
                "恶意刷分",
                "装萌新",
                "这人傻逼到无法形容",
                "不喜欢小白白",
                "长得太丑",
                "不符合社会主义核心价值观",
                "有男/女朋友"
        };

        constexpr const char * g_szReasonForMap[] = {
                "根据玩家投票要求",
                "发生BUG需要处理",
                "模式或活动的需要",
                "这个地图不适合这个模式",
                "这个地图非常不平衡",
                "这个地图BUG非常多",
                "这个地图刚才玩过了",
                "这个地图大家玩腻了",
                "需要更新服务器模式"
        };

        void PrintToChat(CBasePlayer *target, const std::string &what)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgSayText, {}, target->edict());
            WRITE_BYTE(0);
            WRITE_STRING(what.c_str());
            MESSAGE_END();
        }

        void PrintToChatAll(const std::string &what)
        {
            MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
            WRITE_BYTE(0);
            WRITE_STRING(what.c_str());
            MESSAGE_END();
        }

        std::string GetPlayerName(CBasePlayer *player)
        {
            return STRING(player->pev->netname);
        }

        void KickAndBan(CBasePlayer *target, int time = 30)
        {
            SERVER_COMMAND(UTIL_VarArgs("kick #%d ^\"%s^\";wait;banid ^\"0^\" ^\"%s^\";wait;writeid\n", GETPLAYERUSERID(target->edict()), "你被管理员封禁",GETPLAYERAUTHID(target->edict())));
        }

        void LogAndPrintAdmin(CBasePlayer *adminid, const std::string &action, const std::string &reason, const std::string &targetname = "")
        {
            std::string adminname = GetPlayerName(adminid);
            if (targetname.empty())
            {
                PrintToChatAll(" ^3[柑橘CitruS] ^7 管理员 ^2" + adminname + " ^7因为" + reason + "使用权限 ^2" + action + " ^7");
                UTIL_LogPrintf("%s 因为 %s 使用权限 %s", adminname.c_str(), reason.c_str(), action.c_str());
            }
            else
            {
                PrintToChatAll(" ^3[柑橘CitruS] ^7 管理员 ^2" + adminname + " ^7因为" + reason + "把 ^2" + targetname + " ^7进行 ^2" + action + " ^7");
                UTIL_LogPrintf("%s 因为 %s 把 %s 进行", adminname.c_str(), reason.c_str(), targetname.c_str(), action.c_str());
            }

        }

        void DelayedChangeLevel(std::string map)
        {
            CHANGE_LEVEL(map.c_str(), NULL);
        }

        template<class InputIter, class CallbackFn = void(*)()>
        void ShowActionReasonMenu(CBasePlayer *adminid, const std::string& szAction, InputIter reasonsfirst, InputIter reasonslast, CallbackFn&& fnAction, const std::string& szTarget = "")
        {
            static_assert(std::is_invocable<CallbackFn>::value);
            ImMenu(adminid, [szAction, fnAction, reasonsfirst, reasonslast, szTarget](auto context, CBasePlayer *adminid) {
                context.begin("管理员装逼菜单 / Admin\n"
                              "选择进行" + szAction + "的原因");

                std::for_each(reasonsfirst, reasonslast, [&](auto &&reason){
                    if (context.item(reason, reason))
                    {
                        fnAction();
                        LogAndPrintAdmin(adminid, szAction, reason, szTarget);
                    }
                });

                context.end(adminid);
            });
        }

        template<class CallbackFn = void(*)(CBasePlayer * target), class FilterFn = bool(*)(CBasePlayer *target)>
        void ShowSelectTargetPlayerMenu(CBasePlayer *adminid, const std::string& szAction, CallbackFn&& fnAction, FilterFn&& fnFilter = [](CBasePlayer *) { return true;  })
        {
            static_assert(std::is_invocable<CallbackFn, CBasePlayer *>::value);
            ImMenu(adminid, [szAction, fnAction, fnFilter](auto &&context, CBasePlayer *adminid){
                context.begin("管理员装逼菜单 / Admin\n"
                              "选择进行" + szAction + "的目标玩家");
                for(auto target : moe::range::PlayersList())
                {
                    std::string targetname = GetPlayerName(target);
                    if (!fnFilter(target))
                        context.disabled();
                    if(context.item(targetname, targetname))
                    {
                        EntityHandle<CBasePlayer> weakPlayer(target);
                        ShowActionReasonMenu(adminid, szAction,
                                             std::begin(g_szReasonForPunishPlayer), std::end(g_szReasonForPunishPlayer),
                                             [fnAction, weakPlayer](){ if(weakPlayer) fnAction(weakPlayer); },
                                             targetname
                        );
                    }
                }
                context.end(adminid);
            });
        }

        template<class CallbackFn = void(*)(std::string map)>
        void ShowSelectMapMenu(CBasePlayer *adminid, const std::string &szAction, CallbackFn &&fnAction)
        {
            static_assert(std::is_invocable<CallbackFn, std::string>::value);
            ImMenu(adminid, [szAction, fnAction](auto context, CBasePlayer *adminid) {
                context.begin("管理员装逼菜单 / Admin\n"
                              "选择进行" + szAction + "的原因");

                for(const auto &[map, i] : g_pGameRules->m_vecVoteMaps)
                {
                    if(context.item(map, map))
                    {
                        ShowActionReasonMenu(adminid, szAction + "(" + map + ")", std::begin(g_szReasonForMap), std::end(g_szReasonForMap), std::bind(fnAction, map));
                    }
                }
                context.end(adminid);
            });
        }

        void EveryoneRTV()
        {
            SV_Start_VoteMaps();
        }

        void RestartGame()
        {
            SERVER_COMMAND("sv_restart 1\n");
        }

        template<class CallbackFn = void(*)(int amount)>
        void ShowSelectAmountMenu(CBasePlayer *adminid, const std::string& szAction, CallbackFn&& fnAction, const std::string& szQuantifier = "个")
        {
            ImMenu(adminid, [szAction, fnAction, szQuantifier](auto context, CBasePlayer *adminid) {
                context.begin("管理员装逼菜单 / Admin\n"
                              "选择要" + szAction + "的数量（单位：" + szQuantifier + "）");

                for (int amount : { 1, 5, 10, 20, 50, 100})
                {
                    if (context.item(std::to_string(amount), std::to_string(amount) + " " + szQuantifier))
                        fnAction(amount);
                }

                context.end(adminid);
            });
        }

        void ShowGiveWeaponMenu(CBasePlayer *adminid)
        {
            ImMenu(adminid, [](auto context, CBasePlayer *adminid) {
                context.begin("管理员装逼菜单 / Admin\n"
                              "选择要发的武器");
                for (const MoEWeaponBuyInfo_s &ii : g_MoEWeaponBuyInfo)
                {
                    if (context.item(ii.pszClassName, ii.pszDisplayName))
                    {
                        ShowSelectTargetPlayerMenu(adminid, std::string() + "发武器(" + ii.pszDisplayName + ")", [ii](CBasePlayer *player) {
                            if(player && player->IsAlive())
                            {
                                switch (ii.iSlot) {
                                case PRIMARY_WEAPON_SLOT:
                                    DropPrimary(player);
                                    break;
                                case PISTOL_SLOT:
                                    DropSecondary(player);
                                    break;
                                case KNIFE_SLOT:             
                                    player->RemovePlayerItem(player->m_rgpPlayerItems[KNIFE_SLOT]);
                                    player->m_iKnifeID = AliasToKnifeType(ii.pszClassName);                  
                                    break;
                                case GRENADE_SLOT:
                                    player->m_iGrenadeID = AliasToGrenadeType(ii.pszClassName);
                                    break;
                                default:
                                    break;
                                }
                                player->GiveNamedItem(ii.pszClassName);
                            }
                        });
                    }
                }
                context.end(adminid);
            });
        }

        bool IsAdmin(CBasePlayer *adminid)
        {
            return true;
        }

        void ShowAdminMenu(CBasePlayer *adminid)
        {
            if(!IsAdmin(adminid))
            {
                PrintToChat(adminid, " ^3[柑橘CitruS] ^7 您不是管理员，不能装逼。");
                return;
            }

            ImMenu(adminid, [](auto context, CBasePlayer *adminid){
                context.begin("管理员装逼菜单 / Admin");
                    if (context.item("warn", "警告"))
                        ShowSelectTargetPlayerMenu(adminid, "警告", [](CBasePlayer * target) { return true; });
                    if (context.item("kill", "处死"))
                        ShowSelectTargetPlayerMenu(adminid, "处死", [](CBasePlayer * target) { return ClientKill(target->edict()), true; });
                    if (context.item("kick", "踢出"))
                        ShowSelectTargetPlayerMenu(adminid, "踢出", [](CBasePlayer * target) { return KickAndBan(target, 30), true; });
                    if (context.item("respawn", "复活"))
                        ShowSelectTargetPlayerMenu(adminid, "复活", [](CBasePlayer * target) { return target->RoundRespawn(), true; });
                    if (context.item("teamct", "传送至CT"))
                        ShowSelectTargetPlayerMenu(adminid, "传送至CT", [](CBasePlayer *target) {
                            target->pev->team = CT;
                            target->m_iTeam = CT;
                            TeamChangeUpdate(target, target->m_iTeam);
                            return true;
                        }, [adminid](CBasePlayer *target) { return target->m_iTeam != CT; });
                    if (context.item("teamt", "传送至TR"))
                        ShowSelectTargetPlayerMenu(adminid, "传送至TR", [](CBasePlayer *target) {
                            target->pev->team = TERRORIST;
                            target->m_iTeam = TERRORIST;
                            TeamChangeUpdate(target, target->m_iTeam);
                            return true;
                        }, [adminid](CBasePlayer *target) { return target->m_iTeam != TERRORIST; });

                    if (context.item("teamspec", "变成观察者"))
                        ShowSelectTargetPlayerMenu(adminid, "变成观察者", [](CBasePlayer * target) {
                            ClientKill(target->edict());
                            target->pev->team = SPECTATOR;
                            target->m_iTeam = SPECTATOR;
                            TeamChangeUpdate(target, target->m_iTeam);
                            return true;
                        }, [](CBasePlayer *target) { return target->m_iTeam != SPECTATOR; });

                    if (context.item("everyone_rtv", "开启RTV菜单"))
                        ShowActionReasonMenu(adminid, "开启RTV菜单", std::begin(g_szReasonForMap), std::end(g_szReasonForMap), EveryoneRTV);
                    if (context.item("restart", "刷新服务器"))
                        ShowActionReasonMenu(adminid, "刷新服务器", std::begin(g_szReasonForMap), std::end(g_szReasonForMap), RestartGame);
                    if (context.item("terminate", "结束回合"))
                        ShowActionReasonMenu(adminid, "结束回合", std::begin(g_szReasonForMap), std::end(g_szReasonForMap), [] { g_pGameRules->TerminateRound(6.0s, WINSTATUS_DRAW); });
                    if (context.item("changelevel", "强制换图"))
                        ShowSelectMapMenu(adminid, "强制换图", DelayedChangeLevel);

                    if (context.item("give_wpn", "发枪"))
                        ShowGiveWeaponMenu(adminid);
                    if (context.item("give_ammo", "发子弹"))
                        ShowSelectTargetPlayerMenu(adminid, "发子弹", [](CBasePlayer * target) {
                            for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT, KNIFE_SLOT })
                                target->m_pModStrategy->GiveSlotAmmo(target, iSlot);
                            return true;
                        });
                    if (context.item("give_default_weapon", "重发随机武器"))
                        ShowSelectTargetPlayerMenu(adminid, "重发随机武器", [](CBasePlayer * target) {
                            target->GiveDefaultItems();
                            return true;
                        });
                    if (context.item("disarm", "缴枪"))
                        ShowSelectTargetPlayerMenu(adminid, "缴枪", [](CBasePlayer *target) { return target->RemoveAllItems(false), true; });
                context.end(adminid);
            });
        }
    }
}