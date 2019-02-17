#hlsdk-2.3 client port for android
#Copyright (c) mittorn

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := client
APP_PLATFORM := android-8
LOCAL_CONLYFLAGS += -std=c99

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
LOCAL_MODULE_FILENAME = libclient_hardfp
endif

LOCAL_CFLAGS += -fsigned-char -DCLIENT_DLL=1 -DCLIENT_WEAPONS=1
LOCAL_CPPFLAGS += -std=c++11 -fexceptions

SRCS := \
	./cdll_int.cpp \
	./demo.cpp \
	./entity.cpp \
	./in_camera.cpp \
	./input.cpp \
	./rain.cpp \
	./tri.cpp \
	./util.cpp \
	./view.cpp \
	./input_xash3d.cpp \
	./vgui_parser.cpp \
	../public/unicode_strtools.cpp \
	./draw_util.cpp \
	../pm_shared/pm_debug.c \
	../pm_shared/pm_math.c \
	../pm_shared/pm_shared.c \
	./studio/GameStudioModelRenderer.cpp \
	./studio/StudioModelRenderer.cpp \
	./studio/studio_util.cpp \
	./hud/ammo.cpp \
	./hud/ammo_secondary.cpp \
	./hud/ammohistory.cpp \
	./hud/battery.cpp \
	./hud/geiger.cpp \
	./hud/health.cpp \
	./hud/hud.cpp \
	./hud/hud_msg.cpp \
	./hud/hud_redraw.cpp \
	./hud/hud_spectator.cpp \
	./hud/hud_update.cpp \
	./hud/sniperscope.cpp \
	./hud/nvg.cpp \
	./hud/menu.cpp \
	./hud/message.cpp \
	./hud/status_icons.cpp \
	./hud/statusbar.cpp \
	./hud/text_message.cpp \
	./hud/train.cpp \
	./hud/scoreboard.cpp \
	./hud/MOTD.cpp \
	./hud/radio.cpp \
	./hud/radar.cpp \
	./hud/timer.cpp \
	./hud/money.cpp \
	./hud/flashlight.cpp \
	./hud/death.cpp \
	./hud/saytext.cpp \
	./hud/spectator_gui.cpp \
	./hud/followicon.cpp \
	./hud/headname.cpp \
	./hud/zbs/zbs.cpp \
	./hud/zbs/zbs_level.cpp \
	./hud/zbs/zbs_scoreboard.cpp \
	./hud/zbs/zbs_kill.cpp \
	./hud/zbs/zbs_roundclear.cpp \
	./hud/retina.cpp \
	./hud/scenariostatus.cpp \
	./hud/zb2/zb2.cpp \
	./hud/zb2/zb2_skill.cpp \
	./hud/legacy/hud_radar_legacy.cpp \
	./hud/legacy/hud_scoreboard_legacy.cpp \
	./hud/modern/hud_radar_modern.cpp \
	./com_weapons.cpp \
	./cs_wpn/cs_baseentity.cpp \
	./cs_wpn/cs_weapons.cpp \
	./cs_wpn/bte_weapons.cpp \
	../dlls/wpn_shared/wpn_ak47.cpp \
	../dlls/wpn_shared/wpn_aug.cpp \
	../dlls/wpn_shared/wpn_awp.cpp \
	../dlls/wpn_shared/wpn_c4.cpp \
	../dlls/wpn_shared/wpn_deagle.cpp \
	../dlls/wpn_shared/wpn_elite.cpp \
	../dlls/wpn_shared/wpn_famas.cpp \
	../dlls/wpn_shared/wpn_fiveseven.cpp \
	../dlls/wpn_shared/wpn_flashbang.cpp \
	../dlls/wpn_shared/wpn_g3sg1.cpp \
	../dlls/wpn_shared/wpn_galil.cpp \
	../dlls/wpn_shared/wpn_glock18.cpp \
	../dlls/wpn_shared/wpn_hegrenade.cpp \
	../dlls/wpn_shared/wpn_knife.cpp \
	../dlls/wpn_shared/wpn_m249.cpp \
	../dlls/wpn_shared/wpn_m3.cpp \
	../dlls/wpn_shared/wpn_m4a1.cpp \
	../dlls/wpn_shared/wpn_mac10.cpp \
	../dlls/wpn_shared/wpn_mp5navy.cpp \
	../dlls/wpn_shared/wpn_p228.cpp \
	../dlls/wpn_shared/wpn_p90.cpp \
	../dlls/wpn_shared/wpn_scout.cpp \
	../dlls/wpn_shared/wpn_sg550.cpp \
	../dlls/wpn_shared/wpn_sg552.cpp \
	../dlls/wpn_shared/wpn_smokegrenade.cpp \
	../dlls/wpn_shared/wpn_tmp.cpp \
	../dlls/wpn_shared/wpn_ump45.cpp \
	../dlls/wpn_shared/wpn_usp.cpp \
	../dlls/wpn_shared/wpn_xm1014.cpp \
	../dlls/wpn_shared/wpn_ak47l.cpp \
	../dlls/wpn_shared/wpn_mp7a1d.cpp \
	../dlls/wpn_shared/wpn_deagled.cpp \
