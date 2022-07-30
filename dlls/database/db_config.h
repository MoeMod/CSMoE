#pragma once

#include <string>

namespace db {

struct DBConfig
{
    std::string host;
    int port;
    std::string user;
    std::string pass;
    std::string schema;
};

const DBConfig &GetDBConfig();

}