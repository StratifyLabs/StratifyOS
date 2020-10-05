set(COMMAND "${SOS_SDK_PATH}/Tools/gcc/bin/arm-none-eabi-objcopy -W __aeabi_fmul -W __aeabi_dmul -W __aeabi_fdiv -W __aeabi_ddiv -W __aeabi_dadd -W __adddf3 -W __aeabi_fadd -W __addsf3 -W __divdc3 -W __divsc3 -W __muldc3 libgcc.a libgcc-hard.a")
set(DIRECTORY_LIST thumb/v7+fp/hard  thumb/v7e-m+dp/hard thumb/v7e-m+fp/hard)
foreach (DIR ${DIRECTORY_LIST})
	execute_process(COMMAND
		${COMMAND}
		WORKING_DIRECTORY
		# this needs to be the compiler internal lib
		${SOS_SDK_PATH}/Tools/gcc/lib/8.3.1/arm-non-eabi/lib/${DIR}
		)
endforeach(DIR)
