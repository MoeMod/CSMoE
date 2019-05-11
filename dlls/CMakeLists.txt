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

cmake_minimum_required(VERSION 3.1.3)

project (SERVER)
set(CMAKE_CXX_STANDARD 11)

if (APPLE)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -stdlib=libc++")
elseif(MSVC)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
else()
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -static-libstdc++")
endif()

set (SERVER_LIBRARY server)

#--------------
# USER DEFINES \
################\
set(GAMEDIR "csmoe" CACHE STRING "Gamedir path")
set(SERVER_INSTALL_DIR "dlls" CACHE STRING "Where put server dll")
if(NOT WIN32)
set(SERVER_LIBRARY_NAME "cs" CACHE STRING "Library name for Linux/MacOS")
else()
set(SERVER_LIBRARY_NAME "mp" CACHE STRING "Library name for Windows")
endif()

#-----------------
# MAIN BUILD CODE \
###################\

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
	./player/csdm_randomspawn.cpp
	./player/hintmessage.cpp
	./player/player_msg.cpp
	./player/player_spawnpoint.cpp
	./player/player_model.cpp
	./player/player_zombie.cpp
	./player/player_spawnprotection.cpp
	./player/player_knockback.cpp
	./player/player_human_level.cpp
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
	./weapons_buy.cpp
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
	./gamemode/mod_dm.cpp
	./gamemode/mod_zb1.cpp
	./gamemode/mod_zbs.cpp
	./gamemode/zbs/zs_subs.cpp
	./gamemode/zbs/monster_entity.cpp
	./../public/unicode_strtools.cpp
	./gamemode/mod_zb2.cpp
	./gamemode/zb2/supplybox.cpp
	./bot/cs_bot_bhopjump.cpp
	./weapons_data.cpp
	./weapons_msg.cpp
	./player/player_mod_strategy.cpp
	./gamemode/zbs/monster_manager.cpp
	./player/player_account.cpp
	./cbase/cbase_memory.cpp
	./gamemode/mod_zb3.cpp
	./gamemode/mod_gd.cpp
	./gamemode/zb2/zb2_skill_human.cpp
	./gamemode/zb2/zb2_skill_zombie.cpp
	./gamemode/zb2/zb2_zclass.cpp
	./gamemode/zb2/zb2_zclass_tank.cpp
	./gamemode/zb2/zb2_zclass_speed.cpp
	./gamemode/mod_zbb.cpp
	./weapons_moe_buy.cpp
	./weapons_precache.cpp
	./gamemode/zbb/zbb_basebuilder.cpp
		./gamemode/zbb/zbb_ghost.cpp
		./gamemode/zb1/zb1_countdown.cpp
	)

file (GLOB HLWPN_SRCS
	./wpn_shared/wpn_*.cpp
	./wpn_shared/knife_*.cpp
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

if(APPLE)
	add_definitions(
		-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf
		-D_alloca=alloca -D_strdup=strdup -D_write=write -DNO_MALLOC_OVERRIDE
		-D_close=close -D_write=write
	)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(
		-fpermissive -fno-strict-aliasing -fno-exceptions -fexpensive-optimizations
		-DLINUX -D_LINUX -DGNUC
		-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf
		-D_alloca=alloca -D_strdup=strdup -D_write=write -DNO_MALLOC_OVERRIDE
		-D_close=close -D_write=write
	)
endif()

if(MINGW)
add_definitions(
	-fpermissive -fno-strict-aliasing -fno-exceptions -fexpensive-optimizations
	-D_EMULATE_GLIBC -DGNUC
	-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
	-D_alloca=alloca -D_vsnprintf=vsnprintf -DNO_MALLOC_OVERRIDE
		)
endif()

if(MSVC)
add_definitions(
	-DWIN32 -DDEDICATED -D_USING_V110_SDK71_ -D_WINDLL -D_MBCS
	-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE
)
endif()

add_definitions(
	-DCLIENT_WEAPONS
	-DQUIVER -DQUAKE2 -DVALVE_DLL
	)

target_link_libraries( ${SERVER_LIBRARY} ${CMAKE_DL_LIBS} )

set_target_properties(${SERVER_LIBRARY} PROPERTIES
		PREFIX "" OUTPUT_NAME ${SERVER_LIBRARY_NAME})

install( TARGETS ${SERVER_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GAMEDIR}/${SERVER_INSTALL_DIR})