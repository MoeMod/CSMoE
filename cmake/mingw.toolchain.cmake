
set(MINGW_PREFIX "/usr/local/Cellar/mingw-w64/8.0.0_3/toolchain-x86_64/mingw" CACHE PATH "MinGW path")

# Target system name
set(CMAKE_SYSTEM_NAME   Windows)

# Compilers and utilities
set(CMAKE_C_COMPILER    x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER  x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER   x86_64-w64-mingw32-windres)

# Croscompiler path
set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} ${MINGW_PREFIX})

# This helps find_path(), otherwise it's not able to find e.g.
# /usr/x86_64-w64-mingw32/share/cmake/Corrade/. I don't know why it's needed when
# there is already CMAKE_FIND_ROOT_PATH, but probably related to
# http://public.kitware.com/Bug/view.php?id=14337
set(CMAKE_PREFIX_PATH ${MINGW_PREFIX})

# Find executables in root path, libraries and includes are in crosscompiler
# path
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Directories where to search for DLLs
set(DLL_SEARCH_PATH
        ${MINGW_PREFIX}/lib
        ${MINGW_PREFIX}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")