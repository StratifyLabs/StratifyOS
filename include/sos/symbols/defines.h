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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef SOS_SYMBOLS_TABLE_DEFINES_H_
#define SOS_SYMBOLS_TABLE_DEFINES_H_

#if SYMBOLS_IGNORE_MATH_F != 0
#define asinhf 0
#define atanf 0
#define cbrtf 0
#define ceilf 0
#define copysignf 0
#define cosf 0
#define erff 0
#define erfcf 0
#define fabsf 0
#define fdimf 0
#define floorf 0
#define fmaf 0
#define fmaxf 0
#define fminf 0
#define frexpf 0
#define ilogbf 0
#define ldexpf 0
#define llrintf 0
#define llroundf 0
#define log1pf 0
#define log2f 0
#define logbf 0
#define lroundf 0
#define modff 0
#define nanf 0
#define nearbyintf 0
#define nextafterf 0
#define remquof 0
#define rintf 0
#define roundf 0
#define scalblnf 0
#define scalbnf 0
#define sinf 0
#define tanf 0
#define tanhf 0
#define truncf 0
#define acosf 0
#define acoshf 0
#define asinf 0
#define atan2f 0
#define atanhf 0
#define coshf 0
#define expf 0
#define exp2f 0
#define fmodf 0
#define hypotf 0
#define lgammaf 0
#define logf 0
#define log10f 0
#define powf 0
#define remainderf 0
#define sinhf 0
#define sqrtf 0
#define tgammaf 0
#define expm1f 0
#define finitef 0
#define matherr 0
#define abs 0
#define atoff 0
#endif


#if (SYMBOLS_IGNORE_DCOMPLEX) != 0 || (SYMBOLS_IGNORE_COMPLEX != 0)
#define __muldc3 0
#define __divdc3 0
#endif

#if (SYMBOLS_IGNORE_SCOMPLEX != 0) || (SYMBOLS_IGNORE_COMPLEX != 0)
#define __mulsc3 0
#define __divsc3 0
#endif

#if SYMBOLS_IGNORE_DOUBLE != 0
#define __aeabi_dneg __aeabi_fneg
#define __aeabi_dsub __aeabi_fsub
#define __aeabi_dadd __aeabi_fadd
#define __aeabi_ui2d __aeabi_ui2f
#define __aeabi_i2d __aeabi_i2f
//#define __aeabi_f2d __aeabi_f2d
//#define __aeabi_d2f __aeabi_d2f
#define __aeabi_ul2d __aeabi_ul2f
#define __aeabi_l2d __aeabi_l2f
#define __aeabi_dmul __aeabi_fmul
#define __aeabi_ddiv __aeabi_fdiv
#define __aeabi_dcmpun __aeabi_fcmpun
#define __aeabi_d2iz __aeabi_f2iz
#define __aeabi_d2uiz __aeabi_f2uiz
#define __aeabi_d2lz __aeabi_f2lz
#define __aeabi_d2ulz __aeabi_f2ulz
#define __aeabi_drsub __aeabi_frsub
#define __gedf2 __gesf2
#define __ledf2 __lesf2
#define __cmpdf2 __cmpsf2
#define __aeabi_cdrcmple __aeabi_cfrcmple
#define __aeabi_cdcmpeq __aeabi_cfcmpeq
#define __aeabi_dcmpeq __aeabi_fcmpeq
#define __aeabi_dcmplt __aeabi_fcmplt
#define __aeabi_dcmple __aeabi_fcmple
#define __aeabi_dcmpge __aeabi_fcmpge
#define __aeabi_dcmpgt __aeabi_fcmpgt
#define __powidf2 __powisf2
#if !defined SYMBOLS_IGNORE_COMPLEX && !defined SYMBOLS_IGNORE_DCOMPLEX
#define __muldc3 __mulsc3
#define __divdc3 __divsc3
#endif
#endif

#if SYMBOLS_IGNORE_SCHED != 0
#define sched_get_priority_max 0
#define sched_get_priority_min 0
#define sched_getparam 0
#define sched_getscheduler 0
#define sched_rr_get_interval 0
#define sched_setparam 0
#define sched_setscheduler 0
#define sched_yield 0
#endif

#if SYMBOLS_IGNORE_SCANF != 0
#define scanf 0
#define sscanf 0
#define vsscanf 0
#define vscanf 0
#define vfscanf 0
#endif

