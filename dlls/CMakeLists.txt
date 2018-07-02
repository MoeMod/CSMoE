#
# Copyright (c) 2016 Alibek Omarov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 2.6.0)

# Install custom module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# include(VSForceXPToolchain) # Force XP toolchain for Visual Studio

project (SERVER)

#--------------
# USER DEFINES \
################\
option(64BIT "Allow 64 Bit builds" OFF)
option(USE_VGUI "Enable VGUI1. UNDONE" OFF)
option(USE_VGUI2 "Enable VGUI2. UNDONE" OFF)
option(USE_VOICEMGR "Enable VOICE MANAGER." OFF)
option(GOLDSOURCE_SUPPORT "Build goldsource compatible client library" OFF)
option(64BIT "Disable auto -m32 appending to compiler flags" OFF)
set(GAMEDIR "valve" CACHE STRING "Gamedir path")
set(SERVER_INSTALL_DIR "dlls" CACHE STRING "Where put server dll")
set(CLIENT_INSTALL_DIR "cl_dlls" CACHE STRING "Where put client dll")
set(SERVER_LIBRARY_NAME "cs" CACHE STRING "Library name for Linux/MacOS/Windows")

#-----------------
# MAIN BUILD CODE \
###################\

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT 64BIT)
	if(MSVC)
		error("UNDONE: set 32 build flags")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
	endif()
	set(CMAKE_SIZEOF_VOID_P 4)
endif()

if(64BIT AND CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "You enabled XASH_64BIT, but compiler can't create 64 bit code!")
endif()

if(64BIT)
	message(STATUS "Building for 64 Bit")
else()
	message(STATUS "Building for 32 Bit")
endif()

