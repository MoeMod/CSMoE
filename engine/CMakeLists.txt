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
cmake_minimum_required(VERSION  3.6.0)
project(XASH_ENGINE C CXX)

set(CMAKE_CXX_STANDARD 17)

# fix C++17 register
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	add_compile_options(-Wno-register)
endif()

set(XASH_ENGINE xash) # Lib name

option(XASH_DEDICATED "Enable dedicated build. Servers must choose this." OFF) # Servers must choose this
if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	option(XASH_64BIT "Enable experimental 64-bit build. Note that you must also provide 64-bit game binaries" ON)
else()
	option(XASH_64BIT "Enable experimental 64-bit build. Note that you must also provide 64-bit game binaries" OFF)
endif()
option(XASH_GLES   "Enable if Xash3D is running over GL to GLES translator." OFF)
option(XASH_NANOGL "Use NanoGL(GLES1). Implicitly enables XASH_GLES." OFF)
option(XASH_WES    "Use gl-wes-v2(GLES2). Implicitly enables XASH_GLES." OFF)
if(WIN32)
	option(XASH_QINDIEGL    "Use QindieGL(D3D9). Only avaible on Win32" OFF)
endif()
if(XASH_QINDIEGL)
	set(XASH_QINDIEGL_SRC "${CMAKE_SOURCE_DIR}/3rdparty/QindieGL" CACHE PATH "QindieGL path")
endif()
option(XASH_NO_ASYNC_NS_RESOLVE "Disable asynchronous domain name resolving." OFF)

option(XASH_VECTORIZE_SINCOS "Try to use vectorized versions of sin(), cos() and sincos()" ON)

# Desktop or dedicated version options
if(XASH_DEDICATED)
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" ON)
else()
	option(XASH_SINGLE_BINARY "Don't build game launcher and build engine as executable" OFF)
endif()

set(XASH_ENGINE_SOURCES 
	common/avikit.c
	common/build.c
	common/base_cmd.c
	common/cfgscript.c
	common/cmd.c
	common/common.c
	common/con_utils.c
	common/crclib.c
	common/crtlib.c
	common/cvar.c
	common/filesystem.c
	common/host.c
	common/hpak.c
	common/infostring.c
	common/identification.c
#	common/library.c
	common/masterlist.c
	common/mathlib.c
	common/matrixlib.c
	common/mod_studio.c
	common/model.c
	common/net_buffer.c
	common/net_chan.c
	common/net_encode.c
	common/net_huff.c
	common/network.c
	common/pm_surface.c
	common/pm_trace.c
	common/random.c
	common/sys_con.c
	common/system.c
	common/titles.c
	common/world.c
	common/zone.c
	common/crashhandler.c
	common/imagelib/img_bmp.c
	common/imagelib/img_main.c
	common/imagelib/img_quant.c
	common/imagelib/img_tga.c
	common/imagelib/img_utils.c
	common/imagelib/img_wad.c
	common/imagelib/img_dds.c
	common/soundlib/snd_main.c
	common/soundlib/snd_mp3.c
	common/soundlib/snd_utils.c
	common/soundlib/snd_wav.c
	common/soundlib/libmpg/dct36.c
	common/soundlib/libmpg/dct64.c
	common/soundlib/libmpg/format.c
	common/soundlib/libmpg/frame.c
	common/soundlib/libmpg/index.c
	common/soundlib/libmpg/layer3.c
	common/soundlib/libmpg/libmpg.c
	common/soundlib/libmpg/mpg123.c
	common/soundlib/libmpg/parse.c
	common/soundlib/libmpg/reader.c
	common/soundlib/libmpg/synth.c
	common/soundlib/libmpg/tabinit.c
	common/Sequence.c
	common/IceKey.cpp
	common/mod_decryptor.cpp 


	server/sv_client.c
	server/sv_cmds.c
	server/sv_custom.c
	server/sv_frame.c
	server/sv_filter.c
	server/sv_game.c
	server/sv_init.c
	server/sv_main.c
	server/sv_log.c
	server/sv_move.c
	server/sv_phys.c
	server/sv_pmove.c
	server/sv_save.c
	server/sv_world.c

)

if(XASH_STATIC_GAMELIB)
	set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} common/library_static.c)
else()
	set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} common/library.c)
endif()

if(NOT XASH_DEDICATED)
	set(XASH_ENGINE_WIN32_SOURCES
		./platform/win32/con_win.c
		./platform/win32/ime_win32.cpp
	)
