project(csmoe)
cmake_minimum_required(VERSION  3.10.2)
enable_language(C CXX ASM)

# Install custom module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

option(XASH_DEDICATED "Enable dedicated build. Servers must choose this." OFF) # Servers must choose this

set(HLSDK_PATH ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to hlsdk")
set(HL2SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/SourceSDK CACHE PATH "Path to hl2sdk")
include(cmake/platform_config.cmake)
include(cmake/video_config.cmake)

# Desktop or dedicated version options
if(XASH_DEDICATED)
	option(XASH_SDL "Enable SDL." OFF) # Dedicated servers still can use SDL
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" ON)
else()
	option(XASH_SDL "Enable SDL." ON)
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" OFF)
endif()

set(XASH_BUILD_COMMIT "${CMAKE_BUILD_POSTFIX}")
set(XASH_LIBDL true)
if(NOT ANDROID AND NOT IOS)
	option(XASH_ANGLE "Use OpenGL ES on desktop platforms" OFF) # use angle on desktop platforms
endif()

set(CMAKE_CXX_STANDARD 20)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
	option(XASH_PCH "Enable PCH support." ON)
	option(XASH_UNITY_BUILD "Enable Unity Build support." OFF)
endif()

option(XASH_VCPKG "Enable VCPKG support." OFF)

if(XASH_DEDICATED)
	set(XASH_SDL  OFF) # Dedicated servers still can use SDL
else()
	option(XASH_SDL "Enable SDL." ON)
endif()
set(XASH_STATIC_GAMELIB ON)

if(XASH_PCH)
	if(${CMAKE_VERSION} VERSION_LESS "3.16.0")
		message(FATAL_ERROR "PCH is not supported. Please update to CMake 3.16 or higher.")
	endif()
	add_definitions(-DXASH_PCH=1)
endif()

if(XASH_UNITY_BUILD)
	if(${CMAKE_VERSION} VERSION_LESS "3.16.0")
		message(FATAL_ERROR "Unity build is not supported. Please update to CMake 3.16 or higher.")
	endif()
	add_definitions(-DXASH_UNITY_BUILD=1)
endif()

if(XASH_STATIC_GAMELIB)
	add_definitions(-DXASH_STATIC_GAMELIB=1)
endif()

if(NOT XASH_DEDICATED)
	option(XASH_IMGUI "Imgui support for CSMoE" ON)
	option(XASH_VGUI2 "VGUI2 support for CSMoE" ON)
	option(XASH_RAGDOLL "Bullet3 RagDoll support for CSMoE" ON)
else()
	set(XASH_IMGUI OFF)
	set(XASH_VGUI2 OFF)
	set(XASH_RAGDOLL OFF)
endif()

option(XASH_LUASH "LUA support for CSMoE. Expected lua 5.3" ON)

if (APPLE OR ANDROID)
	add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-invalid-offsetof -Wno-format-security -Wno-missing-braces -Wno-missing-field-initializers)
elseif(MSVC)

else()
	add_compile_options( -Wall -Wextra -Wno-unused-parameter -Wl,--no-undefined )
endif()

# Important, as HLSDK and engine shares some symbol names!
if(NOT WIN32) # Windows by default have hidden visibility and better linker
	add_compile_options(-fvisibility=hidden)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-Wpedantic)
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

if(XASH_DEDICATED)
	add_definitions(-DXASH_DEDICATED)
endif()

if(APPLE)
	add_definitions(-DHAVE_STDINT_H)
endif()

set(XASH_64BIT PLATFORM_64BIT)

# Desktop or dedicated version options
if(XASH_DEDICATED)
	option(XASH_SDL "Enable SDL." OFF) # Dedicated servers still can use SDL
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" ON)
else()
	option(XASH_SDL "Enable SDL." ON)
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" OFF)
endif()

set(XASH_BUILD_COMMIT "${CMAKE_BUILD_POSTFIX}")
set(XASH_LIBDL true)

add_subdirectory(3rdparty EXCLUDE_FROM_ALL)

if(XASH_IMGUI)
	# TODO : since vcpkg doesn't provide with imgui_internal.h, just use our own one
    if(XASH_VCPKG AND FALSE)
		find_package(imgui CONFIG REQUIRED)
		add_library(imgui INTERFACE)
		target_link_libraries(imgui INTERFACE imgui::imgui)
	else()
		set(IMGUI_SRC
				imgui/imgui_demo.cpp
				imgui/imgui_draw.cpp
				imgui/imgui_widgets.cpp
				imgui/imgui.cpp)
		add_library(imgui STATIC ${IMGUI_SRC})
		target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/imgui)
	endif()
	add_library(imgui_ext STATIC 
		imgui_ext/ImGuiFileDialog/ImGuiFileDialog.cpp
		imgui_ext/imgui_lua/imgui_lua.cpp
		)
	target_include_directories(imgui_ext PUBLIC ${CMAKE_SOURCE_DIR}/imgui_ext)
	if(WIN32)
		target_include_directories(imgui_ext PRIVATE ${CMAKE_SOURCE_DIR}/imgui_ext/dirent)
	endif()
	target_link_libraries(imgui_ext PUBLIC imgui lua54_3p)
	if(XASH_UNITY_BUILD)
		set_target_properties(imgui PROPERTIES UNITY_BUILD ON)
		set_target_properties(imgui_ext PROPERTIES UNITY_BUILD ON)
	endif()
endif()

if(XASH_LUASH)
	add_subdirectory(luash)
endif()

ADD_SUBDIRECTORY(SourceSDK)
if(NOT XASH_DEDICATED)
	ADD_SUBDIRECTORY(cl_dll)
	if(XASH_VGUI2)
		ADD_SUBDIRECTORY(vgui2_support)
	else()
		ADD_SUBDIRECTORY(mainui_cpp)
	endif()
	#	ADD_SUBDIRECTORY(hymenu)
endif()
#ADD_SUBDIRECTORY(hydb)
ADD_SUBDIRECTORY(dlls)
ADD_SUBDIRECTORY(engine)
ADD_SUBDIRECTORY(cqmiao)
if(NOT XASH_SINGLE_BINARY AND NOT ANDROID AND NOT IOS)
	ADD_SUBDIRECTORY(game_launch)
endif()
