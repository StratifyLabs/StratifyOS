if(SOS_IS_ARM)
	set(STRATIFYOS_CMAKE_CONFIG_LIST release debug)
	sos_sdk_include_target(StratifyOS_crt "${STRATIFYOS_CMAKE_CONFIG_LIST}")
endif()
