
option(SOS_VERBOSE "Create verbose makefiles" OFF)


set(BUILD_ARCH ${SOS_ARCH})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-build-flags.cmake)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-sdk.cmake)
sos_get_git_hash()

if(SOS_VERBOSE)
	set(CMAKE_VERBOSE_MAKEFILE 1)
endif()


if( ${SOS_CONFIG} MATCHES "release$" ) # matches if the config ends with "release"
	set(BUILD_TYPE "")
	set(BUILD_NAME build_${SOS_CONFIG})
	set(BUILD_UNDEFINE_SYMBOL symbols_table)
	set(BUILD_EXTRA_LIBRARIES "")
elseif( ${SOS_CONFIG} MATCHES "debug$" ) # matches if the config ends with "debug"
	set(BUILD_TYPE "_debug")
	set(BUILD_NAME build_${SOS_CONFIG})
	set(BUILD_UNDEFINE_SYMBOL symbols_table)
	set(BUILD_EXTRA_LIBRARIES "")
elseif( ${SOS_CONFIG} MATCHES "release_boot$" ) # matches if the config ends with "release_boot"
	set(BUILD_TYPE "")
	set(BUILD_NAME build_${SOS_CONFIG})
	set(BUILD_EXTRA_LIBRARIES -lsos_boot)
	set(BUILD_UNDEFINE_SYMBOL _main)
elseif( ${SOS_CONFIG} MATCHES "debug_boot$" ) # matches if the config ends with "debug_boot"
	set(BUILD_TYPE "_debug")
	set(BUILD_NAME build_${SOS_CONFIG})
	set(BUILD_UNDEFINE_SYMBOL _main)
	set(BUILD_EXTRA_LIBRARIES -lsos_boot_debug)
else()
	set(BUILD_TYPE "")
	set(BUILD_NAME build_${SOS_CONFIG})
	set(BUILD_UNDEFINE_SYMBOL symbols_table)
	set(BUILD_EXTRA_LIBRARIES "")
endif()

set(BUILD_HARDWARD_ID ,--defsym=mcu_core_hardware_id=${SOS_HARDWARD_ID})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${BUILD_NAME})

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if(SOS_OPTIMIZATION)
	set(BUILD_OPTIMIZATION ${SOS_OPTIMIZATION})
else()
	set(BUILD_OPTIMIZATION "-Os")
endif()

if(SOS_LINKER_FILE)
	set(LINKER_FILE ${SOS_LINKER_FILE})
else()
	set(LINKER_FILE ldscripts/${SOS_DEVICE}-rom.ld)
endif()

if(SOS_LIBRARIES)
	set(BUILD_LIBRARIES "${SOS_LIBRARIES} -lsos_sys${BUILD_TYPE} -lsos_mcu_${SOS_DEVICE_FAMILY}${BUILD_TYPE} -lm -lc -lsos_sys${BUILD_TYPE} -l${SOS_BUILD_GCC_LIB}")
else()
	set(BUILD_LIBRARIES "-lsos_sys${BUILD_TYPE} -lsos_mcu_${SOS_DEVICE_FAMILY}${BUILD_TYPE} -lm -lc -lsos_sys${BUILD_TYPE} -l${SOS_BUILD_GCC_LIB}")
endif()
set(BUILD_FLAGS ${SOS_BUILD_FLOAT_OPTIONS} ${BUILD_OPTIMIZATION} ${SOS_BUILD_FLAGS})
set(LINKER_FLAGS
	"${SOS_LINKER_FLAGS} -L${TOOLCHAIN_LIB_DIR}/${SOS_BUILD_INSTALL_DIR}/${SOS_BUILD_FLOAT_DIR} -L${TOOLCHAIN_DIR}/lib/gcc/${TOOLCHAIN_HOST}/${CMAKE_CXX_COMPILER_VERSION}/${SOS_BUILD_INSTALL_DIR}/${SOS_BUILD_FLOAT_DIR} -Wl,--print-memory-usage,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SOS_NAME}_${SOS_CONFIG}.map,--gc-sections${BUILD_HARDWARD_ID} -Ttext=${SOS_START_ADDRESS} -T${LINKER_FILE} -nostdlib -u ${BUILD_UNDEFINE_SYMBOL} -u mcu_core_vector_table"
	)

set(BUILD_TARGET ${SOS_NAME}_${SOS_CONFIG}.elf)

add_executable(${BUILD_TARGET} ${SOS_SOURCELIST})
add_custom_target(bin_${SOS_NAME}_${SOS_CONFIG} DEPENDS ${BUILD_TARGET} COMMAND ${CMAKE_OBJCOPY} -j .boot_hdr -j .text -j .data -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SOS_NAME}.bin)
add_custom_target(asm_${SOS_NAME}_${SOS_CONFIG} DEPENDS bin_${SOS_NAME}_${SOS_CONFIG} COMMAND ${CMAKE_OBJDUMP} -S -j .boot_hdr -j .tcim -j .text -j .priv_code -j .data -j .bss -j .sysmem -d ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_TARGET} > ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SOS_NAME}.lst)
add_custom_target(size_${SOS_NAME}_${SOS_CONFIG} DEPENDS asm_${SOS_NAME}_${SOS_CONFIG} COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_TARGET})
add_custom_target(${SOS_CONFIG} ALL DEPENDS size_${SOS_NAME}_${SOS_CONFIG})
add_custom_target(flash_${SOS_NAME}_${SOS_CONFIG}
	DEPENDS size_${SOS_NAME}_${SOS_CONFIG}
	COMMAND ${SOS_TOOLCHAIN_CMAKE_PATH}/../../bin/sl os.install:path=${SOS_NAME},build=${SOS_CONFIG}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../
	)

target_compile_definitions(${BUILD_TARGET} PUBLIC __StratifyOS__ __${BUILD_TYPE} __${BULD_ARCH} __HARDWARE_ID=${SOS_HARDWARD_ID} __${SOS_DEVICE_FAMILY} __${SOS_DEVICE} ${SOS_DEFINITIONS} MCU_SOS_GIT_HASH=${SOS_GIT_HASH})
target_link_libraries(${BUILD_TARGET} ${BUILD_EXTRA_LIBRARIES} ${BUILD_LIBRARIES})
set_target_properties(${BUILD_TARGET} PROPERTIES LINK_FLAGS ${LINKER_FLAGS})
target_compile_options(${BUILD_TARGET} PUBLIC ${BUILD_FLAGS})

if(SOS_INCLUDE_DIRECTORIES)
	target_include_directories(${BUILD_TARGET} PUBLIC ${SOS_INCLUDE_DIRECTORIES})
endif()

target_include_directories(${BUILD_TARGET} PUBLIC ${SOS_BUILD_SYSTEM_INCLUDES})
