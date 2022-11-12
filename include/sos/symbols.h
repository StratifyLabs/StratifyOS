// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_SYMBOLS_H_
#define SOS_SYMBOLS_H_

#include <sdk/types.h>

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
extern long long int __muldi3(int,int);
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
// extern u32 __aeabi_atexit();

extern u32 _Unwind_GetRegionStart();
extern u32 _Unwind_GetTextRelBase();
extern u32 _Unwind_GetDataRelBase();
extern u32 _Unwind_VRS_Set();
extern u32 _Unwind_Resume();
extern u32 __gnu_unwind_frame();
extern u32 _Unwind_GetLanguageSpecificData();
extern u32 _Unwind_Complete();
extern u32 _Unwind_DeleteException();
extern u32 _Unwind_RaiseException();
extern u32 _Unwind_Resume_or_Rethrow();
extern u32 _Unwind_VRS_Get();
extern u32 __aeabi_unwind_cpp_pr0();
extern u32 __aeabi_unwind_cpp_pr1();
extern u32 __aeabi_atexit();

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

#include "aio.h"
#include "arpa/inet.h"
#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "device/mem.h"
#include "device/sys.h"
#include "mqueue.h"
#include "semaphore.h"
#include "sos/dev/sys.h"
#include "sos/sos.h"
#include "trace.h"

// These values are defined in the linker script
extern u32 _top_of_stack;
extern u32 _text;
extern u32 _etext;
extern u32 _tcim;
extern u32 _etcim;
extern u32 _data;
extern u32 _edata;
extern u32 _bss;
extern u32 _ebss;
extern u32 _sys;
extern u32 _esys;
extern u32 _sos_hardware_id;

extern void __cxa_pure_virtual();
extern void __cxa_atexit();
extern void __cxa_finalize();

extern void __div0();

useconds_t _EXFUN(ualarm, (useconds_t __useconds, useconds_t __interval));
extern void crt_load_data(void * global_reent, int code_size, int data_size);
extern char ** const crt_import_argv(int argc, char * const argv[]);

#include "sys/socket.h"

#ifndef USE_FLOAT
#define USE_FLOAT 1
#endif

#define SYMBOL_IGNORE 0


#endif /* SOS_SYMBOLS_H_ */
