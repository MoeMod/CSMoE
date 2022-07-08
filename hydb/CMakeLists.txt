
set(CMAKE_CXX_STANDARD 20)
add_library(hydb STATIC
        DatabaseConfig.cpp
        DatabaseConfig.h
        MySqlConnectionPool.cpp
        MySqlConnectionPool.h
        MySqlConnection.h
        )

target_include_directories(hydb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hydb PUBLIC Boost::boost Boost::date_time)
target_link_libraries(hydb PUBLIC Boost::mysql openssl_3p xorstr_3p)