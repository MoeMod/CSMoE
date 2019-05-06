
cmake_minimum_required(VERSION  2.8.0)

# Install custom module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (APPLE)
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
	add_compile_options(-march=native -Wall -Wextra -Wno-unused-parameter -Wl,--no-undefined)
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -O3")
	set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os")
	set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -ggdb -O3")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-Wpedantic Wno-unused-private-field)
endif()

set(SDL2_PATH ${CMAKE_SOURCE_DIR}/SDL2 CACHE PATH "SDL2 path")

# force xp build
if(MSVC)
	add_definitions(-D_USING_V110_SDK71_)
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
	else()
		set (CMAKE_INSTALL_PREFIX "~/CSMoE/")
	endif()
endif()


set(XASH_BUILD_COMMIT "${CMAKE_BUILD_POSTFIX}")
set(SDL_SHARED true)
set(XASH_LIBDL true)


ADD_SUBDIRECTORY(cl_dll)
ADD_SUBDIRECTORY(dlls)
ADD_SUBDIRECTORY(engine)
ADD_SUBDIRECTORY(game_launch)
ADD_SUBDIRECTORY(mainui_cpp)

