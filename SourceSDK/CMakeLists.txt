

add_library(vinterface STATIC
        ${HL2SDK_PATH}/tier1/interface.cpp
)
target_include_directories(vinterface PUBLIC
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/tier0
        ${HL2SDK_PATH}/public/tier1
)
target_link_libraries(vinterface PUBLIC platform_config)

add_library(tier0 STATIC
        ${HL2SDK_PATH}/tier0/assert_dialog.cpp
        ${HL2SDK_PATH}/tier0/commandline.cpp
        ${HL2SDK_PATH}/tier0/cpu.cpp
        ${HL2SDK_PATH}/tier0/cpumonitoring.cpp
        ${HL2SDK_PATH}/tier0/cpu_usage.cpp
        ${HL2SDK_PATH}/tier0/dbg.cpp
        ${HL2SDK_PATH}/tier0/dynfunction.cpp
        ${HL2SDK_PATH}/tier0/fasttimer.cpp
        ${HL2SDK_PATH}/tier0/mem.cpp
        ${HL2SDK_PATH}/tier0/mem_helpers.cpp
        ${HL2SDK_PATH}/tier0/memdbg.cpp
        ${HL2SDK_PATH}/tier0/memstd.cpp
        ${HL2SDK_PATH}/tier0/memvalidate.cpp
        ${HL2SDK_PATH}/tier0/minidump.cpp
        ${HL2SDK_PATH}/tier0/pch_tier0.cpp
        #${HL2SDK_PATH}/tier0/PMELib.cpp
        ${HL2SDK_PATH}/tier0/progressbar.cpp
        ${HL2SDK_PATH}/tier0/security.cpp
        ${HL2SDK_PATH}/tier0/systeminformation.cpp
        ${HL2SDK_PATH}/tier0/stacktools.cpp
        ${HL2SDK_PATH}/tier0/thread.cpp
        ${HL2SDK_PATH}/tier0/threadtools.cpp
        ${HL2SDK_PATH}/tier0/tier0_strtools.cpp
        ${HL2SDK_PATH}/tier0/tslist.cpp
        ${HL2SDK_PATH}/tier0/vprof.cpp
)
if( WIN32 )
    target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/platform.cpp)
    target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/vcrmode.cpp)
    target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/pme_posix.cpp)
    #target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/etwprof.cpp)
elseif( POSIX )
    target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/cpu_posix.cpp)
    target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/platform_posix.cpp)
    target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/pme_posix.cpp)
    target_sources(tier0 PUBLIC ${HL2SDK_PATH}/tier0/vcrmode_posix.cpp)
endif()

target_include_directories(tier0 PUBLIC
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/tier0
        ${HL2SDK_PATH}
        )
target_link_libraries(tier0 PUBLIC platform_config video_config)
if( WIN32 )
    target_link_libraries(tier0 PUBLIC ws2_32)
endif()
target_link_libraries(tier0 PUBLIC 3rdparty-sdl2)

add_library(tier1 STATIC
        ${HL2SDK_PATH}/tier1/bitbuf.cpp
        ${HL2SDK_PATH}/tier1/byteswap.cpp
        ${HL2SDK_PATH}/tier1/characterset.cpp
        ${HL2SDK_PATH}/tier1/checksum_crc.cpp
        ${HL2SDK_PATH}/tier1/checksum_md5.cpp
        ${HL2SDK_PATH}/tier1/convar.cpp
        ${HL2SDK_PATH}/tier1/datamanager.cpp
        ${HL2SDK_PATH}/tier1/diff.cpp
        ${HL2SDK_PATH}/tier1/generichash.cpp
        ${HL2SDK_PATH}/tier1/KeyValues.cpp
        ${HL2SDK_PATH}/tier1/mempool.cpp
        ${HL2SDK_PATH}/tier1/memstack.cpp
        ${HL2SDK_PATH}/tier1/NetAdr.cpp
        ${HL2SDK_PATH}/tier1/processor_detect.cpp
        ${HL2SDK_PATH}/tier1/rangecheckedvar.cpp
        ${HL2SDK_PATH}/tier1/stringpool.cpp
        ${HL2SDK_PATH}/tier1/strtools.cpp
        ${HL2SDK_PATH}/tier1/strtools_unicode.cpp
        ${HL2SDK_PATH}/tier1/tier1.cpp
        ${HL2SDK_PATH}/tier1/utlbuffer.cpp
        ${HL2SDK_PATH}/tier1/utlstring.cpp
        ${HL2SDK_PATH}/tier1/utlsymbol.cpp
        ${HL2SDK_PATH}/tier1/ilocalize.cpp
        )
