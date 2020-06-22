project(csmoe)
cmake_minimum_required(VERSION  2.8.0)

# Install custom module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

option(XASH_STATIC_GAMELIB "Build static game library" ON)

option(XASH_DEDICATED "Enable dedicated build. Servers must choose this." OFF) # Servers must choose this

option(XASH_PCH "Enable PCH support." ON)

option(XASH_VCPKG "Enable VCPKG support." OFF)

if(XASH_PCH)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/CMakePCHCompiler/")
	include(CMakePCHCompiler)
endif()

if(XASH_PCH AND XASH_VCPKG)
	message(FATAL_ERROR "XASH_PCH and XASH_VCPKG are not compatible")
endif()

if(XASH_STATIC_GAMELIB)
	add_definitions(-DXASH_STATIC_GAMELIB=1)
endif()

if(NOT XASH_DEDICATED)
	option(XASH_IMGUI "Imgui support for CSMoE" ON)
else()
	set(XASH_IMGUI OFF)
endif()

if (APPLE OR ANDROID)
	add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-invalid-offsetof)
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os")
	set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -g")
elseif(MSVC)
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Od")
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ox")
	set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -O1")
	set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O1")
else()
	add_compile_options( -Wall -Wextra -Wno-unused-parameter -Wl,--no-undefined )
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -O3")
	set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os")
	set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -ggdb -O3")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-Wpedantic)
	add_compile_options(-fPIC)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-Wno-unused-private-field)
endif()


if(MSVC)
	# force xp build
	add_definitions(-D_USING_V110_SDK71_)

	# MultiByte charset
	add_compile_options(/utf-8)

	# static CRT
	if(XASH_STATIC_GAMELIB)
		set(CompilerFlags
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL
			CMAKE_CXX_FLAGS_RELWITHDEBINFO
			CMAKE_C_FLAGS
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			CMAKE_C_FLAGS_MINSIZEREL
			CMAKE_C_FLAGS_RELWITHDEBINFO
			)
		foreach(CompilerFlag ${CompilerFlags})
		  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
		endforeach()
	endif()
endif()

if(APPLE)
	add_definitions(-DHAVE_STDINT_H)
endif()

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	message(STATUS "Building for 64 Bit")
	set(XASH_64BIT ON)
else()
	message(STATUS "Building for 32 Bit")
endif()

if(NOT ${CMAKE_INSTALL_PREFIX})
	if(WIN32)
		set (CMAKE_INSTALL_PREFIX "D:/CSMoE")
	elseif(APPLE)
		set(CMAKE_INSTALL_PREFIX "/Applications")
	else()
		set (CMAKE_INSTALL_PREFIX "~/CSMoE/")
	endif()
endif()

set(SDL2_PATH ${CMAKE_SOURCE_DIR}/SDL2 CACHE PATH "SDL2 path")

# Desktop or dedicated version options
if(XASH_DEDICATED)
	option(XASH_SDL "Enable SDL." OFF) # Dedicated servers still can use SDL
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" ON)
else()
	option(XASH_SDL "Enable SDL." ON)
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" OFF)
endif()

set(XASH_BUILD_COMMIT "${CMAKE_BUILD_POSTFIX}")
set(SDL_SHARED true)
set(XASH_LIBDL true)

if(XASH_IMGUI)
	# TODO : since vcpkg doesn't provide with imgui_internal.h, just use our own one
    if(XASH_VCPKG AND FALSE)
		find_package(imgui CONFIG REQUIRED)
		add_library(imgui INTERFACE)
		target_link_libraries(imgui INTERFACE imgui::imgui)
	else()
		set(IMGUI_SRC
	#			imgui/imgui_demo.cpp
				imgui/imgui_draw.cpp
				imgui/imgui_widgets.cpp
				imgui/imgui.cpp)
		add_library(imgui STATIC ${IMGUI_SRC})
		target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/imgui)
	endif()
endif()

if(NOT XASH_DEDICATED)
	ADD_SUBDIRECTORY(cl_dll)
	ADD_SUBDIRECTORY(mainui_cpp)
endif()
ADD_SUBDIRECTORY(dlls)
ADD_SUBDIRECTORY(engine)
if(NOT XASH_SINGLE_BINARY AND NOT ANDROID AND NOT IOS)
	ADD_SUBDIRECTORY(game_launch)
endif()