#if SYMBOLS_IGNORE_STDIO_FILE != 0
#define fclose 0
#define feof 0
#define ferror 0
#define fgetc 0
#define fgetpos 0
#define fgets 0
#define fileno 0
#define fopen 0
#define fputc 0
#define fputs 0
#define fread 0
#define freopen 0
#define fscanf 0
#define fseek 0
#define fsetpos 0
#define ftell 0
#define fwrite 0
#define fprintf 0
#define vfprintf 0
#define rewind 0
#define fputwc 0
#define clearerr 0
//#define fflush 0
#endif

#if SYMBOLS_IGNORE_SIGNAL != 0
#define signal 0
#define sigaction 0
#define pthread_sigmask 0
#define sigprocmask 0
#define sigpending 0
#define sigsuspend 0
#define pthread_kill 0
#define sigqueue 0
#define sigwait 0
#define sigtimedwait 0
#define sigwaitinfo 0
#define sigaddset 0
#define sigdelset 0
#define sigismember 0
#define sigfillset 0
#define sigemptyset 0
#define raise 0
#endif

#if SYMBOLS_IGNORE_PTHREAD_MUTEX != 0
#define pthread_mutexattr_getprioceiling 0
#define pthread_mutexattr_setprioceiling 0
#define pthread_mutexattr_getprotocol 0
#define pthread_mutexattr_setprotocol 0
#define pthread_mutexattr_getpshared 0
#define pthread_mutexattr_setpshared 0
#define pthread_mutexattr_gettype 0
#define pthread_mutexattr_settype 0
#define pthread_mutexattr_init 0
#define pthread_mutexattr_destroy 0
//#define pthread_mutex_init 0 //This is used by CRT -- must be present
#define pthread_mutex_lock 0
#define pthread_mutex_trylock 0
#define pthread_mutex_unlock 0
#define pthread_mutex_destroy 0
#define pthread_mutex_getprioceiling 0
#define pthread_mutex_setprioceiling 0
#endif

#if SYMBOLS_IGNORE_PTHREAD_COND != 0
#define pthread_cond_init 0
#define pthread_cond_destroy 0
#define pthread_cond_broadcast 0
#define pthread_cond_signal 0
#define pthread_cond_wait 0
#define pthread_cond_timedwait 0
#define pthread_condattr_init 0
#define pthread_condattr_destroy 0
#define pthread_condattr_getpshared 0
#define pthread_condattr_setpshared 0
#define pthread_condattr_getclock 0
#define pthread_condattr_setclock 0
#endif

#if SYMBOLS_IGNORE_PTHREAD != 0
#define pthread_create 0
#define pthread_join 0
#define pthread_getschedparam 0
#define pthread_setschedparam 0
#define pthread_cancel 0
#endif

#if SYMBOLS_IGNORE_AIO != 0
#define aio_cancel 0
#define aio_error 0
#define aio_fsync 0
#define aio_read 0
#define aio_write 0
#define aio_return 0
#define aio_suspend 0
#define lio_listio 0
#endif

#if SYMBOLS_IGNORE_CTYPE != 0
#define isalnum 0
#define isalpha 0
#define isblank 0
#define iscntrl 0
#define isctype 0
#define isdigit 0
#define isgraph 0
#define islower 0
#define isprint 0
#define ispunct 0
#define isspace 0
#define isupper 0
#define isxdigit 0
#define toctrans 0
#define tolower 0
#define toupper 0
#endif

#if SYMBOLS_IGNORE_WCTYPE != 0
#define iswalnum 0
#define iswalpha 0
#define iswblank 0
#define iswcntrl 0
#define iswctype 0
#define iswdigit 0
#define iswgraph 0
#define iswlower 0
#define iswprint 0
#define iswpunct 0
#define iswspace 0
#define iswupper 0
#define iswxdigit 0
#define towctrans 0
#define towlower 0
#define towupper 0
#define wcscasecmp 0
#define wcscmp 0
#define wcsncasecmp 0
#define wcsncmp 0
#define wcstombs 0
#define wctomb 0
#define wctrans 0
#define wctype 0
#endif

