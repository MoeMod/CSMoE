#
# Copyright (c) 2015 Pavlo Lavrenenko
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
	project (client CXX CXXPCH)
else()
	project (client)
endif()

set(CMAKE_CXX_STANDARD 14)

if (APPLE OR ANDROID)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -stdlib=libc++")
elseif(MSVC)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
else()
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -static-libstdc++")
endif()
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ")

#find_package(SDL2 REQUIRED)

set (CLIENT_LIBRARY client)

#--------------
# USER DEFINES \
################\
set(GAMEDIR "csmoe" CACHE STRING "Gamedir path")
set(CLIENT_INSTALL_DIR "cl_dlls" CACHE STRING "Where put client dll")
set(CLIENT_LIBRARY_NAME "client" CACHE STRING "Library name for client")

#-----------------
# MAIN BUILD CODE \
###################\

file (GLOB WEAPONEVENT_SRCS
	./ev_common.cpp
	./events/ev_cs16.cpp
	./events/hl_events.cpp
	./events/event_*.cpp
)

set (CLWEAPON_SRCS
	./com_weapons.cpp
	./cs_wpn/cs_baseentity.cpp
	./cs_wpn/cs_weapons.cpp
	./cs_wpn/bte_weapons.cpp
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
	../dlls/wpn_shared/knife_dragonsword.cpp
	../dlls/wpn_shared/wpn_gungnir.cpp
	../dlls/wpn_shared/wpn_mg3.cpp
	../dlls/wpn_shared/wpn_mp7a1c.cpp
	../dlls/wpn_shared/wpn_mp7a1p.cpp
	../dlls/wpn_shared/wpn_infinitys.cpp
	../dlls/wpn_shared/wpn_m14ebr.cpp
	../dlls/wpn_shared/wpn_balrog7.cpp
	../dlls/wpn_shared/wpn_gatling.cpp
	../dlls/wpn_shared/wpn_m2.cpp
	../dlls/wpn_shared/wpn_m134.cpp
	../dlls/wpn_shared/wpn_m134ex.cpp
	../dlls/wpn_shared/wpn_m134xmas.cpp
	../dlls/wpn_shared/wpn_mg3xmas.cpp
	../dlls/wpn_shared/wpn_m95xmas.cpp
	../dlls/wpn_shared/wpn_janus7xmas.cpp
	../dlls/wpn_shared/wpn_starchaserar.cpp
	../dlls/wpn_shared/wpn_svdex.cpp
	../dlls/wpn_shared/wpn_infinityex1.cpp
	../dlls/wpn_shared/wpn_infinityex2.cpp
	../dlls/wpn_shared/z4b_infinityx.cpp
	../dlls/wpn_shared/knife_dualsword.cpp
	../dlls/wpn_shared/wpn_anaconda.cpp
	../dlls/wpn_shared/wpn_mg36.cpp
	../dlls/wpn_shared/z4b_Frequency9.cpp
	../dlls/wpn_shared/z4b_stormgiantx.cpp
	../dlls/wpn_shared/z4b_lc_sword.cpp
	../dlls/wpn_shared/z4b_oppovivo.cpp
	../dlls/wpn_shared/z4b_strongknifex.cpp
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

set (HUD_SRCS
	./hud/ammo.cpp
	./hud/ammo_secondary.cpp
	./hud/ammohistory.cpp
	./hud/battery.cpp
	./hud/geiger.cpp
	./hud/health.cpp
	./hud/hud.cpp
	./hud/hud_msg.cpp
	./hud/hud_redraw.cpp
	./hud/hud_spectator.cpp
	./hud/hud_update.cpp
	./hud/sniperscope.cpp
	./hud/nvg.cpp
	./hud/hud_menu.cpp
	./hud/message.cpp
	./hud/status_icons.cpp
	./hud/statusbar.cpp
	./hud/text_message.cpp
	./hud/train.cpp
	./hud/scoreboard.cpp
	./hud/MOTD.cpp
	./hud/radio.cpp
	./hud/radar.cpp
	./hud/timer.cpp
	./hud/money.cpp
	./hud/flashlight.cpp
	./hud/death.cpp
	./hud/saytext.cpp
	./hud/spectator_gui.cpp
	./hud/followicon.cpp
	./hud/headname.cpp
	./hud/zbs/zbs.cpp
	./hud/zbs/zbs_level.cpp
	./hud/zbs/zbs_scoreboard.cpp
	./hud/zbs/zbs_kill.cpp
	./hud/zbs/zbs_roundclear.cpp
	./hud/retina.cpp
	./hud/scenariostatus.cpp
	./hud/zb2/zb2.cpp
	./hud/zb2/zb2_skill.cpp
	./hud/legacy/hud_radar_legacy.cpp
	./hud/legacy/hud_scoreboard_legacy.cpp
	./hud/modern/hud_radar_modern.cpp
	./hud/zb3/zb3.cpp
	./hud/zb3/zb3_morale.cpp
	./hud/zb3/zb3_rage.cpp
	./hud/moe/moe_touch.cpp

)

set (STUDIORENDER_SRCS
	./studio/GameStudioModelRenderer.cpp
	./studio/StudioModelRenderer.cpp
	./studio/studio_util.cpp

)

set(PM_SRCS
	../pm_shared/pm_shared.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
)

set (CLDLL_SOURCES
	./cdll_int.cpp
    ./demo.cpp
    ./entity.cpp
    ./in_camera.cpp
    ./input.cpp
	./rain.cpp
    ./tri.cpp
	./util.cpp
	./view.cpp
	./input_xash3d.cpp
	#./input_sdl.cpp
    ./vgui_parser.cpp
    ../public/unicode_strtools.cpp
	./draw_util.cpp
	./calcscreen.cpp
	./input_touch.cpp

	${PM_SRCS}
	${HUD_SRCS}
	${WEAPONEVENT_SRCS}
	${CLWEAPON_SRCS}
	${HLWPN_SRCS}
	${STUDIORENDER_SRCS}
)

include_directories (
	hud/
	studio/
	../cl_dll/
	../common/
	../engine/
	../pm_shared/
	../dlls/
	../game_shared/
	../public/
	../public/tier1/
	)

if(XASH_STATIC_GAMELIB)
	add_library (${CLIENT_LIBRARY} STATIC ${CLDLL_SOURCES})
else()
	add_library (${CLIENT_LIBRARY} SHARED ${CLDLL_SOURCES})
endif()

if(APPLE OR ANDROID)
	add_definitions(
			-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
	)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(
		-DLINUX -D_LINUX
		-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
		)
endif()

if(MINGW)
add_definitions(
	-D_EMULATE_GLIBC
	-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
		)
endif()

if(MSVC)
add_definitions(
	-DWIN32 -DDEDICATED -D_USING_V110_SDK71_ -D_WINDLL -D_MBCS
	-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE
)
endif()

add_definitions( -D_CS16CLIENT_ENABLE_GSRC_SUPPORT
	-DCLIENT_WEAPONS -DCLIENT_DLL
	-D_CS16CLIENT_ALLOW_SPECIAL_SCRIPTING
	 )

target_link_libraries( ${CLIENT_LIBRARY} ${CMAKE_DL_LIBS} )

if(ANDROID OR IOS)
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			OUTPUT_NAME ${CLIENT_LIBRARY_NAME})
else()
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			PREFIX "" OUTPUT_NAME ${CLIENT_LIBRARY_NAME})
endif()

if(XASH_64BIT AND NOT ANDROID AND NOT IOS)
	get_target_property(CLIENT_LIBRARY_NAME ${CLIENT_LIBRARY} OUTPUT_NAME)
	if(NOT CLIENT_LIBRARY_NAME)
		set(CLIENT_LIBRARY_NAME ${CLIENT_LIBRARY})
	endif()
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			OUTPUT_NAME ${CLIENT_LIBRARY_NAME}64)
endif()

if(NOT XASH_STATIC_GAMELIB)
	install( TARGETS ${CLIENT_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GAMEDIR}/${CLIENT_INSTALL_DIR} )
endif()

if(XASH_PCH)
	target_precompiled_header(${CLIENT_LIBRARY} stdafx.h)
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			CXX_STANDARD ${CMAKE_CXX_STANDARD}
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO
			)
endif()
