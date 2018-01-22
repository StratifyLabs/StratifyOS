
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)



set(TOOLCHAIN_HOST arm-none-eabi)
set(TOOLCHAIN_DIR C:/Qt/Tools/mingw)

include(${CMAKE_CURRENT_LIST_DIR}/sos-gcc-toolchain.cmake)

set(TOOLCHAIN_C_FLAGS "" CACHE INTERNAL "common c compiler flags")
set(TOOLCHAIN_CXX_FLAGS "" CACHE INTERNAL "common c compiler flags")

set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE INTERNAL "verbose make")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS "${TOOLCHAIN_C_FLAGS}" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS "${TOOLCHAIN_C_FLAGS} ${TOOLCHAIN_CXX_FLAGS}" CACHE INTERNAL "cxx compiler flags release")
