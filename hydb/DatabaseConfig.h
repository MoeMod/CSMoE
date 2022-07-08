#pragma once

#include <string>

struct DatabaseConfig
{
    std::string host;
    std::string port;
    std::string user;
    std::string pass;
    std::string schema;
};

const DatabaseConfig &GetDatabaseConfig();