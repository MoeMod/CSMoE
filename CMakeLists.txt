project(csmoe)
cmake_minimum_required(VERSION  3.10.2)

# Install custom module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

option(XASH_DEDICATED "Enable dedicated build. Servers must choose this." OFF) # Servers must choose this

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

set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
	option(XASH_PCH "Enable PCH support." ON)
	option(XASH_UNITY_BUILD "Enable Unity Build support." ON)
endif()

option(XASH_VCPKG "Enable VCPKG support." OFF)

if(XASH_DEDICATED)
	set(XASH_SDL  OFF) # Dedicated servers still can use SDL
	set(XASH_STATIC_GAMELIB OFF)
else()
	option(XASH_SDL "Enable SDL." ON)
	option(XASH_STATIC_GAMELIB "Build static game library" ON)
endif()

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
else()
	set(XASH_IMGUI OFF)
endif()

option(XASH_LUASH "LUA support for CSMoE. Expected lua 5.3" ON)

if (APPLE OR ANDROID)
	add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-invalid-offsetof)
elseif(MSVC)

else()
	add_compile_options( -Wall -Wextra -Wno-unused-parameter -Wl,--no-undefined )
endif()

# Important, as HLSDK and engine shares some symbol names!
if(NOT WIN32) # Windows by default have hidden visibility and better linker
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
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

set(CMAKE_CXX_STANDARD 17)

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
		)
	target_include_directories(imgui_ext PUBLIC ${CMAKE_SOURCE_DIR}/imgui_ext)
	if(WIN32)
		target_include_directories(imgui_ext PRIVATE ${CMAKE_SOURCE_DIR}/imgui_ext/dirent)
	endif()
	target_link_libraries(imgui_ext PUBLIC imgui)
	if(XASH_UNITY_BUILD)
		set_target_properties(imgui PROPERTIES UNITY_BUILD ON)
		set_target_properties(imgui_ext PROPERTIES UNITY_BUILD ON)
	endif()
endif()

if(XASH_LUASH)
	add_subdirectory(luash)
endif()

if(NOT XASH_DEDICATED)
	ADD_SUBDIRECTORY(cl_dll)
	ADD_SUBDIRECTORY(mainui_cpp)
#	ADD_SUBDIRECTORY(hymenu)
endif()
ADD_SUBDIRECTORY(dlls)
ADD_SUBDIRECTORY(engine)
if(NOT XASH_SINGLE_BINARY AND NOT ANDROID AND NOT IOS)
	ADD_SUBDIRECTORY(game_launch)
endif()
