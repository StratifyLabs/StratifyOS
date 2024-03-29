// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_SYMBOLS_TABLE_H_
#define SOS_SYMBOLS_TABLE_H_

#include <assert.h>
#include <sdk/types.h>

#include "sos/arch.h"

#include "arpa/inet.h"
#include "sos/fs.h"
#include "sos/power.h"
#include "sos/process.h"
#include "sos/sos.h"

#include "defines.h"

#if !defined SYMBOLS_TABLE
#define SYMBOLS_TABLE_WEAK MCU_WEAK
#else
#define SYMBOLS_TABLE_WEAK
#endif

extern int seteuid(uid_t uid);

u32 const symbols_table[] SYMBOLS_TABLE_WEAK;
u32 const symbols_table[] = {
  // The first position is the signature
  (u32)0x00000410 + __CORTEX_M, (u32)__cxa_pure_virtual, (u32)&_global_impure_ptr,
  (u32)&_impure_ptr, (u32)&__sf_fake_stdin, (u32)&__sf_fake_stdout,
  (u32)&__sf_fake_stderr, (u32)__aeabi_uldivmod, (u32)__aeabi_ldivmod, (u32)__aeabi_uidiv,
  (u32)__aeabi_uidivmod, (u32)__aeabi_idiv, (u32)__aeabi_idivmod, (u32)__umodsi3,
  (u32)__modsi3, (u32)__div0, (u32)__aeabi_llsr, (u32)__aeabi_lasr, (u32)__aeabi_llsl,
  (u32)__clzsi2, (u32)__clzdi2, (u32)__muldi3, (u32)__negdi2, (u32)__cmpdi2,
  (u32)__ucmpdi2, (u32)__absvsi2, (u32)__absvdi2, (u32)__addvsi3, (u32)__addvdi3,
  (u32)__subvsi3, (u32)__subvdi3, (u32)__mulvsi3, (u32)__mulvdi3, (u32)__negvsi2,
  (u32)__negvdi2, (u32)__ffsdi2, (u32)__popcountsi2, (u32)__popcountdi2, (u32)__paritysi2,
  (u32)__paritydi2, (u32)__mulsc3, (u32)__muldc3, (u32)__divsc3, (u32)__divdc3,
  (u32)__bswapsi2, (u32)__bswapdi2, (u32)__divdi3, (u32)__moddi3, (u32)__udivdi3,
  (u32)__umoddi3, (u32)abort, (u32)asctime, (u32)asctime_r, (u32)exit, (u32)atexit,
  (u32)atoi, (u32)atol, (u32)bsearch, (u32)clearerr, (u32)clock, (u32)ctime,
  (u32)difftime, (u32)div, (u32)__errno, (u32)fclose, (u32)feof, (u32)ferror, (u32)fflush,
  (u32)fgetc, (u32)fgetpos, (u32)fgets, (u32)fileno, (u32)fopen, (u32)fputc, (u32)fputs,
  (u32)fread, (u32)freopen, (u32)fscanf, (u32)fseek, (u32)fsetpos, (u32)ftell,
  (u32)fwrite, (u32)getc, (u32)getchar, (u32)getenv, (u32)gets, (u32)gmtime,
  (u32)gmtime_r, (u32)iswalnum, (u32)iswalpha, (u32)iswblank, (u32)iswcntrl,
  (u32)iswctype, (u32)iswdigit, (u32)iswgraph, (u32)iswlower, (u32)iswprint,
  (u32)iswpunct, (u32)iswspace, (u32)iswupper, (u32)iswxdigit, (u32)labs, (u32)localtime,
  (u32)localtime_r, (u32)ldiv, (u32)lldiv, (u32)mblen, (u32)mbstowcs, (u32)mbtowc,
  (u32)memchr, (u32)memcmp, (u32)memcpy, (u32)memmove, (u32)memset, (u32)mktime,
  (u32)perror, (u32)putc, (u32)putchar, (u32)puts, (u32)qsort, (u32)raise, (u32)srand,
  (u32)rand, (u32)remove, (u32)rename, (u32)rewind, (u32)scanf, (u32)setbuf, (u32)setvbuf,
  (u32)sscanf, (u32)strcat, (u32)strchr, (u32)strcmp, (u32)strcoll, (u32)strcpy,
  (u32)strcspn, (u32)strerror, (u32)strftime, (u32)strlen, (u32)strncat, (u32)strncmp,
  (u32)strncpy, (u32)strnlen, (u32)strpbrk, (u32)strptime, (u32)strrchr, (u32)strspn,
  (u32)strstr, (u32)strtod, (u32)strtof, (u32)strtok, (u32)strtok_r, (u32)strtol,
  (u32)strtoll, (u32)strtoul, (u32)strtoull, (u32)strxfrm, (u32)close, (u32)execve,
  (u32)fcntl, (u32)fstat, (u32)getpid, (u32)gettimeofday, (u32)kill, (u32)link,
  (u32)lseek, (u32)open, (u32)fsync, (u32)read, (u32)stat, (u32)system, (u32)times,
  (u32)unlink, (u32)wait, (u32)write, (u32)time, (u32)towctrans, (u32)towlower,
  (u32)towupper, (u32)ungetc, (u32)vfscanf, (u32)vscanf, (u32)vsscanf, (u32)wcscasecmp,
  (u32)wcscmp, (u32)wcsncasecmp, (u32)wcsncmp, (u32)wcstombs, (u32)wctomb, (u32)wctrans,
  (u32)wctype, (u32)fputwc, (u32)setlocale, (u32)localeconv, (u32)mbrtowc,
  (u32)strncasecmp, (u32)wcrtomb, (u32)aio_cancel, (u32)aio_error, (u32)aio_fsync,
  (u32)aio_read, (u32)aio_write, (u32)aio_return, (u32)aio_suspend, (u32)lio_listio,
  (u32)mkfs, (u32)sched_get_priority_max, (u32)sched_get_priority_min,
  (u32)sched_getparam, (u32)sched_getscheduler, (u32)sched_rr_get_interval,
  (u32)sched_setparam, (u32)sched_setscheduler, (u32)sched_yield, (u32)access, (u32)ioctl,
  (u32)mkdir, (u32)rmdir, (u32)sleep, (u32)usleep, (u32)chmod, (u32)mallinfo, (u32)malloc,
  (u32)realloc, (u32)free, (u32)calloc, (u32)_malloc_r, (u32)_free_r,
  (u32)clock_getcpuclockid, (u32)clock_gettime, (u32)clock_getres, (u32)clock_settime,
  (u32)hibernate, (u32)powerdown, (u32)pthread_create, (u32)pthread_join,
  (u32)pthread_getschedparam, (u32)pthread_setschedparam, (u32)pthread_cancel,
  (u32)pthread_cond_init, (u32)pthread_cond_destroy, (u32)pthread_cond_broadcast,
  (u32)pthread_cond_signal, (u32)pthread_cond_wait, (u32)pthread_cond_timedwait,
  (u32)pthread_condattr_init, (u32)pthread_condattr_destroy,
  (u32)pthread_condattr_getpshared, (u32)pthread_condattr_setpshared,
  (u32)pthread_condattr_getclock, (u32)pthread_condattr_setclock,
  (u32)pthread_attr_getdetachstate, (u32)pthread_attr_setdetachstate,
  (u32)pthread_attr_getguardsize, (u32)pthread_attr_setguardsize,
  (u32)pthread_attr_getinheritsched, (u32)pthread_attr_setinheritsched,
  (u32)pthread_attr_getschedparam, (u32)pthread_attr_setschedparam,
  (u32)pthread_attr_getschedpolicy, (u32)pthread_attr_setschedpolicy,
  (u32)pthread_attr_getscope, (u32)pthread_attr_setscope, (u32)pthread_attr_getstacksize,
  (u32)pthread_attr_setstacksize, (u32)pthread_attr_getstackaddr,
  (u32)pthread_attr_setstackaddr, (u32)pthread_attr_init, (u32)pthread_attr_destroy,
  (u32)pthread_mutexattr_getprioceiling, (u32)pthread_mutexattr_setprioceiling,
  (u32)pthread_mutexattr_getprotocol, (u32)pthread_mutexattr_setprotocol,
  (u32)pthread_mutexattr_getpshared, (u32)pthread_mutexattr_setpshared,
  (u32)pthread_mutexattr_gettype, (u32)pthread_mutexattr_settype,
  (u32)pthread_mutexattr_init, (u32)pthread_mutexattr_destroy, (u32)pthread_mutex_init,
  (u32)pthread_mutex_lock, (u32)pthread_mutex_trylock, (u32)pthread_mutex_unlock,
  (u32)pthread_mutex_destroy, (u32)pthread_mutex_getprioceiling,
  (u32)pthread_mutex_setprioceiling, (u32)pthread_self, (u32)signal, (u32)sigaction,
  (u32)pthread_sigmask, (u32)sigprocmask, (u32)sigpending, (u32)sigsuspend,
  (u32)pthread_kill, (u32)sigqueue, (u32)sigwait, (u32)sigtimedwait, (u32)sigwaitinfo,
  (u32)waitpid, (u32)_wait, (u32)sem_init, (u32)sem_destroy, (u32)sem_getvalue,
  (u32)sem_open, (u32)sem_post, (u32)sem_timedwait, (u32)sem_trywait, (u32)sem_unlink,
  (u32)sem_close, (u32)sem_wait, (u32)mq_getattr, (u32)mq_setattr, (u32)mq_open,
  (u32)mq_close, (u32)mq_unlink, (u32)mq_notify, (u32)mq_timedreceive, (u32)mq_receive,
  (u32)mq_timedsend, (u32)mq_send, (u32)_exit, (u32)chown, (u32)closedir, (u32)opendir,
  (u32)readdir_r, (u32)readdir, (u32)rewinddir, (u32)seekdir, (u32)telldir, (u32)fprintf,
  (u32)printf, (u32)snprintf, (u32)vfprintf, (u32)sprintf, (u32)vprintf, (u32)vsnprintf,
  (u32)vsprintf, (u32)asinhf, (u32)atanf, (u32)cbrtf, (u32)ceilf, (u32)copysignf,
  (u32)cosf, (u32)erff, (u32)erfcf, (u32)fabsf, (u32)fdimf, (u32)floorf, (u32)fmaf,
  (u32)fmaxf, (u32)fminf, (u32)frexpf, (u32)ilogbf, (u32)ldexpf, (u32)llrintf,
  (u32)llroundf, (u32)log1pf, (u32)log2f, (u32)logbf, (u32)lroundf, (u32)modff, (u32)nanf,
  (u32)nearbyintf, (u32)nextafterf, (u32)remquof, (u32)rintf, (u32)roundf, (u32)scalblnf,
  (u32)scalbnf, (u32)sinf, (u32)tanf, (u32)tanhf, (u32)truncf, (u32)acosf, (u32)acoshf,
  (u32)asinf, (u32)atan2f, (u32)atanhf, (u32)coshf, (u32)expf, (u32)exp2f, (u32)fmodf,
  (u32)hypotf, (u32)lgammaf, (u32)logf, (u32)log10f, (u32)powf, (u32)remainderf,
  (u32)sinhf, (u32)sqrtf, (u32)tgammaf, (u32)expm1f, (u32)finitef, (u32)matherr, (u32)abs,
  (u32)atoff,
  (u32)__aeabi_fneg,   // alias:.global __negsf2; __negsf2 = __aeabi_fneg;
  (u32)__aeabi_fsub,   // alias:.global __subsf3; __subsf3 = __aeabi_fsub;
  (u32)__aeabi_fadd,   // alias:.global __addsf3; __addsf3 = __aeabi_fadd;
  (u32)__aeabi_ui2f,   // alias:.global __floatunsisf; __floatunsisf = __aeabi_ui2f;
  (u32)__aeabi_i2f,    // alias:.global __floatsisf; __floatsisf = __aeabi_i2f;
  (u32)__aeabi_ul2f,   // alias:.global __floatundisf; __floatundisf = __aeabi_ul2f;
  (u32)__aeabi_l2f,    // alias:.global __floatdisf; __floatdisf = __aeabi_l2f;
  (u32)__aeabi_fmul,   // alias:.global __mulsf3; __mulsf3 = __aeabi_fmul;
  (u32)__aeabi_fdiv,   // alias:.global __divsf3; __divsf3 = __aeabi_fdiv;
  (u32)__aeabi_fcmpun, // alias:.global __unordsf2; __unordsf2 = __aeabi_fcmpun;
  (u32)__aeabi_f2iz,   // alias:.global __fixsfsi; __fixsfsi = __aeabi_f2iz;
  (u32)__aeabi_f2uiz,  // alias:.global __fixunssfsi; __fixunssfsi = __aeabi_f2uiz;
  (u32)__aeabi_f2lz,   // alias:.global __fixsfdi; __fixsfdi = __aeabi_f2lz;
  (u32)__aeabi_f2ulz,  // alias:.global __fixunssfdi; __fixunssfdi = __aeabi_f2ulz;
  (u32)__aeabi_frsub,
  (u32)__gesf2,  // alias:.global __gtsf2; __gtsf2 = __gesf2;
  (u32)__lesf2,  // alias:.global __ltsf2; __ltsf2 = __lesf2;
  (u32)__cmpsf2, // alias:.global __nesf2; __nesf2 = __cmpsf2; //alias:.global __eqsf2;
                 // __eqsf2 = __cmpsf2;
  (u32)__aeabi_cfrcmple, (u32)__aeabi_cfcmpeq, (u32)__aeabi_fcmpeq, (u32)__aeabi_fcmplt,
  (u32)__aeabi_fcmple, (u32)__aeabi_fcmpge, (u32)__aeabi_fcmpgt, (u32)__powisf2,
  (u32)__aeabi_dneg,   // alias:.global __negdf2; __negdf2 = __aeabi_dneg;
  (u32)__aeabi_dsub,   // alias:.global __subdf3; __subdf3 = __aeabi_dsub;
  (u32)__aeabi_dadd,   // alias:.global __adddf3; __adddf3 = __aeabi_dadd;
  (u32)__aeabi_ui2d,   // alias:.global __floatunsidf; __floatunsidf = __aeabi_ui2d;
  (u32)__aeabi_i2d,    // alias:.global __floatsidf; __floatsidf = __aeabi_i2d;
  (u32)__aeabi_f2d,    // alias:.global __extendsfdf2; __extendsfdf2 = __aeabi_f2d;
  (u32)__aeabi_d2f,    // alias:.global __truncdfsf2; __truncdfsf2 = __aeabi_d2f;
  (u32)__aeabi_ul2d,   // alias:.global __floatundidf; __floatundidf = __aeabi_ul2d;
  (u32)__aeabi_l2d,    // alias:.global __floatdidf; __floatdidf = __aeabi_l2d;
  (u32)__aeabi_dmul,   // alias:.global __muldf3; __muldf3 = __aeabi_dmul;
  (u32)__aeabi_ddiv,   // alias:.global __divdf3; __divdf3 = __aeabi_ddiv;
  (u32)__aeabi_dcmpun, // alias:.global __unorddf2; __unorddf2 = __aeabi_dcmpun;
  (u32)__aeabi_d2iz,   // alias:.global __fixdfsi; __fixdfsi = __aeabi_d2iz;
  (u32)__aeabi_d2uiz,  // alias:.global __fixunsdfsi; __fixunsdfsi = __aeabi_d2uiz;
  (u32)__aeabi_d2lz,   // alias:.global __fixdfdi; __fixdfdi = __aeabi_d2lz;
  (u32)__aeabi_d2ulz,  // alias:.global __fixunsdfdi; __fixunsdfdi = __aeabi_d2ulz;
  (u32)__gnu_h2f_ieee, // alias:.global __extendhfsf2; __extendhfsf2 = __gnu_h2f_ieee;
  (u32)__gnu_f2h_ieee, // alias:.global __truncsfhf2; __truncsfhf2 = __gnu_f2h_ieee;
  (u32)__aeabi_drsub,
  (u32)__gedf2,  // alias:.global __gtdf2; __gtdf2 = __gedf2;
  (u32)__ledf2,  // alias:.global __ltdf2; __ltdf2 = __ledf2;
  (u32)__cmpdf2, // alias:.global __nedf2; __nedf2 = __cmpdf2; //alias:.global __eqdf2;
                 // __eqdf2 = __cmpdf2;
  (u32)__aeabi_cdrcmple, //__aeabi_cdrcmple
  (u32)__aeabi_cdcmpeq, (u32)__aeabi_dcmpeq, (u32)__aeabi_dcmplt, (u32)__aeabi_dcmple,
  (u32)__aeabi_dcmpge, (u32)__aeabi_dcmpgt, (u32)__powidf2, (u32)devfs_signal_callback,
  (u32)__sinit, (u32)task_setstackguard, (u32)crt_import_argv, (u32)crt_load_data,
  (u32)posix_trace_attr_destroy, (u32)posix_trace_attr_getclockres,
  (u32)posix_trace_attr_getcreatetime, (u32)posix_trace_attr_getgenversion,
  (u32)posix_trace_attr_getinherited, (u32)posix_trace_attr_getlogfullpolicy,
  (u32)posix_trace_attr_getlogsize, (u32)posix_trace_attr_getmaxdatasize,
  (u32)posix_trace_attr_getmaxsystemeventsize, (u32)posix_trace_attr_getmaxusereventsize,
  (u32)posix_trace_attr_getname, (u32)posix_trace_attr_getstreamfullpolicy,
  (u32)posix_trace_attr_getstreamsize, (u32)posix_trace_attr_init,
  (u32)posix_trace_attr_setinherited, (u32)posix_trace_attr_setlogfullpolicy,
  (u32)posix_trace_attr_setlogsize, (u32)posix_trace_attr_setmaxdatasize,
  (u32)posix_trace_attr_setname, (u32)posix_trace_attr_setstreamsize,
  (u32)posix_trace_attr_setstreamfullpolicy, (u32)posix_trace_clear,
  (u32)posix_trace_close, (u32)posix_trace_create, (u32)posix_trace_create_withlog,
  (u32)posix_trace_event, (u32)posix_trace_eventid_equal,
  (u32)posix_trace_eventid_get_name, (u32)posix_trace_eventid_open,
  (u32)posix_trace_eventset_add, (u32)posix_trace_eventset_del,
  (u32)posix_trace_eventset_empty, (u32)posix_trace_eventset_fill,
  (u32)posix_trace_eventset_ismember, (u32)posix_trace_eventtypelist_getnext_id,
  (u32)posix_trace_eventtypelist_rewind, (u32)posix_trace_flush,
  (u32)posix_trace_get_attr, (u32)posix_trace_get_filter, (u32)posix_trace_get_status,
  (u32)posix_trace_getnext_event, (u32)posix_trace_open, (u32)posix_trace_rewind,
  (u32)posix_trace_set_filter, (u32)posix_trace_shutdown, (u32)posix_trace_start,
  (u32)posix_trace_stop, (u32)posix_trace_timedgetnext_event,
  (u32)posix_trace_trid_eventid_open, (u32)posix_trace_trygetnext_event,
  (u32)posix_trace_event_addr, (u32)mount, (u32)unmount, (u32)launch, (u32)isalnum,
  (u32)isalpha, (u32)islower, (u32)isupper, (u32)isdigit, (u32)isxdigit, (u32)iscntrl,
  (u32)isgraph, (u32)isspace, (u32)isblank, (u32)isprint, (u32)ispunct, (u32)tolower,
  (u32)toupper, (u32)install, (u32)accept, (u32)bind, (u32)shutdown, (u32)getpeername,
  (u32)getsockname, (u32)getsockopt, (u32)setsockopt, (u32)connect, (u32)listen,
  (u32)recv, (u32)recvfrom, (u32)send, (u32)sendmsg, (u32)sendto, (u32)socket,
  (u32)select, (u32)kernel_request_api, (u32)kernel_request, (u32)sos_trace_event,
  (u32)gethostbyname, (u32)gethostbyname_r, (u32)freeaddrinfo, (u32)getaddrinfo,
  (u32)inet_addr, (u32)inet_ntoa, (u32)inet_ntop, (u32)inet_pton, (u32)htonl, (u32)htons,
  (u32)ntohl, (u32)ntohs, (u32)timer_create, (u32)timer_delete, (u32)timer_gettime,
  (u32)timer_settime, (u32)timer_getoverrun, (u32)alarm, (u32)ualarm, (u32)fgetwc,
  (u32)fgetws, (u32)fputwc, (u32)fputws, (u32)fwide, (u32)fwprintf, (u32)fwscanf,
  (u32)getwc, (u32)getwchar, (u32)putwc, (u32)putwchar, (u32)swprintf, (u32)swscanf,
  (u32)ungetwc, (u32)vfwprintf, (u32)vfwscanf, (u32)vswprintf, (u32)vswscanf,
  (u32)vwprintf, (u32)vwscanf, (u32)wprintf, (u32)wscanf, (u32)wcstod, (u32)wcstof,
  (u32)wcstol, (u32)wcstold, (u32)wcstoll, (u32)wcstoul, (u32)wcstoull, (u32)btowc,
  (u32)mbrlen, (u32)mbrtowc, (u32)mbsinit, (u32)mbsrtowcs, (u32)wcrtomb, (u32)wctob,
  (u32)wcsrtombs, (u32)wcscat, (u32)wcschr, (u32)wcscmp, (u32)wcscoll, (u32)wcscpy,
  (u32)wcscspn, (u32)wcslen, (u32)wcsncat, (u32)wcsncpy, (u32)wcspbrk, (u32)wcsrchr,
  (u32)wcsspn, (u32)wcsstr, (u32)wcstok, (u32)wcsxfrm, (u32)wmemchr, (u32)wmemcmp,
  (u32)wmemcpy, (u32)wmemmove, (u32)wmemset, (u32)wcsftime, (u32)_ctype_,
  (u32)__locale_mb_cur_max, (u32)_Unwind_GetRegionStart, (u32)_Unwind_GetTextRelBase,
  (u32)_Unwind_GetDataRelBase, (u32)_Unwind_VRS_Set, (u32)_Unwind_Resume,
  (u32)__gnu_unwind_frame, (u32)_Unwind_GetLanguageSpecificData, (u32)_Unwind_Complete,
  (u32)_Unwind_DeleteException, (u32)_Unwind_RaiseException,
  (u32)_Unwind_Resume_or_Rethrow, (u32)_Unwind_VRS_Get, (u32)__aeabi_unwind_cpp_pr0,
  (u32)__aeabi_unwind_cpp_pr1, (u32)__cxa_atexit, (u32)getuid, (u32)setuid, (u32)geteuid,
  (u32)seteuid, (u32)sos_trace_stack, (u32)__assert_func, (u32)setenv, (u32)pthread_exit,
  (u32)pthread_testcancel, (u32)pthread_setcancelstate, (u32)pthread_setcanceltype,
  (u32)__aeabi_atexit, (u32)settimeofday, (u32)getppid, (u32)pthread_mutex_timedlock, 1};

u32 symbols_total();

#endif /* SOS_SYMBOLS_TABLE_H_ */
