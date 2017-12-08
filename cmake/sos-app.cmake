


if(NOT BUILD_ARCH)
	set(BUILD_ARCH armv7-m CACHE INTERNAL "build arch")
	message("BUILD_ARCH not specified; using armv7-m (cortex-m3)")
endif()


if( ${BUILD_ARCH} STREQUAL armv7-m ) 
	set(BUILD_DEVICE armv7m CACHE INTERNAL "build device")
	set(BUILD_FLOAT .)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_release_armv7m CACHE INTERNAL "target directory")
else()
	set(BUILD_FLOAT fpu/fpv5-sp-d16)
	set(BUILD_DEVICE armv7em CACHE INTERNAL "build device")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_release_armv7em_fpu CACHE INTERNAL "target directory")
	set(BUILD_FLOAT_OPTIONS -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED CACHE INTERNAL "build float options")
endif()



set(SOS_APP_BUILD_LIBRARIES api sos_crt CACHE INTERNAL "sos build libraries")
set(SOS_APP_BUILD_FLAGS -march=${BUILD_ARCH} -D__${BUILD_DEVICE} ${BUILD_FLOAT_OPTIONS} CACHE INTERNAL "sos app build options")
set(SOS_APP_LINKER_FLAGS "-nostartfiles -nostdlib -L${TOOLCHAIN_LIB_DIR}/${BUILD_ARCH}/${BUILD_FLOAT}/. -Wl,-Map,${NAME}.map,--defsym=_app_ram_size=${BUILD_RAM_SIZE},--gc -Tldscripts/app.ld -u crt" CACHE INTERNAL "sos app linker options")


add_custom_target(${NAME}_${BUILD_ARCH}.bin DEPENDS ${NAME}_${BUILD_ARCH}.elf COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${BUILD_ARCH}.elf ${NAME})
add_custom_target(size_${BUILD_ARCH} DEPENDS ${NAME}_${BUILD_ARCH}.bin COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_${BUILD_ARCH}.elf)

add_custom_target(${BUILD_ARCH} DEPENDS size_${BUILD_ARCH})

add_executable(${NAME}_${BUILD_ARCH}.elf ${CMAKE_SOURCE_DIR}/src/main.cpp)

target_link_libraries(${NAME}_${BUILD_ARCH}.elf ${SOS_APP_BUILD_LIBRARIES})
set_target_properties(${NAME}_${BUILD_ARCH}.elf PROPERTIES LINK_FLAGS ${SOS_APP_LINKER_FLAGS})
target_compile_options(${NAME}_${BUILD_ARCH}.elf PUBLIC ${SOS_APP_BUILD_FLAGS})

