LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := server

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
LOCAL_MODULE_FILENAME = libserver_hardfp
endif

LOCAL_CFLAGS += -w -D_LINUX -DCLIENT_WEAPONS -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_stricmp=strcasecmp -D_vsnprintf=vsnprintf -D_strdup=strdup -D_write=write -D_close=close \
	-fno-exceptions
	
LOCAL_C_INCLUDES := $(LOCAL_PATH)/. \
		    $(LOCAL_PATH)/.. \
                    $(LOCAL_PATH)/hostage \
                    $(LOCAL_PATH)/bot \
		    $(LOCAL_PATH)/../common \
		    $(LOCAL_PATH)/../engine/common \
		    $(LOCAL_PATH)/../engine \
		    $(LOCAL_PATH)/../public \
                    $(LOCAL_PATH)/../public/tier1 \
		    $(LOCAL_PATH)/../pm_shared \
		    $(LOCAL_PATH)/../game_shared \
                    $(LOCAL_PATH)/../game_shared/bot \

LOCAL_SRC_FILES := \
	airtank.cpp \
	ammo.cpp \
	animating.cpp \
	animation.cpp \
	bmodels.cpp \
	bot/cs_bot_chatter.cpp \
	bot/cs_bot.cpp \
	bot/cs_bot_event.cpp \
	bot/cs_bot_init.cpp \
	bot/cs_bot_learn.cpp \
	bot/cs_bot_listen.cpp \
	bot/cs_bot_manager.cpp \
	bot/cs_bot_nav.cpp \
	bot/cs_bot_pathfind.cpp \
	bot/cs_bot_radio.cpp \
	bot/cs_bot_statemachine.cpp \
	bot/cs_bot_update.cpp \
	bot/cs_bot_vision.cpp \
	bot/cs_bot_weapon.cpp \
	bot/cs_gamestate.cpp \
	bot/states/cs_bot_attack.cpp \
	bot/states/cs_bot_buy.cpp \
	bot/states/cs_bot_defuse_bomb.cpp \
	bot/states/cs_bot_escape_from_bomb.cpp \
	bot/states/cs_bot_fetch_bomb.cpp \
	bot/states/cs_bot_follow.cpp \
	bot/states/cs_bot_hide.cpp \
	bot/states/cs_bot_hunt.cpp \
	bot/states/cs_bot_idle.cpp \
	bot/states/cs_bot_investigate_noise.cpp \
	bot/states/cs_bot_move_to.cpp \
	bot/states/cs_bot_plant_bomb.cpp \
	bot/states/cs_bot_use_entity.cpp \
	buttons.cpp \
	career_tasks.cpp \
	cbase.cpp \
	client.cpp \
	combat.cpp \
	debug.cpp \
	doors.cpp \
	effects.cpp \
	explode.cpp \
	func_break.cpp \
	func_tank.cpp \
	game.cpp \
	gamerules.cpp \
	ggrenade.cpp \
	globals.cpp \
	h_ai.cpp \
	h_battery.cpp \
	h_cycler.cpp \
	healthkit.cpp \
	h_export.cpp \
	player/csdm_randomspawn.cpp \
	player/hintmessage.cpp \
	player/player_model.cpp \
	player/player_msg.cpp \
	player/player_spawnpoint.cpp \
	player/player_zombie.cpp \
	player/player_spawnprotection.cpp \
	hostage/hostage.cpp \
	hostage/hostage_improv.cpp \
	hostage/hostage_localnav.cpp \
	hostage/states/hostage_animate.cpp \
	hostage/states/hostage_escape.cpp \
	hostage/states/hostage_follow.cpp \
	hostage/states/hostage_idle.cpp \
	hostage/states/hostage_retreat.cpp \
	items.cpp \
	lights.cpp \
	maprules.cpp \
	mortar.cpp \
	mpstubb.cpp \
	multiplay_gamerules.cpp \
	observer.cpp \
	pathcorner.cpp \
	plane.cpp \
	plats.cpp \
	player.cpp \
	singleplay_gamerules.cpp \
	skill.cpp \
	sound.cpp \
	soundent.cpp \
	spectator.cpp \
	subs.cpp \
	training_gamerules.cpp \
	triggers.cpp \
	tutor_base_states.cpp \
	tutor_base_tutor.cpp \
	tutor.cpp \
	tutor_cs_states.cpp \
	tutor_cs_tutor.cpp \
	util.cpp \
	vehicle.cpp \
	weapons.cpp \
	weapons_buy.cpp \
	world.cpp \
	wpn_shared/wpn_ak47.cpp \
	wpn_shared/wpn_aug.cpp \
	wpn_shared/wpn_awp.cpp \
	wpn_shared/wpn_c4.cpp \
	wpn_shared/wpn_deagle.cpp \
	wpn_shared/wpn_elite.cpp \
	wpn_shared/wpn_famas.cpp \
	wpn_shared/wpn_fiveseven.cpp \
	wpn_shared/wpn_flashbang.cpp \
	wpn_shared/wpn_g3sg1.cpp \
	wpn_shared/wpn_galil.cpp \
	wpn_shared/wpn_glock18.cpp \
	wpn_shared/wpn_hegrenade.cpp \
	wpn_shared/wpn_knife.cpp \
	wpn_shared/wpn_m249.cpp \
	wpn_shared/wpn_m3.cpp \
	wpn_shared/wpn_m4a1.cpp \
	wpn_shared/wpn_mac10.cpp \
	wpn_shared/wpn_mp5navy.cpp \
	wpn_shared/wpn_p228.cpp \
	wpn_shared/wpn_p90.cpp \
	wpn_shared/wpn_scout.cpp \
	wpn_shared/wpn_sg550.cpp \
	wpn_shared/wpn_sg552.cpp \
	wpn_shared/wpn_smokegrenade.cpp \
	wpn_shared/wpn_tmp.cpp \
	wpn_shared/wpn_ump45.cpp \
	wpn_shared/wpn_usp.cpp \
	wpn_shared/wpn_xm1014.cpp \
	wpn_shared/wpn_knife_zombi.cpp \
	gamemode/mods.cpp \
	gamemode/mod_base.cpp \
	gamemode/mod_none.cpp \
	gamemode/mod_tdm.cpp \
	gamemode/mod_dm.cpp \
	gamemode/mod_zb1.cpp \
	../public/unicode_strtools.cpp \
	../game_shared/bot/bot.cpp \
	../game_shared/bot/bot_manager.cpp \
	../game_shared/bot/bot_profile.cpp \
	../game_shared/bot/bot_util.cpp \
	../game_shared/bot/nav_area.cpp \
	../game_shared/bot/nav_file.cpp \
	../game_shared/bot/nav_node.cpp \
	../game_shared/bot/nav_path.cpp \
	../game_shared/shared_util.cpp \
	../game_shared/voice_gamemgr.cpp \
	../pm_shared/pm_debug.c \
	../pm_shared/pm_math.c \
	../pm_shared/pm_shared.c \
	../public/MemPool.cpp \


LOCAL_CPPFLAGS += -frtti -std=c++11

include $(BUILD_SHARED_LIBRARY)
