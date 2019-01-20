/* Copyright 2011-2018 Tyler Gilbert; 
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

#ifndef SOS_SYMBOLS_H_
#define SOS_SYMBOLS_H_

#include "mcu/types.h"

extern u32 __aeabi_uldivmod();
extern u32 __aeabi_ldivmod();

extern u32 __aeabi_uidiv();
extern u32 __aeabi_uidivmod();
extern u32 __aeabi_idiv();
extern u32 __aeabi_idivmod();
extern u32 __umodsi3();
extern u32 __modsi3();
extern void __div0();
extern u32 __aeabi_llsr();
extern u32 __aeabi_lasr();
extern u32 __aeabi_llsl();
extern u32 __aeabi_dneg();
extern u32 __aeabi_drsub();
extern u32 __aeabi_dsub();
extern u32 __adddf3();
extern u32 __aeabi_ui2d();
extern u32 __aeabi_i2d();
extern u32 __aeabi_f2d();
extern u32 __aeabi_ul2d();
extern u32 __aeabi_l2d();
extern u32 __aeabi_dmul();
extern u32 __aeabi_ddiv();
extern u32 __gedf2();
extern u32 __ledf2();
extern u32 __cmpdf2();
extern u32 __aeabi_cdrcmple();
extern u32 __aeabi_cdcmpeq();
extern u32 __aeabi_dcmpeq();
extern u32 __aeabi_dcmplt();
extern u32 __aeabi_dcmple();
extern u32 __aeabi_dcmpge();
extern u32 __aeabi_dcmpgt();
extern u32 __aeabi_dcmpun();
extern u32 __aeabi_d2iz();
extern u32 __aeabi_d2uiz();
extern u32 __aeabi_d2f();
extern u32 __aeabi_fneg();
extern u32 __aeabi_frsub();
extern u32 __aeabi_fsub();
extern u32 __aeabi_fadd();
extern u32 __addsf3();
extern u32 __aeabi_ui2f();
extern u32 __aeabi_i2f();
extern u32 __aeabi_ul2f();
extern u32 __aeabi_l2f();
extern u32 __aeabi_fmul();
extern u32 __aeabi_fdiv();
extern u32 __gesf2();
extern u32 __lesf2();
extern u32 __cmpsf2();
extern u32 __aeabi_cfrcmple();
extern u32 __aeabi_cfcmpeq();
extern u32 __aeabi_fcmpeq();
extern u32 __aeabi_fcmplt();
extern u32 __aeabi_fcmple();
extern u32 __aeabi_fcmpge();
extern u32 __aeabi_fcmpgt();
extern u32 __aeabi_fcmpun();
extern u32 __aeabi_f2iz();
extern u32 __aeabi_f2uiz();
extern u32 __aeabi_f2ulz();
extern u32 __clzsi2();
extern u32 __clzdi2();
extern u32 __muldi3();
extern u32 __negdi2();
extern u32 __cmpdi2();
extern u32 __ucmpdi2();
extern u32 __absvsi2();
extern u32 __absvdi2();
extern u32 __addvsi3();
extern u32 __addvdi3();
extern u32 __subvsi3();
extern u32 __subvdi3();
extern u32 __mulvsi3();
extern u32 __mulvdi3();
extern u32 __negvsi2();
extern u32 __negvdi2();
extern u32 __ffsdi2();
extern u32 __popcountsi2();
extern u32 __popcountdi2();
extern u32 __paritysi2();
extern u32 __paritydi2();
extern u32 __powisf2();
extern u32 __powidf2();
extern float _Complex __mulsc3(float __a, float __b, float __c, float __d);
extern double _Complex __muldc3(double __a, double __b, double __c, double __d);
extern float _Complex __divsc3(float __a, float __b, float __c, float __d);
extern double _Complex __divdc3(double __a, double __b, double __c, double __d);
extern u32 __bswapsi2();
extern u32 __bswapdi2();
extern u32 __fixsfdi();
extern u32 __fixdfdi();
extern u32 __fixunssfdi();
extern u32 __fixunsdfdi();
extern u32 __divdi3();
extern u32 __moddi3();
extern u32 __udivdi3();
extern u32 __umoddi3();
extern u32 __aeabi_f2lz();
extern u32 __aeabi_dadd();
extern u32 __aeabi_d2lz();
extern u32 __aeabi_d2ulz();
extern u32 __gnu_f2h_ieee();
extern u32 __gnu_h2f_ieee();

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
#include <netdb.h>

#include "trace.h"
#include "semaphore.h"
#include "mqueue.h"
#include "aio.h"
#include "sos/sos.h"
#include "device/sys.h"
#include "sos/dev/sys.h"
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
#include "cortexm/task.h"
#include "cortexm/cortexm.h"
#include "arpa/inet.h"

extern void __cxa_pure_virtual();
void __cxa_atexit();
void __cxa_finalize();
extern char __aeabi_unwind_cpp_pr0[0];

extern void __div0();

useconds_t _EXFUN(ualarm, (useconds_t __useconds, useconds_t __interval));
extern void crt_load_data(void * global_reent, int code_size, int data_size);
extern char ** const crt_import_argv(int argc, char * const argv[]);

extern void sg_api();
extern void son_api();

#include "sys/socket.h"

#ifndef USE_FLOAT
#define USE_FLOAT 1
#endif

#define SYMBOL_IGNORE 0


#endif /* SOS_SYMBOLS_H_ */
