/*
 * libgcc.c
 *
 *  Created on: Dec 14, 2014
 *      Author: tgil
 */

#include "mcu/arch.h"

#include <complex.h>

#if __FPU_USED == 1
#if 1
/*
export PATH=$PATH:/Applications/StratifyLabs-SDK/Tools/gcc/bin
cd /Applications/StratifyLabs-SDK/Tools/gcc/lib/gcc/arm-none-eabi/5.4.1/armv7e-m/fpu/fpv5-sp-d16
arm-none-eabi-objcopy \
	-W __aeabi_fmul \
	-W __aeabi_dmul \
	-W __aeabi_fdiv \
	-W __aeabi_ddiv \
	-W __aeabi_dadd \
	-W __adddf3 \
	-W __aeabi_fadd \
	-W __addsf3 \
	-W __divdc3 \
	-W __divsc3 \
	-W __muldc3 \
	libgcc.a \
	libgcc-hard.a
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

