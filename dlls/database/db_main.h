#include "db_config.h"

#include "mysql.h"
#include "mysqld_error.h"

#include <string>
#include <optional>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

namespace db {

struct UidData
{
    /** 唯一ID */
    int64_t uid = 0;
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
    void ClientToken(sv::CBasePlayer* pPlayer);
    void ClientLogin(sv::CBasePlayer* pPlayer);

    MYSQL_RES* Query(std::string sql);
    void AsyncQuery(std::string sql, std::function<void(MYSQL_RES*)> callback);

    std::string QueryQQTokenDataSQL(int64_t qq, int32_t token);
    QQTokenData QueryQQTokenData(int64_t qq, int32_t token);
    void QueryQQTokenDataAsync(int64_t qq, int32_t token, std::function<void(QQTokenData)> callback);

    std::string InsertQQTokenDataSQL(int64_t qq, int32_t token);
    void InsertQQTokenData(int64_t qq, int32_t token);
    void InsertQQTokenDataAsync(int64_t qq, int32_t token, std::optional<std::function<void(void)>> callback = std::nullopt);

    std::string QueryAccountDataSQL(int64_t qq);
    AccountData QueryAccountData(int64_t qq);
    void QueryAccountDataAsync(int64_t qq, std::function<void(AccountData)> callback);

    std::string InsertAccountDataSQL(int64_t qq, std::string loginauth, std::string loginip);
    void InsertAccountData(int64_t qq, std::string loginauth, std::string loginip);
    void InsertAccountDataAsync(int64_t qq, std::string loginauth, std::string loginip, std::optional<std::function<void(void)>> callback = std::nullopt);

    bool IsDataValid(UidData data) const { return data.uid != 0; }
private:
    std::mutex m;
    DBConfig config;
    net_async_status async_status;
    int status;
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
    std::map<int64_t, QQTokenData> m_qqtokens;
    std::map<int64_t, AccountData> m_accounts;
    std::map<int64_t, PlayerData> m_players;
};

extern CDataBaseManager* g_pDataBase;
void InstallDataBaseManager();
}