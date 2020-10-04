include(${CMAKE_CURRENT_LIST_DIR}/sos-variables.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sos-arm-arch.cmake)

if( CMAKE_BINARY_DIR MATCHES ".*_link" )
	set(SOS_BUILD_CONFIG link CACHE INTERNAL "sos build config is link")
elseif( CMAKE_BINARY_DIR MATCHES ".*_arm" )
	set(SOS_BUILD_CONFIG arm CACHE INTERNAL "sos build config is arm")
else()
	message( FATAL_ERROR "No Configuration available build in *_link or *_arm directory")
endif()

if(NOT SOS_SDK_PATH)
	message(FATAL "Must define 'SOS_SDK_PATH' in environment or use '-DSOS_SDK_PATH=<path>'")
endif()

set(SOS_SDK_EXEC_SUFFIX "")
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
	set(SOS_SDK_EXEC_SUFFIX ".exe")
endif()
set(SOS_SDK_GIT_EXEC git${SOS_SDK_EXEC_SUFFIX})

if(SOS_VERBOSE)
	set(CMAKE_VERBOSE_MAKEFILE 1)
endif()

function(sos_sdk_pull PROJECT_PATH)
	execute_process(COMMAND ${SOS_SDK_GIT_EXEC} pull WORKING_DIRECTORY ${PROJECT_PATH} OUTPUT_VARIABLE OUTPUT RESULT_VARIABLE RESULT)
	message(STATUS "git pull " ${PROJECT_PATH} "\n" ${OUTPUT})
	if(RESULT)
		message(FATAL_ERROR " Failed to pull " ${PROJECT_PATH})
	endif()
endfunction()

function(sos_sdk_add_subdirectory INPUT_LIST DIRECTORY)
	add_subdirectory(${DIRECTORY})
	set(INPUT_SOURCES ${${INPUT_LIST}})
	set(TEMP_SOURCES "")
	foreach(entry ${SOURCES})
		list(APPEND TEMP_SOURCES ${DIRECTORY}/${entry})
	endforeach()
	list(APPEND TEMP_SOURCES ${INPUT_SOURCES})
	set(${INPUT_LIST} ${TEMP_SOURCES} PARENT_SCOPE)
endfunction()

function(sos_sdk_add_out_of_source_directory INPUT_LIST DIRECTORY BINARY_DIRECTORY)
	add_subdirectory(${DIRECTORY} ${BINARY_DIRECTORY})
	set(INPUT_SOURCES ${${INPUT_LIST}})
	set(TEMP_SOURCES "")
	foreach(entry ${SOURCES})
		list(APPEND TEMP_SOURCES ${DIRECTORY}/${entry})
	endforeach()
	list(APPEND TEMP_SOURCES ${INPUT_SOURCES})
	set(${INPUT_LIST} ${TEMP_SOURCES} PARENT_SCOPE)
endfunction()

function(sos_sdk_git_status PROJECT_PATH)
	message(STATUS "GIT STATUS OF " ${PROJECT_PATH})
	execute_process(COMMAND ${SOS_SDK_GIT_EXEC} status WORKING_DIRECTORY ${PROJECT_PATH} RESULT_VARIABLE RESULT)
endfunction()

function(sos_sdk_clone REPO_URL WORKSPACE_PATH)
	execute_process(COMMAND ${SOS_SDK_GIT_EXEC} clone ${REPO_URL} WORKING_DIRECTORY ${WORKSPACE_PATH} OUTPUT_VARIABLE OUTPUT RESULT_VARIABLE RESULT)
	message(STATUS "git clone " ${REPO_URL} to ${WORKSPACE_PATH} "\n" ${OUTPUT})
	if(RESULT)
		message(FATAL_ERROR " Failed to clone " ${PROJECT_PATH})
	endif()
endfunction()

function(sos_sdk_clone_or_pull PROJECT_PATH REPO_URL WORKSPACE_PATH)
	#if ${PROJECT_PATH} directory doesn't exist -- clone from the URL
	if(EXISTS ${PROJECT_PATH}/.git)
		message(STATUS ${PROJECT_PATH} " already exists: pulling")
		sos_sdk_pull(${PROJECT_PATH})
	else()
		file(REMOVE_RECURSE ${PROJECT_PATH})
		message(STATUS ${PROJECT_PATH} " does not exist: cloning")
		sos_sdk_clone(${REPO_URL} ${WORKSPACE_PATH})
	endif()
endfunction()

function(sos_sdk_checkout PROJECT_PATH GIT_PATH)
	execute_process(COMMAND ${SOS_SDK_GIT_EXEC} checkout ${GIT_PATH} WORKING_DIRECTORY ${PROJECT_PATH} OUTPUT_VARIABLE OUTPUT RESULT_VARIABLE RESULT)
	message(STATUS "git checkout " ${GIT_PATH} " in " ${PROJECT_PATH} "\n" ${OUTPUT})
	if(RESULT)
		message(FATAL_ERROR " Failed to checkout " ${PROJECT_PATH} ${GIT_PATH})
	endif()
