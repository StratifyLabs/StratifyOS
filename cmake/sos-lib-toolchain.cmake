include(${CMAKE_CURRENT_LIST_DIR}/sos-toolchain.cmake)

set(CMAKE_C_FLAGS "${TOOLCHAIN_C_FLAGS}" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS "${TOOLCHAIN_C_FLAGS} ${TOOLCHAIN_CXX_FLAGS}" CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS "${TOOLCHAIN_ASM_FLAGS}" CACHE INTERNAL "asm compiler flags release")
