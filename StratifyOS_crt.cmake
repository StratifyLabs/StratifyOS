if(CMSDK_IS_ARM)
	set(STRATIFYOS_CMAKE_CONFIG_LIST release debug)
	cmsdk_include_target(StratifyOS_crt "${STRATIFYOS_CMAKE_CONFIG_LIST}")
endif()
