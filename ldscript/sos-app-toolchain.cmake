include(${CMAKE_CURRENT_LIST_DIR}/sos-toolchain.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sos-build-flags.cmake)

set(CMAKE_C_FLAGS "${SOS_BUILD_C_FLAGS} -mlong-calls" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS "${SOS_BUILD_C_FLAGS} ${SOS_BUILD_CXX_FLAGS}" CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS "${SOS_BUILD_ASM_FLAGS}" CACHE INTERNAL "asm compiler flags release")