endfunction()

function(sos_get_git_hash)
	execute_process(
		COMMAND git log -1 --format=%h
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_HASH_OUTPUT_VARIABLE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE RESULT
		)

	if(RESULT)
		set(SOS_GIT_HASH "0000000" PARENT_SCOPE)
	else()
		set(SOS_GIT_HASH ${GIT_HASH_OUTPUT_VARIABLE} PARENT_SCOPE)
	endif()
endfunction()

sos_get_git_hash()


macro(build_target_name BASE_NAME OPTION_NAME CONFIG_NAME ARCH_NAME)
	set(SOS_SDK_TMP_TARGET ${BASE_NAME})
	set(SOS_SDK_TMP_INSTALL ${BASE_NAME})
	set(SOS_SDK_TMP_NO_CONFIG ${BASE_NAME})
	set(SOS_SDK_TMP_NO_NAME "")

	string(COMPARE EQUAL "${OPTION_NAME}" "" OPTION_MISSING)

	if(NOT OPTION_MISSING)
		set(SOS_SDK_TMP_OPTION ${OPTION_NAME})
		set(SOS_SDK_TMP_TARGET ${SOS_SDK_TMP_TARGET}_${SOS_SDK_TMP_OPTION})
		set(SOS_SDK_TMP_NO_CONFIG ${SOS_SDK_TMP_NO_CONFIG}_${SOS_SDK_TMP_OPTION})
		set(SOS_SDK_TMP_INSTALL ${SOS_SDK_TMP_INSTALL}_${SOS_SDK_TMP_OPTION})
		set(SOS_SDK_TMP_NO_NAME ${OPTION_NAME}_)
	else()
		set(SOS_SDK_TMP_OPTION "SOS_SDK_OPTION_EMPTY")
	endif()

	if(NOT CONFIG_NAME STREQUAL "__none__")
		set(SOS_SDK_TMP_CONFIG ${CONFIG_NAME})
	else()
		set(SOS_SDK_TMP_CONFIG "release")
	endif()

	if(NOT SOS_SDK_TMP_CONFIG STREQUAL release)
		set(SOS_SDK_TMP_INSTALL ${SOS_SDK_TMP_INSTALL}_${SOS_SDK_TMP_CONFIG})
	endif()

	set(SOS_SDK_TMP_TARGET ${SOS_SDK_TMP_TARGET}_${SOS_SDK_TMP_CONFIG})
	set(SOS_SDK_TMP_NO_NAME ${SOS_SDK_TMP_NO_NAME}${SOS_SDK_TMP_CONFIG})

	if(NOT ARCH_NAME STREQUAL "")
		set(SOS_SDK_TMP_TARGET ${SOS_SDK_TMP_TARGET}_${ARCH_NAME})
	else()
		message(FATAL " SOS SDK ARCH cannot be empty")
	endif()

	message(STATUS "SOS SDK Target ${SOS_SDK_TMP_TARGET}")
endmacro()

function(sos_sdk_copy_target SOURCE_TARGET DEST_TARGET)
	foreach (PROPERTY ${SOS_SHARED_PROPERTIES})
		string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" prop ${PROPERTY})
		get_property(PROPERTY_VALUE TARGET ${SOURCE_TARGET} PROPERTY ${PROPERTY} SET)

		if (PROPERTY_VALUE)
			get_target_property(PROPERTY_VALUE ${SOURCE_TARGET} ${PROPERTY})
			set_property(TARGET ${DEST_TARGET} PROPERTY ${PROPERTY} ${PROPERTY_VALUE})
		endif()

	endforeach(PROPERTY)
endfunction()

macro(is_arch_enabled ARCH)
	SET(ARCH_ENABLED OFF)
	sos_arm_arch(${ARCH})

	if(SOS_ARCH_ARM_ALL)
		set(ARCH_ENABLED ON)
	else()
		if((IS_V7M) AND (SOS_ARCH_ARM_V7M))
			set(ARCH_ENABLED ON)
		endif()
		if((IS_V7EM) AND (SOS_ARCH_ARM_V7EM))
			set(ARCH_ENABLED ON)
		endif()
		if((IS_V7EM_F4SH) AND (SOS_ARCH_ARM_V7EM_F4SH))
			set(ARCH_ENABLED ON)
		endif()
		if((IS_V7EM_F5SH) AND (SOS_ARCH_ARM_V7EM_F5SH))
			set(ARCH_ENABLED ON)
		endif()
		if((IS_V7EM_F5DH) AND (SOS_ARCH_ARM_V7EM_F5DH))
			set(ARCH_ENABLED ON)
		endif()
	endif()
