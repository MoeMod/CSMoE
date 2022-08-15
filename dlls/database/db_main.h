#pragma once

#include "db_config.h"

#include "mysql.h"
#include "mysqld_error.h"

#include <string>
#include <optional>
#include <mutex>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <nlohmann/json.hpp>

namespace db {

struct UidData
{
    /** 唯一ID */
    uint32_t uid = 0;
};

struct QQTokenData
{
    /** QQ */
    int64_t qq = 0;
    /** 验证码 */
    int32_t token = 0;
    /** 时间戳 */
    uint32_t timestamp = 0;
};

struct AccountData : public UidData
{
    /** QQ */
	int64_t qq = 0;
    /** 上次登录时间戳 */
    uint32_t logintime = 0;
    /** 上次登录AUTH */
    std::string loginauth;
    /** 上次登录IP */
    std::string loginip;
    /** 封禁到期时间戳,0代表正常 */
    uint32_t bantime = 0;
};

struct PlayerData : public UidData
{
    /** 等级 */
    uint32_t level = 0;
    /** 经验 */
    uint32_t exp = 0;
    /** 名称 */
    std::string name;
    /** 签名 */
    std::string signature;
    /** 头衔 */
    std::string tag;
};

struct PlayerItemData
{
    /** 道具ID */
    uint32_t id = 0;
    /** 数量 */
    uint32_t num = 0;
    /** 自定义json */
    nlohmann::json custom;
};

class CBasePlayer;
class CDataBaseManager
{
public:
	CDataBaseManager(const DBConfig &c = GetDBConfig());
	~CDataBaseManager();

	void ServerActivate();
	void ServerDeactivate();
    void StartFrame();
    void ClientPutInServer(sv::CBasePlayer *pPlayer);
    void ClientUserInfoChanged(sv::CBasePlayer *pPlayer, char *infobuffer);
    void ClientToken(sv::CBasePlayer* pPlayer, const char* qq);
    void ClientLogin(sv::CBasePlayer* pPlayer, const char* qq, const char* token);
    void ClientLogined(sv::CBasePlayer* pPlayer, uint32_t uid);

    MYSQL_RES* Query(std::string sql);
    void AsyncQuery(std::string sql, std::function<void(MYSQL_RES*)> callback);

    std::string QueryQQTokenDataSQL(std::string qq, std::string token);
    std::optional<QQTokenData> QueryQQTokenData(std::string qq, std::string token);
    void QueryQQTokenDataAsync(std::string qq, std::string token, std::function<void(std::optional<QQTokenData>)> callback);

    std::string InsertQQTokenDataSQL(std::string qq, std::string token);
    void InsertQQTokenData(std::string qq, std::string token);
    void InsertQQTokenDataAsync(std::string qq, std::string token, std::optional<std::function<void(void)>> callback = std::nullopt);

    std::string QueryAccountDataSQL(std::string qq);
    std::optional<AccountData> QueryAccountData(std::string qq);
    void QueryAccountDataAsync(std::string qq, std::function<void(std::optional<AccountData>)> callback);

    std::string InsertAccountDataSQL(std::string qq, std::string loginauth, std::string loginip);
    void InsertAccountData(std::string qq, std::string loginauth, std::string loginip);
    void InsertAccountDataAsync(std::string qq, std::string loginauth, std::string loginip, std::optional<std::function<void(void)>> callback = std::nullopt);

    std::string QueryItemDataSQL(uint32_t uid);
    void QueryItemDataAsync(uint32_t uid, std::function<void(std::optional<std::unordered_map<uint32_t, PlayerItemData>>)> callback);

    std::string InsertItemDataSQL(uint32_t uid, uint32_t num);
    void InsertItemDataAsync(uint32_t uid, uint32_t num, std::optional<std::function<void(void)>> callback = std::nullopt);

private:
    std::mutex m;
    DBConfig config;
    MYSQL *mysql_local;

public:
    auto getQQTokens() { return m_qqtokens; }
    auto getAccounts() { return m_accounts; }
    auto getPlayers() { return m_players; }
    std::optional<QQTokenData> getQQTokenData(int64_t qq) const {
        auto iter = m_qqtokens.find(qq);
        if (iter == m_qqtokens.end())
            return std::nullopt;
        return iter->second;
    }
    std::optional<AccountData> getAccountData(uint32_t uid) const {
        auto iter = m_accounts.find(uid);
        if (iter == m_accounts.end())
            return std::nullopt;
        return iter->second;
    }
    std::optional<PlayerData> getPlayerData(uint32_t uid) const {
        auto iter = m_players.find(uid);
        if (iter == m_players.end())
            return std::nullopt;
        return iter->second;
    }
private:
    std::unordered_map<int64_t, QQTokenData> m_qqtokens;
    std::unordered_map<int64_t, AccountData> m_accounts;
    std::unordered_map<int64_t, PlayerData> m_players;
    std::unordered_map<int64_t, std::unordered_map<uint32_t, PlayerItemData>> m_playeritems;
};

extern CDataBaseManager* g_pDataBase;
void InstallDataBaseManager();
}