#if SYMBOLS_IGNORE_WCHAR != 0
#define fgetwc 0
#define fgetws 0
#define fputwc 0
#define fputws 0
#define fwide 0
#define fwprintf 0
#define fwscanf 0
#define getwc 0
#define getwchar 0
#define putwc 0
#define putwchar 0
#define swprintf 0
#define swscanf 0
#define ungetwc 0
#define vfwprintf 0
#define vfwscanf 0
#define vswprintf 0
#define vswscanf 0
#define vwprintf 0
#define vwscanf 0
#define wprintf 0
#define wscanf 0
#define wcstod 0
#define wcstof 0
#define wcstol 0
#define wcstold 0
#define wcstoll 0
#define wcstoul 0
#define wcstoull 0
#define btowc 0
#define mbrlen 0
#define mbrtowc 0
#define mbsinit 0
#define mbsrtowcs 0
#define wcrtomb 0
#define wctob 0
#define wcsrtombs 0
#define wcscat 0
#define wcschr 0
#define wcscmp 0
#define wcscoll 0
#define wcscpy 0
#define wcscspn 0
#define wcslen 0
#define wcsncat 0
#define wcsncpy 0
#define wcspbrk 0
#define wcsrchr 0
#define wcsspn 0
#define wcsstr 0
#define wcstok 0
#define wcsxfrm 0
#define wmemchr 0
#define wmemcmp 0
#define wmemcpy 0
#define wmemmove 0
#define wmemset 0
#define wcsftime 0
#define _ctype_ 0
#define __locale_mb_cur_max 0
#endif

#if SYMBOLS_IGNORE_STR != 0
#define strcat 0
#define strchr 0
#define strcmp 0
#define strcoll 0
#define strcpy 0
#define strcspn 0
#define strerror 0
#define strftime 0
#define strlen 0
#define strncat 0
#define strncmp 0
#define strncpy 0
#define strnlen 0
#define strpbrk 0
#define strptime 0
#define strrchr 0
#define strspn 0
#define strstr 0
#define strtod 0
#define strtof 0
#define strtok 0
#define strtok_r 0
#define strtol 0
#define strtoll 0
#define strtoul 0
#define strtoull 0
#define strxfrm 0
#endif


#if SYMBOLS_IGNORE_HWPL != 0
#define mcu_adc_ioctl 0
#define mcu_core_ioctl 0
#define mcu_dac_ioctl 0
#define mcu_eint_ioctl 0
#define mcu_mem_read 0
#define mcu_i2c_ioctl 0
#define mcu_pio_ioctl 0
#define mcu_pwm_ioctl 0
#define mcu_tmr_ioctl 0
#define mcu_qei_ioctl 0
#define mcu_tmr_ioctl 0
#define mcu_rtc_ioctl 0
#define mcu_spi_ioctl 0
#define mcu_tmr_ioctl 0
#define mcu_tmr_ioctl 0
#define mcu_uart_ioctl 0
#define mcu_usb_ioctl 0
#endif


#if SYMBOLS_IGNORE_SEM != 0
#define sem_init 0
#define sem_destroy 0
#define sem_getvalue 0
#define sem_open 0
#define sem_post 0
#define sem_timedwait 0
#define sem_trywait 0
#define sem_unlink 0
#define sem_close 0
#define sem_wait 0
#endif


#if SYMBOLS_IGNORE_MQ != 0
#define mq_getattr 0
#define mq_setattr 0
#define mq_open 0
#define mq_close 0
#define mq_unlink 0
#define mq_notify 0
#define mq_timedreceive 0
#define mq_receive 0
#define mq_timedsend 0
#define mq_send 0
#endif

#if !defined SYMBOLS_IGNORE_POSIX_TRACE
#define SYMBOLS_IGNORE_POSIX_TRACE 1
#endif

