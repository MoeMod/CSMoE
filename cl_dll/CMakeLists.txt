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

project (client)

set(CMAKE_CXX_STANDARD 11)

if (APPLE)
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

set (CLDLL_LIBRARY client)

#--------------
# USER DEFINES \
################\
set(GAMEDIR "csmoe" CACHE STRING "Gamedir path")
set(CLIENT_INSTALL_DIR "cl_dlls" CACHE STRING "Where put client dll")
if(XASH_64BIT)
	set(CLIENT_LIBRARY_NAME "client64" CACHE STRING "Library name for 64bit")
else()
	set(CLIENT_LIBRARY_NAME "client" CACHE STRING "Library name for 32bit")
endif()

#-----------------
# MAIN BUILD CODE \
###################\

file (GLOB WEAPONEVENT_SRCS
	./ev_common.cpp
	./events/ev_cs16.cpp
	./events/hl_events.cpp
	./events/event_*.cpp
)

file (GLOB CLWEAPON_SRCS
	./com_weapons.cpp
	./cs_wpn/cs_baseentity.cpp
	./cs_wpn/cs_weapons.cpp
	./cs_wpn/bte_weapons.cpp
	../dlls/wpn_shared/wpn_*.cpp
	../dlls/wpn_shared/knife_*.cpp
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
	./hud/menu.cpp
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

)

set (STUDIORENDER_SRCS
	./studio/GameStudioModelRenderer.cpp
	./studio/StudioModelRenderer.cpp
	./studio/studio_util.cpp

)

set (PLAYERMOVE_SRCS
	../pm_shared/pm_debug.c
	../pm_shared/pm_math.c
	../pm_shared/pm_shared.c
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

	${PLAYERMOVE_SRCS}
	${HUD_SRCS}
	${WEAPONEVENT_SRCS}
	${CLWEAPON_SRCS}
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
add_library (${CLDLL_LIBRARY} SHARED ${CLDLL_SOURCES})

if(APPLE)
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

target_link_libraries( ${CLDLL_LIBRARY} ${CMAKE_DL_LIBS} )

set_target_properties(${CLDLL_LIBRARY} PROPERTIES
		PREFIX "" OUTPUT_NAME ${CLIENT_LIBRARY_NAME})

install( TARGETS ${CLDLL_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GAMEDIR}/${CLIENT_INSTALL_DIR} )