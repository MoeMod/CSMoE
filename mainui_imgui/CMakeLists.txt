cmake_minimum_required(VERSION 3.10.2)
project (mainui_imgui)

set(CMAKE_CXX_STANDARD 14)

set(MAINUI_LIBRARY xashmenu)

include_directories (./ ../common ../engine ../pm_shared ../mainui)
set (MAINUI_SOURCES
	udll_int.cpp
	Utils.cpp
	UIFuncs.cpp
 	UIFuncs.cpp

	GameUI/GameUI.cpp
	GameUI/GameMenu.cpp
	GameUI/OptionsDialog.cpp
)

if(XASH_STATIC_GAMELIB)
	add_library (${MAINUI_LIBRARY} STATIC ${MAINUI_SOURCES})
else()
	add_library (${MAINUI_LIBRARY} SHARED ${MAINUI_SOURCES})
endif()

target_link_libraries(${MAINUI_LIBRARY} imgui imgui_ext)

# Name library as it named in Unkle Mike's Xash3D
if(MAINUI_NAME)
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
			OUTPUT_NAME ${MAINUI_NAME})
elseif(ANDROID OR IOS)
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
			OUTPUT_NAME "menu")
elseif(WIN32)
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
			OUTPUT_NAME menu PREFIX "")
endif()

if(XASH_64BIT AND NOT ANDROID AND NOT IOS)
	get_target_property(MAINUI_NAME ${MAINUI_LIBRARY} OUTPUT_NAME)
	if(NOT MAINUI_NAME)
		set(MAINUI_NAME ${MAINUI_LIBRARY})
	endif()
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
			OUTPUT_NAME ${MAINUI_NAME}64)
endif()

if(NOT XASH_STATIC_GAMELIB)
	install( TARGETS ${MAINUI_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/csmoe/cl_dlls )
endif()