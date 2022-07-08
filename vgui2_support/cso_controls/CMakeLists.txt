
add_library(cso_controls STATIC
	TexturedButton.cpp
	RoundPanel.cpp
	SignalImagePanel.cpp
)
target_include_directories(cso_controls PUBLIC
		../interfaces/
		${HL2SDK_PATH}/public
		${HL2SDK_PATH}/public/tier0
		${HL2SDK_PATH}/public/tier1
		${HL2SDK_PATH}/common
)
target_include_directories(cso_controls PUBLIC
		../
		../interfaces/
		${HLSDK_PATH}/common
		${HLSDK_PATH}/engine
		${HLSDK_PATH}/pm_shared
)
target_link_libraries(cso_controls PUBLIC vgui_controls)