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
project(XASH_ENGINE CXX)

set(CMAKE_CXX_STANDARD 20)

# fix C++17 register
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	add_compile_options(-Wno-register)
endif()

option(XASH_DEDICATED "Enable dedicated build. Servers must choose this." OFF) # Servers must choose this

if(XASH_DEDICATED)
	set(XASH_ENGINE xashds) # Lib name
else()
	set(XASH_ENGINE xash) # Lib name
endif()

option(XASH_GLES   "Enable if Xash3D is running over GL to GLES translator." OFF)
option(XASH_NANOGL "Use NanoGL(GLES1). Implicitly enables XASH_GLES." OFF)
if(XASH_NANOGL)
	set(XASH_NANOGL_SRC "${CMAKE_SOURCE_DIR}/nanogl" CACHE PATH "NanoGL path")
endif()
option(XASH_WES    "Use gl-wes-v2(GLES2). Implicitly enables XASH_GLES." OFF)
if(XASH_WES)
	set(XASH_WES_SRC "${CMAKE_SOURCE_DIR}/gl-wes-v2" CACHE PATH "GL WES path")
endif()
option(XASH_GL4ES    "Use gl4es(GLES2). Implicitly enables XASH_GLES." OFF)
if(WIN32)
	option(XASH_QINDIEGL    "Use QindieGL(D3D9). Only avaible on Win32" OFF)
endif()
if(XASH_QINDIEGL)
	set(XASH_QINDIEGL_SRC "${CMAKE_SOURCE_DIR}/3rdparty/QindieGL" CACHE PATH "QindieGL path")
endif()
option(XASH_NO_ASYNC_NS_RESOLVE "Disable asynchronous domain name resolving." OFF)

option(XASH_VECTORIZE_SINCOS "Try to use vectorized versions of sin(), cos() and sincos()" ON)
option(XASH_SKIPCRTLIB "Use system CRT" ON)

option(XASH_CPUINFO "Build with cpuinfo support" OFF)
set(XASH_ASTC ON)


if(NOT APPLE AND NOT ANDROID)
	# CRT malloc is the best in iOS
	option(XASH_TBBMALLOC "Use tbbmalloc" ON)
else()
	set(XASH_TBBMALLOC OFF)
endif()

set(XASH_ENGINE_SOURCES 
	common/avikit.cpp
	common/build.cpp
	common/base_cmd.cpp
	common/cfgscript.cpp
	common/cmd.cpp
	common/common.cpp
	common/con_utils.cpp
	common/crclib.cpp
	common/crtlib.cpp
	common/cvar.cpp
	common/filesystem.cpp
	common/host.cpp
	common/hpak.cpp
	common/infostring.cpp
	common/identification.cpp
#	common/library.cpp
	common/masterlist.cpp
	common/mathlib.cpp
	common/matrixlib.cpp
	common/mod_studio.cpp
	common/model.cpp
	common/net_buffer.cpp
	common/net_chan.cpp
	common/net_encode.cpp
	common/net_huff.cpp
		common/net_http.cpp
	common/network.cpp
	common/pm_surface.cpp
	common/pm_trace.cpp
	common/random.cpp
	common/sys_con.cpp
	common/system.cpp
	common/titles.cpp
	common/world.cpp
	common/llm.cpp
	common/crashhandler.cpp
	common/imagelib/img_bmp.cpp
	common/imagelib/img_main.cpp
	common/imagelib/img_quant.cpp
	common/imagelib/img_tga.cpp
	common/imagelib/img_utils.cpp
	common/imagelib/img_wad.cpp
	common/imagelib/img_dds.cpp
	common/imagelib/img_astc.cpp
	common/soundlib/snd_main.cpp
	common/soundlib/snd_mp3.cpp
	common/soundlib/snd_utils.cpp
	common/soundlib/snd_wav.cpp
	common/soundlib/libmpg/libmpg.cpp
	common/Sequence.cpp
	common/mod_decryptor.cpp 
	common/mod_extend_seq.cpp 
	common/cpu.cpp
	common/textconsole.cpp
	common/cook.cpp
	common/model_encrypt.cpp
	common/fs_mmap.cpp
	server/sv_client.cpp
	server/sv_cmds.cpp
	server/sv_custom.cpp
	server/sv_frame.cpp
	server/sv_filter.cpp
	server/sv_game.cpp
	server/sv_init.cpp
	server/sv_main.cpp
	server/sv_log.cpp
	server/sv_move.cpp
	server/sv_phys.cpp
	server/sv_pmove.cpp
	server/sv_save.cpp
	server/sv_world.cpp
	server/sv_security.cpp

)

if(XASH_STATIC_GAMELIB)
	set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} common/library_static.cpp)
else()
	set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} common/library.cpp)
endif()

