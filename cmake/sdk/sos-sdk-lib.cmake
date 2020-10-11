function(sos_sdk_library_target OUTPUT BASE_NAME OPTION CONFIG ARCH)
	sos_sdk_internal_build_target_name("${BASE_NAME}" "${OPTION}" "${CONFIG}" "${ARCH}")
	set(${OUTPUT}_OPTIONS "${BASE_NAME};${OPTION};${CONFIG};${ARCH}" PARENT_SCOPE)
	set(${OUTPUT}_TARGET ${SOS_SDK_TMP_TARGET} PARENT_SCOPE)
endfunction()

function(sos_sdk_library_add_arch_targets OPTION_LIST ARCH DEPENDENCIES)

	string(COMPARE EQUAL ${ARCH} link IS_LINK)

	list(GET OPTION_LIST 0 BASE_NAME)
	list(GET OPTION_LIST 1 OPTION)
	list(GET OPTION_LIST 2 CONFIG)

	if(IS_LINK)

		sos_sdk_library_target(BUILD ${BASE_NAME} "${OPTION}" "${CONFIG}" link)

		sos_sdk_library("${OPTION_LIST}")

		foreach(DEPENDENCY ${DEPENDENCIES})
			target_link_libraries(${BUILD_TARGET}
				PUBLIC
				${DEPENDENCY}_${CONFIG}_${ARCH}
				)

		endforeach()

		get_target_property(LIBS ${BUILD_TARGET} LINK_LIBRARIES)
		message("LIBS ARE ${LIBS} for ${BUILD_TARGET}")

	else()

		set(ARCH_LIST v7em v7em_f4sh v7em_f5sh v7em_f5dh)

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

				foreach(DEPENDENCY ${DEPENDENCIES})
					message("-Adding ${DEPENDENCY}_${CONFIG}_${ARCH}")
					target_link_libraries(${BUILD_TARGET}
						PUBLIC
						${DEPENDENCY}_${CONFIG}_${ARCH}
						)
				endforeach()

				# this applies architecture specific options
				sos_sdk_library("${BUILD_OPTIONS}")
			endif()
		endforeach(ARCH)
		sos_sdk_library("${BUILD_V7M_OPTIONS}")

		foreach(DEPENDENCY ${DEPENDENCIES})
			target_link_libraries(${BUILD_V7M_TARGET}
				PUBLIC
				${DEPENDENCY}_${CONFIG}_v7m
				)
		endforeach()
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
		PRIVATE
		__${ARCH}
		___${SOS_SDK_TMP_CONFIG}
		__${SOS_SDK_TMP_OPTION}
		MCU_SOS_GIT_HASH=${SOS_GIT_HASH}
		)

	if(SOS_IS_ARM)

		#target_include_directories(${SOS_SDK_TMP_TARGET}
		#	PRIVATE
		#	${SOS_BUILD_SYSTEM_INCLUDES}
		#	)

		sos_sdk_internal_arm_arch(${ARCH})

		target_compile_definitions(${SOS_SDK_TMP_TARGET}
			PUBLIC
			__StratifyOS__
			)

		target_compile_options(${SOS_SDK_TMP_TARGET}
			PRIVATE
			-mthumb -ffunction-sections -fdata-sections -fomit-frame-pointer
			${SOS_ARM_ARCH_BUILD_FLOAT_OPTIONS}
			)

		set_target_properties(${SOS_SDK_TMP_TARGET}
			PROPERTIES NO_SYSTEM_FROM_IMPORTED TRUE
			)

	endif()

	get_target_property(TARGET_BINARY_DIR ${SOS_SDK_TMP_TARGET} BINARY_DIR)

	install(TARGETS ${SOS_SDK_TMP_TARGET} EXPORT ${SOS_SDK_TMP_TARGET} DESTINATION lib)
	install(EXPORT ${SOS_SDK_TMP_TARGET} DESTINATION cmake/targets)

endfunction()