if (MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--add-stdcall-alias")
endif()

# add_compile_options for older cmake versions
if(${CMAKE_VERSION} VERSION_LESS "3.0.2")
	macro(add_compile_options)
		set(list_var "${ARGV}")
		foreach(arg IN LISTS list_var)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${arg}")
		endforeach()
	endmacro()
endif()

set(HLDLL_SRCS
	./airtank.cpp
	./ammo.cpp
	./animating.cpp
	./animation.cpp
	./bmodels.cpp
	./buttons.cpp
	./career_tasks.cpp
	./cbase.cpp
	./client.cpp
	./combat.cpp
	./debug.cpp
	./doors.cpp
	./effects.cpp
	./explode.cpp
	./func_break.cpp
	./func_tank.cpp
	./game.cpp
	./gamerules.cpp
	./ggrenade.cpp
	./globals.cpp
	./h_ai.cpp
	./h_battery.cpp
	./h_cycler.cpp
	./healthkit.cpp
	./h_export.cpp
	./hintmessage.cpp
	./items.cpp
	./lights.cpp
	./maprules.cpp
	./mortar.cpp
	./mpstubb.cpp
	./multiplay_gamerules.cpp
	./observer.cpp
	./pathcorner.cpp
	./plane.cpp
	./plats.cpp
	./player.cpp
	./singleplay_gamerules.cpp
	./skill.cpp
	./sound.cpp
	./soundent.cpp
	./spectator.cpp
	./subs.cpp
	./training_gamerules.cpp
	./triggers.cpp
	./tutor_base_states.cpp
	./tutor_base_tutor.cpp
	./tutor.cpp
	./tutor_cs_states.cpp
	./tutor_cs_tutor.cpp
	./util.cpp
	./vehicle.cpp
	./weapons.cpp
	./weapontype.cpp
	./world.cpp
	./bot/cs_bot_chatter.cpp
	./bot/cs_bot.cpp
	./bot/cs_bot_event.cpp
	./bot/cs_bot_init.cpp
	./bot/cs_bot_learn.cpp
	./bot/cs_bot_listen.cpp
	./bot/cs_bot_manager.cpp
	./bot/cs_bot_nav.cpp
	./bot/cs_bot_pathfind.cpp
	./bot/cs_bot_radio.cpp
	./bot/cs_bot_statemachine.cpp
	./bot/cs_bot_update.cpp
	./bot/cs_bot_vision.cpp
	./bot/cs_bot_weapon.cpp
	./bot/cs_gamestate.cpp
	./bot/states/cs_bot_attack.cpp
	./bot/states/cs_bot_buy.cpp
	./bot/states/cs_bot_defuse_bomb.cpp
	./bot/states/cs_bot_escape_from_bomb.cpp
	./bot/states/cs_bot_fetch_bomb.cpp
	./bot/states/cs_bot_follow.cpp
	./bot/states/cs_bot_hide.cpp
	./bot/states/cs_bot_hunt.cpp
	./bot/states/cs_bot_idle.cpp
	./bot/states/cs_bot_investigate_noise.cpp
	./bot/states/cs_bot_move_to.cpp
	./bot/states/cs_bot_plant_bomb.cpp
	./bot/states/cs_bot_use_entity.cpp
	./hostage/hostage.cpp
	./hostage/hostage_improv.cpp
	./hostage/hostage_localnav.cpp
	./hostage/states/hostage_animate.cpp
	./hostage/states/hostage_escape.cpp
	./hostage/states/hostage_follow.cpp
	./hostage/states/hostage_idle.cpp
	./hostage/states/hostage_retreat.cpp
	./gamemode/mods.cpp
	./gamemode/mod_base.cpp
	./gamemode/mod_none.cpp
	./gamemode/mod_tdm.cpp
	./../public/unicode_strtools.cpp
	)

set(HLWPN_SRCS
	./wpn_shared/wpn_ak47.cpp
	./wpn_shared/wpn_aug.cpp
	./wpn_shared/wpn_awp.cpp
	./wpn_shared/wpn_c4.cpp
	./wpn_shared/wpn_deagle.cpp
	./wpn_shared/wpn_elite.cpp
	./wpn_shared/wpn_famas.cpp
	./wpn_shared/wpn_fiveseven.cpp
	./wpn_shared/wpn_flashbang.cpp
	./wpn_shared/wpn_g3sg1.cpp
	./wpn_shared/wpn_galil.cpp
	./wpn_shared/wpn_glock18.cpp
	./wpn_shared/wpn_hegrenade.cpp
	./wpn_shared/wpn_knife.cpp
	./wpn_shared/wpn_m249.cpp
	./wpn_shared/wpn_m3.cpp
	./wpn_shared/wpn_m4a1.cpp
	./wpn_shared/wpn_mac10.cpp
	./wpn_shared/wpn_mp5navy.cpp
	./wpn_shared/wpn_p228.cpp
	./wpn_shared/wpn_p90.cpp
	./wpn_shared/wpn_scout.cpp
	./wpn_shared/wpn_sg550.cpp
	./wpn_shared/wpn_sg552.cpp
	./wpn_shared/wpn_smokegrenade.cpp
	./wpn_shared/wpn_tmp.cpp
	./wpn_shared/wpn_ump45.cpp
	./wpn_shared/wpn_usp.cpp
	./wpn_shared/wpn_xm1014.cpp
	)

set(PM_SRCS
    ../pm_shared/pm_shared.c
	../pm_shared/pm_math.c
	../pm_shared/pm_debug.c
    )

set(GAME_SHARED_SRCS
    ../game_shared/voice_gamemgr.cpp
	../game_shared/shared_util.cpp
	../game_shared/bot/bot.cpp
	../game_shared/bot/bot_manager.cpp
	../game_shared/bot/bot_profile.cpp
	../game_shared/bot/bot_util.cpp
	../game_shared/bot/nav_area.cpp
	../game_shared/bot/nav_file.cpp
	../game_shared/bot/nav_node.cpp
	../game_shared/bot/nav_path.cpp
    )

set(PUBLIC_SRCS
	../public/MemPool.cpp
    )

set (SERVER_LIBRARY server)
#file (GLOB SERVER_SOURCES *.cpp ../pm_shared/*.c)

set (SERVER_SOURCES
	${HLDLL_SRCS}
	${HLWPN_SRCS}
	${PM_SRCS}
	${GAME_SHARED_SRCS}
	${PUBLIC_SRCS}
)

include_directories (
	../
	../common/
	../engine/
	../pm_shared/
	../public/
	../public/tier1/
	../dlls/
	../game_shared/)
add_library (${SERVER_LIBRARY} SHARED ${SERVER_SOURCES})

add_definitions( 
	-DLINUX -D_LINUX -DCLIENT_WEAPONS
	-fpermissive -fno-strict-aliasing -DNDEBUG  -DGNUC
	-D_stricmp=strcasecmp -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp  -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_strdup=strdup -D_write=write -D_close=close
	-w -Wl,--no-undefined
	-DQUIVER -DQUAKE2 -DVALVE_DLL -D_alloca=alloca -fno-exceptions -fexpensive-optimizations -D_vsnprintf=vsnprintf -DNO_MALLOC_OVERRIDE -Werror=return-type)

target_link_libraries( ${SERVER_LIBRARY} ${CMAKE_DL_LIBS} )