if(NOT XASH_DEDICATED)
	set(XASH_ENGINE_WIN32_SOURCES
		./platform/win32/con_win.cpp
		./platform/win32/ime_win32.cpp
	)

	set(XASH_ENGINE_SDL_SOURCES
		./platform/sdl/events.cpp
		./platform/sdl/gl_sdl.cpp
		./platform/sdl/snd_sdl.cpp
		./platform/sdl/vid_sdl.cpp
	)

	set(XASH_ENGINE_ANDROID_SOURCES
		common/launcher.cpp
		platform/android/android_nosdl.cpp
		platform/android/snd_opensles.cpp
		platform/ime_stub.cpp
	)

	if(IOS)
	set(XASH_ENGINE_APPLE_SOURCES
		common/launcher.cpp
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

endif()

set(XASH_ENGINE_CLIENT_SOURCES
   client/cl_cmds.cpp
   client/cl_demo.cpp
   client/cl_events.cpp
   client/cl_frame.cpp
   client/cl_game.cpp
   client/cl_main.cpp
   client/cl_menu.cpp
   client/cl_mobile.cpp
   client/cl_parse.cpp
   client/cl_pmove.cpp
   client/cl_remap.cpp
   client/cl_scrn.cpp
   client/cl_tent.cpp
   client/cl_video.cpp
   client/cl_view.cpp
   client/cl_netgraph.cpp
   client/gl_backend.cpp
   client/gl_beams.cpp
   client/gl_cull.cpp
   client/gl_decals.cpp
   client/gl_draw.cpp
   client/gl_image.cpp
   client/gl_mirror.cpp
   client/gl_refrag.cpp
   client/gl_rlight.cpp
   client/gl_rmain.cpp
   client/gl_rmath.cpp
   client/gl_rmisc.cpp
   client/gl_rpart.cpp
   client/gl_rsurf.cpp
   client/gl_rstrobe.cpp
   client/gl_sprite.cpp
   client/gl_studio.cpp
   client/vid_common.cpp
   client/gl_warp.cpp
   client/joyinput.cpp
   client/input.cpp
   client/keys.cpp
   client/input_evdevkey.cpp
   client/console.cpp
   client/touch.cpp
   client/gamma.cpp
   client/s_dsp.cpp
   client/s_load.cpp
   client/s_main.cpp
   client/s_mix.cpp
   client/s_mouth.cpp
   client/s_stream.cpp
   client/s_utils.cpp
   client/s_vox.cpp
   client/vgui/vgui_draw.cpp
   client/voice.cpp
)

set(XASH_ENGINE_IMGUI_SOURCES
	client/imgui_impl_xash.cpp
	client/imgui_lcsm_warning.cpp
	client/imgui_console.cpp
	client/imgui_imewindow.cpp
	client/imgui_sprview.cpp
	client/imgui_connectprogress.cpp
	client/imgui_surface.cpp
	client/imgui_menu_server.cpp
	client/imgui_menu_update.cpp
	client/imgui_menu_msgbox.cpp
)

set(XASH_ENGINE_VGUI2_SOURCES

)

set(XASH_ENGINE_DEDICATED_SOURCES
	common/dedicated.cpp
)

set(XASH_ENGINE_LAUNCHER_SOURCES
	common/launcher.cpp
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
if(XASH_VGUI2)
	set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_VGUI2_SOURCES})
	add_definitions(-DXASH_VGUI2=1)
endif()
if(XASH_SINGLE_BINARY)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} ${XASH_ENGINE_LAUNCHER_SOURCES})
endif()
if(NOT XASH_DEDICATED AND XASH_RAGDOLL)
set(XASH_ENGINE_SOURCES ${XASH_ENGINE_SOURCES} client/physics.cpp)
add_definitions(-DXASH_RAGDOLL=1)
endif()

if(MSVC)
	add_compile_options(/fp:fast)
else()
	add_compile_options(-ffast-math)
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
	../public
	../SourceSDK/public
)

if(XASH_NANOGL)
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
	add_definitions(-DXASH_WES -DWES_MANGLE_PREPEND)
elseif(XASH_GL4ES)
	add_definitions(-DXASH_GL4ES)
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

# ----- Conditions -----

if(XASH_VECTORIZE_SINCOS) # I know what I am doing and I want to build version that requires SSE
	add_definitions(-DVECTORIZE_SINCOS)
endif()
if(XASH_SKIPCRTLIB)
	add_definitions(-DXASH_SKIPCRTLIB)
endif()

if(ANDROID)
	add_definitions(-DSINGLE_BINARY)
	if(XASH_SDL)
		add_definitions(-DXASH_SDLMAIN)
	endif()
	add_library(${XASH_ENGINE} SHARED ${XASH_ENGINE_SOURCES})
elseif(XASH_STATIC_GAMELIB)
	add_definitions(-DSINGLE_BINARY)
	if(XASH_SDL)
		add_definitions(-DXASH_SDLMAIN)
	endif()
	if(XASH_SINGLE_BINARY)
		add_executable(${XASH_ENGINE} ${XASH_ENGINE_SOURCES})
	else()
		add_library(${XASH_ENGINE} STATIC ${XASH_ENGINE_SOURCES})
	endif()
