function(sos_sdk_library_target OUTPUT BASE_NAME OPTION CONFIG ARCH)
	sos_sdk_internal_build_target_name("${BASE_NAME}" "${OPTION}" "${CONFIG}" "${ARCH}")
	set(${OUTPUT}_OPTIONS "${BASE_NAME};${OPTION};${CONFIG};${ARCH}" PARENT_SCOPE)
	set(${OUTPUT}_TARGET ${SOS_SDK_TMP_TARGET} PARENT_SCOPE)
endfunction()

function(sos_sdk_library_add_arch_targets OPTION_LIST ARCH)

	string(COMPARE EQUAL ${ARCH} link IS_LINK)

	if(IS_LINK)
		sos_sdk_library("${OPTION_LIST}")
	else()

		set(ARCH_LIST v7em v7em_f4sh v7em_f5sh v7em_f5dh)

		list(GET OPTION_LIST 0 BASE_NAME)
		list(GET OPTION_LIST 1 OPTION)
		list(GET OPTION_LIST 2 CONFIG)

		sos_sdk_library_target(BUILD_V7M ${BASE_NAME} "${OPTION}" "${CONFIG}" v7m)

		foreach (ARCH ${ARCH_LIST})
			sos_sdk_internal_is_arch_enabled(${ARCH})
			if(ARCH_ENABLED)
				set(TARGET_NAME ${BASE_NAME})
				if(NOT OPTION STREQUAL "")
					set(TARGET_NAME ${TARGET_NAME}_${OPTION})
				endif()

				sos_sdk_library_target(BUILD ${BASE_NAME} "${OPTION}" "${CONFIG}" ${ARCH})

				add_library(${BUILD_TARGET} STATIC)
				sos_sdk_copy_target(
					${BUILD_V7M_TARGET}
					${BUILD_TARGET}
					)
				# this applies architecture specific options
				sos_sdk_library("${BUILD_OPTIONS}")
			endif()
		endforeach(ARCH)
		sos_sdk_library("${BUILD_V7M_OPTIONS}")
	endif()
endfunction()

function(sos_sdk_library OPTION_LIST)
	list(GET OPTION_LIST 0 BASE_NAME)
	list(GET OPTION_LIST 1 OPTION)
	list(GET OPTION_LIST 2 CONFIG)
	list(GET OPTION_LIST 3 ARCH)

	sos_sdk_internal_build_target_name(${BASE_NAME} "${OPTION}" "${CONFIG}" ${ARCH})
	sos_sdk_internal_arm_arch(${ARCH})

	message(STATUS "SOS SDK Library ${SOS_SDK_TMP_TARGET}")

	target_compile_definitions(${SOS_SDK_TMP_TARGET}
		PUBLIC
		___${SOS_SDK_TMP_CONFIG}
		__${SOS_SDK_TMP_OPTION}
		__${ARCH}
		MCU_SOS_GIT_HASH=${SOS_GIT_HASH}
		)

	if(NOT ARCH STREQUAL "link")

		target_include_directories(${SOS_SDK_TMP_TARGET}
			PUBLIC
			${SOS_BUILD_SYSTEM_INCLUDES}
			)

		sos_sdk_internal_arm_arch(${ARCH})

		target_compile_definitions(${SOS_SDK_TMP_TARGET}
			PUBLIC
			__StratifyOS__
			)

		target_compile_options(${SOS_SDK_TMP_TARGET}
			PUBLIC
			-mthumb -D__StratifyOS__ -ffunction-sections -fdata-sections -fomit-frame-pointer
			${SOS_ARM_ARCH_BUILD_FLOAT_OPTIONS}
			)

	elseif()

		target_compile_options(${SOS_SDK_TMP_TARGET}
			PUBLIC
			${SOS_TOOLCHAIN_COMPILE_OPTIONS}
			)

	endif()

	get_target_property(TARGET_BINARY_DIR ${SOS_SDK_TMP_TARGET} BINARY_DIR)

	install(FILES ${TARGET_BINARY_DIR}/lib${SOS_SDK_TMP_TARGET}.a DESTINATION lib/${SOS_ARM_ARCH_BUILD_INSTALL_DIR}/${SOS_ARM_ARCH_BUILD_FLOAT_DIR} RENAME lib${SOS_SDK_TMP_INSTALL}.a)

endfunction()
