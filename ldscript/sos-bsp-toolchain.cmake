include(${CMAKE_CURRENT_LIST_DIR}/sos-toolchain.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sos-build-flags.cmake)

set(CMAKE_C_FLAGS_DEBUG "${SOS_BUILD_C_FLAGS} -D___debug" CACHE INTERNAL "bsp debug c compiler flags release")
set(CMAKE_CXX_FLAGS_DEBUG "${SOS_BUILD_C_FLAGS} ${SOS_BUILD_CXX_FLAGS}" CACHE INTERNAL "bsp debug cxx compiler flags release")
set(CMAKE_ASM_FLAGS_DEBUG "${SOS_BUILD_ASM_FLAGS}" CACHE INTERNAL "bsp debug asm compiler flags release")

set(CMAKE_C_FLAGS "${SOS_BUILD_C_FLAGS}" CACHE INTERNAL "bsp c compiler flags release")
set(CMAKE_CXX_FLAGS "${SOS_BUILD_C_FLAGS} ${SOS_BUILD_CXX_FLAGS}" CACHE INTERNAL "bsp cxx compiler flags release")
set(CMAKE_ASM_FLAGS "${SOS_BUILD_ASM_FLAGS}" CACHE INTERNAL "bsp asm compiler flags release")
