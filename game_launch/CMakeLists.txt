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
if(XASH_SDL AND NOT WIN32)
	set(SDL2_BUILDING_EXECUTABLE YES) # Link SDL2main also
	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIR})
	add_definitions(-DXASH_SDL)
endif()

include_directories(
	.
)

if(WIN32)
	add_executable(${XASH_LAUNCHER} WIN32 xash.c game.rc)
	target_link_libraries(${XASH_LAUNCHER} ${CMAKE_DL_LIBS})
	set_target_properties(${XASH_LAUNCHER} PROPERTIES
		OUTPUT_NAME csmoe_win32) # Change name to your gamedir, if you building a standalone game
	install( TARGETS ${XASH_LAUNCHER} DESTINATION ${CMAKE_INSTALL_PREFIX} )
elseif(APPLE)
	set(APP_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/DragNDrop/Icon.icns")
	set_source_files_properties(${APP_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	find_library(SDL2_LIBRARY_FILE SDL2)
	#set_source_files_properties(${SDL2_LIBRARY_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)

	add_executable(${XASH_LAUNCHER} MACOSX_BUNDLE xash_static.cpp ${APP_ICNS})

	set_target_properties(${XASH_LAUNCHER} PROPERTIES
			OUTPUT_NAME CSMoE) # Change name to your gamedir, if you building a standalone game

	set_target_properties(${XASH_LAUNCHER} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in")
	set_target_properties(${XASH_LAUNCHER} PROPERTIES MACOSX_BUNDLE_ICON_FILE "Icon.icns")

	target_link_libraries(${XASH_LAUNCHER} xash)

	include(InstallRequiredSystemLibraries)

	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_PACKAGE_VENDOR "MoeMod")
	set(CPACK_PACKAGE_FILE_NAME "CSMoE-macOS-${CMAKE_SYSTEM_PROCESSOR}")
	set(CPACK_DMG_VOLUME_NAME "CSMoE")
	set(CPACK_DMG_FORMAT "UDBZ") #UDRW UDBZ
	set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/cmake/DragNDrop/background.png")
	set(CPACK_DMG_DS_STORE "${CMAKE_SOURCE_DIR}/cmake/DragNDrop/DS_Store")

	set(CMAKE_INSTALL_PREFIX "/Applications")
	set(APPS "\${CMAKE_INSTALL_PREFIX}/CSMoE.app")
	install(TARGETS ${XASH_LAUNCHER} BUNDLE DESTINATION ".")
	install(CODE "
		include(BundleUtilities)
		fixup_bundle(\"${APPS}\"   \"\"   \"${SDL2_LIBRARY_FILE}\")
		")

	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/csmoe DESTINATION "./CSMoE.app/Contents/Resources" OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/cstrike DESTINATION "./CSMoE.app/Contents/Resources" OPTIONAL)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/CSMoE-Full/valve DESTINATION "./CSMoE.app/Contents/Resources" OPTIONAL)

	set(CPACK_BINARY_DRAGNDROP ON)
	include(CPack)
	add_custom_target(csmoe_cpack_dmg ALL
			COMMAND "cpack"
			DEPENDS ${XASH_LAUNCHER} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" COMMENT "Generating DMG file")

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(${XASH_LAUNCHER} xash.c)
	target_link_libraries(${XASH_LAUNCHER} ${CMAKE_DL_LIBS})
	set_target_properties(${XASH_LAUNCHER} PROPERTIES
		OUTPUT_NAME csmoe_linux) # Change name to your gamedir, if you building a standalone game
	install( TARGETS ${XASH_LAUNCHER} DESTINATION ${CMAKE_INSTALL_PREFIX} )
endif()

if(XASH_SDL AND NOT WIN32)
	target_link_libraries(${XASH_LAUNCHER} ${SDL2_LIBRARY})
endif()