cmake_minimum_required(VERSION 3.12)

project(mpg123 LANGUAGES C ASM)

include(cmake/search_libs.cmake)

set(PACKAGE_VERSION ${PROJECT_VERSION})
add_subdirectory("src")
