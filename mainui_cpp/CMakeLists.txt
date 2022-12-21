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

cmake_minimum_required(VERSION  2.8.0)
project (MAINUI)

# By default we require C++11 support.
# But it can be overriden with -DMY_COMPILER_SUCKS compiler option
# It's not recommended and may not work(as it's done only for MSVC6 support)
set(CMAKE_CXX_STANDARD 20)

set(MAINUI_LIBRARY xashmenu)
option(MAINUI_USE_CUSTOM_FONT_RENDER "Use custom font rendering" ON)
option(MAINUI_USE_STB "Use stb_truetype.h for rendering(*nix-only)" OFF)
option(MAINUI_RENDER_PICBUTTON_TEXT "Use custom font render to render picbuttons(experimental)" OFF)
option(MAINUI_FONT_SCALE "Scale fonts by height" OFF)

if(XASH_IMGUI)
	set(MAINUI_USE_CUSTOM_FONT_RENDER OFF)
endif()

if(NOT XASH_SDK)
	set(XASH_SDK "../")
endif()

set(MAINUI_CONTROLS_SOURCES
	controls/Framework.cpp
	controls/BaseItem.cpp
	controls/Action.cpp
	controls/Bitmap.cpp
	controls/CheckBox.cpp
	controls/ItemsHolder.cpp
	controls/Field.cpp
	controls/PicButton.cpp
	controls/Slider.cpp
	controls/SpinControl.cpp
	controls/YesNoMessageBox.cpp
	controls/MessageBox.cpp
	controls/Editable.cpp
	controls/Switch.cpp
	controls/BaseWindow.cpp
	controls/ProgressBar.cpp
	controls/BackgroundBitmap.cpp
	controls/Table.cpp
	controls/TabView.cpp
	controls/ScrollView.cpp
)

set(MAINUI_MENUS_SOURCES
	menus/AdvancedControls.cpp
	menus/Audio.cpp
	menus/Configuration.cpp
	menus/Controls.cpp
	menus/CreateGame.cpp
	menus/Credits.cpp
	menus/CustomGame.cpp
	menus/FileDialog.cpp
	menus/GameOptions.cpp
	menus/Gamepad.cpp
	menus/InputDevices.cpp
	menus/LoadGame.cpp
	menus/Main.cpp
	menus/Multiplayer.cpp
	menus/NewGame.cpp
	menus/PlayerSetup.cpp
	menus/SaveLoad.cpp
	menus/ServerBrowser.cpp
	menus/TouchButtons.cpp
	menus/Touch.cpp
	menus/TouchEdit.cpp
	menus/TouchOptions.cpp
	menus/Video.cpp
	menus/VideoModes.cpp
	menus/VideoOptions.cpp
	menus/PlayerIntroduceDialog.cpp
	menus/ConnectionProgress.cpp
	menus/ConnectionWarning.cpp
	menus/dynamic/ScriptMenu.cpp
)

set(MAINUI_FONT_RENDER_SOURCES
	font/WinAPIFont.cpp
	font/BitmapFont.cpp
	font/FontManager.cpp
	font/FreeTypeFont.cpp
	font/StbFont.cpp
	font/BaseFontBackend.cpp
)

set(MAINUI_LUASH_SOURCES
	luash_menu/luash_menu.cpp
	luash_menu/luash_menu_engfunc.cpp
	luash_menu/luash_menu_textfunc.cpp
	luash_menu/luash_menu_filesystem.cpp
)

set_source_files_properties(
    ${MAINUI_LUASH_SOURCES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
)

set(MAINUI_SOURCES
	utl/unicode_strtools.cpp
	EventSystem.cpp
	EngineCallback.cpp
	BaseMenu.cpp
	Btns.cpp
	Color.cpp
	MenuStrings.cpp
	CFGScript.cpp
	Utils.cpp
	Scissor.cpp
	udll_int.cpp
)

if(CS16CLIENT)
	add_definitions(-DCS16CLIENT)
	list(APPEND MAINUI_SOURCES
		${XASH_SDK}/common/interface.cpp
		menus/Scoreboard.cpp
		menus/client/JoinGame.cpp
		menus/client/JoinClass.cpp
		menus/client/ClientWindow.cpp
		)
endif()

if(XASH_IMGUI)
	add_definitions(-DXASH_IMGUI)
	list(APPEND MAINUI_SOURCES
		imgui_menu/imgui_menu.cpp
		imgui_menu/imgui_menu_msgbox.cpp
		imgui_menu/imgui_menu_server.cpp
		imgui_menu/imgui_menu_update.cpp
	)
endif()

add_definitions(-DMAINUI_DLL=1)

list(APPEND MAINUI_SOURCES ${MAINUI_CONTROLS_SOURCES})
list(APPEND MAINUI_SOURCES ${MAINUI_MENUS_SOURCES})
list(APPEND MAINUI_SOURCES ${MAINUI_FONT_RENDER_SOURCES})
list(APPEND MAINUI_SOURCES ${MAINUI_LUASH_SOURCES})

if(XASH_STATIC_GAMELIB)
	add_library (${MAINUI_LIBRARY} STATIC ${MAINUI_SOURCES})
else()
	add_library (${MAINUI_LIBRARY} SHARED ${MAINUI_SOURCES})
endif()

if(XASH_IMGUI)
	target_link_libraries(${MAINUI_LIBRARY} PRIVATE imgui imgui_ext)
endif()
target_link_libraries(${MAINUI_LIBRARY} PRIVATE luash)
target_link_libraries(${MAINUI_LIBRARY} PRIVATE cqmiao json_3p)

if(XASH_SAILFISH)
	add_definitions(-D__SAILFISH__)
endif()

if(MAINUI_FONT_SCALE)
	add_definitions(-DMAINUI_FONT_SCALE)
endif()

# Force stb_truetype for Apple devices, as there is no Apple native font renderer
# And freetype&fontconfig isn't used on this platform by default
if(APPLE)
	set(MAINUI_USE_STB TRUE)
endif()

include_directories(${XASH_SDK}/common ${XASH_SDK}/engine ${XASH_SDK}/pm_shared . controls/ menus/ font/ model/ ../public/)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Font Rendering(FreeType or WinAPI)
if(MAINUI_USE_CUSTOM_FONT_RENDER)
	# Win32 will always use GDI font renderer
	if(NOT WIN32)
		if(MAINUI_USE_STB)
			# Use stbtt
			add_definitions(-DMAINUI_USE_STB)
		else()
			# Use freetype2
			find_package(PkgConfig)
			pkg_check_modules(FC REQUIRED fontconfig)
			include_directories(${FC_INCLUDE_DIRS})
			target_link_libraries(${MAINUI_LIBRARY} ${FC_LIBRARIES})
			add_definitions(-DMAINUI_USE_FREETYPE)
		endif()
	endif()

	add_definitions(-DMAINUI_USE_CUSTOM_FONT_RENDER)
	if(MAINUI_RENDER_PICBUTTON_TEXT)
		add_definitions(-DMAINUI_RENDER_PICBUTTON_TEXT)
	endif()
endif()

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

if(XASH_UNITY_BUILD)
	#set_target_properties(${MAINUI_LIBRARY} PROPERTIES UNITY_BUILD ON)
endif()

if(NOT XASH_STATIC_GAMELIB)
	install( TARGETS ${MAINUI_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/csmoe/cl_dlls )
endif()
