
add_library(video_config INTERFACE)

if(XASH_SDL)
    target_compile_definitions(video_config INTERFACE -DUSE_SDL -DXASH_SDL)
endif()

if(XASH_IMGUI)
    target_compile_definitions(video_config INTERFACE -DXASH_IMGUI)
endif()