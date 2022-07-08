
add_library(GameUI STATIC
	BackgroundMenuButton.cpp
	BasePanel.cpp
	BitmapImagePanel.cpp
	CreateMultiplayerGameBotPage.cpp
	CreateMultiplayerGameDialog.cpp
	CreateMultiplayerGameGameplayPage.cpp
	CreateMultiplayerGameServerPage.cpp
	CvarNegateCheckButton.cpp
	CvarSlider.cpp
	CvarTextEntry.cpp
	CvarToggleCheckButton.cpp
	GameConsole.cpp
	GameConsoleDialog.cpp
	GameLoading.cpp
	GameUI_Interface.cpp
	KeyToggleCheckButton.cpp
	LabeledCommandComboBox.cpp
	LoadingDialog.cpp
	ModInfo.cpp
	MultiplayerAdvancedDialog.cpp
	OptionsDialog.cpp
	OptionsSubAdvanced.cpp
	OptionsSubAudio.cpp
	OptionsSubKeyboard.cpp
	OptionsSubMouse.cpp
	OptionsSubMultiplayer.cpp
	OptionsSubVideo.cpp
	OptionsSubVoice.cpp
	PanelListPanel.cpp
	ScriptObject.cpp
	ToolBar.cpp
	URLButton.cpp
	VControlsListPanel.cpp
	OptionsSubTouch.cpp
	MoeSettings.cpp
	OptionsSubButtonSettings.cpp
	OptionsSubMoeSettings.cpp
)

if(SE_WIN32)
	target_sources(GameUI PRIVATE
			LogoFile.cpp
			)
endif()

target_link_libraries(GameUI PUBLIC vgui_controls)
target_include_directories(GameUI PUBLIC
		${HLSDK_PATH}/public
		)