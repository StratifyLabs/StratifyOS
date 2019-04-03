
option(SOS_VERBOSE "Create verbose makefiles" OFF)

set(BUILD_ARCH ${SOS_ARCH})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-build-flags.cmake)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-sdk.cmake)
sos_get_git_hash()

if(SOS_VERBOSE)
	set(CMAKE_VERBOSE_MAKEFILE 1)
endif()

if(NOT SOS_CONFIG OR SOS_CONFIG STREQUAL "")
	set(SOS_CONFIG release)
endif()
set(BUILD_NAME build_${SOS_CONFIG}_${SOS_ARCH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${BUILD_NAME})

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if(SOS_OPTIMIZATION)
	set(BUILD_OPTIMIZATION ${SOS_OPTIMIZATION})
else()
	set(BUILD_OPTIMIZATION "-Os")
endif()


if(SOS_ARCH STREQUAL "link")
	set(BUILD_TARGET ${SOS_NAME}_${SOS_CONFIG})
	set(BUILD_LIBRARIES ${SOS_LIBRARIES} api sos_link)
	set(BUILD_FLAGS ${BUILD_OPTIMIZATION} ${SOS_BUILD_FLAGS})
	if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
		set(LINKER_FLAGS "-L${TOOLCHAIN_LIB_DIR} -L${SOS_SDK_LIB_DIR} -static -static-libstdc++ -static-libgcc")
	else()
		set(LINKER_FLAGS "-L${TOOLCHAIN_LIB_DIR} -L${SOS_SDK_LIB_DIR}")
	endif()
	add_executable(${BUILD_TARGET} ${SOS_SOURCELIST})

	target_compile_definitions(${BUILD_TARGET} PUBLIC __${SOS_ARCH} __${SOS_CONFIG} ${SOS_DEFINITIONS} MCU_SOS_GIT_HASH=${SOS_GIT_HASH})

else()

	if(SOS_PLUS)
		#use full support for C++ RTTI, exceptions and cstdlib++
		set(BUILD_PLUS_LIBRARIES api)
	else()
		#only support embedded C++ (no RTTI, no exceptions and no cstdlib++
		set(BUILD_PLUS_LIBRARIES api)
	endif()

	set(BUILD_TARGET ${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH}.elf)
	set(BUILD_LIBRARIES ${SOS_LIBRARIES} api stdc++ supc++ sos_crt)
	set(BUILD_FLAGS -mlong-calls ${SOS_BUILD_FLOAT_OPTIONS} ${BUILD_OPTIMIZATION} ${SOS_BUILD_FLAGS})
	set(LINKER_FLAGS "-nostartfiles -nostdlib -L${TOOLCHAIN_LIB_DIR}/${SOS_BUILD_INSTALL_DIR}/${SOS_BUILD_FLOAT_DIR}/. -Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH}.map,--defsym=_app_ram_size=${SOS_RAM_SIZE},--gc-sections${SOS_LINKER_FLAGS} -Tldscripts/app.ld -u crt")

	add_executable(${BUILD_TARGET} ${SOS_SOURCELIST})
	add_custom_target(bin_${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH} DEPENDS ${BUILD_TARGET} COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SOS_NAME})
	add_custom_target(asm_${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH} DEPENDS bin_${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH} COMMAND ${CMAKE_OBJDUMP} -S -j .text -j .priv_code -j .data -j .bss -j .sysmem -d ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_TARGET} > ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SOS_NAME}_${SOS_CONFIG}.lst)
	add_custom_target(size_${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH} DEPENDS asm_${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH} COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_TARGET})
	add_custom_target(${SOS_CONFIG}_${SOS_ARCH} ALL DEPENDS size_${SOS_NAME}_${SOS_CONFIG}_${SOS_ARCH})

	target_compile_definitions(${BUILD_TARGET} PUBLIC __${SOS_ARCH} __StratifyOS__ ${SOS_DEFINITIONS} MCU_SOS_GIT_HASH=${SOS_GIT_HASH})
endif()

target_link_libraries(${BUILD_TARGET} ${BUILD_LIBRARIES})
set_target_properties(${BUILD_TARGET} PROPERTIES LINK_FLAGS ${LINKER_FLAGS})
target_compile_options(${BUILD_TARGET} PUBLIC ${BUILD_FLAGS})

if(SOS_INCLUDE_DIRECTORIES)
	target_include_directories(${BUILD_TARGET} PUBLIC ${SOS_INCLUDE_DIRECTORIES})
endif()

target_include_directories(${BUILD_TARGET} PUBLIC ${SOS_BUILD_SYSTEM_INCLUDES})
