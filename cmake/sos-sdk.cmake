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

	string(COMPARE EQUAL "${OPTION_NAME}" "" OPTION_AVAILABLE)

	if(NOT OPTION_AVAILABLE)
		set(SOS_SDK_TMP_OPTION ${OPTION_NAME})
		set(SOS_SDK_TMP_TARGET ${SOS_SDK_TMP_TARGET}_${SOS_SDK_TMP_OPTION})
		set(SOS_SDK_TMP_INSTALL ${SOS_SDK_TMP_INSTALL}_${SOS_SDK_TMP_OPTION})
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

	if(NOT ARCH_NAME STREQUAL "")
		set(SOS_SDK_TMP_TARGET ${SOS_SDK_TMP_TARGET}_${ARCH_NAME})
	else()
		message(FATAL " SOS SDK ARCH cannot be empty")
	endif()

	message(STATUS "SOS SDK Target ${SOS_SDK_TMP_TARGET}")
	message(STATUS "SOS SDK Install Target ${SOS_SDK_TMP_INSTALL}")

endmacro()

function(sos_sdk_target_library_target OUTPUT BASE_NAME OPTION_NAME CONFIG_NAME ARCH_NAME)
	build_target_name(${BASE_NAME} ${OPTION_NAME} ${CONFIG_NAME} ${ARCH_NAME})
	set(OUTPUT ${SOS_SDK_TMP_TARGET} PARENT_SCOPE)
endfunction()

function(sos_sdk_copy_target SOURCE_TARGET DEST_TARGET)

	#execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

	# Convert command output into a CMake list
	#STRING(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
	#STRING(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

	foreach (PROPERTY ${SOS_SHARED_PROPERTIES})
		string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" prop ${PROPERTY})
		# Fix https://stackoverflow.com/questions/32197663/how-can-i-remove-the-the-location-property-may-not-be-read-from-target-error-i
		#if(prop STREQUAL "LOCATION" OR prop MATCHES "^LOCATION_" OR prop MATCHES "_LOCATION$")
		#	continue()
		#endif()

		# message ("Checking ${PROPERTY}")
		get_property(PROPERTY_VALUE TARGET ${SOURCE_TARGET} PROPERTY ${PROPERTY} SET)

		if (PROPERTY_VALUE)
			get_target_property(PROPERTY_VALUE ${SOURCE_TARGET} ${PROPERTY})
			set_property(TARGET ${DEST_TARGET} PROPERTY ${PROPERTY} ${PROPERTY_VALUE})
		endif()

	endforeach(PROPERTY)

endfunction()

macro(is_arch_enabled ARCH)
	SET(ARCH_ENABLED OFF)
	if(SOS_ARCH_ARM_ALL)
		set(ARCH_ENABLED ON)
	endif()
	if((ARCH STREQUAL"v7m") AND (SOS_ARCH_ARM_V7M))
		set(ARCH_ENABLED ON)
	endif()
	if((ARCH STREQUAL"v7em") AND (SOS_ARCH_ARM_V7EM))
		set(ARCH_ENABLED ON)
	endif()
	if((ARCH STREQUAL"v7em_f4sh") AND (SOS_ARCH_ARM_V7EM_F4SH))
		set(ARCH_ENABLED ON)
	endif()
	if((ARCH STREQUAL"v7em_f5sh") AND (SOS_ARCH_ARM_V7EM_F5SH))
		set(ARCH_ENABLED ON)
	endif()
	if((ARCH STREQUAL"v7em_f5dh") AND (SOS_ARCH_ARM_V7EM_F5DH))
		set(ARCH_ENABLED ON)
	endif()
endmacro()

function(sos_sdk_library_add_arm_targets BASE_NAME OPTION CONFIG)
	set(ARCH_LIST v7em v7em_f4sh v7em_f5sh v7em_f5dh)
	foreach (ARCH ${ARCH_LIST})
		is_arch_enabled(${ARCH})
		if(ARCH_ENABLED)
			set(TARGET_NAME ${BASE_NAME})
			if(NOT OPTION STREQUAL "")
				set(TARGET_NAME ${TARGET_NAME}_${OPTION})
			endif()
			set(TARGET_NAME ${TARGET_NAME}_${CONFIG})
			add_library(${TARGET_NAME}_${ARCH} STATIC)
			sos_sdk_copy_target(
				${TARGET_NAME}_v7m
				${TARGET_NAME}_${ARCH}
				)
			# this applies architecture specific options
			sos_sdk_library(${BASE_NAME} ${OPTION} ${CONFIG} ${ARCH})
		endif()
	endforeach(ARCH)
	sos_sdk_library(${BASE_NAME} ${OPTION} ${CONFIG} v7m)
