/*! \file */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "sos/debug.h"

#if MCU_DEBUG
#define VERSION "3.16.0d"
#else
#define VERSION "3.16.0"
#endif


#include <stdint.h>


#define PTHREAD_MAX_LOCKS 1024
#define PTHREAD_MUTEX_PRIO_CEILING 0
#define PTHREAD_STACK_MIN 128
#define SCHED_LOWEST_PRIORITY 0
#define SCHED_HIGHEST_PRIORITY 31
#define SCHED_DEFAULT_PRIORITY 0
#define SCHED_RR_DURATION 10

#define USE_STDIO 1

#include "sos/arch.h"

#endif
