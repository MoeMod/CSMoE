set(FONTCONFIG_SRC_DIR "${CMAKE_SOURCE_DIR}/3rdparty/fontconfig")

add_custom_command(OUTPUT fcalias.h fcaliastail.h
        COMMAND sh ${FONTCONFIG_SRC_DIR}/src/makealias ${FONTCONFIG_SRC_DIR}/src fcalias.h fcaliastail.h ${FONTCONFIG_SRC_DIR}/fontconfig/fontconfig.h ${FONTCONFIG_SRC_DIR}/src/fcdeprecate.h ${FONTCONFIG_SRC_DIR}/fontconfig/fcprivate.h
        DEPENDS ${FONTCONFIG_SRC_DIR}/src/fcdeprecate.h ${FONTCONFIG_SRC_DIR}/fontconfig/fontconfig.h  ${FONTCONFIG_SRC_DIR}/fontconfig/fcprivate.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT fcftalias.h fcftaliastail.h
        COMMAND sh ${FONTCONFIG_SRC_DIR}/src/makealias ${FONTCONFIG_SRC_DIR}/src fcftalias.h fcftaliastail.h ${FONTCONFIG_SRC_DIR}/fontconfig/fcfreetype.h
        DEPENDS ${FONTCONFIG_SRC_DIR}/fontconfig/fcfreetype.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT fcstdint.h
        COMMAND ${CMAKE_COMMAND} -E copy ${FONTCONFIG_SRC_DIR}/src/fcstdint.h.in fcstdint.h
        DEPENDS ${FONTCONFIG_SRC_DIR}/src/fcstdint.h.in
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB ORTH_FILES ${FONTCONFIG_SRC_DIR}/fc-lang/*.orth)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../fc-lang/fclang.h
        COMMAND python3 ${FONTCONFIG_SRC_DIR}/fc-lang/fc-lang.py --template ${FONTCONFIG_SRC_DIR}/fc-lang/fclang.tmpl.h --output ${CMAKE_CURRENT_BINARY_DIR}/../fc-lang/fclang.h --directory ${FONTCONFIG_SRC_DIR}/fc-lang ${ORTH_FILES}
        DEPENDS ${FONTCONFIG_SRC_DIR}/fc-lang/fc-lang.py ${FONTCONFIG_SRC_DIR}/fc-lang/fclang.tmpl.h ${ORTH_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../fc-case/fccase.h
        COMMAND python3 ${FONTCONFIG_SRC_DIR}/fc-case/fc-case.py ${FONTCONFIG_SRC_DIR}/fc-case/CaseFolding.txt --template ${FONTCONFIG_SRC_DIR}/fc-case/fccase.tmpl.h --output ${CMAKE_CURRENT_BINARY_DIR}/../fc-case/fccase.h
        DEPENDS ${FONTCONFIG_SRC_DIR}/fc-case/fc-case.py ${FONTCONFIG_SRC_DIR}/fc-case/CaseFolding.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT fcobjshash.h
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/fcobjshash.h fcobjshash.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fcobjshash.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_library( 3rdparty-fontconfig STATIC
        ${FONTCONFIG_SRC_DIR}/src/fcarch.c
        ${FONTCONFIG_SRC_DIR}/src/fcatomic.c
        # ${FONTCONFIG_SRC_DIR}/src/fcblanks.c
        ${FONTCONFIG_SRC_DIR}/src/fccache.c
        ${FONTCONFIG_SRC_DIR}/src/fccfg.c
        ${FONTCONFIG_SRC_DIR}/src/fccharset.c
        ${FONTCONFIG_SRC_DIR}/src/fccompat.c
        ${FONTCONFIG_SRC_DIR}/src/fcdbg.c
        ${FONTCONFIG_SRC_DIR}/src/fcdefault.c
        ${FONTCONFIG_SRC_DIR}/src/fcdir.c
        ${FONTCONFIG_SRC_DIR}/src/fcformat.c
        ${FONTCONFIG_SRC_DIR}/src/fcfreetype.c
        ${FONTCONFIG_SRC_DIR}/src/fcfs.c
        ${FONTCONFIG_SRC_DIR}/src/fcinit.c
        ${FONTCONFIG_SRC_DIR}/src/fclang.c
        ${FONTCONFIG_SRC_DIR}/src/fclist.c
        ${FONTCONFIG_SRC_DIR}/src/fcmatch.c
        ${FONTCONFIG_SRC_DIR}/src/fcmatrix.c
        ${FONTCONFIG_SRC_DIR}/src/fcname.c
        ${FONTCONFIG_SRC_DIR}/src/fcobjs.c
        ${FONTCONFIG_SRC_DIR}/src/fcpat.c
        ${FONTCONFIG_SRC_DIR}/src/fcrange.c
        ${FONTCONFIG_SRC_DIR}/src/fcserialize.c
        ${FONTCONFIG_SRC_DIR}/src/fcstat.c
        ${FONTCONFIG_SRC_DIR}/src/fcstr.c
        ${FONTCONFIG_SRC_DIR}/src/fcweight.c
        ${FONTCONFIG_SRC_DIR}/src/fcxml.c
        ${FONTCONFIG_SRC_DIR}/src/ftglue.c
        ${FONTCONFIG_SRC_DIR}/src/fcptrlist.c
        ${FONTCONFIG_SRC_DIR}/src/fchash.c

        ${CMAKE_CURRENT_BINARY_DIR}/fcstdint.h
        ${CMAKE_CURRENT_BINARY_DIR}/fcalias.h
        ${CMAKE_CURRENT_BINARY_DIR}/fcaliastail.h
        ${CMAKE_CURRENT_BINARY_DIR}/fcftalias.h
        ${CMAKE_CURRENT_BINARY_DIR}/fcftaliastail.h
        ${CMAKE_CURRENT_BINARY_DIR}/../fc-case/fccase.h
        ${CMAKE_CURRENT_BINARY_DIR}/../fc-lang/fclang.h
        ${CMAKE_CURRENT_BINARY_DIR}/fcobjshash.h
)
target_compile_definitions(3rdparty-fontconfig PRIVATE -DCONFIGDIR="")
target_link_libraries(3rdparty-fontconfig PRIVATE 3rdparty-freetype 3rdparty-expat)
target_include_directories( 3rdparty-fontconfig PRIVATE
        ${FONTCONFIG_SRC_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/
)
target_include_directories( 3rdparty-fontconfig PUBLIC
        ${FONTCONFIG_SRC_DIR}/
)
include(ConfigureChecks.cmake)