endfunction()

function(sos_sdk_library BASE_NAME OPTION_NAME CONFIG_NAME ARCH_NAME)
	message(STATUS "SOS SDK Library ${BASE_NAME}_${OPTION_NAME}_${CONFIG_NAME}_${ARCH_NAME}")
	build_target_name(${BASE_NAME} ${OPTION_NAME} ${CONFIG_NAME} ${ARCH_NAME})

	target_compile_definitions(${SOS_SDK_TMP_TARGET}
		PUBLIC
		__${SOS_SDK_TMP_CONFIG}
		__${SOS_SDK_TMP_OPTION}
		__${ARCH_NAME}
		MCU_SOS_GIT_HASH=${SOS_GIT_HASH}
		)

	if(NOT ARCH_NAME STREQUAL "link")

		target_include_directories(${SOS_SDK_TMP_TARGET}
			PUBLIC
			${SOS_BUILD_SYSTEM_INCLUDES}
			)

		sos_arm_arch(${ARCH_NAME})

		target_compile_definitions(${SOS_SDK_TMP_TARGET}
			PUBLIC
			__StratifyOS__
			)

		target_compile_options(${SOS_SDK_TMP_TARGET}
			PUBLIC
			-mthumb -D__StratifyOS__ -ffunction-sections -fdata-sections -fomit-frame-pointer
			${SOS_ARM_ARCH_BUILD_FLAGS}
			${SOS_ARM_ARCH_BUILD_FLOAT_OPTIONS}
			)

	endif()

	install(FILES ${CMAKE_BINARY_DIR}/lib${SOS_SDK_TMP_TARGET}.a DESTINATION lib/${SOS_ARM_ARCH_BUILD_INSTALL_DIR}/${SOS_ARM_ARCH_BUILD_FLOAT_DIR} RENAME lib${SOS_SDK_TMP_INSTALL}.a)

endfunction()

function(sos_sdk_bsp_target_name OUTPUT BASE_NAME OPTION CONFIG ARCH)
	build_target_name("${BASE_NAME}" "${OPTION}" "${CONFIG}" "${ARCH}")
	set(${OUTPUT} ${SOS_SDK_TMP_INSTALL}.elf PARENT_SCOPE)
endfunction()

function(sos_sdk_bsp BASE_NAME OPTION CONFIG ARCH)
	message(STATUS "SOS SDK BSP ${BASE_NAME}_${OPTION_NAME}_${CONFIG_NAME}_${ARCH_NAME}")
	build_target_name("${BASE_NAME}" "${OPTION}" "${CONFIG}" "${ARCH}")

	set(TARGET_NAME ${SOS_SDK_TMP_INSTALL}.elf)

	target_compile_definitions(${TARGET_NAME}
		PUBLIC
		__${SOS_SDK_TMP_CONFIG}
		__${SOS_SDK_TMP_OPTION}
		__${ARCH}
		MCU_SOS_GIT_HASH=${SOS_GIT_HASH}
		__StratifyOS__
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

	set_target_properties(${TARGET_NAME}
		PROPERTIES
		LINK_FLAGS
		-L${SOS_SDK_PATH}/Tools/gcc/lib/${SOS_ARM_ARCH_BUILD_INSTALL_DIR}/${SOS_ARM_ARCH_BUILD_FLOAT_DIR}
		)

	set_target_properties(${TARGET_NAME}
		PROPERTIES
		LINK_FLAGS
		-L${SOS_SDK_PATH}/Tools/gcc/lib/gcc/arm-none-eabi/${CMAKE_CXX_COMPILER_VERSION}/${SOS_ARM_ARCH_BUILD_INSTALL_DIR}/${SOS_ARM_ARCH_BUILD_FLOAT_DIR})

endfunction()

function(sos_sdk_app TARGET_NAME CONFIG ARCH)

endfunction()


