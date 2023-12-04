
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
	MoeLogin.cpp
	MoEBulletIn.cpp

	InventoryMenu_V2/InventoryMainMenu.cpp
	
	InventoryMenu_V2/Support/STLHelper.cpp
	InventoryMenu_V2/Util/Util.cpp

	InventoryMenu_V2/Shared/CSearchBar/CSearchBar.cpp
	InventoryMenu_V2/Shared/CTexturedButton/CTexturedButton.cpp
	InventoryMenu_V2/Shared/CTexturedMixButton/CTexturedMixButton.cpp

	InventoryMenu_V2/WeaponEnchant/CWeaponEnchant.cpp
	InventoryMenu_V2/WeaponPaint/CWeaponPaint.cpp

	InventoryMenu_V2/CharacterManager/CCharacterManager.cpp

	InventoryMenu_V2/WeaponEnchant/Class/CWeaponListLeftPanel.cpp
	InventoryMenu_V2/WeaponEnchant/Class/CWeaponListRightPanel.cpp
	InventoryMenu_V2/WeaponEnchant/Class/CWeaponListRightPanelEle.cpp
	InventoryMenu_V2/WeaponEnchant/Class/CWeaponLevelGauge.cpp
	InventoryMenu_V2/WeaponEnchant/Class/CWeaponDescription.cpp
 )

if(SE_WIN32)
	target_sources(GameUI PRIVATE
			LogoFile.cpp
			)
endif()

target_link_libraries(GameUI PUBLIC vgui_controls)
target_link_libraries(GameUI PUBLIC Boost::boost json_3p)
target_link_libraries(GameUI PRIVATE cqmiao)

target_include_directories(GameUI PUBLIC
		${HLSDK_PATH}/public
		)