endif()

set(XASH_ENGINE_SDL_SOURCES
	./platform/sdl/events.c
	./platform/sdl/gl_sdl.c
	./platform/sdl/snd_sdl.c
	./platform/sdl/vid_sdl.c
)

set(XASH_ENGINE_ANDROID_SOURCES
   platform/android/vid_android.c
   platform/android/android_nosdl.c
   platform/android/snd_opensles.c
   platform/ime_stub.cpp
)

if(IOS)
set(XASH_ENGINE_APPLE_SOURCES
	common/launcher.c
	platform/iOS/TapicEngine.mm
	platform/ime_stub.cpp
)
else()
set(XASH_ENGINE_APPLE_SOURCES
	platform/macos/TouchBar.mm
	platform/macos/vid_macos.mm
	platform/ime_stub.cpp
)
endif()

set(XASH_ENGINE_CLIENT_SOURCES
   client/cl_cmds.c
   client/cl_demo.c
   client/cl_events.c
   client/cl_frame.c
   client/cl_game.c
   client/cl_main.c
   client/cl_menu.c
   client/cl_mobile.c
   client/cl_parse.c
   client/cl_pmove.c
   client/cl_remap.c
   client/cl_scrn.c
   client/cl_tent.c
   client/cl_video.c
   client/cl_view.c
   client/cl_netgraph.c
   client/gl_backend.c
   client/gl_beams.c
   client/gl_cull.c
   client/gl_decals.c
   client/gl_draw.c
   client/gl_image.c
   client/gl_mirror.c
   client/gl_refrag.c
   client/gl_rlight.c
   client/gl_rmain.c
   client/gl_rmath.c
   client/gl_rmisc.c
   client/gl_rpart.c
   client/gl_rsurf.c
   client/gl_rstrobe.c
   client/gl_sprite.c
   client/gl_studio.c
   client/vid_common.c
   client/gl_warp.c
   client/joyinput.c
   client/input.c
   client/keys.c
   client/input_evdevkey.c
   client/console.c
   client/touch.c
   client/gamma.c
   client/s_dsp.c
   client/s_load.c
   client/s_main.c
   client/s_mix.c
   client/s_mouth.c
   client/s_stream.c
   client/s_utils.c
   client/s_vox.c
   client/vgui/vgui_draw.c
)

set(XASH_ENGINE_IMGUI_SOURCES
	client/imgui_impl_xash.cpp
	client/imgui_lcsm_warning.cpp
	client/imgui_console.cpp
	client/imgui_imewindow.cpp
	client/imgui_sprview.cpp
)

set(XASH_ENGINE_DEDICATED_SOURCES
	common/dedicated.c
)

set(XASH_ENGINE_LAUNCHER_SOURCES
	common/launcher.c
)

if(WIN32)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_WIN32_SOURCES})
endif()
if(XASH_SDL)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_SDL_SOURCES})
endif()
if(ANDROID)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_ANDROID_SOURCES})
endif()
if(APPLE)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_APPLE_SOURCES})
endif()
if(NOT XASH_DEDICATED)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_CLIENT_SOURCES})
else()
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_DEDICATED_SOURCES})
endif()
if(XASH_IMGUI)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_IMGUI_SOURCES})
add_definitions(-DXASH_IMGUI=1)
endif()
if(XASH_SINGLE_BINARY)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_LAUNCHER_SOURCES})
endif()


include_directories(
	.
	common/
	common/imagelib/
	common/soundlib/
	client/
	client/vgui/
	server/
	../common
	../pm_shared
)

