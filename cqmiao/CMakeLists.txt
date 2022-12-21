cmake_minimum_required(VERSION  3.10.2)

set(CMAKE_CXX_STANDARD 20)

add_library(cqmiao STATIC
		net_buffer.cpp
		TSourceEngineQuery.cpp
		Reciter.cpp
        pinyin.cpp
		strsearch.cpp)
target_include_directories(cqmiao PRIVATE "${CMAKE_SOURCE_DIR}/cl_dll")
target_include_directories(cqmiao PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(cqmiao PUBLIC Boost::asio Boost::beast openssl_3p json_3p)