../dlls/wpn_shared/wpn_as50.cpp \
../dlls/wpn_shared/wpn_cannon.cpp \
../dlls/wpn_shared/wpn_infinity.cpp \
../dlls/wpn_shared/wpn_k1a.cpp \
../dlls/wpn_shared/wpn_knife_zombi.cpp \
../dlls/wpn_shared/wpn_kriss.cpp \
../dlls/wpn_shared/wpn_m95.cpp \
../dlls/wpn_shared/wpn_m1887.cpp \
../dlls/wpn_shared/wpn_scarh.cpp \
../dlls/wpn_shared/wpn_scarl.cpp \
../dlls/wpn_shared/wpn_tar21.cpp \
../dlls/wpn_shared/wpn_thompson.cpp \
../dlls/wpn_shared/wpn_wa2000.cpp \
../dlls/wpn_shared/wpn_xm8c.cpp \
../dlls/wpn_shared/wpn_xm8s.cpp \
../dlls/wpn_shared/knife_skullaxe.cpp \
../dlls/wpn_shared/wpn_gungnir.cpp \
	./events/ev_cs16.cpp \
	./events/event_ak47.cpp \
	./events/event_aug.cpp \
	./events/event_awp.cpp \
	./events/event_createexplo.cpp \
	./events/event_createsmoke.cpp \
	./events/event_deagle.cpp \
	./events/event_decal_reset.cpp \
	./events/event_elite_left.cpp \
	./events/event_elite_right.cpp \
	./events/event_famas.cpp \
	./events/event_fiveseven.cpp \
	./events/event_g3sg1.cpp \
	./events/event_galil.cpp \
	./events/event_glock18.cpp \
	./events/event_knife.cpp \
	./events/event_m249.cpp \
	./events/event_m3.cpp \
	./events/event_m4a1.cpp \
	./events/event_mac10.cpp \
	./events/event_mp5n.cpp \
	./events/event_p228.cpp \
	./events/event_p90.cpp \
	./events/event_scout.cpp \
	./events/event_sg550.cpp \
	./events/event_sg552.cpp \
	./events/event_tmp.cpp \
	./events/event_ump45.cpp \
	./events/event_usp.cpp \
	./events/event_vehicle.cpp \
	./events/event_xm1014.cpp \
	./events/hl_events.cpp \
	./events/event_ak47l.cpp \
	./events/event_mp7a1d.cpp \
	./events/event_deagled.cpp \
./events/event_as50.cpp \
./events/event_cannon.cpp \
./events/event_infinity.cpp \
./events/event_k1a.cpp \
./events/event_kriss.cpp \
./events/event_m95.cpp \
./events/event_m1887.cpp \
./events/event_scarh.cpp \
./events/event_scarl.cpp \
./events/event_tar21.cpp \
./events/event_thompson.cpp \
./events/event_wa2000.cpp \
./events/event_xm8c.cpp \
./events/event_xm8s.cpp \
	./ev_common.cpp \
	./calcscreen.cpp 
	


DEFINES = -Wno-write-strings -DLINUX -D_LINUX -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -DCLIENT_WEAPONS -DCLIENT_DLL -Wl,--no-undefined

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/hud \
	$(LOCAL_PATH)/studio \
	$(LOCAL_PATH)/../common \
	$(LOCAL_PATH)/../engine \
	$(LOCAL_PATH)/../game_shared \
	$(LOCAL_PATH)/../dlls \
	$(LOCAL_PATH)/../pm_shared \
	$(LOCAL_PATH)/../public \
	$(LOCAL_PATH)/../public/tier1
LOCAL_CFLAGS += $(DEFINES)

LOCAL_SRC_FILES := $(SRCS)

include $(BUILD_SHARED_LIBRARY)
