#include "DatabaseConfig.h"

#define JM_XORSTR_DISABLE_AVX_INTRINSICS
#include <xorstr.hpp>

#include <fstream>
const DatabaseConfig & GetDatabaseConfig()
{
    static auto host = xorstr("z4cs.com");
    static auto port = xorstr("3306");
    static auto user = xorstr("root");
    static auto pass = xorstr("111503");
    static auto tuple = xorstr("hy");
    static DatabaseConfig x = {host.crypt_get(), port.crypt_get(), user.crypt_get(), pass.crypt_get(), tuple.crypt_get()};
    return x;
}