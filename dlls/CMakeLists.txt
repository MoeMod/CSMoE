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

if(XASH_PCH)
	project (server CXX CXXPCH)
else()
	project (server)
endif()
set(CMAKE_CXX_STANDARD 14)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
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
if(ANDROID OR IOS)
	set(SERVER_LIBRARY_NAME "server" CACHE STRING "Library name for Android/iOS")
elseif(NOT WIN32)
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
	./weapons_precache.cpp
	./gamemode/zbb/zbb_basebuilder.cpp
	./gamemode/zbb/zbb_ghost.cpp
	./gamemode/zb1/zb1_countdown.cpp
	./cbase/cbase_entity_factory.cpp
	./cbase/cbase_physint.cpp
	./gamemode/zbb/zbb_zombie.cpp
	)

set (HLWPN_SRCS
	../dlls/wpn_shared/wpn_ak47.cpp
	../dlls/wpn_shared/wpn_aug.cpp
	../dlls/wpn_shared/wpn_awp.cpp
	../dlls/wpn_shared/wpn_c4.cpp
	../dlls/wpn_shared/wpn_deagle.cpp
	../dlls/wpn_shared/wpn_elite.cpp
	../dlls/wpn_shared/wpn_famas.cpp
	../dlls/wpn_shared/wpn_fiveseven.cpp
	../dlls/wpn_shared/wpn_flashbang.cpp
	../dlls/wpn_shared/wpn_g3sg1.cpp
	../dlls/wpn_shared/wpn_galil.cpp
	../dlls/wpn_shared/wpn_glock18.cpp
	../dlls/wpn_shared/wpn_hegrenade.cpp
	../dlls/wpn_shared/wpn_knife.cpp
	../dlls/wpn_shared/wpn_m249.cpp
	../dlls/wpn_shared/wpn_m3.cpp
	../dlls/wpn_shared/wpn_m4a1.cpp
	../dlls/wpn_shared/wpn_mac10.cpp
	../dlls/wpn_shared/wpn_mp5navy.cpp
	../dlls/wpn_shared/wpn_p228.cpp
	../dlls/wpn_shared/wpn_p90.cpp
	../dlls/wpn_shared/wpn_scout.cpp
	../dlls/wpn_shared/wpn_sg550.cpp
	../dlls/wpn_shared/wpn_sg552.cpp
	../dlls/wpn_shared/wpn_smokegrenade.cpp
	../dlls/wpn_shared/wpn_tmp.cpp
	../dlls/wpn_shared/wpn_ump45.cpp
	../dlls/wpn_shared/wpn_usp.cpp
	../dlls/wpn_shared/wpn_xm1014.cpp
	../dlls/wpn_shared/knife_zombi.cpp
	../dlls/wpn_shared/wpn_ak47l.cpp
	../dlls/wpn_shared/wpn_mp7a1d.cpp
	../dlls/wpn_shared/wpn_deagled.cpp
	../dlls/wpn_shared/wpn_as50.cpp
	../dlls/wpn_shared/wpn_cannon.cpp
	../dlls/wpn_shared/wpn_sgdrill.cpp
	../dlls/wpn_shared/z4b_cannontiger.cpp
	../dlls/wpn_shared/z4b_cannonplasma.cpp
	../dlls/wpn_shared/wpn_infinity.cpp
	../dlls/wpn_shared/wpn_k1a.cpp
	../dlls/wpn_shared/wpn_kriss.cpp
	../dlls/wpn_shared/wpn_m95.cpp
	../dlls/wpn_shared/wpn_m1887.cpp
	../dlls/wpn_shared/wpn_scarh.cpp
	../dlls/wpn_shared/wpn_scarl.cpp
	../dlls/wpn_shared/wpn_tar21.cpp
	../dlls/wpn_shared/wpn_thompson.cpp
	../dlls/wpn_shared/wpn_wa2000.cpp
	../dlls/wpn_shared/wpn_xm8c.cpp
	../dlls/wpn_shared/wpn_xm8s.cpp
	../dlls/wpn_shared/knife_skullaxe.cpp
	../dlls/wpn_shared/knife_katana.cpp
	../dlls/wpn_shared/knife_dragonsword.cpp
	../dlls/wpn_shared/wpn_gungnir.cpp
	../dlls/wpn_shared/wpn_mg3.cpp
	../dlls/wpn_shared/wpn_desperado.cpp
	../dlls/wpn_shared/wpn_svdex.cpp
	../dlls/wpn_shared/wpn_mp7a1c.cpp
	../dlls/wpn_shared/wpn_mp7a1p.cpp
	../dlls/wpn_shared/wpn_infinitys.cpp
	../dlls/wpn_shared/wpn_m14ebr.cpp
	../dlls/wpn_shared/wpn_infinityex1.cpp
	../dlls/wpn_shared/wpn_infinityex2.cpp
	../dlls/wpn_shared/knife_dualsword.cpp
	../dlls/wpn_shared/wpn_anaconda.cpp
	../dlls/wpn_shared/wpn_tbarrel.cpp
	../dlls/wpn_shared/wpn_mg36.cpp
	../dlls/wpn_shared/wpn_balrog7.cpp
	../dlls/wpn_shared/wpn_gatling.cpp
	../dlls/wpn_shared/wpn_bow.cpp
	../dlls/wpn_shared/wpn_m2.cpp
	../dlls/wpn_shared/wpn_janus7xmas.cpp
	../dlls/wpn_shared/wpn_chainsaw.cpp
	../dlls/wpn_shared/wpn_m134.cpp
	../dlls/wpn_shared/wpn_m134xmas.cpp
	../dlls/wpn_shared/wpn_mg3xmas.cpp
	../dlls/wpn_shared/wpn_m95xmas.cpp
	../dlls/wpn_shared/wpn_m134ex.cpp
	../dlls/wpn_shared/wpn_starchaserar.cpp
	../dlls/wpn_shared/z4b_Frequency9.cpp
	../dlls/wpn_shared/z4b_stormgiantx.cpp
	../dlls/wpn_shared/z4b_lc_sword.cpp
	../dlls/wpn_shared/z4b_oppovivo.cpp
	../dlls/wpn_shared/z4b_strongknifex.cpp
	../dlls/wpn_shared/z4b_infinityx.cpp
	../dlls/wpn_shared/z4b_nataknifedx.cpp
	../dlls/wpn_shared/z4b_facelessvoid.cpp
	../dlls/wpn_shared/z4b_m134heroi.cpp
	../dlls/wpn_shared/z4b_deagleb.cpp
	../dlls/wpn_shared/z4b_xm2010pc.cpp
	../dlls/wpn_shared/z4b_dmp7a1x.cpp
	../dlls/wpn_shared/z4b_m4a1razer.cpp
	../dlls/wpn_shared/z4b_awpnvidia.cpp
	../dlls/wpn_shared/z4b_m4a1x.cpp
	../dlls/wpn_shared/z4b_m4a1mw.cpp
	../dlls/wpn_shared/z4b_ak47x.cpp
	)

