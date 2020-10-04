set(SDK_PATH $ENV{SOS_SDK_PATH})
set(COMMAND ${SDK_PATH}/Tools/gcc/bin/arm-none-eabi-objcopy)
set(ARGUMENTS "-W __aeabi_fmul -W __aeabi_dmul -W __aeabi_fdiv -W __aeabi_ddiv -W __aeabi_dadd -W __adddf3 -W __aeabi_fadd -W __addsf3 -W __divdc3 -W __divsc3 -W __muldc3 libgcc.a libgcc-hard.a")
set(BASE_DIRECTORY ${SDK_PATH}/Tools/gcc/lib/gcc/arm-none-eabi/8.3.1)
set(DIRECTORY_LIST thumb/v7+fp/hard  thumb/v7e-m+dp/hard thumb/v7e-m+fp/hard)
foreach (DIR ${DIRECTORY_LIST})
	message(STATUS "EXECUTE ${COMMAND} ${ARGUMENTS}")
	set(WORKING_DIRECTORY ${BASE_DIRECTORY}/${DIR})
	message(STATUS "Working directory ${WORKING_DIRECTORY}")
	execute_process(COMMAND ${COMMAND} -W __aeabi_fmul -W __aeabi_dmul -W __aeabi_fdiv -W __aeabi_ddiv -W __aeabi_dadd -W __adddf3 -W __aeabi_fadd -W __addsf3 -W __divdc3 -W __divsc3 -W __muldc3 libgcc.a libgcc-hard.a
		WORKING_DIRECTORY ${WORKING_DIRECTORY}
		)
endforeach(DIR)
