
add_library(binkw32 STATIC
    binkacd.cpp
    binkasyncRR.c
    binkasyncthread.c
    binkcomm.c
    binkfile.c
    binkmarkers.c
    binkplugin.c
    binkread.c
    binkutil.c
    expand2.c
    popmal.c
    radfft.cpp
)

if(IOS)
    target_sources(binkw32 PRIVATE
            binkca.c
            binkpluginios.c
            binkssmix.c
            cpp_rratomics.cpp
            posix_defread.c
            posix_radmem.c
            posix_rrCpu.c
            posix_rrThreads2.c
            )
elseif(ANDROID)
    target_sources(binkw32 PRIVATE
            binkand.cpp
            binkpluginlnx.c
            binkssmix.c
            cpp_rratomics.cpp
            posix_radmem.c
            posix_rrCpu.c
            posix_rrThreads2.c
            )
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_sources(binkw32 PRIVATE
            iphonerad.c
            mac_radmem.c
            binkca.c
            binkssmix.c
            cpp_rratomics.cpp
            posix_defread.c
            posix_rrCpu.c
            posix_rrThreads2.c
            )
    target_compile_definitions(binkw32 PRIVATE
            -DTARGET_API_MAC_CARBON
            -DOPAQUE_UPP_TYPES
            -DOPAQUE_TOOLBOX_STRUCTS
    )
elseif(WIN32)
    target_sources(binkw32 PRIVATE
            win32_radmem.c
            win32_rrAtomics.cpp
            win32_rrCpu.c
            win32_rrThreads2.c
            wintimer.c
            win64_rrAtomics_x64.asm
            binkpluginw32.c
            binkxa2.cpp
            )
    target_compile_definitions(binkw32 PRIVATE
            -DBINK2W64DLL
            )
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_sources(binkw32 PRIVATE
            binklnx.c
            binkpluginlnx.c
            binkssmix.c
            cpp_rratomics.cpp
            binkpa.c
            posix_defread.c
            posix_radmem.c
            posix_rrCpu.c
            posix_rrThreads2.c
            )
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(i[0-9]86|x86|X86|amd64|AMD64|x86_64)")
    target_sources(binkw32 PRIVATE
            x86_cpu.c
            )
endif()

target_include_directories(binkw32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(binkw32 PRIVATE
    -D__RADFINAL__
    -DUSING_EGT
    -DWRAP_PUBLICS=Bink
    -DBINKTEXTURESINDIRECTBINKCALLS
    -DBINKTEXTURESGPUAPITYPE
    -DBUILDING_FOR_UNREAL_ONLY
    -DINC_BINK2
    -DSUPPORT_BINKGPU
    -DNO_BINK20
    -DNO_BINK10_SUPPORT
    -DNO_BINK_BLITTERS
    -D__RADINSTATICLIB__
)