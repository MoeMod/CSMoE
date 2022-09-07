LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)	

LOCAL_MODULE := gl-wes
LOCAL_CFLAGS    := -DBUILD_ANDROID -frtti -fexceptions 
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \

LOCAL_SRC_FILES :=  \
		src/wes.c \
		src/wes_begin.c \
		src/wes_fragment.c \
		src/wes_matrix.c \
		src/wes_shader.c \
		src/wes_state.c \
		src/wes_texture.c
					
LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