endmacro()

include(${CMAKE_CURRENT_LIST_DIR}/sos-lib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sos-bsp.cmake)

function(sos_sdk_app_target OUTPUT BASE_NAME OPTION CONFIG ARCH)
	build_target_name("${BASE_NAME}" "${OPTION}" "${CONFIG}" "${ARCH}")
	set(${OUTPUT}_OPTIONS "${BASE_NAME};${OPTION};${CONFIG};${ARCH}" PARENT_SCOPE)
	set(${OUTPUT}_TARGET ${SOS_SDK_TMP_INSTALL}.elf PARENT_SCOPE)
endfunction()

function(sos_sdk_app BASE_NAME OPTION CONFIG ARCH HARDWARE_ID START_ADDRESS)
	build_target_name("${BASE_NAME}" "${OPTION}" "${CONFIG}" "${ARCH}")

	set(TARGET_NAME ${SOS_SDK_TMP_INSTALL}.elf)

	set(BINARY_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build_${CONFIG})

	set_target_properties(${TARGET_NAME}
		PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})

	file(MAKE_DIRECTORY ${BINARY_OUTPUT_DIR})

	target_compile_definitions(${TARGET_NAME}
		PUBLIC
		__StratifyOS__
		__${SOS_SDK_TMP_CONFIG}
		__${SOS_SDK_TMP_OPTION}
		__${ARCH}
		__HARDWARE_ID=${HARDWARE_ID}
		MCU_SOS_GIT_HASH=${SOS_GIT_HASH}
		)

	target_include_directories(${TARGET_NAME}
		PUBLIC
		${SOS_BUILD_SYSTEM_INCLUDES}
		)

	sos_arm_arch(${ARCH})

	target_compile_options(${TARGET_NAME}
		PUBLIC
		-mthumb -D__StratifyOS__ -ffunction-sections -fdata-sections -fomit-frame-pointer
		${SOS_ARM_ARCH_BUILD_FLAGS}
		${SOS_ARM_ARCH_BUILD_FLOAT_OPTIONS}
		)

	get_target_property(EXIST_LINK_FLAGS ${TARGET_NAME} LINK_FLAGS)

	set(BSP_LINK_FLAGS
		-L${SOS_SDK_PATH}/Tools/gcc/arm-none-eabi/lib/${SOS_ARM_ARCH_BUILD_INSTALL_DIR}/${SOS_ARM_ARCH_BUILD_FLOAT_DIR}
		-L${SOS_SDK_PATH}/Tools/gcc/lib/gcc/arm-none-eabi/${CMAKE_CXX_COMPILER_VERSION}/${SOS_ARM_ARCH_BUILD_INSTALL_DIR}/${SOS_ARM_ARCH_BUILD_FLOAT_DIR}
		-Wl,--print-memory-usage,-Map,${BINARY_OUTPUT_DIR}/${SOS_SDK_TMP_INSTALL}.map,--gc-sections,--defsym=mcu_core_hardware_id=${HARDWARE_ID}
		-Ttext=${START_ADDRESS}
		-nostdlib
		-u mcu_core_vector_table
		${EXIST_LINK_FLAGS}
		)

	list(JOIN BSP_LINK_FLAGS " " LINK_FLAGS)

	set_target_properties(${TARGET_NAME}
		PROPERTIES
		LINK_FLAGS
		"${LINK_FLAGS}"
		)

	add_custom_target(bin_${TARGET_NAME} DEPENDS ${TARGET_NAME} COMMAND ${CMAKE_OBJCOPY} -j .boot_hdr -j .text -j .data -O binary ${BINARY_OUTPUT_DIR}/${TARGET_NAME} ${BINARY_OUTPUT_DIR}/${SOS_SDK_TMP_NO_CONFIG}.bin)
	add_custom_target(asm_${TARGET_NAME} DEPENDS bin_${TARGET_NAME} COMMAND ${CMAKE_OBJDUMP} -S -j .boot_hdr -j .tcim -j .text -j .priv_code -j .data -j .bss -j .sysmem -d ${BINARY_OUTPUT_DIR}/${TARGET_NAME} > ${BINARY_OUTPUT_DIR}/${SOS_SDK_TMP_INSTALL}.lst)
	add_custom_target(size_${TARGET_NAME} DEPENDS asm_${TARGET_NAME} COMMAND ${CMAKE_SIZE} ${BINARY_OUTPUT_DIR}/${TARGET_NAME})
	add_custom_target(${CONFIG} ALL DEPENDS size_${TARGET_NAME})

endfunction()


function(sos_sdk_app TARGET_NAME CONFIG ARCH)

endfunction()


