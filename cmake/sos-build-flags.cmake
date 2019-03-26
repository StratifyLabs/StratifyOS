
# v7m or v7em for M3 or M4 ISA
# fxyz x = 4 or 5 for FPU version, y = s or d for single or double, z = s or h for soft or hard
#CM3: v7m (no FPU)
#CM4: v7em, v7em_f4ss, v7em_f4sh
#CM7: v7em, v7em_f5ss, v7em_f5sh, v7em_f5ds, v7em_f5dh
#These are values for SOS_ARCH


set(SOS_BUILD_FLOAT_OPTIONS_V7M -march=armv7-m -DARM_MATH_CM3=1)
set(SOS_BUILD_FLOAT_OPTIONS_V7EM -march=armv7e-m -DARM_MATH_CM4=1)
set(SOS_BUILD_FLOAT_OPTIONS_V7EM_F4SS -march=armv7e-m -mfloat-abi=soft -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_PRESENT=1 -DARM_MATH_CM4=1)
set(SOS_BUILD_FLOAT_OPTIONS_V7EM_F4SH -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_PRESENT=1 -DARM_MATH_CM4=1)
set(SOS_BUILD_FLOAT_OPTIONS_V7EM_F5SS -march=armv7e-m -mfloat-abi=soft -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_PRESENT=1 -DARM_MATH_CM4=1)
set(SOS_BUILD_FLOAT_OPTIONS_V7EM_F5SH -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_PRESENT=1 -DARM_MATH_CM4=1)
set(SOS_BUILD_FLOAT_OPTIONS_V7EM_F5DS -march=armv7e-m -mfloat-abi=soft -mfpu=fpv5-d16 -U__SOFTFP__ -D__FPU_PRESENT=1 -DARM_MATH_CM7=1)
set(SOS_BUILD_FLOAT_OPTIONS_V7EM_F5DH -march=armv7e-m -mfloat-abi=hard -mfpu=fpv5-d16 -U__SOFTFP__ -D__FPU_PRESENT=1 -DARM_MATH_CM7=1)

set(SOS_BUILD_FLOAT_DIR_V7M ".")
set(SOS_BUILD_FLOAT_DIR_V7EM ".")
set(SOS_BUILD_FLOAT_DIR_V7EM_F4SS "fpv4-sp/softfp") #single precision soft ABI
set(SOS_BUILD_FLOAT_DIR_V7EM_F4SH "fpv4-sp/hard")   #single precision hard ABI
set(SOS_BUILD_FLOAT_DIR_V7EM_F5SS "fpv5-sp/softfp") #single precision soft ABI
set(SOS_BUILD_FLOAT_DIR_V7EM_F5SH "fpv5-sp/hard")   #single precision hard ABI
set(SOS_BUILD_FLOAT_DIR_V7EM_F5DS "fpv5/softfp")    #double precision soft ABI
set(SOS_BUILD_FLOAT_DIR_V7EM_F5DH "fpv5/hard")      #double precision hard ABI

set(SOS_BUILD_INSTALL_DIR_V7M "thumb/v7-m")         #thumb ARMV7M
set(SOS_BUILD_INSTALL_DIR_V7EM "thumb/v7e-m")       #thumb ARMV7EM

string(COMPARE EQUAL "${BUILD_ARCH}" v7m IS_V7M)
string(COMPARE EQUAL "${BUILD_ARCH}" v7em IS_V7EM)
string(COMPARE EQUAL "${BUILD_ARCH}" v7em_f4ss IS_V7EM_F4SS)
string(COMPARE EQUAL "${BUILD_ARCH}" v7em_f4sh IS_V7EM_F4SH)
string(COMPARE EQUAL "${BUILD_ARCH}" v7em_f5ss IS_V7EM_F5SS)
string(COMPARE EQUAL "${BUILD_ARCH}" v7em_f5sh IS_V7EM_F5SH)
string(COMPARE EQUAL "${BUILD_ARCH}" v7em_f5ds IS_V7EM_F5DS)
string(COMPARE EQUAL "${BUILD_ARCH}" v7em_f5dh IS_V7EM_F5DH)

set(IS_NOT_LINK 1)

if(IS_V7M) #armv7m soft float
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7M})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7M})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7M})
	set(SOS_BUILD_GCC_LIB gcc)
elseif(IS_V7EM) #armv7em soft float
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7EM})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7EM})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7EM})
	set(SOS_BUILD_GCC_LIB gcc)
elseif(IS_V7EM_F4SS) #armv7em fpu4 single precision soft abi
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7EM})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7EM_F4SS})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7EM_F4SS})
	set(SOS_BUILD_GCC_LIB gcc)
elseif(IS_V7EM_F4SH) #armv7em fpu4 single precision hard abi
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7EM})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7EM_F4SH})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7EM_F4SH})
	set(SOS_BUILD_GCC_LIB gcc-hard)
elseif(IS_V7EM_F5SS) #armv7em fpu5 single precision soft abi
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7EM})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7EM_F5SS})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7EM_F5SS})
	set(SOS_BUILD_GCC_LIB gcc)
elseif(IS_V7EM_F5SH) #armv7em fpu5 single precision hard abi
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7EM})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7EM_F5SH})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7EM_F5SH})
	set(SOS_BUILD_GCC_LIB gcc-hard)
elseif(IS_V7EM_F5DS) #armv7em fpu5 double precision soft abi
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7EM})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7EM_F5DS})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7EM_F5DS})
	set(SOS_BUILD_GCC_LIB gcc)
elseif(IS_V7EM_F5DH) #armv7em fpu5 double precision hard abi
	set(SOS_BUILD_INSTALL_DIR ${SOS_BUILD_INSTALL_DIR_V7EM})
	set(SOS_BUILD_FLOAT_DIR ${SOS_BUILD_FLOAT_DIR_V7EM_F5DH})
	set(SOS_BUILD_FLOAT_OPTIONS ${SOS_BUILD_FLOAT_OPTIONS_V7EM_F5DH})
	set(SOS_BUILD_GCC_LIB gcc-hard)
elseif( BUILD_ARCH STREQUAL link )
	set(SOS_BUILD_INSTALL_DIR ".")
	set(SOS_BUILD_FLOAT_DIR ".")
	set(SOS_BUILD_FLOAT_OPTIONS "")
	set(IS_NOT_LINK 0)
endif()

if(IS_NOT_LINK)
	set(SOS_BUILD_SYSTEM_INCLUDES
		${CMAKE_INSTALL_PREFIX}/../lib/gcc/arm-none-eabi/${CMAKE_CXX_COMPILER_VERSION}/include
		${CMAKE_INSTALL_PREFIX}/../lib/gcc/arm-none-eabi/${CMAKE_CXX_COMPILER_VERSION}/include-fixed
		${CMAKE_INSTALL_PREFIX}/include
		${CMAKE_INSTALL_PREFIX}/include/c++/${CMAKE_CXX_COMPILER_VERSION}
		${CMAKE_INSTALL_PREFIX}/include/c++/${CMAKE_CXX_COMPILER_VERSION}/${TOOLCHAIN_HOST}
		)
endif()
