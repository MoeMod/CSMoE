#ifdef XASH_DEDICATED

#include "db_main.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "debug.h"

#include <sstream>


namespace db {

CDataBaseManager* g_pDataBase;

static QQTokenData UserQQTokenDataFromSqlResult(MYSQL_RES* result)
{
    if (!result)
        return {};

    MYSQL_ROW row = mysql_fetch_row(result);
    mysql_free_result(result);
    if (!row)
        return {};

    std::stringstream ss;
    ss << row[0];
    int64_t r0; // qq
    ss >> r0;
    ss.clear();
    ss.str("");

    ss << row[1]; // token
    int32_t r1;
    ss >> r1;


    ss << row[2]; // timestamp
    uint32_t r2;
    ss >> r2;
    
    return QQTokenData{ r0, r1, r2 };
}

static AccountData UserAccountDataFromSqlResult(MYSQL_RES* result)
{
    if (!result)
        return {};

    MYSQL_ROW row = mysql_fetch_row(result);
    mysql_free_result(result);
    if(!row)
        return {};

    std::stringstream ss;
    ss << row[0];
    int64_t r0; // uid
    ss >> r0;
    ss.clear();
    ss.str("");

    ss << row[1]; // qq
    int32_t r1;
    ss >> r1;

    ss << row[2];
    uint32_t r2; // logintime
    ss >> r2;
    ss.clear();
    ss.str("");

    ss << row[5];
    uint32_t r5;
    ss >> r5;

    return AccountData{ r0, r1, r2, std::string(row[3]), std::string(row[4]), r5};
}

boost::asio::io_context ioc_database;
boost::asio::thread_pool ioc_database_worker;
std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> ioc_database_work_guard;
std::optional<boost::asio::executor_work_guard<boost::asio::thread_pool::executor_type>> ioc_database_worker_work_guard;

CDataBaseManager::CDataBaseManager(const DBConfig & c) : config(c)
{

}

CDataBaseManager::~CDataBaseManager() = default;

void CDataBaseManager::ServerActivate()
{
    if (!(mysql_local = mysql_init(NULL)))
    {
		sv::UTIL_DPrintf("mysql_init() failed\n");
        return;
    }

    if (!mysql_real_connect(mysql_local, config.host.c_str(), config.user.c_str(), config.pass.c_str(), config.schema.c_str(), config.port, NULL, 0))
    {
        sv::UTIL_DPrintf("mysql_real_connect() failed\n");
        mysql_close(mysql_local);
        return;
    }
    sv::UTIL_DPrintf("mysql_real_connect() successful\n");

    ioc_database.restart();
    ioc_database_work_guard.emplace(boost::asio::make_work_guard(ioc_database));
    ioc_database_worker_work_guard.emplace(boost::asio::make_work_guard(ioc_database_worker));

    /* for (int i = 0; i < 15; i++) {
        //InsertAccountData(1282788018 + i, i, std::to_string(i));

        InsertAccountDataAsync(1282788018 + i, std::to_string(i), std::to_string(i), [i]() {
        });

        QueryAccountDataAsync(1282788018 + i, [this, i](AccountData data) {
            if(IsDataValid(data))
                sv::UTIL_DPrintf("#%d test %d %d\n", i, data.uid, data.qq);
            else
                sv::UTIL_DPrintf("#%d error\n", i);
            });
    }*/
}

void CDataBaseManager::ServerDeactivate()
{
    // ioc_database_work_guard.reset();
    // ioc_database_worker_work_guard.reset();
    // ioc_database.stop();
    // ioc_database_worker.stop();
    if (mysql_local)
    {
        mysql_close(mysql_local);
    }
}

void CDataBaseManager::StartFrame()
{
    if(!mysql_local)
    {
        return;
    }

    ioc_database.poll();
}

MYSQL_RES* CDataBaseManager::Query(std::string sql)
{
    std::lock_guard l(m); // 先加锁
    if(!mysql_local)
    {
        sv::UTIL_DPrintf("mysql was not inited");
        return NULL;
    }

    const char* stmt_text = sql.c_str();
    if (mysql_real_query(mysql_local, stmt_text, (unsigned long)strlen(stmt_text)))
    {
        sv::UTIL_DPrintf("Could not execute statement(s): %s\n", mysql_error(mysql_local));
        return NULL;
    }
    
    // 获取本次请求的结果集
    MYSQL_RES *result = mysql_store_result(mysql_local);
    if (!result)
    {
        if (mysql_field_count(mysql_local) == 0)
        {
            sv::UTIL_DPrintf("%lld rows affected\n", mysql_affected_rows(mysql_local));
        }
        else  /* some error occurred */
        {
            sv::UTIL_DPrintf("Error: %s\n", mysql_error(mysql_local));
        }
        return NULL;
    }

    return result;
}

void CDataBaseManager::AsyncQuery(std::string sql, std::function<void(MYSQL_RES*)> callback)
{
    boost::asio::dispatch(ioc_database_worker, [this, sql, callback]() {
        auto result = Query(sql);
        boost::asio::dispatch(ioc_database, std::bind(callback, result));
    });
}

std::string CDataBaseManager::QueryQQTokenDataSQL(int64_t qq, int32_t token)
{
    return std::string("SELECT * FROM qqtoken WHERE `qq` = '" + std::to_string(qq) + "' AND `token` = '" + std::to_string(token) + "';");
}

QQTokenData CDataBaseManager::QueryQQTokenData(int64_t qq, int32_t token)
{
    return UserQQTokenDataFromSqlResult(Query(QueryQQTokenDataSQL(qq, token)));
}

void CDataBaseManager::QueryQQTokenDataAsync(int64_t qq, int32_t token, std::function<void(QQTokenData)> callback)
{
    AsyncQuery(QueryQQTokenDataSQL(qq, token), [callback](auto result) {
        callback(UserQQTokenDataFromSqlResult(result));
    });
}

std::string CDataBaseManager::InsertQQTokenDataSQL(int64_t qq, int32_t token)
{
    return std::string("REPLACE INTO qqtoken (qq, token, timestamp) VALUES ('" + std::to_string(qq) + "','" + std::to_string(token) + "',unix_timestamp(now()));");
}

void CDataBaseManager::InsertQQTokenData(int64_t qq, int32_t token)
{
    Query(InsertQQTokenDataSQL(qq, token));
}

void CDataBaseManager::InsertQQTokenDataAsync(int64_t qq, int32_t token, std::optional<std::function<void(void)>> callback)
{
    AsyncQuery(InsertQQTokenDataSQL(qq, token), [callback](auto result) {
        if (callback) callback.value();
    });
}

std::string CDataBaseManager::QueryAccountDataSQL(int64_t qq)
{
	return std::string("SELECT uid, qq, logintime, loginauth, loginip, bantime FROM account WHERE `qq` = '" + std::to_string(qq) + "';");
}

AccountData CDataBaseManager::QueryAccountData(int64_t qq)
{
    return UserAccountDataFromSqlResult(Query(QueryAccountDataSQL(qq)));
}

void CDataBaseManager::QueryAccountDataAsync(int64_t qq, std::function<void(AccountData)> callback)
{
    AsyncQuery(QueryAccountDataSQL(qq), [callback](auto result){
        callback(UserAccountDataFromSqlResult(result));
    });
}

std::string CDataBaseManager::InsertAccountDataSQL(int64_t qq, std::string loginauth, std::string loginip)
{
    return std::string("INSERT IGNORE INTO account (qq, logintime, loginauth, loginip) VALUES ('" + std::to_string(qq) + "',unix_timestamp(now()),'" + loginauth + "','" + loginip + "');");
}

void CDataBaseManager::InsertAccountData(int64_t qq, std::string loginauth, std::string loginip)
{
    Query(InsertAccountDataSQL(qq, loginauth, loginip));
}

void CDataBaseManager::InsertAccountDataAsync(int64_t qq, std::string loginauth, std::string loginip, std::optional<std::function<void(void)>> callback)
{
    AsyncQuery(InsertAccountDataSQL(qq, loginauth, loginip), [callback](auto result){
        if (callback) callback.value();
    });
}

void CDataBaseManager::ClientPutInServer(sv::CBasePlayer* pPlayer)
{
    char* infobuffer = sv::GET_INFO_BUFFER(pPlayer->edict());
    const char* pszQQ = sv::GET_KEY_VALUE(infobuffer, "qq");
    const char* pszToken = sv::GET_KEY_VALUE(infobuffer, "token");

    if (Q_strcmp(pszQQ, ""))
    {
        std::stringstream ss;
        ss << pszQQ;
        ss >> pPlayer->m_iQQ;
    }
    else
        pPlayer->m_iQQ = 0;

    if (Q_strcmp(pszToken, ""))
        pPlayer->m_iToken = Q_atoi(pszToken);
    else
        pPlayer->m_iToken = 0;
}

void CDataBaseManager::ClientUserInfoChanged(sv::CBasePlayer* pPlayer, char* infobuffer)
{
    const char* pszQQ = sv::GET_KEY_VALUE(infobuffer, "qq");
    const char* pszToken = sv::GET_KEY_VALUE(infobuffer, "token");
    auto oldQQ = pPlayer->m_iQQ;
    auto oldToken = pPlayer->m_iToken;
    if (Q_strcmp(pszQQ, ""))
    {
        std::stringstream ss;
        ss << pszQQ;
        ss >> pPlayer->m_iQQ;
    }
    else
        pPlayer->m_iQQ = 0;

    if (Q_strcmp(pszToken, ""))
        pPlayer->m_iToken = Q_atoi(pszToken);
    else
        pPlayer->m_iToken = 0;

    if ((oldQQ != pPlayer->m_iQQ || oldToken != pPlayer->m_iToken))
    {
        if(pPlayer->m_bLogin)
            pPlayer->m_bLogin = false;
    }
}

void CDataBaseManager::ClientToken(sv::CBasePlayer* pPlayer)
{
    if (pPlayer->m_iQQ <= 0 || pPlayer->m_iQQ >= 10000000000)
    {
        sv::ClientPrint(pPlayer->pev, HUD_PRINTTALK, "* please enter qq");
        return;
    }

    if (pPlayer->m_bLogin)
    {
        sv::ClientPrint(pPlayer->pev, HUD_PRINTTALK, "* already login");
        return;
    }

    std::random_device rd;
    InsertQQTokenDataAsync(pPlayer->m_iQQ, std::uniform_int_distribution<int32_t>(100000, 999999)(rd));
    sv::ClientPrint(pPlayer->pev, HUD_PRINTTALK, "* go to chat to qqbot and send 'login'");
}

void CDataBaseManager::ClientLogin(sv::CBasePlayer* pPlayer)
{
    if (pPlayer->m_iToken <= 0 || pPlayer->m_iQQ <= 0)
    {
        sv::ClientPrint(pPlayer->pev, HUD_PRINTTALK, "* please enter qq or token");
        return;
    }

    if (pPlayer->m_bLogin)
    {
        sv::ClientPrint(pPlayer->pev, HUD_PRINTTALK, "* already login");
        return;
    }

    auto index = pPlayer->entindex();
    auto qq = pPlayer->m_iQQ;
    auto token = pPlayer->m_iToken;
    QueryQQTokenDataAsync(qq, token, [this, qq, index](auto data) {
        sv::CBasePlayer* pPlayer = static_cast<sv::CBasePlayer*>(sv::UTIL_PlayerByIndex(index));
        if (!pPlayer)
        {
            sv::UTIL_DPrintf("player disconnected\n");
            return;
        }

        if (pPlayer->m_iQQ != qq)
        {
            sv::UTIL_DPrintf("qq changed\n");
            return;
        }

        if (!data.qq)
        {
            sv::ClientPrint(pPlayer->pev, HUD_PRINTTALK, "* token error");
            return;
        }
        
        QueryAccountDataAsync(qq, [this, index, qq](auto data) {

            // registered
            if (IsDataValid(data))
                return;

            sv::CBasePlayer* pPlayer = static_cast<sv::CBasePlayer*>(sv::UTIL_PlayerByIndex(index));
            if (!pPlayer)
            {
                sv::UTIL_DPrintf("player disconnected\n");
                return;
            }

            if (pPlayer->m_iQQ != qq)
            {
                sv::UTIL_DPrintf("qq changed\n");
                return;
            }

            // TODO get player ip
            InsertAccountDataAsync(qq, sv::GETPLAYERAUTHID(pPlayer->edict()), "");
        });
        pPlayer->m_bLogin = true;
        sv::ClientPrint(pPlayer->pev, HUD_PRINTTALK, "* login successful");
    });
}

void InstallDataBaseManager()
{
    if (g_pDataBase != NULL)
    {
        delete g_pDataBase;
        g_pDataBase = NULL;
    }

    g_pDataBase = new CDataBaseManager;
}
}
#endif