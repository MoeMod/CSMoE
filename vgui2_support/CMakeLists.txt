
set(CMAKE_CXX_STANDARD 20)

add_library(vgui2_interfaces INTERFACE)
target_include_directories(vgui2_interfaces INTERFACE 
    ./ 
    interfaces/
    ${HLSDK_PATH}/common
	${HLSDK_PATH}/engine
	${HLSDK_PATH}/pm_shared
)
target_link_libraries(vgui2_interfaces INTERFACE vinterface mathlib)

add_library(vguimatsurface STATIC
		vguimatsurface/Clip2D.cpp
		vguimatsurface/FontTextureCache.cpp
		)
target_include_directories(vguimatsurface PRIVATE
		./
		vgui2
		${HL2SDK_PATH}/public
		${HL2SDK_PATH}/public/tier0
		${HL2SDK_PATH}/public/tier1
		${HL2SDK_PATH}/common
		interfaces/
		vguimatsurface/
		${HLSDK_PATH}/common
		${HLSDK_PATH}/engine
		${HLSDK_PATH}/public
		)
target_link_libraries(vguimatsurface PRIVATE vgui_surfacelib)

add_subdirectory(vgui_controls)
add_subdirectory(cso_controls)
add_subdirectory(GameUI)

add_library(vgui2 STATIC
		vgui2/src/Bitmap.cpp
		vgui2/src/Border.cpp
		vgui2/src/input.cpp
		vgui2/src/LocalizedStringTable.cpp
		vgui2/src/MemoryBitmap.cpp
		vgui2/src/Scheme.cpp
		${HL2SDK_PATH}/public/UnicodeFileHelpers.cpp
		vgui2/src/vgui.cpp
		vgui2/src/VGuiKeyValues.cpp
		vgui2/vgui_key_translation.cpp
		vgui2/src/VPanel.cpp
		vgui2/src/VPanelWrapper.cpp
		vgui_controls/controls.cpp
		)
if(WIN32)
	target_sources(vgui2 PRIVATE
			vgui2/src/system_win32.cpp
			vgui2/src/HtmlWindow.cpp
			)
endif()
if(POSIX)
	target_sources(vgui2 PRIVATE
			vgui2/src/system_posix.cpp
			)
endif()
target_compile_definitions(vgui2 PRIVATE -DDONT_PROTECT_FILEIO_FUNCTIONS=1)
target_include_directories(vgui2 PUBLIC
		.
		vgui2
		interfaces/
		${HL2SDK_PATH}/public
		${HL2SDK_PATH}/public/tier0
		${HL2SDK_PATH}/public/tier1
		${HL2SDK_PATH}/common
		)
target_link_libraries(vgui2 PRIVATE platform_config tier0 tier1 vstdlib 3rdparty-sdl2)

add_library(vgui_surfacelib STATIC
		vgui2/vgui_surfacelib/FontAmalgam.cpp
		vgui2/vgui_surfacelib/FontManager.cpp
		vgui2/vgui_surfacelib/FontEffects.cpp
		)
if(WIN32)
	target_sources(vgui_surfacelib PRIVATE
			# MoeMod : use freetype to support emoji...
			#vgui2/vgui_surfacelib/Win32Font.cpp
			vgui2/vgui_surfacelib/linuxfont.cpp
			)
	target_link_libraries(vgui_surfacelib PUBLIC freetype)
endif()
if(OSX)
	target_sources(vgui_surfacelib PRIVATE
			# MoeMod : We dont use Carbon API any more...
			#vgui2/vgui_surfacelib/osxfont.cpp
			vgui2/vgui_surfacelib/linuxfont.cpp
			)
	target_link_libraries(vgui_surfacelib PUBLIC 3rdparty-freetype)
endif()
if(LINUX)
	target_sources(vgui_surfacelib PRIVATE
			# MoeMod : We dont use Carbon API any more...
			#vgui2/vgui_surfacelib/osxfont.cpp
			vgui2/vgui_surfacelib/linuxfont.cpp
			)
if(NOT ANDROID)
	find_package(Fontconfig REQUIRED)
	target_link_libraries(vgui_surfacelib PUBLIC Fontconfig::Fontconfig)
endif()
	target_link_libraries(vgui_surfacelib PUBLIC 3rdparty-freetype)
endif()
target_include_directories(vgui_surfacelib PUBLIC
		vgui2
		interfaces/
		${HL2SDK_PATH}/common
		${HL2SDK_PATH}/public
		${HL2SDK_PATH}/public/tier0
		${HL2SDK_PATH}/public/tier1
		)
target_link_libraries(vgui_surfacelib PUBLIC platform_config video_config)

add_library(dead_wrapper STATIC
    dead_wrapper/deadfilesystem.cpp
    dead_wrapper/deadinput.cpp
    dead_wrapper/deadipanel.cpp
    dead_wrapper/deadscheme.cpp
    dead_wrapper/deadsurface.cpp
	${HL2SDK_PATH}/public/filesystem_helpers.cpp
)
target_link_libraries(dead_wrapper PUBLIC vgui2_interfaces)
target_link_libraries(dead_wrapper PRIVATE vguimatsurface)
target_link_libraries(dead_wrapper PRIVATE tier0)

add_library(vgui2_support STATIC
	BaseUISurface.cpp
    CBaseUI.cpp
	vgui2_int.cpp
    CGameUIFuncs.cpp
    vgui_int.cpp
    udll_int.cpp
    interface_static.cpp
    filesystem_xash.cpp
    vgui2_surface.cpp
)

target_link_libraries(vgui2_support PUBLIC vgui2_interfaces)
target_link_libraries(vgui2_support PRIVATE platform_config)
target_link_libraries(vgui2_support PRIVATE video_config)
target_link_libraries(vgui2_support PRIVATE vgui_controls)
target_link_libraries(vgui2_support PRIVATE cso_controls)
target_link_libraries(vgui2_support PRIVATE vgui_surfacelib)
target_link_libraries(vgui2_support PRIVATE vguimatsurface)
target_link_libraries(vgui2_support PRIVATE dead_wrapper)
target_link_libraries(vgui2_support PRIVATE tier0 tier1 vstdlib mathlib)
