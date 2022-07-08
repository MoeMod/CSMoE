#pragma once

#include <memory>
#include <string>
#include <mutex>

#include <vector>
#include "MySqlConnection.h"
#include "DatabaseConfig.h"

class MySqlConnection;

class MySqlConnectionPool
{
public:
    MySqlConnectionPool(boost::asio::io_context &ioc, const DatabaseConfig &c = GetDatabaseConfig());
    ~MySqlConnectionPool();

public:
    // ensures not nullptr
    std::shared_ptr<boost::mysql::tcp_connection> acquire();
    void clear();
    void reserve(size_t n);

private:
    boost::asio::io_context &ioc;
    std::mutex m;
    std::vector<std::shared_ptr<MySqlConnection>> v;
    DatabaseConfig config;
};