if(XASH_NANOGL)
	set(XASH_NANOGL_SRC ${CMAKE_SOURCE_DIR}/nanogl CACHE PATH "NanoGL path")
	file(GLOB_RECURSE XASH_ENGINE_GLESWRAP_SOURCES ${XASH_NANOGL_SRC}/*.cpp) # NanoGL version
	if(NOT XASH_ENGINE_GLESWRAP_SOURCES)
		message(FATAL_ERROR "Clone both nanogl sources to engine folder!")
	endif()
	list(APPEND XASH_ENGINE_SOURCES ${XASH_ENGINE_GLESWRAP_SOURCES})

	include_directories(${XASH_NANOGL_SRC}/ ${XASH_NANOGL_SRC}/GL/)
	add_definitions(-DXASH_NANOGL -D__MULTITEXTURE_SUPPORT__)
elseif(XASH_WES)
	file(GLOB XASH_ENGINE_GLESWRAP_SOURCES ${XASH_WES_SRC}/src/*.c) # wesgl version
	if(NOT XASH_ENGINE_GLESWRAP_SOURCES)
		message(FATAL_ERROR "Clone both gl-wes-v2 and nanogl sources to engine folder!")
	endif()
	list(APPEND XASH_ENGINE_SOURCES ${XASH_ENGINE_GLESWRAP_SOURCES})

	include_directories(${XASH_NANOGL_SRC}/ ${XASH_NANOGL_SRC}/GL/ ${XASH_WES_SRC}/src/)
	add_definitions(-DXASH_WES)
elseif(XASH_QINDIEGL)
	file(GLOB XASH_ENGINE_GLESWRAP_SOURCES 
		${XASH_QINDIEGL_SRC}/code/d3d_array.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_blend.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_clip.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_combiners.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_eval.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_extension.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_feedback.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_get.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_global.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_immediate.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_light.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_lists.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_material.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_matrix.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_matrix_stack.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_misc.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_object.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_pixels.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_state.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_stencil.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_texgen.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_texture.cpp
		${XASH_QINDIEGL_SRC}/code/d3d_wrapper.cpp
		${XASH_QINDIEGL_SRC}/msvc/opengl32.def
	)
	if(NOT XASH_ENGINE_GLESWRAP_SOURCES)
		message(FATAL_ERROR "Clone QindieGL sources to engine folder!")
	endif()

	find_package(DirectX REQUIRED)
	if(NOT ${DirectX9_FOUND})
		message(FATAL_ERROR "DirectX SDK not found! Please install DirectX SDK from https://www.microsoft.com/en-us/download/details.aspx?id=6812")
	endif()
	
	add_library(QindieGL STATIC ${XASH_ENGINE_GLESWRAP_SOURCES})
	target_include_directories(QindieGL PRIVATE ${DirectX9_INCLUDE_DIR})
	target_include_directories(QindieGL PRIVATE ${XASH_QINDIEGL_SRC}/code)
	target_include_directories(QindieGL PUBLIC ${XASH_QINDIEGL_SRC}/code/gl_headers)
	target_compile_definitions(QindieGL PUBLIC -DXASH_QINDIEGL)

	target_link_libraries(QindieGL PUBLIC ${DirectX9_LIBRARY} ${DirectX9_D3DX9_LIBRARY} ${DirectX9_DXERR_LIBRARY} legacy_stdio_definitions)
	# install( TARGETS QindieGL DESTINATION ".")
endif()

add_definitions(-DXASH_FORCEINLINE)

# Important, as HLSDK and engine shares some symbol names!
if(NOT WIN32) # Windows by default have hidden visibility and better linker
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

# ----- Conditions -----

if(XASH_VECTORIZE_SINCOS) # I know what I am doing and I want to build version that requires SSE
	add_definitions(-DVECTORIZE_SINCOS)
endif()

if(ANDROID)
	add_library(${XASH_ENGINE} SHARED ${XASH_ENGINE_SOURCES})
elseif(XASH_STATIC_GAMELIB)
	add_definitions(-DSINGLE_BINARY)
	if(XASH_SDL)
		add_definitions(-DXASH_SDLMAIN)
	endif()
	add_library(${XASH_ENGINE} STATIC ${XASH_ENGINE_SOURCES})
elseif(XASH_SINGLE_BINARY) # Set executable or library
	add_definitions(-DSINGLE_BINARY)
	add_executable(${XASH_ENGINE} ${XASH_ENGINE_SOURCES})
else()
	add_library(${XASH_ENGINE} SHARED ${XASH_ENGINE_SOURCES})
endif()

if(XASH_STATIC_GAMELIB)
	#target_link_libraries(${XASH_ENGINE} xashmenu)
	target_link_libraries(${XASH_ENGINE} xashmenu client server)
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

if(MINGW OR LINUX)
	add_definitions(-DGDB_BREAK)
elseif(WIN32 AND NOT XASH_64BIT)
	add_definitions( -DDBGHELP ) # dbghelp crashhandler
endif()

if(MINGW)
	target_link_libraries(${XASH_ENGINE} -luser32 -lkernel32 -lgdi32 -ldbghelp -lpsapi -lcomctl32)
endif()

if(NOT WIN32 OR XASH_64BIT)
	if(XASH_USE_SELECT)
		add_definitions(-DUSE_SELECT)
	endif()
	add_definitions(-DCOLORIZE_CONSOLE)
endif()

if(NOT XASH_64BIT AND NOT MSVC)
	add_definitions(-DXASH_FASTSTR)
endif()

if(XASH_IPX)
	add_definitions(-DXASH_IPX)
endif()

if(XASH_NONSTANDARD_LOAD)
	add_definitions(-DXASH_NONSTANDARD_LOAD)
endif()

if(XASH_DEDICATED)
	add_definitions(-DXASH_DEDICATED)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND NOT XASH_SDL)
		target_link_libraries(${XASH_ENGINE} -lrt)
	endif()
endif()

if(XASH_IMGUI)
	target_link_libraries(${XASH_ENGINE} imgui imgui_ext)
endif()

if(XASH_SDL)
	set(SDL2_BUILDING_EXECUTABLE ${XASH_SINGLE_BINARY}) # link sdl2main, if needed
	add_definitions(-DXASH_SDL)
	
	target_link_libraries(${XASH_ENGINE} sdl2)
endif()

if(XASH_GLES)
	add_definitions(-DXASH_GLES -DXASH_GL_STATIC)
endif()

if(XASH_QINDIEGL)
	add_definitions(-DXASH_D3D -DXASH_GL_STATIC -D__MULTITEXTURE_SUPPORT__)
	target_link_libraries(${XASH_ENGINE} QindieGL)
endif()

if(XASH_SAILFISH)
	add_definitions(-D__SAILFISH__)
endif()

if(XASH_DLL_LOADER)
	add_definitions(-DDLL_LOADER)

	include_directories(../loader)

	# See target_link_vgui_hack

	# Not needed anymore, loader is static always
	# add_custom_command(TARGET ${XASH_ENGINE} PRE_LINK COMMAND
	#    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:loader> $<TARGET_FILE_DIR:${XASH_ENGINE}>)

	target_link_libraries(${XASH_ENGINE} loader)
endif()

if(XASH_LIBDL)
	target_link_libraries(${XASH_ENGINE} ${CMAKE_DL_LIBS})
else()
	add_definitions(-DNO_LIBDL)
endif()

if(APPLE)
	target_link_libraries(${XASH_ENGINE} "-framework AppKit -framework AudioToolBox")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
	SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-D_DEBUG)
endif()

if(XASH_RELEASE)
	add_definitions(-DXASH_RELEASE)
else()
	execute_process(COMMAND "git" "rev-parse" "--short" "HEAD"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		OUTPUT_VARIABLE SHORT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	if(SHORT_HASH)
		message(STATUS "git hash: ${SHORT_HASH}")
		add_definitions(-DXASH_BUILD_COMMIT="${SHORT_HASH}")
	else()
		message(STATUS "git hash: not set")
	endif()
endif()

if(XASH_NO_ASYNC_NS_RESOLVE)
	add_definitions(-DXASH_NO_ASYNC_NS_RESOLVE)
endif()

if(XASH_USE_STB_SPRINTF)
	add_definitions(-DXASH_USE_STB_SPRINTF)
endif()

if(ANDROID)
	target_link_libraries(${XASH_ENGINE} -ldl -llog)
elseif(NOT WIN32)
	if(NOT HAIKU)
		target_link_libraries(${XASH_ENGINE} -lm -lpthread)
	else()
		target_link_libraries(${XASH_ENGINE} -lm -lpthread -lnetwork)
	endif()
	# For single binary builds we change name to game_launch's executable name
	# For single binary builds we change name to game_launch's executable name
	if(XASH_SINGLE_BINARY)
		set_target_properties(${XASH_ENGINE} PROPERTIES
			    OUTPUT_NAME "xash3d")
	endif()
else()
	# For Win32 we have split DLL names
	if(XASH_DEDICATED)
		set_target_properties(${XASH_ENGINE} PROPERTIES
			OUTPUT_NAME "xash_dedicated")
	else()
		set_target_properties(${XASH_ENGINE} PROPERTIES
			OUTPUT_NAME "xash_sdl")
	endif()
endif()

if( NOT XASH_STATIC_GAMELIB )
	install( TARGETS ${XASH_ENGINE} DESTINATION ".")
endif()