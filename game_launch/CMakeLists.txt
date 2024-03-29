#
# Copyright (c) 2015 Pavlo Lavrenenko, Alibek Omarov
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

cmake_minimum_required(VERSION 3.0)
project(XASH_LAUNCHER)

set(XASH_LAUNCHER game_launch)
set(CMAKE_CXX_STANDARD 14)

# SDL2 provides it's own crossplatform main() implementation and simple message boxes
# Win32 uses WinMain() implemented in launcher code and system message boxes
# Otherwise it prints errors in stdin.
if(XASH_SDL)
	set(SDL2_BUILDING_EXECUTABLE YES) # Link SDL2main also
	add_definitions(-DXASH_SDL)
endif()

include_directories(
	.
)

if(WIN32)

	if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
		set(APP_MANIFEST_FILE "${CMAKE_SOURCE_DIR}/msvc/Package.appxmanifest")
		configure_file(
			${APP_MANIFEST_NAME}
			${CMAKE_CURRENT_BINARY_DIR}/${APP_MANIFEST_NAME}
			@ONLY
		)

		set(CONTENT_FILES "")
		set(ASSET_FILES "")

		file(GLOB_RECURSE PATHED_ICON_FILES "${CMAKE_SOURCE_DIR}/msvc/Images/*.png")
		foreach(PATHED ${PATHED_ICON_FILES})
			list(APPEND ASSET_FILES ${PATHED})
		endforeach()
		set_property(SOURCE ${APP_MANIFEST_NAME} ${CONTENT_FILES} ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
		set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")

		set_target_properties(${XASH_LAUNCHER} PROPERTIES VS_CPPWINRT true)
		set_target_properties(${XASH_LAUNCHER} PROPERTIES VS_GLOBAL_ROOTNAMESPACE "CSMoE-UWP")
		set_target_properties(${XASH_LAUNCHER} PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION 10.0.15063.0)

		add_executable(${XASH_LAUNCHER} xash_static.cpp ${APP_MANIFEST_FILE} ${CONTENT_FILES} ${ASSET_FILES})
		target_link_libraries(${XASH_LAUNCHER} PRIVATE WindowsApp)
		set_property(TARGET ${XASH_LAUNCHER} PROPERTY VS_WINRT_COMPONENT TRUE)
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${XASH_LAUNCHER})
	else()
		add_executable(${XASH_LAUNCHER} WIN32 xash_static.cpp game.rc)
		if(XASH_SDL)
			# add missing libs
			target_link_libraries(${XASH_LAUNCHER} version.lib hid.lib winmm.lib setupapi.lib)
		endif()
	endif()
	
	target_link_libraries(${XASH_LAUNCHER} xash)
	string(REGEX REPLACE "/machine:(.+)" "\\1" XASH_ARCH "${CMAKE_EXE_LINKER_FLAGS}")
	string(TOLOWER ${XASH_ARCH} XASH_ARCH)
	set_target_properties(${XASH_LAUNCHER} PROPERTIES
		OUTPUT_NAME csmoe_win32_${XASH_ARCH} ) # Change name to your gamedir, if you building a standalone game

	set(CPACK_GENERATOR "NSIS")
	set(CPACK_PACKAGE_NAME "CSMoE")
	set(CPACK_PACKAGE_VENDOR "MoeMod")
	set(CPACK_PACKAGE_FILE_NAME "CSMoE-Win32-${XASH_ARCH}")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "CSMoE")
	set(CPACK_PACKAGE_EXECUTABLES "..\\\\csmoe_win32" "${CPACK_PACKAGE_NAME}")
	set(CPACK_CREATE_DESKTOP_LINKS ${XASH_LAUNCHER})
	set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
	set(CPACK_NSIS_MODIFY_PATH OFF)
	set(CPACK_NSIS_MUI_FINISHPAGE_RUN ${XASH_LAUNCHER})

	install(TARGETS ${XASH_LAUNCHER} DESTINATION ".")
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/csmoe DESTINATION "." OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/cstrike DESTINATION "." OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/valve DESTINATION "." OPTIONAL)

	include(CPack)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		add_custom_target(csmoe_cpack_nsis
			COMMAND "cpack.exe" "--config" "CPackConfig.cmake" "-G" "NSIS64" "-V"
			DEPENDS ${XASH_LAUNCHER} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" COMMENT "Generating NSIS x64")
	else()
		add_custom_target(csmoe_cpack_nsis
			COMMAND "cpack.exe" "--config" "CPackConfig.cmake" "-G" "NSIS" "-V"
			DEPENDS ${XASH_LAUNCHER} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" COMMENT "Generating NSIS x86")
	endif()

elseif(APPLE)
	set(APP_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/DragNDrop/Icon.png")
	set_source_files_properties(${APP_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	#find_library(SDL2_LIBRARY_FILE SDL2)
	#set_source_files_properties(${SDL2_LIBRARY_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)

	add_executable(${XASH_LAUNCHER} MACOSX_BUNDLE xash_static.cpp ${APP_ICNS})

	set_target_properties(${XASH_LAUNCHER} PROPERTIES
			OUTPUT_NAME CSMoE) # Change name to your gamedir, if you building a standalone game

	set_target_properties(${XASH_LAUNCHER} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in")
	set_target_properties(${XASH_LAUNCHER} PROPERTIES MACOSX_BUNDLE_ICON_FILE "Icon.png")

	target_link_libraries(${XASH_LAUNCHER} xash)

	include(InstallRequiredSystemLibraries)

	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_PACKAGE_VENDOR "MoeMod")
	if(CMAKE_OSX_ARCHITECTURES)
		set(CPACK_PACKAGE_FILE_NAME "CSMoE-macOS-Universal")
	else()
		set(CPACK_PACKAGE_FILE_NAME "CSMoE-macOS-${CMAKE_SYSTEM_PROCESSOR}")
	endif()
	set(CPACK_DMG_VOLUME_NAME "CSMoE")
	set(CPACK_DMG_FORMAT "UDBZ") #UDRW UDBZ
	set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/cmake/DragNDrop/background.png")
	set(CPACK_DMG_DS_STORE "${CMAKE_SOURCE_DIR}/cmake/DragNDrop/DS_Store")

	set(CMAKE_INSTALL_PREFIX "/Applications")
	set(APPS "\${CMAKE_INSTALL_PREFIX}/CSMoE.app")
	install(TARGETS ${XASH_LAUNCHER} BUNDLE DESTINATION ".")
	#install(CODE "
	#	include(BundleUtilities)
	#	fixup_bundle(\"${APPS}\"   \"\"   \"${SDL2_LIBRARY_FILE}\")
	#	")

	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/csmoe DESTINATION "./CSMoE.app/Contents/Resources" OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/cstrike DESTINATION "./CSMoE.app/Contents/Resources" OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/valve DESTINATION "./CSMoE.app/Contents/Resources" OPTIONAL)

	set(CPACK_BINARY_DRAGNDROP ON)
	include(CPack)
	add_custom_target(csmoe_cpack_dmg
			COMMAND "cpack"
			DEPENDS ${XASH_LAUNCHER} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" COMMENT "Generating DMG file")

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(${XASH_LAUNCHER} xash.c)
	target_link_libraries(${XASH_LAUNCHER} ${CMAKE_DL_LIBS})
	set_target_properties(${XASH_LAUNCHER} PROPERTIES
		OUTPUT_NAME csmoe_linux) # Change name to your gamedir, if you building a standalone game
	install( TARGETS ${XASH_LAUNCHER} DESTINATION ${CMAKE_INSTALL_PREFIX} )
endif()
