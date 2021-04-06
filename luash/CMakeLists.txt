cmake_minimum_required(VERSION  3.10.2)

find_package(LuaJit REQUIRED)
add_library(luajit INTERFACE)
target_link_libraries(luajit INTERFACE ${LUAJIT_LIBRARIES})
target_include_directories(luajit INTERFACE ${LUAJIT_INCLUDE_DIR})

find_package(nameof CONFIG REQUIRED)
add_library(nameof INTERFACE)
target_link_libraries(nameof INTERFACE nameof::nameof)

add_library(luash INTERFACE)
target_include_directories(luash INTERFACE "${CMAKE_SOURCE_DIR}/luash")
#target_compile_definitions(luash INTERFACE "-DLUA_32BITS=1")
target_compile_definitions(luash INTERFACE "-DXASH_LUASH=1")
target_link_libraries(luash INTERFACE luajit nameof)