if(LINUX)
    target_sources(tier1 PRIVATE
            ${HL2SDK_PATH}/tier1/qsort_s.cpp
            )
endif()
target_include_directories(tier1 PUBLIC
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/tier1
        )
target_link_libraries(tier1 PUBLIC platform_config)
if(ANDROID)
    target_link_libraries(tier1 PRIVATE android) # iconv
endif()

add_library(mathlib STATIC
        ${HL2SDK_PATH}/mathlib/color_conversion.cpp
        ${HL2SDK_PATH}/mathlib/halton.cpp
        ${HL2SDK_PATH}/mathlib/lightdesc.cpp
        ${HL2SDK_PATH}/mathlib/mathlib_base.cpp
        ${HL2SDK_PATH}/mathlib/powsse.cpp
        ${HL2SDK_PATH}/mathlib/sparse_convolution_noise.cpp
        ${HL2SDK_PATH}/mathlib/sseconst.cpp
        ${HL2SDK_PATH}/mathlib/sse.cpp
        ${HL2SDK_PATH}/mathlib/ssenoise.cpp
        ${HL2SDK_PATH}/mathlib/anorms.cpp
        ${HL2SDK_PATH}/mathlib/bumpvects.cpp
        ${HL2SDK_PATH}/mathlib/IceKey.cpp
        ${HL2SDK_PATH}/mathlib/imagequant.cpp
        ${HL2SDK_PATH}/mathlib/polyhedron.cpp
        ${HL2SDK_PATH}/mathlib/quantize.cpp
        ${HL2SDK_PATH}/mathlib/randsse.cpp
        ${HL2SDK_PATH}/mathlib/spherical.cpp
        ${HL2SDK_PATH}/mathlib/simdvectormatrix.cpp
        ${HL2SDK_PATH}/mathlib/vector.cpp
        ${HL2SDK_PATH}/mathlib/vmatrix.cpp
        ${HL2SDK_PATH}/mathlib/almostequal.cpp
        )
if(NOT MSVC)
    target_compile_options(mathlib PRIVATE -Wno-c++11-narrowing)
endif()
target_include_directories(mathlib PUBLIC
    ${HL2SDK_PATH}/public
    ${HL2SDK_PATH}/common
    ${HL2SDK_PATH}/public/tier0
    ${HL2SDK_PATH}/public/mathlib
    ${HLSDK_PATH}/public
)
target_link_libraries(mathlib PUBLIC platform_config)

target_link_libraries(tier1 PUBLIC vinterface tier0 mathlib)

add_library(vstdlib STATIC
        ${HL2SDK_PATH}/vstdlib/coroutine.cpp
        ${HL2SDK_PATH}/vstdlib/cvar.cpp
        ${HL2SDK_PATH}/vstdlib/jobthread.cpp
        ${HL2SDK_PATH}/vstdlib/KeyValuesSystem.cpp
        ${HL2SDK_PATH}/vstdlib/osversion.cpp
        ${HL2SDK_PATH}/vstdlib/random.cpp
        ${HL2SDK_PATH}/vstdlib/vcover.cpp
#        ${HL2SDK_PATH}/public/tier0/memoverride.cpp
        )
if(WIN32)
    target_sources(vstdlib PRIVATE
            ${HL2SDK_PATH}/vstdlib/processutils.cpp
            )
endif()
target_link_libraries(vstdlib PRIVATE tier0 tier1)
target_compile_definitions(vstdlib PRIVATE -DVSTDLIB_DLL_EXPORT=1)
if(OSX)
    # fix for Gestalt
    target_link_libraries(vstdlib PUBLIC "-framework CoreServices")
endif()

add_library(SourceSDK INTERFACE)
target_include_directories(SourceSDK INTERFACE ./ ./public)