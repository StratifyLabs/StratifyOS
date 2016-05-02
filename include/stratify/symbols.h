/* Copyright 2011-2016 Tyler Gilbert; 
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

#ifndef STRATIFY_SYMBOLS_H_
#define STRATIFY_SYMBOLS_H_

#include "mcu/types.h"

extern u32 __aeabi_uidiv(void);
extern u32 __aeabi_uidivmod(void);
extern u32 __aeabi_idiv(void);
extern u32 __aeabi_idivmod(void);
extern u32 __umodsi3(void);
extern u32 __modsi3(void);
extern void __div0(void);
extern u32 __aeabi_llsr(void);
extern u32 __aeabi_lasr(void);
extern u32 __aeabi_llsl(void);
extern u32 __aeabi_dneg(void);
extern u32 __aeabi_drsub(void);
extern u32 __aeabi_dsub(void);
extern u32 __adddf3(void);
extern u32 __aeabi_ui2d(void);
extern u32 __aeabi_i2d(void);
extern u32 __aeabi_f2d(void);
extern u32 __aeabi_ul2d(void);
extern u32 __aeabi_l2d(void);
extern u32 __aeabi_dmul(void);
extern u32 __aeabi_ddiv(void);
extern u32 __gedf2(void);
extern u32 __ledf2(void);
extern u32 __cmpdf2(void);
extern u32 __aeabi_cdrcmple(void);
extern u32 __aeabi_cdcmpeq(void);
extern u32 __aeabi_dcmpeq(void);
extern u32 __aeabi_dcmplt(void);
extern u32 __aeabi_dcmple(void);
extern u32 __aeabi_dcmpge(void);
extern u32 __aeabi_dcmpgt(void);
extern u32 __aeabi_dcmpun(void);
extern u32 __aeabi_d2iz(void);
extern u32 __aeabi_d2uiz(void);
extern u32 __aeabi_d2f(void);
extern u32 __aeabi_fneg(void);
extern u32 __aeabi_frsub(void);
extern u32 __aeabi_fsub(void);
extern u32 __aeabi_fadd(void);
extern u32 __addsf3(void);
extern u32 __aeabi_ui2f(void);
extern u32 __aeabi_i2f(void);
extern u32 __aeabi_ul2f(void);
extern u32 __aeabi_l2f(void);
extern u32 __aeabi_fmul(void);
extern u32 __aeabi_fdiv(void);
extern u32 __gesf2(void);
extern u32 __lesf2(void);
extern u32 __cmpsf2(void);
extern u32 __aeabi_cfrcmple(void);
extern u32 __aeabi_cfcmpeq(void);
extern u32 __aeabi_fcmpeq(void);
extern u32 __aeabi_fcmplt(void);
extern u32 __aeabi_fcmple(void);
extern u32 __aeabi_fcmpge(void);
extern u32 __aeabi_fcmpgt(void);
extern u32 __aeabi_fcmpun(void);
extern u32 __aeabi_f2iz(void);
extern u32 __aeabi_f2uiz(void);
extern u32 __aeabi_f2ulz(void);
extern u32 __clzsi2(void);
extern u32 __clzdi2(void);
extern u32 __muldi3(void);
extern u32 __negdi2(void);
extern u32 __cmpdi2(void);
extern u32 __ucmpdi2(void);
extern u32 __absvsi2(void);
extern u32 __absvdi2(void);
extern u32 __addvsi3(void);
extern u32 __addvdi3(void);
extern u32 __subvsi3(void);
extern u32 __subvdi3(void);
extern u32 __mulvsi3(void);
extern u32 __mulvdi3(void);
extern u32 __negvsi2(void);
extern u32 __negvdi2(void);
extern u32 __ffsdi2(void);
extern u32 __popcountsi2(void);
extern u32 __popcountdi2(void);
extern u32 __paritysi2(void);
extern u32 __paritydi2(void);
extern u32 __powisf2(void);
extern u32 __powidf2(void);
extern float _Complex __mulsc3(float __a, float __b, float __c, float __d);
extern double _Complex __muldc3(double __a, double __b, double __c, double __d);
extern float _Complex __divsc3(float __a, float __b, float __c, float __d);
extern double _Complex __divdc3(double __a, double __b, double __c, double __d);
extern u32 __bswapsi2(void);
extern u32 __bswapdi2(void);
extern u32 __fixsfdi(void);
extern u32 __fixdfdi(void);
extern u32 __fixunssfdi(void);
extern u32 __fixunsdfdi(void);
extern u32 __divdi3(void);
extern u32 __moddi3(void);
extern u32 __udivdi3(void);
extern u32 __umoddi3(void);
extern u32 __aeabi_f2lz(void);
extern u32 __aeabi_dadd(void);
extern u32 __aeabi_d2lz(void);
extern u32 __aeabi_d2ulz(void);
extern u32 __gnu_f2h_ieee(void);
extern u32 __gnu_h2f_ieee(void);

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <tgmath.h>
#include <float.h>
#include <signal.h>
#include <dirent.h>
#include <grp.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <errno.h>
#include <locale.h>
#include <malloc.h>
#include <sys/wait.h>
#include <sys/reent.h>
#include <signal.h>
#include <sys/signal.h>
#include <time.h>
#include <sys/times.h>

#include "trace.h"
#include "semaphore.h"
#include "mqueue.h"
#include "aio.h"
#include "../stratify/stratify.h"
#include "mcu/sys.h"
#include <device/sys.h>
#include "mcu/core.h"
#include "mcu/adc.h"
#include "mcu/dac.h"
#include "mcu/mem.h"
#include "mcu/spi.h"
#include "mcu/uart.h"
#include "mcu/usb.h"
#include "mcu/i2c.h"
#include "mcu/pwm.h"
#include "mcu/rtc.h"
#include "mcu/eint.h"
#include "mcu/pio.h"
#include "mcu/qei.h"
#include "mcu/tmr.h"
#include "mcu/task.h"

extern void __cxa_pure_virtual(void);
void __cxa_atexit();
void __cxa_finalize();
extern char __aeabi_unwind_cpp_pr0[0];

extern void __div0(void);

extern void crt_load_data(void * global_reent, int code_size, int data_size);
extern char ** const crt_import_argv(int argc, char * const argv[]);

#include <stfy/son.h>
#include <stfy/mg.h>

#ifndef USE_FLOAT
#define USE_FLOAT 1
#endif

#define SYMBOL_IGNORE 0


#endif /* STRATIFY_SYMBOLS_H_ */
