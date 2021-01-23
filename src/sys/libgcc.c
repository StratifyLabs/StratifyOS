// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "sos/arch.h"

#include <complex.h>

#if __FPU_USED == 1
#if 1
/*
 * -W weakens the symbols -- doesn't remove them. It is OK to just do libgcc.a libgcc.a instead of libgcc-hard.a and then link to libgcc rather than libgcc-hard
 *
 *

export PATH=$PATH:/Applications/StratifyLabs-SDK/bin
export CMD="arm-none-eabi-objcopy -W __aeabi_fmul -W __aeabi_dmul -W __aeabi_fdiv -W __aeabi_ddiv -W __aeabi_dadd -W __adddf3 -W __aeabi_fadd -W __addsf3 -W __divdc3 -W __divsc3 -W __muldc3 libgcc.a libgcc-hard.a"
cd /Applications/StratifyLabs-SDK/lib/gcc/arm-none-eabi/6.3.1/thumb/v7e-m/fpv4-sp/hard
$CMD
cd /Applications/StratifyLabs-SDK/lib/gcc/arm-none-eabi/6.3.1/thumb/v7e-m/fpv5-sp/hard
$CMD
cd /Applications/StratifyLabs-SDK/lib/gcc/arm-none-eabi/6.3.1/thumb/v7e-m/fpv5/hard
$CMD

*/

float __aeabi_fmul(float a, float b){
	return a*b;
}

float __aeabi_fdiv(float a, float b){
	return a / b;
}

float __aeabi_fadd(float a, float b){
	return a + b;
}

float __addsf3(float a, float b){
	return a + b;
}

/*
//define use float to calc doubles
double __aeabi_dmul(double a, double b){
	return (float)a * (float)b;
}

double __aeabi_ddiv(double a, double b){
	return (float)a / (float)b;
}

double __aeabi_dadd(double a, double b){
	return (float)a + (float)b;
}

double __adddf3(double a, double b){
	return (float)a + (float)b;
}
*/

complex double __divdc3 (double a, double b, double c, double d){
	return __divsc3((float)a, (float)b, (float)c, (float)d);
}

complex double __muldc3 (double a, double b, double c, double d){
	return __mulsc3((float)a, (float)b, (float)c, (float)d);
}


#endif
#endif

