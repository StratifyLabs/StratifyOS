cmake_minimum_required (VERSION 3.6)

set(SOS_TOOLCHAIN_CMAKE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-sdk.cmake)

option(CLONE "Clone SDK Libraries" ON)
option(BUILD "Build SDK Libraries " ON)

if(CLONE)
    file(REMOVE_RECURSE ../sgfx)
    file(REMOVE_RECURSE ../StratifyOS-mbedtls)
    file(REMOVE_RECURSE ../StratifyOS-jansson)
    file(REMOVE_RECURSE ../StratifyAPI)
    file(REMOVE_RECURSE ../UsbAPI)
    file(REMOVE_RECURSE ../StratifyOS-libusb)
    file(REMOVE_RECURSE ../CloudServiceAPI)
    file(REMOVE_RECURSE ../sl)

    sos_sdk_clone(https://github.com/StratifyLabs/sgfx.git ..)
    sos_sdk_clone(https://github.com/StratifyLabs/StratifyOS-mbedtls.git ..)
    sos_sdk_clone(https://github.com/StratifyLabs/StratifyOS-jansson.git ..)
    sos_sdk_clone(https://github.com/StratifyLabs/StratifyAPI.git ..)
    sos_sdk_clone(https://github.com/StratifyLabs/UsbAPI.git ..)
    sos_sdk_clone(https://github.com/StratifyLabs/StratifyOS-libusb.git ..)
    sos_sdk_clone(ssh://git@github.com/tyler-gilbert/CloudServiceAPI.git ..)
    sos_sdk_clone(ssh://git@github.com/tyler-gilbert/sl.git ..)
endif()

if(BUILD)
    sos_sdk_build_lib(. ON link)
    sos_sdk_build_lib(../sgfx ON link)
    execute_process(COMMAND cmake -P bootstrap.cmake WORKING_DIRECTORY ../StratifyOS-mbedtls)
    execute_process(COMMAND cp -f ../StratifyOS-mbedtls/config/mbedtls/config.h ../StratifyOS-mbedtls/mbedtls/include/mbedtls/config.h)
    sos_sdk_build_lib(../StratifyOS-mbedtls ON link)
    execute_process(COMMAND cmake -P bootstrap.cmake WORKING_DIRECTORY ../StratifyOS-jansson)
    sos_sdk_build_lib(../StratifyOS-jansson ON link)
    sos_sdk_build_lib(../StratifyAPI ON link)
    sos_sdk_build_lib(../CloudServiceAPI ON link)
    execute_process(COMMAND cmake -P bootstrap.cmake WORKING_DIRECTORY ../StratifyOS-libusb)
    sos_sdk_build_lib(../UsbAPI ON link)
    sos_sdk_build_lib(../sl ON link)
endif()