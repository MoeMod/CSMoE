#ifdef XASH_DEDICATED
#include "db_config.h"

#include <fstream>
namespace db
{

const DBConfig & GetDBConfig()
{
    static auto host = "localhost";
    static auto port = 3306;
    static auto user = "root";
    static auto pass = "root";
    static auto tuple = "csmoe";
    static DBConfig x = {host, port, user, pass, tuple};
    return x;
}

}
#endif