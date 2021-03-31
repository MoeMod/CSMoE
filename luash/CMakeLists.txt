cmake_minimum_required(VERSION  3.10.2)

include(FindLua)
find_package(Lua REQUIRED)

add_library(luash INTERFACE)
target_link_libraries(luash INTERFACE ${LUA_LIBRARIES})
target_include_directories(luash INTERFACE ${LUA_INCLUDE_DIR})
target_include_directories(luash INTERFACE "${CMAKE_SOURCE_DIR}/luash")
target_compile_definitions(luash INTERFACE "-DLUA_32BITS=1")
target_compile_definitions(luash INTERFACE "-DXASH_LUASH=1")