elseif(XASH_SINGLE_BINARY) # Set executable or library
	add_definitions(-DSINGLE_BINARY)
	add_executable(${XASH_ENGINE} ${XASH_ENGINE_SOURCES})
else()
	add_library(${XASH_ENGINE} SHARED ${XASH_ENGINE_SOURCES})
endif()

if(XASH_STATIC_GAMELIB)
	#target_link_libraries(${XASH_ENGINE} xashmenu)
	target_link_libraries(${XASH_ENGINE} PRIVATE server)
	if(NOT XASH_DEDICATED)
		target_link_libraries(${XASH_ENGINE} PRIVATE client)
		if(XASH_VGUI2)
			target_link_libraries(${XASH_ENGINE} PUBLIC vgui2_support)
			target_link_libraries(${XASH_ENGINE} PUBLIC vgui2 GameUI)
		else()
			target_link_libraries(${XASH_ENGINE} PRIVATE xashmenu)
		endif()
		target_link_libraries(${XASH_ENGINE} PUBLIC 3rdparty-bullet3 )
	endif()
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
	target_link_libraries(${XASH_ENGINE} PRIVATE imgui imgui_ext)
endif()

if(XASH_SDL)
	set(SDL2_BUILDING_EXECUTABLE ${XASH_SINGLE_BINARY}) # link sdl2main, if needed
	add_definitions(-DXASH_SDL)

	target_link_libraries(${XASH_ENGINE} PUBLIC 3rdparty-sdl2)
endif()

if(XASH_GLES)
	add_definitions(-DXASH_GLES)
endif()

if(XASH_ANGLE)
	add_definitions(-DXASH_ANGLE -DXASH_GL_STATIC)
	target_link_libraries(${XASH_ENGINE} PRIVATE 3rdparty-angle)
	target_link_libraries(${XASH_ENGINE} PRIVATE 3rdparty-spirv_cross 3rdparty-spirv_tools 3rdparty-glslang)
endif()

if(XASH_QINDIEGL)
	add_definitions(-DXASH_GL_STATIC -D__MULTITEXTURE_SUPPORT__)
	target_link_libraries(${XASH_ENGINE} PRIVATE QindieGL)
endif()

if(XASH_GL4ES)
	add_definitions(-DXASH_GL_STATIC)
	target_link_libraries(${XASH_ENGINE} PRIVATE 3rdparty-gl4es)
endif()

if(XASH_CPUINFO)
	add_definitions(-DXASH_CPUINFO=1)

	target_link_libraries(${XASH_ENGINE} PRIVATE cpuinfo_3p)
endif()

if(XASH_ASTC)
	add_definitions(-DXASH_ASTC=1)

	target_link_libraries(${XASH_ENGINE} PRIVATE astcenc_3p)
endif()

if(XASH_TBBMALLOC)
	add_definitions(-DXASH_TBBMALLOC)

	target_link_libraries(${XASH_ENGINE} PRIVATE 3rdparty-tbbmalloc)
endif()

target_link_libraries(${XASH_ENGINE} PRIVATE xorstr_3p qrcode openssl_3p lua54_3p Boost::container 3rdparty-opus 3rdparty-mpg123)

if(WIN32)
	target_link_libraries(${XASH_ENGINE} PRIVATE 3rdparty-mman-win32)
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
	target_link_libraries(${XASH_ENGINE} PRIVATE ${CMAKE_DL_LIBS})
else()
	add_definitions(-DNO_LIBDL)
endif()

if(APPLE)
	target_link_libraries(${XASH_ENGINE} PUBLIC "-framework AppKit -framework AudioToolBox -framework IOSurface")
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
	target_link_libraries(${XASH_ENGINE} PUBLIC -ldl -llog -landroid -lEGL)
elseif(NOT WIN32)
	if(NOT HAIKU)
		target_link_libraries(${XASH_ENGINE} PUBLIC -lm -lpthread)
	else()
		target_link_libraries(${XASH_ENGINE} PUBLIC -lm -lpthread -lnetwork)
	endif()
endif()

if(XASH_DEDICATED)
	if(XASH_STATIC_GAMELIB)
		set_target_properties(${XASH_ENGINE} PROPERTIES
				OUTPUT_NAME "csmoe_dedicated")
	else()
		set_target_properties(${XASH_ENGINE} PROPERTIES
				OUTPUT_NAME "xash_dedicated")
	endif()
endif()

if(ANDROID)
	set_target_properties(${XASH_ENGINE} PROPERTIES
			OUTPUT_NAME "UE4")
endif()

# engine doesnt support unity build now
set_target_properties(${XASH_ENGINE} PROPERTIES UNITY_BUILD OFF)

if( NOT XASH_STATIC_GAMELIB OR XASH_SINGLE_BINARY )
	install( TARGETS ${XASH_ENGINE} DESTINATION ".")
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/csmoe DESTINATION "." OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/cstrike DESTINATION "." OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/valve DESTINATION "." OPTIONAL)
endif()