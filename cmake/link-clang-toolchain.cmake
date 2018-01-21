

if(TOOLCHAIN_DIR)
  message(STATUS "User provided toolchain directory" ${TOOLCHAIN_DIR})
else()
  set(TOOLCHAIN_DIR "/Applications/StratifyLabs-SDK/Tools/gcc" CACHE INTERNAL "toolchain dir")
endif()

message(STATUS "Use Clang toolchain install dir: " ${TOOLCHAIN_DIR})
set(CMAKE_INSTALL_PREFIX ${TOOLCHAIN_DIR} CACHE INTERNAL "install prefix")
include_directories(SYSTEM ${TOOLCHAIN_DIR}/include)

set(TOOLCHAIN_C_FLAGS "-m64 -arch x86_64 -mmacosx-version-min=10.8" CACHE INTERNAL "common c compiler flags")
set(TOOLCHAIN_CXX_FLAGS "" CACHE INTERNAL "common c compiler flags")

set(CMAKE_C_FLAGS "-D__macosx ${TOOLCHAIN_C_FLAGS}" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS "${TOOLCHAIN_C_FLAGS} ${TOOLCHAIN_CXX_FLAGS}" CACHE INTERNAL "cxx compiler flags release")

set(CMAKE_C_COMPILER clang CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER clang++ CACHE INTERNAL "cxx compiler")
