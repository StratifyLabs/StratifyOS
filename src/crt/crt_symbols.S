/* Copyright 2011-2020 Tyler Gilbert; All Rights Reserved */

#if defined __v7m
#define __CORTEX_M 3
#elif defined __v7em || defined __v7em_f4sh || defined __v7em_f4ss
#define __CORTEX_M 4
#elif defined __v7em_f5sh || defined __v7em_f5ss || defined __v7em_f5dh || defined __v7em_f5ds
#define __CORTEX_M 7
#endif

// offset must be (__LINE__ - <line number of _signature>) * 4 + 1
#define LINK_ADDR (0xDFFF8001 + ((__LINE__ - 14) * 4))

.global _signature; _signature = 0x00000410 + __CORTEX_M;
//.global __cxa_pure_virtual; __cxa_pure_virtual = LINK_ADDR;
.global _global_impure_ptr; _global_impure_ptr = LINK_ADDR;
.global _impure_ptr; _impure_ptr = LINK_ADDR;
.global __sf_fake_stdin; __sf_fake_stdin = LINK_ADDR;
.global __sf_fake_stdout; __sf_fake_stdout = LINK_ADDR;
.global __sf_fake_stderr; __sf_fake_stderr = LINK_ADDR;
.global __aeabi_uldivmod; __aeabi_uldivmod = LINK_ADDR;
.global __aeabi_ldivmod; __aeabi_ldivmod = LINK_ADDR;
.global __aeabi_uidiv; __aeabi_uidiv = LINK_ADDR;
.global __aeabi_uidivmod; __aeabi_uidivmod = LINK_ADDR;
.global __aeabi_idiv; __aeabi_idiv = LINK_ADDR;
.global __aeabi_idivmod; __aeabi_idivmod = LINK_ADDR;
.global __umodsi3; __umodsi3 = LINK_ADDR;
.global __modsi3; __modsi3 = LINK_ADDR;
.global __div0; __div0 = LINK_ADDR;
.global __aeabi_llsr; __aeabi_llsr = LINK_ADDR;
.global __aeabi_lasr; __aeabi_lasr = LINK_ADDR;
.global __aeabi_llsl; __aeabi_llsl = LINK_ADDR;
.global __clzsi2; __clzsi2 = LINK_ADDR;
.global __clzdi2; __clzdi2 = LINK_ADDR;
.global __muldi3; __muldi3 = LINK_ADDR;
.global __negdi2; __negdi2 = LINK_ADDR;
.global __cmpdi2; __cmpdi2 = LINK_ADDR;
.global __ucmpdi2; __ucmpdi2 = LINK_ADDR;
.global __absvsi2; __absvsi2 = LINK_ADDR;
.global __absvdi2; __absvdi2 = LINK_ADDR;
.global __addvsi3; __addvsi3 = LINK_ADDR;
.global __addvdi3; __addvdi3 = LINK_ADDR;
.global __subvsi3; __subvsi3 = LINK_ADDR;
.global __subvdi3; __subvdi3 = LINK_ADDR;
.global __mulvsi3; __mulvsi3 = LINK_ADDR;
.global __mulvdi3; __mulvdi3 = LINK_ADDR;
.global __negvsi2; __negvsi2 = LINK_ADDR;
.global __negvdi2; __negvdi2 = LINK_ADDR;
.global __ffsdi2; __ffsdi2 = LINK_ADDR;
.global __popcountsi2; __popcountsi2 = LINK_ADDR;
.global __popcountdi2; __popcountdi2 = LINK_ADDR;
.global __paritysi2; __paritysi2 = LINK_ADDR;
.global __paritydi2; __paritydi2 = LINK_ADDR;
.global __mulsc3; __mulsc3 = LINK_ADDR;
.global __muldc3; __muldc3 = LINK_ADDR;
.global __divsc3; __divsc3 = LINK_ADDR;
.global __divdc3; __divdc3 = LINK_ADDR;
.global __bswapsi2; __bswapsi2 = LINK_ADDR;
.global __bswapdi2; __bswapdi2 = LINK_ADDR;
.global __divdi3; __divdi3 = LINK_ADDR;
.global __moddi3; __moddi3 = LINK_ADDR;
.global __udivdi3; __udivdi3 = LINK_ADDR;
.global __umoddi3; __umoddi3 = LINK_ADDR;
.global abort; abort = LINK_ADDR;
.global asctime; asctime = LINK_ADDR;
.global asctime_r; asctime_r = LINK_ADDR;
.global exit; exit = LINK_ADDR;
.global atexit; atexit = LINK_ADDR;
.global atoi; atoi = LINK_ADDR;
.global atol; atol = LINK_ADDR;
.global bsearch; bsearch = LINK_ADDR;
.global clearerr; clearerr = LINK_ADDR;
.global clock; clock = LINK_ADDR;
.global ctime; ctime = LINK_ADDR;
.global difftime; difftime = LINK_ADDR;
.global div; div = LINK_ADDR;
.global __errno; __errno = LINK_ADDR;
.global fclose; fclose = LINK_ADDR;
.global feof; feof = LINK_ADDR;
.global ferror; ferror = LINK_ADDR;
.global fflush; fflush = LINK_ADDR;
.global fgetc; fgetc = LINK_ADDR;
.global fgetpos; fgetpos = LINK_ADDR;
.global fgets; fgets = LINK_ADDR;
.global fileno; fileno = LINK_ADDR;
.global fopen; fopen = LINK_ADDR;
.global fputc; fputc = LINK_ADDR;
.global fputs; fputs = LINK_ADDR;
.global fread; fread = LINK_ADDR;
.global freopen; freopen = LINK_ADDR;
.global fscanf; fscanf = LINK_ADDR;
.global fseek; fseek = LINK_ADDR;
.global fsetpos; fsetpos = LINK_ADDR;
.global ftell; ftell = LINK_ADDR;
.global fwrite; fwrite = LINK_ADDR;
.global getc; getc = LINK_ADDR;
.global getchar; getchar = LINK_ADDR;
.global getenv; getenv = LINK_ADDR;
.global gets; gets = LINK_ADDR;
.global gmtime; gmtime = LINK_ADDR;
.global gmtime_r; gmtime_r = LINK_ADDR;
.global iswalnum; iswalnum = LINK_ADDR;
.global iswalpha; iswalpha = LINK_ADDR;
.global iswblank; iswblank = LINK_ADDR;
.global iswcntrl; iswcntrl = LINK_ADDR;
.global iswctype; iswctype = LINK_ADDR;
.global iswdigit; iswdigit = LINK_ADDR;
.global iswgraph; iswgraph = LINK_ADDR;
.global iswlower; iswlower = LINK_ADDR;
.global iswprint; iswprint = LINK_ADDR;
.global iswpunct; iswpunct = LINK_ADDR;
.global iswspace; iswspace = LINK_ADDR;
.global iswupper; iswupper = LINK_ADDR;
.global iswxdigit; iswxdigit = LINK_ADDR;
.global labs; labs = LINK_ADDR;
.global localtime; localtime = LINK_ADDR;
.global localtime_r; localtime_r = LINK_ADDR;
.global ldiv; ldiv = LINK_ADDR;
.global lldiv; lldiv = LINK_ADDR;
.global mblen; mblen = LINK_ADDR;
.global mbstowcs; mbstowcs = LINK_ADDR;
.global mbtowc; mbtowc = LINK_ADDR;
.global memchr; memchr = LINK_ADDR;
.global memcmp; memcmp = LINK_ADDR;
.global memcpy; memcpy = LINK_ADDR;
.global memmove; memmove = LINK_ADDR;
.global memset; memset = LINK_ADDR;
.global mktime; mktime = LINK_ADDR;
.global perror; perror = LINK_ADDR;
.global putc; putc = LINK_ADDR;
.global putchar; putchar = LINK_ADDR;
.global puts; puts = LINK_ADDR;
.global qsort; qsort = LINK_ADDR;
.global raise; raise = LINK_ADDR;
.global srand; srand = LINK_ADDR;
.global rand; rand = LINK_ADDR;
.global remove; remove = LINK_ADDR;
.global rename; rename = LINK_ADDR;
.global rewind; rewind = LINK_ADDR;
.global scanf; scanf = LINK_ADDR;
.global setbuf; setbuf = LINK_ADDR;
.global setvbuf; setvbuf = LINK_ADDR;
.global sscanf; sscanf = LINK_ADDR;
.global strcat; strcat = LINK_ADDR;
.global strchr; strchr = LINK_ADDR;
.global strcmp; strcmp = LINK_ADDR;
.global strcoll; strcoll = LINK_ADDR;
.global strcpy; strcpy = LINK_ADDR;
.global strcspn; strcspn = LINK_ADDR;
.global strerror; strerror = LINK_ADDR;
.global strftime; strftime = LINK_ADDR;
.global strlen; strlen = LINK_ADDR;
.global strncat; strncat = LINK_ADDR;
.global strncmp; strncmp = LINK_ADDR;
.global strncpy; strncpy = LINK_ADDR;
.global strnlen; strnlen = LINK_ADDR;
.global strpbrk; strpbrk = LINK_ADDR;
.global strptime; strptime = LINK_ADDR;
.global strrchr; strrchr = LINK_ADDR;
.global strspn; strspn = LINK_ADDR;
.global strstr; strstr = LINK_ADDR;
.global strtod; strtod = LINK_ADDR;
.global strtof; strtof = LINK_ADDR;
.global strtok; strtok = LINK_ADDR;
.global strtok_r; strtok_r = LINK_ADDR;
.global strtol; strtol = LINK_ADDR;
.global strtoll; strtoll = LINK_ADDR;
.global strtoul; strtoul = LINK_ADDR;
.global strtoull; strtoull = LINK_ADDR;
.global strxfrm; strxfrm = LINK_ADDR;
.global close; close = LINK_ADDR;
.global execve; execve = LINK_ADDR;
.global fcntl; fcntl = LINK_ADDR;
.global fstat; fstat = LINK_ADDR;
.global getpid; getpid = LINK_ADDR;
.global gettimeofday; gettimeofday = LINK_ADDR;
.global kill; kill = LINK_ADDR;
.global link; link = LINK_ADDR;
.global lseek; lseek = LINK_ADDR;
.global open; open = LINK_ADDR;
.global fsync; fsync = LINK_ADDR;
.global read; read = LINK_ADDR;
.global stat; stat = LINK_ADDR;
.global system; system = LINK_ADDR;
.global times; times = LINK_ADDR;
.global unlink; unlink = LINK_ADDR;
.global wait; wait = LINK_ADDR;
.global write; write = LINK_ADDR;
.global time; time = LINK_ADDR;
.global towctrans; towctrans = LINK_ADDR;
.global towlower; towlower = LINK_ADDR;
.global towupper; towupper = LINK_ADDR;
.global ungetc; ungetc = LINK_ADDR;
.global vfscanf; vfscanf = LINK_ADDR;
.global vscanf; vscanf = LINK_ADDR;
.global vsscanf; vsscanf = LINK_ADDR;
.global wcscasecmp; wcscasecmp = LINK_ADDR;
.global wcscmp; wcscmp = LINK_ADDR;
.global wcsncasecmp; wcsncasecmp = LINK_ADDR;
.global wcsncmp; wcsncmp = LINK_ADDR;
.global wcstombs; wcstombs = LINK_ADDR;
.global wctomb; wctomb = LINK_ADDR;
.global wctrans; wctrans = LINK_ADDR;
.global wctype; wctype = LINK_ADDR;
.global fputwc; fputwc = LINK_ADDR;
.global setlocale; setlocale = LINK_ADDR;
.global localeconv; localeconv = LINK_ADDR;
.global mbrtowc; mbrtowc = LINK_ADDR;
.global strncasecmp; strncasecmp = LINK_ADDR;
.global wcrtomb; wcrtomb = LINK_ADDR;
.global aio_cancel; aio_cancel = LINK_ADDR;
.global aio_error; aio_error = LINK_ADDR;
.global aio_fsync; aio_fsync = LINK_ADDR;
.global aio_read; aio_read = LINK_ADDR;
.global aio_write; aio_write = LINK_ADDR;
.global aio_return; aio_return = LINK_ADDR;
.global aio_suspend; aio_suspend = LINK_ADDR;
.global lio_listio; lio_listio = LINK_ADDR;
.global mkfs; mkfs = LINK_ADDR;
.global sched_get_priority_max; sched_get_priority_max = LINK_ADDR;
.global sched_get_priority_min; sched_get_priority_min = LINK_ADDR;
.global sched_getparam; sched_getparam = LINK_ADDR;
.global sched_getscheduler; sched_getscheduler = LINK_ADDR;
.global sched_rr_get_interval; sched_rr_get_interval = LINK_ADDR;
.global sched_setparam; sched_setparam = LINK_ADDR;
.global sched_setscheduler; sched_setscheduler = LINK_ADDR;
.global sched_yield; sched_yield = LINK_ADDR;
.global access; access = LINK_ADDR;
.global ioctl; ioctl = LINK_ADDR;
.global mkdir; mkdir = LINK_ADDR;
.global rmdir; rmdir = LINK_ADDR;
.global sleep; sleep = LINK_ADDR;
.global usleep; usleep = LINK_ADDR;
.global chmod; chmod = LINK_ADDR;
.global mallinfo; mallinfo = LINK_ADDR;
.global malloc; malloc = LINK_ADDR;
.global realloc; realloc = LINK_ADDR;
.global free; free = LINK_ADDR;
.global calloc; calloc = LINK_ADDR;
.global _malloc_r; _malloc_r = LINK_ADDR;
.global _free_r; _free_r = LINK_ADDR;
.global clock_getcpuclockid; clock_getcpuclockid = LINK_ADDR;
.global clock_gettime; clock_gettime = LINK_ADDR;
.global clock_getres; clock_getres = LINK_ADDR;
.global clock_settime; clock_settime = LINK_ADDR;
.global hibernate; hibernate = LINK_ADDR;
.global powerdown; powerdown = LINK_ADDR;
.global pthread_create; pthread_create = LINK_ADDR;
.global pthread_join; pthread_join = LINK_ADDR;
.global pthread_getschedparam; pthread_getschedparam = LINK_ADDR;
.global pthread_setschedparam; pthread_setschedparam = LINK_ADDR;
.global pthread_cancel; pthread_cancel = LINK_ADDR;
.global pthread_cond_init; pthread_cond_init = LINK_ADDR;
.global pthread_cond_destroy; pthread_cond_destroy = LINK_ADDR;
.global pthread_cond_broadcast; pthread_cond_broadcast = LINK_ADDR;
.global pthread_cond_signal; pthread_cond_signal = LINK_ADDR;
.global pthread_cond_wait; pthread_cond_wait = LINK_ADDR;
.global pthread_cond_timedwait; pthread_cond_timedwait = LINK_ADDR;
.global pthread_condattr_init; pthread_condattr_init = LINK_ADDR;
.global pthread_condattr_destroy; pthread_condattr_destroy = LINK_ADDR;
.global pthread_condattr_getpshared; pthread_condattr_getpshared = LINK_ADDR;
.global pthread_condattr_setpshared; pthread_condattr_setpshared = LINK_ADDR;
.global pthread_condattr_getclock; pthread_condattr_getclock = LINK_ADDR;
.global pthread_condattr_setclock; pthread_condattr_setclock = LINK_ADDR;
.global pthread_attr_getdetachstate; pthread_attr_getdetachstate = LINK_ADDR;
.global pthread_attr_setdetachstate; pthread_attr_setdetachstate = LINK_ADDR;
.global pthread_attr_getguardsize; pthread_attr_getguardsize = LINK_ADDR;
.global pthread_attr_setguardsize; pthread_attr_setguardsize = LINK_ADDR;
.global pthread_attr_getinheritsched; pthread_attr_getinheritsched = LINK_ADDR;
.global pthread_attr_setinheritsched; pthread_attr_setinheritsched = LINK_ADDR;
.global pthread_attr_getschedparam; pthread_attr_getschedparam = LINK_ADDR;
.global pthread_attr_setschedparam; pthread_attr_setschedparam = LINK_ADDR;
.global pthread_attr_getschedpolicy; pthread_attr_getschedpolicy = LINK_ADDR;
.global pthread_attr_setschedpolicy; pthread_attr_setschedpolicy = LINK_ADDR;
.global pthread_attr_getscope; pthread_attr_getscope = LINK_ADDR;
.global pthread_attr_setscope; pthread_attr_setscope = LINK_ADDR;
.global pthread_attr_getstacksize; pthread_attr_getstacksize = LINK_ADDR;
.global pthread_attr_setstacksize; pthread_attr_setstacksize = LINK_ADDR;
.global pthread_attr_getstackaddr; pthread_attr_getstackaddr = LINK_ADDR;
.global pthread_attr_setstackaddr; pthread_attr_setstackaddr = LINK_ADDR;
.global pthread_attr_init; pthread_attr_init = LINK_ADDR;
.global pthread_attr_destroy; pthread_attr_destroy = LINK_ADDR;
.global pthread_mutexattr_getprioceiling; pthread_mutexattr_getprioceiling = LINK_ADDR;
.global pthread_mutexattr_setprioceiling; pthread_mutexattr_setprioceiling = LINK_ADDR;
.global pthread_mutexattr_getprotocol; pthread_mutexattr_getprotocol = LINK_ADDR;
.global pthread_mutexattr_setprotocol; pthread_mutexattr_setprotocol = LINK_ADDR;
.global pthread_mutexattr_getpshared; pthread_mutexattr_getpshared = LINK_ADDR;
.global pthread_mutexattr_setpshared; pthread_mutexattr_setpshared = LINK_ADDR;
.global pthread_mutexattr_gettype; pthread_mutexattr_gettype = LINK_ADDR;
.global pthread_mutexattr_settype; pthread_mutexattr_settype = LINK_ADDR;
.global pthread_mutexattr_init; pthread_mutexattr_init = LINK_ADDR;
.global pthread_mutexattr_destroy; pthread_mutexattr_destroy = LINK_ADDR;
.global pthread_mutex_init; pthread_mutex_init = LINK_ADDR;
.global pthread_mutex_lock; pthread_mutex_lock = LINK_ADDR;
.global pthread_mutex_trylock; pthread_mutex_trylock = LINK_ADDR;
.global pthread_mutex_unlock; pthread_mutex_unlock = LINK_ADDR;
.global pthread_mutex_destroy; pthread_mutex_destroy = LINK_ADDR;
.global pthread_mutex_getprioceiling; pthread_mutex_getprioceiling = LINK_ADDR;
.global pthread_mutex_setprioceiling; pthread_mutex_setprioceiling = LINK_ADDR;
.global pthread_self; pthread_self = LINK_ADDR;
.global signal; signal = LINK_ADDR;
.global sigaction; sigaction = LINK_ADDR;
.global pthread_sigmask; pthread_sigmask = LINK_ADDR;
.global sigprocmask; sigprocmask = LINK_ADDR;
.global sigpending; sigpending = LINK_ADDR;
.global sigsuspend; sigsuspend = LINK_ADDR;
.global pthread_kill; pthread_kill = LINK_ADDR;
.global sigqueue; sigqueue = LINK_ADDR;
.global sigwait; sigwait = LINK_ADDR;
.global sigtimedwait; sigtimedwait = LINK_ADDR;
.global sigwaitinfo; sigwaitinfo = LINK_ADDR;
.global waitpid; waitpid = LINK_ADDR;
.global _wait; _wait = LINK_ADDR;
.global sem_init; sem_init = LINK_ADDR;
.global sem_destroy; sem_destroy = LINK_ADDR;
.global sem_getvalue; sem_getvalue = LINK_ADDR;
.global sem_open; sem_open = LINK_ADDR;
.global sem_post; sem_post = LINK_ADDR;
.global sem_timedwait; sem_timedwait = LINK_ADDR;
.global sem_trywait; sem_trywait = LINK_ADDR;
.global sem_unlink; sem_unlink = LINK_ADDR;
.global sem_close; sem_close = LINK_ADDR;
.global sem_wait; sem_wait = LINK_ADDR;
.global mq_getattr; mq_getattr = LINK_ADDR;
.global mq_setattr; mq_setattr = LINK_ADDR;
.global mq_open; mq_open = LINK_ADDR;
.global mq_close; mq_close = LINK_ADDR;
.global mq_unlink; mq_unlink = LINK_ADDR;
.global mq_notify; mq_notify = LINK_ADDR;
.global mq_timedreceive; mq_timedreceive = LINK_ADDR;
.global mq_receive; mq_receive = LINK_ADDR;
.global mq_timedsend; mq_timedsend = LINK_ADDR;
.global mq_send; mq_send = LINK_ADDR;
.global _exit; _exit = LINK_ADDR;
.global chown; chown = LINK_ADDR;
.global closedir; closedir = LINK_ADDR;
.global opendir; opendir = LINK_ADDR;
.global readdir_r; readdir_r = LINK_ADDR;
.global readdir; readdir = LINK_ADDR;
.global rewinddir; rewinddir = LINK_ADDR;
.global seekdir; seekdir = LINK_ADDR;
.global telldir; telldir = LINK_ADDR;
.global fprintf; fprintf = LINK_ADDR;
.global printf; printf = LINK_ADDR;
.global snprintf; snprintf = LINK_ADDR;
.global vfprintf; vfprintf = LINK_ADDR;
.global sprintf; sprintf = LINK_ADDR;
.global vprintf; vprintf = LINK_ADDR;
.global vsnprintf; vsnprintf = LINK_ADDR;
.global vsprintf; vsprintf = LINK_ADDR;
.global asinhf; asinhf = LINK_ADDR;
.global atanf; atanf = LINK_ADDR;
.global cbrtf; cbrtf = LINK_ADDR;
.global ceilf; ceilf = LINK_ADDR;
.global copysignf; copysignf = LINK_ADDR;
.global cosf; cosf = LINK_ADDR;
.global erff; erff = LINK_ADDR;
.global erfcf; erfcf = LINK_ADDR;
.global fabsf; fabsf = LINK_ADDR;
.global fdimf; fdimf = LINK_ADDR;
.global floorf; floorf = LINK_ADDR;
.global fmaf; fmaf = LINK_ADDR;
.global fmaxf; fmaxf = LINK_ADDR;
.global fminf; fminf = LINK_ADDR;
.global frexpf; frexpf = LINK_ADDR;
.global ilogbf; ilogbf = LINK_ADDR;
.global ldexpf; ldexpf = LINK_ADDR;
.global llrintf; llrintf = LINK_ADDR;
.global llroundf; llroundf = LINK_ADDR;
.global log1pf; log1pf = LINK_ADDR;
.global log2f; log2f = LINK_ADDR;
.global logbf; logbf = LINK_ADDR;
.global lroundf; lroundf = LINK_ADDR;
.global modff; modff = LINK_ADDR;
.global nanf; nanf = LINK_ADDR;
.global nearbyintf; nearbyintf = LINK_ADDR;
.global nextafterf; nextafterf = LINK_ADDR;
.global remquof; remquof = LINK_ADDR;
.global rintf; rintf = LINK_ADDR;
.global roundf; roundf = LINK_ADDR;
.global scalblnf; scalblnf = LINK_ADDR;
.global scalbnf; scalbnf = LINK_ADDR;
.global sinf; sinf = LINK_ADDR;
.global tanf; tanf = LINK_ADDR;
.global tanhf; tanhf = LINK_ADDR;
.global truncf; truncf = LINK_ADDR;
.global acosf; acosf = LINK_ADDR;
.global acoshf; acoshf = LINK_ADDR;
.global asinf; asinf = LINK_ADDR;
.global atan2f; atan2f = LINK_ADDR;
.global atanhf; atanhf = LINK_ADDR;
.global coshf; coshf = LINK_ADDR;
.global expf; expf = LINK_ADDR;
.global exp2f; exp2f = LINK_ADDR;
.global fmodf; fmodf = LINK_ADDR;
.global hypotf; hypotf = LINK_ADDR;
.global lgammaf; lgammaf = LINK_ADDR;
.global logf; logf = LINK_ADDR;
.global log10f; log10f = LINK_ADDR;
.global powf; powf = LINK_ADDR;
.global remainderf; remainderf = LINK_ADDR;
.global sinhf; sinhf = LINK_ADDR;
.global sqrtf; sqrtf = LINK_ADDR;
.global tgammaf; tgammaf = LINK_ADDR;
.global expm1f; expm1f = LINK_ADDR;
.global finitef; finitef = LINK_ADDR;
.global matherr; matherrf = LINK_ADDR;
.global abs; abs = LINK_ADDR;
.global atoff; atoff = LINK_ADDR;
.global __aeabi_fneg; __aeabi_fneg = LINK_ADDR;				.global __negsf2; __negsf2 = __aeabi_fneg;
.global __aeabi_fsub; __aeabi_fsub = LINK_ADDR;				.global __subsf3; __subsf3 = __aeabi_fsub;
.global __aeabi_fadd; __aeabi_fadd = LINK_ADDR;				.global __addsf3; __addsf3 = __aeabi_fadd;
.global __aeabi_ui2f; __aeabi_ui2f = LINK_ADDR;				.global __floatunsisf; __floatunsisf = __aeabi_ui2f;
.global __aeabi_i2f; __aeabi_i2f = LINK_ADDR;				.global __floatsisf; __floatsisf = __aeabi_i2f;
.global __aeabi_ul2f; __aeabi_ul2f = LINK_ADDR;				.global __floatundisf; __floatundisf = __aeabi_ul2f;
.global __aeabi_l2f; __aeabi_l2f = LINK_ADDR;				.global __floatdisf; __floatdisf = __aeabi_l2f;
.global __aeabi_fmul; __aeabi_fmul = LINK_ADDR;				.global __mulsf3; __mulsf3 = __aeabi_fmul;
.global __aeabi_fdiv; __aeabi_fdiv = LINK_ADDR;				.global __divsf3; __divsf3 = __aeabi_fdiv;
.global __aeabi_fcmpun; __aeabi_fcmpun = LINK_ADDR;			.global __unordsf2; __unordsf2 = __aeabi_fcmpun;
.global __aeabi_f2iz; __aeabi_f2iz = LINK_ADDR;				.global __fixsfsi; __fixsfsi = __aeabi_f2iz;
.global __aeabi_f2uiz; __aeabi_f2uiz = LINK_ADDR;			.global __fixunssfsi; __fixunssfsi = __aeabi_f2uiz;
.global __aeabi_f2lz; __aeabi_f2lz = LINK_ADDR;				.global __fixsfdi; __fixsfdi = __aeabi_f2lz;
.global __aeabi_f2ulz; __aeabi_f2ulz = LINK_ADDR;			.global __fixunssfdi; __fixunssfdi = __aeabi_f2ulz;
.global __aeabi_frsub; __aeabi_frsub = LINK_ADDR;
.global __gesf2; __gesf2 = LINK_ADDR;						.global __gtsf2; __gtsf2 = __gesf2;
.global __lesf2; __lesf2 = LINK_ADDR;						.global __ltsf2; __ltsf2 = __lesf2;
.global __cmpsf2; __cmpsf2 = LINK_ADDR;						.global __nesf2; __nesf2 = __cmpsf2; .global __eqsf2; __eqsf2 = __cmpsf2;
.global __aeabi_cfrcmple; __aeabi_cfrcmple = LINK_ADDR;
.global __aeabi_cfcmpeq; __aeabi_cfcmpeq = LINK_ADDR;
.global __aeabi_fcmpeq; __aeabi_fcmpeq = LINK_ADDR;
.global __aeabi_fcmplt; __aeabi_fcmplt = LINK_ADDR;
.global __aeabi_fcmple; __aeabi_fcmple = LINK_ADDR;
.global __aeabi_fcmpge; __aeabi_fcmpge = LINK_ADDR;
.global __aeabi_fcmpgt; __aeabi_fcmpgt = LINK_ADDR;
.global __powisf2; __powisf2 = LINK_ADDR;
.global __aeabi_dneg; __aeabi_dneg = LINK_ADDR;				.global __negdf2; __negdf2 = __aeabi_dneg;
.global __aeabi_dsub; __aeabi_dsub = LINK_ADDR;				.global __subdf3; __subdf3 = __aeabi_dsub;
.global __aeabi_dadd; __aeabi_dadd = LINK_ADDR;				.global __adddf3; __adddf3 = __aeabi_dadd;
.global __aeabi_ui2d; __aeabi_ui2d = LINK_ADDR;				.global __floatunsidf; __floatunsidf = __aeabi_ui2d;
.global __aeabi_i2d; __aeabi_i2d = LINK_ADDR;				.global __floatsidf; __floatsidf = __aeabi_i2d;
.global __aeabi_f2d; __aeabi_f2d = LINK_ADDR;				.global __extendsfdf2; __extendsfdf2 = __aeabi_f2d;
.global __aeabi_d2f; __aeabi_d2f = LINK_ADDR;				.global __truncdfsf2; __truncdfsf2 = __aeabi_d2f;
.global __aeabi_ul2d; __aeabi_ul2d = LINK_ADDR;				.global __floatundidf; __floatundidf = __aeabi_ul2d;
.global __aeabi_l2d; __aeabi_l2d = LINK_ADDR;				.global __floatdidf; __floatdidf = __aeabi_l2d;
.global __aeabi_dmul; __aeabi_dmul = LINK_ADDR;				.global __muldf3; __muldf3 = __aeabi_dmul;
.global __aeabi_ddiv; __aeabi_ddiv = LINK_ADDR;				.global __divdf3; __divdf3 = __aeabi_ddiv;
.global __aeabi_dcmpun; __aeabi_dcmpun = LINK_ADDR;			.global __unorddf2; __unorddf2 = __aeabi_dcmpun;
.global __aeabi_d2iz; __aeabi_d2iz = LINK_ADDR;				.global __fixdfsi; __fixdfsi = __aeabi_d2iz;
.global __aeabi_d2uiz; __aeabi_d2uiz = LINK_ADDR;			.global __fixunsdfsi; __fixunsdfsi = __aeabi_d2uiz;
.global __aeabi_d2lz; __aeabi_d2lz = LINK_ADDR;				.global __fixdfdi; __fixdfdi = __aeabi_d2lz;
.global __aeabi_d2ulz; __aeabi_d2ulz = LINK_ADDR;			.global __fixunsdfdi; __fixunsdfdi = __aeabi_d2ulz;
.global __gnu_h2f_ieee; __gnu_h2f_ieee = LINK_ADDR;			.global __extendhfsf2; __extendhfsf2 = __gnu_h2f_ieee;
.global __gnu_f2h_ieee; __gnu_f2h_ieee = LINK_ADDR;			.global __truncsfhf2; __truncsfhf2 = __gnu_f2h_ieee;
.global __aeabi_drsub; __aeabi_drsub = LINK_ADDR;
.global __gedf2; __gedf2 = LINK_ADDR;						.global __gtdf2; __gtdf2 = __gedf2;
.global __ledf2; __ledf2 = LINK_ADDR;						.global __ltdf2; __ltdf2 = __ledf2;
.global __cmpdf2; __cmpdf2 = LINK_ADDR;						.global __nedf2; __nedf2 = __cmpdf2; .global __eqdf2; __eqdf2 = __cmpdf2;
.global __aeabi_cdrcmple; __aeabi_cdrcmple = LINK_ADDR;
.global __aeabi_cdcmpeq; __aeabi_cdcmpeq = LINK_ADDR;
.global __aeabi_dcmpeq; __aeabi_dcmpeq = LINK_ADDR;
.global __aeabi_dcmplt; __aeabi_dcmplt = LINK_ADDR;
.global __aeabi_dcmple; __aeabi_dcmple = LINK_ADDR;
.global __aeabi_dcmpge; __aeabi_dcmpge = LINK_ADDR;
.global __aeabi_dcmpgt; __aeabi_dcmpgt = LINK_ADDR;
.global __powidf2; __powidf2 = LINK_ADDR;
.global devfs_signal_callback; devfs_signal_callback = LINK_ADDR;
.global __sinit; __sinit = LINK_ADDR;
.global task_setstackguard; task_setstackguard = LINK_ADDR;
.global crt_import_argv; crt_import_argv = LINK_ADDR;
.global crt_load_data; crt_load_data = LINK_ADDR;
.global posix_trace_attr_destroy; posix_trace_attr_destroy = LINK_ADDR;
.global posix_trace_attr_getclockres; posix_trace_attr_getclockres = LINK_ADDR;
.global posix_trace_attr_getcreatetime; posix_trace_attr_getcreatetime = LINK_ADDR;
.global posix_trace_attr_getgenversion; posix_trace_attr_getgenversion = LINK_ADDR;
.global posix_trace_attr_getinherited; posix_trace_attr_getinherited = LINK_ADDR;
.global posix_trace_attr_getlogfullpolicy; posix_trace_attr_getlogfullpolicy = LINK_ADDR;
.global posix_trace_attr_getlogsize; posix_trace_attr_getlogsize = LINK_ADDR;
.global posix_trace_attr_getmaxdatasize; posix_trace_attr_getmaxdatasize = LINK_ADDR;
.global posix_trace_attr_getmaxsystemeventsize; posix_trace_attr_getmaxsystemeventsize = LINK_ADDR;
.global posix_trace_attr_getmaxusereventsize; posix_trace_attr_getmaxusereventsize = LINK_ADDR;
.global posix_trace_attr_getname; posix_trace_attr_getname = LINK_ADDR;
.global posix_trace_attr_getstreamfullpolicy; posix_trace_attr_getstreamfullpolicy = LINK_ADDR;
.global posix_trace_attr_getstreamsize; posix_trace_attr_getstreamsize = LINK_ADDR;
.global posix_trace_attr_init; posix_trace_attr_init = LINK_ADDR;
.global posix_trace_attr_setinherited; posix_trace_attr_setinherited = LINK_ADDR;
.global posix_trace_attr_setlogfullpolicy; posix_trace_attr_setlogfullpolicy = LINK_ADDR;
.global posix_trace_attr_setlogsize; posix_trace_attr_setlogsize = LINK_ADDR;
.global posix_trace_attr_setmaxdatasize; posix_trace_attr_setmaxdatasize = LINK_ADDR;
.global posix_trace_attr_setname; posix_trace_attr_setname = LINK_ADDR;
.global posix_trace_attr_setstreamsize; posix_trace_attr_setstreamsize = LINK_ADDR;
.global posix_trace_attr_setstreamfullpolicy; posix_trace_attr_setstreamfullpolicy = LINK_ADDR;
.global posix_trace_clear; posix_trace_clear = LINK_ADDR;
.global posix_trace_close; posix_trace_close = LINK_ADDR;
.global posix_trace_create; posix_trace_create = LINK_ADDR;
.global posix_trace_create_withlog; posix_trace_create_withlog = LINK_ADDR;
.global posix_trace_event; posix_trace_event = LINK_ADDR;
.global posix_trace_eventid_equal; posix_trace_eventid_equal = LINK_ADDR;
.global posix_trace_eventid_get_name; posix_trace_eventid_get_name = LINK_ADDR;
.global posix_trace_eventid_open; posix_trace_eventid_open = LINK_ADDR;
.global posix_trace_eventset_add; posix_trace_eventset_add = LINK_ADDR;
.global posix_trace_eventset_del; posix_trace_eventset_del = LINK_ADDR;
.global posix_trace_eventset_empty; posix_trace_eventset_empty = LINK_ADDR;
.global posix_trace_eventset_fill; posix_trace_eventset_fill = LINK_ADDR;
.global posix_trace_eventset_ismember; posix_trace_eventset_ismember = LINK_ADDR;
.global posix_trace_eventtypelist_getnext_id; posix_trace_eventtypelist_getnext_id = LINK_ADDR;
.global posix_trace_eventtypelist_rewind; posix_trace_eventtypelist_rewind = LINK_ADDR;
.global posix_trace_flush; posix_trace_flush = LINK_ADDR;
.global posix_trace_get_attr; posix_trace_get_attr = LINK_ADDR;
.global posix_trace_get_filter; posix_trace_get_filter = LINK_ADDR;
.global posix_trace_get_status; posix_trace_get_status = LINK_ADDR;
.global posix_trace_getnext_event; posix_trace_getnext_event = LINK_ADDR;
.global posix_trace_open; posix_trace_open = LINK_ADDR;
.global posix_trace_rewind; posix_trace_rewind = LINK_ADDR;
.global posix_trace_set_filter; posix_trace_set_filter = LINK_ADDR;
.global posix_trace_shutdown; posix_trace_shutdown = LINK_ADDR;
.global posix_trace_start; posix_trace_start = LINK_ADDR;
.global posix_trace_stop; posix_trace_stop = LINK_ADDR;
.global posix_trace_timedgetnext_event; posix_trace_timedgetnext_event = LINK_ADDR;
.global posix_trace_trid_eventid_open; posix_trace_trid_eventid_open = LINK_ADDR;
.global posix_trace_trygetnext_event; posix_trace_trygetnext_event = LINK_ADDR;
.global posix_trace_event_addr; posix_trace_event_addr = LINK_ADDR;
.global mount; mount = LINK_ADDR;
.global unmount; unmount = LINK_ADDR;
.global launch; launch = LINK_ADDR;
.global isalnum; isalnum = LINK_ADDR;
.global isalpha; isalpha = LINK_ADDR;
.global islower; islower = LINK_ADDR;
.global isupper; isupper = LINK_ADDR;
.global isdigit; isdigit = LINK_ADDR;
.global isxdigit; isxdigit = LINK_ADDR;
.global iscntrl; iscntrl = LINK_ADDR;
.global isgraph; isgraph = LINK_ADDR;
.global isspace; isspace = LINK_ADDR;
.global isblank; isblank = LINK_ADDR;
.global isprint; isprint = LINK_ADDR;
.global ispunct; ispunct = LINK_ADDR;
.global tolower; tolower = LINK_ADDR;
.global toupper; toupper = LINK_ADDR;
.global install; install = LINK_ADDR;
.global accept; accept = LINK_ADDR;
.global bind; bind = LINK_ADDR;
.global shutdown; shutdown = LINK_ADDR;
.global getpeername; getpeername = LINK_ADDR;
.global getsockname; getsockname = LINK_ADDR;
.global getsockopt; getsockopt = LINK_ADDR;
.global setsockopt; setsockopt = LINK_ADDR;
.global connect; connect = LINK_ADDR;
.global listen; listen = LINK_ADDR;
.global recv; recv = LINK_ADDR;
.global recvfrom; recvfrom = LINK_ADDR;
.global send; send = LINK_ADDR;
.global sendmsg; sendmsg = LINK_ADDR;
.global sendto; sendto = LINK_ADDR;
.global socket; socket = LINK_ADDR;
.global select; select = LINK_ADDR;
.global kernel_request_api; kernel_request_api = LINK_ADDR;
.global kernel_request; kernel_request = LINK_ADDR;
.global sos_trace_event; sos_trace_event = LINK_ADDR;
.global gethostbyname; gethostbyname = LINK_ADDR;
.global gethostbyname_r; gethostbyname_r = LINK_ADDR;
.global freeaddrinfo; freeaddrinfo = LINK_ADDR;
.global getaddrinfo; getaddrinfo = LINK_ADDR;
.global inet_addr; inet_addr = LINK_ADDR;
.global inet_ntoa; inet_ntoa = LINK_ADDR;
.global inet_ntop; inet_ntop = LINK_ADDR;
.global inet_pton; inet_pton = LINK_ADDR;
.global htonl; htonl = LINK_ADDR;
.global htons; htons = LINK_ADDR;
.global ntohl; ntohl = LINK_ADDR;
.global ntohs; ntohs = LINK_ADDR;
.global timer_create; timer_create = LINK_ADDR;
.global timer_delete; timer_delete = LINK_ADDR;
.global timer_gettime; timer_gettime = LINK_ADDR;
.global timer_settime; timer_settime = LINK_ADDR;
.global timer_getoverrun; timer_getoverrun = LINK_ADDR;
.global alarm; alarm = LINK_ADDR;
.global ualarm; ualarm = LINK_ADDR;
.global fgetwc; fgetwc = LINK_ADDR;
.global fgetws; fgetws = LINK_ADDR;
.global fputwc; fputwc = LINK_ADDR;
.global fputws; fputws = LINK_ADDR;
.global fwide; fwide = LINK_ADDR;
.global fwprintf; fwprintf = LINK_ADDR;
.global fwscanf; fwscanf = LINK_ADDR;
.global getwc; getwc = LINK_ADDR;
.global getwchar; getwchar = LINK_ADDR;
.global putwc; putwc = LINK_ADDR;
.global putwchar; putwchar = LINK_ADDR;
.global swprintf; swprintf = LINK_ADDR;
.global swscanf; swscanf = LINK_ADDR;
.global ungetwc; ungetwc = LINK_ADDR;
.global vfwprintf; vfwprintf = LINK_ADDR;
.global vfwscanf; vfwscanf = LINK_ADDR;
.global vswprintf; vswprintf = LINK_ADDR;
.global vswscanf; vswscanf = LINK_ADDR;
.global vwprintf; vwprintf = LINK_ADDR;
.global vwscanf; vwscanf = LINK_ADDR;
.global wprintf; wprintf = LINK_ADDR;
.global wscanf; wscanf = LINK_ADDR;
.global wcstod; wcstod = LINK_ADDR;
.global wcstof; wcstof = LINK_ADDR;
.global wcstol; wcstol = LINK_ADDR;
.global wcstold; wcstold = LINK_ADDR;
.global wcstoll; wcstoll = LINK_ADDR;
.global wcstoul; wcstoul = LINK_ADDR;
.global wcstoull; wcstoull = LINK_ADDR;
.global btowc; btowc = LINK_ADDR;
.global mbrlen; mbrlen = LINK_ADDR;
.global mbrtowc; mbrtowc = LINK_ADDR;
.global mbsinit; mbsinit = LINK_ADDR;
.global mbsrtowcs; mbsrtowcs = LINK_ADDR;
.global wcrtomb; wcrtomb = LINK_ADDR;
.global wctob; wctob = LINK_ADDR;
.global wcsrtombs; wcsrtombs = LINK_ADDR;
.global wcscat; wcscat = LINK_ADDR;
.global wcschr; wcschr = LINK_ADDR;
.global wcscmp; wcscmp = LINK_ADDR;
.global wcscoll; wcscoll = LINK_ADDR;
.global wcscpy; wcscpy = LINK_ADDR;
.global wcscspn; wcscspn = LINK_ADDR;
.global wcslen; wcslen = LINK_ADDR;
.global wcsncat; wcsncat = LINK_ADDR;
.global wcsncpy; wcsncpy = LINK_ADDR;
.global wcspbrk; wcspbrk = LINK_ADDR;
.global wcsrchr; wcsrchr = LINK_ADDR;
.global wcsspn; wcsspn = LINK_ADDR;
.global wcsstr; wcsstr = LINK_ADDR;
.global wcstok; wcstok = LINK_ADDR;
.global wcsxfrm; wcsxfrm = LINK_ADDR;
.global wmemchr; wmemchr = LINK_ADDR;
.global wmemcmp; wmemcmp = LINK_ADDR;
.global wmemcpy; wmemcpy = LINK_ADDR;
.global wmemmove; wmemmove = LINK_ADDR;
.global wmemset; wmemset = LINK_ADDR;
.global wcsftime; wcsftime = LINK_ADDR;
.global _ctype_; _ctype_ = LINK_ADDR;
.global __locale_mb_cur_max; __locale_mb_cur_max = LINK_ADDR;
.global _Unwind_GetRegionStart; _Unwind_GetRegionStart = LINK_ADDR;
.global _Unwind_GetTextRelBase; _Unwind_GetTextRelBase = LINK_ADDR;
.global _Unwind_GetDataRelBase; _Unwind_GetDataRelBase = LINK_ADDR;
.global _Unwind_VRS_Set; _Unwind_VRS_Set = LINK_ADDR;
.global _Unwind_Resume; _Unwind_Resume = LINK_ADDR;
.global __gnu_unwind_frame; __gnu_unwind_frame = LINK_ADDR;
.global _Unwind_GetLanguageSpecificData; _Unwind_GetLanguageSpecificData = LINK_ADDR;
.global _Unwind_Complete; _Unwind_Complete = LINK_ADDR;
.global _Unwind_DeleteException; _Unwind_DeleteException = LINK_ADDR;
.global _Unwind_RaiseException; _Unwind_RaiseException = LINK_ADDR;
.global _Unwind_Resume_or_Rethrow; _Unwind_Resume_or_Rethrow = LINK_ADDR;
.global _Unwind_VRS_Get; _Unwind_VRS_Get = LINK_ADDR;
.global __aeabi_unwind_cpp_pr0; __aeabi_unwind_cpp_pr0 = LINK_ADDR;
.global __aeabi_unwind_cpp_pr1; __aeabi_unwind_cpp_pr1 = LINK_ADDR;
.global __cxa_atexit; __cxa_atexit = LINK_ADDR;
.global getuid; getuid = LINK_ADDR;
.global setuid; setuid = LINK_ADDR;
.global geteuid; geteuid = LINK_ADDR;
.global seteuid; seteuid = LINK_ADDR;
.global sos_trace_stack; sos_trace_stack = LINK_ADDR;
.global __assert_func; __assert_func = LINK_ADDR;
.global setenv; setenv = LINK_ADDR;
.global pthread_exit; pthread_exit = LINK_ADDR;
.global pthread_testcancel; pthread_testcancel = LINK_ADDR;
.global pthread_setcancelstate; pthread_setcancelstate = LINK_ADDR;
.global pthread_setcanceltype; pthread_setcanceltype = LINK_ADDR;
.global __aeabi_atexit; __aeabi_atexit = LINK_ADDR;
.global settimeofday; settimeofday = LINK_ADDR;
.global getppid; getppid = LINK_ADDR;
.global pthread_mutex_timedlock; pthread_mutex_timedlock = LINK_ADDR;
