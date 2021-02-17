cmake_minimum_required(VERSION 3.12.0)
project (hymenu)

include_directories (../common ../engine ../pm_shared ../mainui)
add_library (hymenu STATIC 
	udll_int.cpp
	Utils.cpp
	UIFuncs.cpp
	ConnectProgress.cpp
	ConnectProgress.h
	OptionsDialog.cpp
	OptionsDialog.h
	SimpleLogin.cpp
	SimpleLogin.h
	Background.h
	Background.cpp
)
target_link_libraries(hymenu imgui imgui_ext qrcode)
