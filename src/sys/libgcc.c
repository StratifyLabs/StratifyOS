/* Copyright 2011-2017 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */
#include "mcu/arch.h"

#include <complex.h>

#if __FPU_USED == 1
#if 1
/*
export PATH=$PATH:/Applications/StratifyLabs-SDK/Tools/gcc/bin
cd /Applications/StratifyLabs-SDK/Tools/gcc/lib/gcc/arm-none-eabi/6.3.1/thumb/v7e-m/fpv4-sp/hard
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