#if SYMBOLS_IGNORE_POSIX_TRACE != 0
#define posix_trace_attr_destroy 0
#define posix_trace_attr_getclockres 0
#define posix_trace_attr_getcreatetime 0
#define posix_trace_attr_getgenversion 0
#define posix_trace_attr_getinherited 0
#define posix_trace_attr_getlogfullpolicy 0
#define posix_trace_attr_getlogsize 0
#define posix_trace_attr_getmaxdatasize 0
#define posix_trace_attr_getmaxsystemeventsize 0
#define posix_trace_attr_getmaxusereventsize 0
#define posix_trace_attr_getname 0
#define posix_trace_attr_getstreamfullpolicy 0
#define posix_trace_attr_getstreamsize 0
#define posix_trace_attr_init 0
#define posix_trace_attr_setinherited 0
#define posix_trace_attr_setlogfullpolicy 0
#define posix_trace_attr_setlogsize 0
#define posix_trace_attr_setmaxdatasize 0
#define posix_trace_attr_setname 0
#define posix_trace_attr_setstreamsize 0
#define posix_trace_attr_setstreamfullpolicy 0
#define posix_trace_clear 0
#define posix_trace_close 0
#define posix_trace_create 0
#define posix_trace_create_withlog 0
#define posix_trace_event 0
#define posix_trace_eventid_equal 0
#define posix_trace_eventid_get_name 0
#define posix_trace_eventid_open 0
#define posix_trace_eventset_add 0
#define posix_trace_eventset_del 0
#define posix_trace_eventset_empty 0
#define posix_trace_eventset_fill 0
#define posix_trace_eventset_ismember 0
#define posix_trace_eventtypelist_getnext_id 0
#define posix_trace_eventtypelist_rewind 0
#define posix_trace_flush 0
#define posix_trace_get_attr 0
#define posix_trace_get_filter 0
#define posix_trace_get_status 0
#define posix_trace_getnext_event 0
#define posix_trace_open 0
#define posix_trace_rewind 0
#define posix_trace_set_filter 0
#define posix_trace_shutdown 0
#define posix_trace_start 0
#define posix_trace_stop 0
#define posix_trace_timedgetnext_event 0
#define posix_trace_trid_eventid_open 0
#define posix_trace_trygetnext_event 0
#define posix_trace_event_addr 0
#endif

#if SYMBOLS_IGNORE_SG != 0
#define sg_api 0
#endif

#if SYMBOLS_IGNORE_SON != 0
#define son_api 0
#endif

#if SYMBOLS_IGNORE_LWIP != 0
#define lwip_api 0
#endif

#if SYMBOLS_IGNORE_SOCKET != 0
#define accept 0
#define bind 0
#define shutdown 0
#define getpeername 0
#define getsockname 0
#define getsockopt 0
#define setsockopt 0
#define connect 0
#define listen 0
#define recv 0
#define recvfrom 0
#define send 0
#define sendmsg 0
#define sendto 0
#define socket 0
#define select 0
#endif

#if SYMBOLS_IGNORE_TIME != 0
#define asctime 0
#define asctime_r 0
#define ctime 0
#define ctime_r 0
#define difftime 0
#define getdate 0
#define gmtime 0
#define gmtime_r 0
#define localtime 0
#define localtime_r 0
#define mktime 0
#define strftime 0
#define strptime 0
#define time 0
#endif

#if SYMBOLS_IGNORE_CLOCK != 0
#define clock 0
#define clock_getres 0
#define clock_gettime 0
#define clock_settime 0
#endif

#if SYMBOLS_IGNORE_RAND != 0
#define rand 0
#define srand 0
#endif

#if SYMBOLS_USE_IPRINTF != 0
#if SYMBOLS_IGNORE_STDIO_FILE == 0
#define fprintf fiprintf
#define vfprintf vfiprintf
#endif
#define sprintf siprintf
#define printf iprintf
#define vprintf viprintf
#define snprintf sniprintf
#define vsnprintf vsniprintf
#define vsprintf vsiprintf

#if SYMBOLS_IGNORE_SCANF == 0
#define scanf iscanf
#define sscanf siscanf
#define vsscanf vsiscanf
#define vscanf viscanf
#if SYMBOLS_IGNORE_STDIO_FILE == 0
#define fscanf fiscanf
#define vfscanf vfiscanf
#endif
#endif
#define strtod 0
#define strtof 0
#endif

#if (!defined SYMBOLS_USE_EXCEPTIONS) || (SYMBOLS_USE_EXCEPTIONS == 0)
#define _Unwind_GetRegionStart 0
#define _Unwind_GetTextRelBase 0
#define _Unwind_GetDataRelBase 0
#define _Unwind_VRS_Set 0
#define _Unwind_Resume 0
#define __gnu_unwind_frame 0
#define _Unwind_GetLanguageSpecificData 0
#define _Unwind_Complete 0
#define _Unwind_DeleteException 0
#define _Unwind_RaiseException 0
#define _Unwind_Resume_or_Rethrow 0
#define _Unwind_VRS_Get 0
#define __aeabi_unwind_cpp_pr0 0
#define __aeabi_unwind_cpp_pr1 0
#define __cxa_atexit 0
#endif


#endif /* SOS_SYMBOLS_TABLE_DEFINES_H_ */
