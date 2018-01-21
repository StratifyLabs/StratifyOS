

#-L"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/armv7-m/." -L"/Applications/StratifyLabs-SDK/Tools/gcc/lib/gcc/arm-none-eabi/5.4.1/armv7-m/." -Wl,-Map,fw.map,--gc,--defsym=mcu_core_hardware_id=0x00000003 -Ttext=0x40000 -Tldscripts/lpc1759-rom.ld -nostdlib -u symbols_table

#-L"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/armv7-m/." -L"/Applications/StratifyLabs-SDK/Tools/gcc/lib/gcc/arm-none-eabi/5.4.1/armv7-m/." -Wl,-Map,fw.map,--gc -Ttext=0 -Tldscripts/lpc1759-rom.ld -nostdlib -u _main

if( ${SOS_BSP_ARCH} STREQUAL armv7-m )
	set(BUILD_FLOAT ${TOOLCHAIN_FLOAT_DIR_ARMV7M})
	set(BUILD_GCC_LIB gcc CACHE INTERNAL "build gcc lib")
	set(BUILD_ARCH armv7m CACHE INTERNAL "build arch")
elseif( ${SOS_BSP_ARCH} STREQUAL armv7e-m )
	set(BUILD_FLOAT ${TOOLCHAIN_FLOAT_DIR_ARMV7EM_FPU})
	set(BUILD_FLOAT_OPTIONS ${TOOLCHAIN_FLOAT_OPTIONS_ARMV7EM_FPU} CACHE INTERNAL "build float options")
	set(BUILD_GCC_LIB gcc-hard CACHE INTERNAL "build gcc lib")
	set(BUILD_ARCH armv7em_fpu CACHE INTERNAL "build arch")
endif()

if( ${SOS_BSP_CONFIG} STREQUAL release )
	set(BUILD_TYPE "" CACHE INTERNAL "build type")
	set(BUILD_NAME build_release CACHE INTERNAL "build name")
	set(BUILD_UNDEFINE_SYMBOL symbols_table)
	set(BUILD_HARDWARD_ID ,--defsym=mcu_core_hardware_id=${SOS_BSP_HARDWARD_ID})
elseif( ${SOS_BSP_CONFIG} STREQUAL debug )
	set(BUILD_TYPE "_debug" CACHE INTERNAL "build type")
	set(BUILD_NAME build_debug CACHE INTERNAL "build name")
	set(BUILD_UNDEFINE_SYMBOL symbols_table)
	set(BUILD_HARDWARD_ID ,--defsym=mcu_core_hardware_id=${SOS_BSP_HARDWARD_ID})
elseif( ${SOS_BSP_CONFIG} STREQUAL release_boot )
	set(BUILD_TYPE "" CACHE INTERNAL "build type")
	set(BUILD_NAME build_release_boot CACHE INTERNAL "build name")
	set(BUILD_UNDEFINE_SYMBOL _main)
	set(BUILD_HARDWARD_ID "")
elseif( ${SOS_BSP_CONFIG} STREQUAL debug_boot )
	set(BUILD_TYPE "_debug" CACHE INTERNAL "build type")
	set(BUILD_NAME build_debug_boot CACHE INTERNAL "build name")
	set(BUILD_UNDEFINE_SYMBOL _main)
	set(BUILD_HARDWARD_ID "")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${BUILD_NAME} CACHE INTERNAL "runtime output")

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(BUILD_LIBRARIES "${SOS_BSP_LIBRARIES} -lsos_sys${BUILD_TYPE} -lsos_mcu_${SOS_BSP_DEVICE_FAMILY}${BUILD_TYPE} -lm -lc -lsos_sys${BUILD_TYPE} -l${BUILD_GCC_LIB}" CACHE INTERNAL "sos build libraries")
set(BUILD_FLAGS -march=${SOS_BSP_ARCH} -DHARDWARE_ID=${SOS_BSP_HARDWARD_ID} -D__${SOS_BSP_DEVICE_FAMILY} -D__${BUILD_ARCH} -D__${SOS_BSP_DEVICE} ${BUILD_FLOAT_OPTIONS} CACHE INTERNAL "sos bsp build options")
set(LINKER_FLAGS "-L${TOOLCHAIN_LIB_DIR}/${SOS_BSP_ARCH}/${BUILD_FLOAT}/. -Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${SOS_BSP_CONFIG}.map,--gc${BUILD_HARDWARD_ID} -Ttext=${SOS_BSP_START_ADDRESS} -Tldscripts/${SOS_BSP_DEVICE}-rom.ld -nostdlib -u ${BUILD_UNDEFINE_SYMBOL}" CACHE INTERNAL "sos app linker options")

add_executable(${NAME}_${SOS_BSP_CONFIG}.elf ${SOURCELIST})
add_custom_target(bin_${NAME}_${SOS_BSP_CONFIG} DEPENDS ${NAME}_${SOS_BSP_CONFIG}.elf COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${SOS_BSP_CONFIG}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}.bin)
add_custom_target(size_${SOS_BSP_CONFIG} DEPENDS bin_${NAME}_${SOS_BSP_CONFIG} COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${SOS_BSP_CONFIG}.elf)
add_custom_target(${SOS_BSP_CONFIG} ALL DEPENDS size_${SOS_BSP_CONFIG})

target_link_libraries(${NAME}_${SOS_BSP_CONFIG}.elf ${BUILD_LIBRARIES})
set_target_properties(${NAME}_${SOS_BSP_CONFIG}.elf PROPERTIES LINK_FLAGS ${LINKER_FLAGS})
target_compile_options(${NAME}_${SOS_BSP_CONFIG}.elf PUBLIC ${BUILD_FLAGS})
