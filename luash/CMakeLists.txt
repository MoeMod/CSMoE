cmake_minimum_required(VERSION  3.10.2)

add_library(luash INTERFACE)
target_include_directories(luash INTERFACE "${CMAKE_SOURCE_DIR}/luash")
#target_compile_definitions(luash INTERFACE "-DLUA_32BITS=1")
target_compile_definitions(luash INTERFACE "-DXASH_LUASH=1")
target_link_libraries(luash INTERFACE luajit_3p nameof_3p)