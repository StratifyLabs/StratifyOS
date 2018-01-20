


set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE INTERNAL "verbose make")


if(NOT BUILD_ARCH)
	set(BUILD_ARCH armv7-m CACHE INTERNAL "build arch")
	message("BUILD_ARCH not specified; using armv7-m (cortex-m3)")
endif()

if( ${BUILD_ARCH} STREQUAL armv7-m )
	set(BUILD_NAME build_release_armv7m CACHE INTERNAL "build name")
	set(BUILD_DEVICE armv7m CACHE INTERNAL "build device")
	set(BUILD_FLOAT .)
else()
	set(BUILD_NAME build_release_armv7em_fpu CACHE INTERNAL "build name")
	set(BUILD_FLOAT fpu/fpv5-sp-d16)
	set(BUILD_DEVICE armv7em CACHE INTERNAL "build device")
	set(BUILD_FLOAT_OPTIONS -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED CACHE INTERNAL "build float options")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${BUILD_NAME} CACHE INTERNAL "runtime output")

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(SOS_APP_BUILD_LIBRARIES api sos_crt CACHE INTERNAL "sos build libraries")
set(SOS_APP_BUILD_FLAGS -march=${BUILD_ARCH} -D__${BUILD_DEVICE} ${BUILD_FLOAT_OPTIONS} CACHE INTERNAL "sos app build options")
set(SOS_APP_LINKER_FLAGS "-nostartfiles -nostdlib -L${TOOLCHAIN_LIB_DIR}/${BUILD_ARCH}/${BUILD_FLOAT}/. -Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${BUILD_ARCH}.map,--defsym=_app_ram_size=${BUILD_RAM_SIZE},--gc -Tldscripts/app.ld -u crt" CACHE INTERNAL "sos app linker options")

add_executable(${NAME}_${BUILD_ARCH}.elf ${SOURCELIST})
add_custom_target(bin_${NAME}_${BUILD_ARCH} DEPENDS ${NAME}_${BUILD_ARCH}.elf COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${BUILD_ARCH}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
add_custom_target(size_${BUILD_ARCH} DEPENDS bin_${NAME}_${BUILD_ARCH} COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${BUILD_ARCH}.elf)
add_custom_target(${BUILD_ARCH} ALL DEPENDS size_${BUILD_ARCH})

target_link_libraries(${NAME}_${BUILD_ARCH}.elf ${SOS_APP_BUILD_LIBRARIES})
set_target_properties(${NAME}_${BUILD_ARCH}.elf PROPERTIES LINK_FLAGS ${SOS_APP_LINKER_FLAGS})
target_compile_options(${NAME}_${BUILD_ARCH}.elf PUBLIC ${SOS_APP_BUILD_FLAGS})