set(PM_SRCS
	../pm_shared/pm_shared.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
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
	${HLWPN_SRCS}
	${HLDLL_SRCS}
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

if(XASH_STATIC_GAMELIB)
	add_library (${SERVER_LIBRARY} STATIC ${SERVER_SOURCES})
else()
	add_library (${SERVER_LIBRARY} SHARED ${SERVER_SOURCES})
endif()

if(APPLE OR ANDROID)
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
	-DQUIVER -DQUAKE2 -DVALVE_DLL -DSERVER_DLL
	)

target_link_libraries( ${SERVER_LIBRARY} ${CMAKE_DL_LIBS} )

if(ANDROID OR IOS)
	set_target_properties(${SERVER_LIBRARY} PROPERTIES
			OUTPUT_NAME ${SERVER_LIBRARY_NAME})
else()
	set_target_properties(${SERVER_LIBRARY} PROPERTIES
			PREFIX "" OUTPUT_NAME ${SERVER_LIBRARY_NAME})
endif()

if(NOT XASH_STATIC_GAMELIB)
	install( TARGETS ${SERVER_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GAMEDIR}/${SERVER_INSTALL_DIR})
endif()

if(XASH_PCH)
	target_precompiled_header(${SERVER_LIBRARY} stdafx.h)
	set_target_properties(${SERVER_LIBRARY} PROPERTIES
			CXX_STANDARD ${CMAKE_CXX_STANDARD}
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO
			)
endif()
