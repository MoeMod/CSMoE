cmake_minimum_required(VERSION  3.10.2)

add_library(luash STATIC
	luash.cpp
 "luash_class.hpp")
target_include_directories(luash PUBLIC "${CMAKE_SOURCE_DIR}/luash")
target_include_directories(luash PUBLIC "${CMAKE_SOURCE_DIR}/public")
#target_compile_definitions(luash PUBLIC "-DLUA_32BITS=1")
target_compile_definitions(luash PUBLIC "-DXASH_LUASH=1")
target_link_libraries(luash PUBLIC luajit_3p nameof_3p)