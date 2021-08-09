/*! \file */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "sos/debug.h"

#define BASE_VERSION (MCU_STRINGIFY(__PROJECT_VERSION_MAJOR) "." MCU_STRINGIFY(                \
  __PROJECT_VERSION_MINOR) "." MCU_STRINGIFY(__PROJECT_VERSION_PATCH))

#if MCU_DEBUG
#define VERSION (BASE_VERSION "d")
#else
#define VERSION (BASE_VERSION)
#endif

#include <stdint.h>

#include "sos/arch.h"

//this file is the user provided configuration file for making
//a few compile time tweaks
#include "sos_config.h"

// PTHREAD CONFIGURATION OPTIONS
#if !defined CONFIG_PTHREAD_MAX_LOCKS
#define CONFIG_PTHREAD_MAX_LOCKS 1024
#endif
#if !defined CONFIG_PTHREAD_MUTEX_PRIO_CEILING
#define CONFIG_PTHREAD_MUTEX_PRIO_CEILING 0
#endif
#if !defined CONFIG_PTHREAD_STACK_MIN
#define CONFIG_PTHREAD_STACK_MIN 128
#endif
#if !defined CONFIG_PTHREAD_DEFAULT_STACK_SIZE
#define CONFIG_PTHREAD_DEFAULT_STACK_SIZE 1536
#endif

// SCHED CONFIGURATION OPTIONS
#if !defined CONFIG_SCHED_LOWEST_PRIORITY
#define CONFIG_SCHED_LOWEST_PRIORITY 0
#endif
#if !defined CONFIG_SCHED_HIGHEST_PRIORITY
#define CONFIG_SCHED_HIGHEST_PRIORITY 31
#endif
#if !defined CONFIG_SCHED_DEFAULT_PRIORITY
#define CONFIG_SCHED_DEFAULT_PRIORITY 0
#endif

#if !defined CONFIG_SCHED_RR_DURATION
#define CONFIG_SCHED_RR_DURATION 10
#endif

//If the chip has double precision floating point and only 8 sections
//this needs to be set to zero
#if !defined CONFIG_TASK_MPU_REGION_OFFSET
#define CONFIG_TASK_MPU_REGION_OFFSET 8
#endif

// Task options
// total number of threads (system and application)
// more tasks use require more sysmem
#if !defined CONFIG_TASK_TOTAL
#define CONFIG_TASK_TOTAL 16
#endif

#if !defined CONFIG_TASK_PROCESS_TIMER_COUNT
#define CONFIG_TASK_PROCESS_TIMER_COUNT 4
#endif

#if !defined CONFIG_TASK_DEFAULT_STACKGUARD_SIZE
#define CONFIG_TASK_DEFAULT_STACKGUARD_SIZE 128
#endif

#if !defined CONFIG_TASK_NUM_SIGNALS
#define CONFIG_TASK_NUM_SIGNALS 32
#endif

//make this larger for less efficient but less fragmented heap
#if !defined CONFIG_MALLOC_CHUNK_SIZE
#define CONFIG_MALLOC_CHUNK_SIZE 32
#endif

//make this larger for less efficient but faster heap allocation
#if !defined CONFIG_MALLOC_SBRK_JUMP_SIZE
#define CONFIG_MALLOC_SBRK_JUMP_SIZE 128
#endif

// require a valid digital signature when installing applications
#if !defined CONFIG_APPFS_IS_VERIFY_SIGNATURE
#define CONFIG_APPFS_IS_VERIFY_SIGNATURE 1
#endif
#if !defined CONFIG_BOOT_IS_VERIFY_SIGNATURE// require the OS to be digitally signed
#define CONFIG_BOOT_IS_VERIFY_SIGNATURE 1
#endif

// Bootloader provides AES cryptography using a device unique secret key
#if !defined CONFIG_BOOT_IS_AES_CRYPTO
#define CONFIG_BOOT_IS_AES_CRYPTO 1
#endif

#if !defined CONFIG_USE_STDIO
#define CONFIG_USE_STDIO 1
#endif

#define TASK_MPU_REGION_OFFSET (sos_config.mcu.task_mpu_region_offset)

// higher numbers take precedence over lower numbers
#define TASK_SYSTEM_SECRET_KEY_REGION (7+TASK_MPU_REGION_OFFSET)
#define TASK_SYSTEM_STACK_MPU_REGION (6+TASK_MPU_REGION_OFFSET)
#define TASK_SYSTEM_CODE_MPU_REGION (5+TASK_MPU_REGION_OFFSET)
#define TASK_SYSTEM_DATA_MPU_REGION (4+TASK_MPU_REGION_OFFSET)
// stack guard must have higher precedence than application data because they overlap
#define TASK_APPLICATION_STACK_GUARD_REGION (3+TASK_MPU_REGION_OFFSET)
#define TASK_APPLICATION_CODE_MPU_REGION (2+TASK_MPU_REGION_OFFSET)
#define TASK_APPLICATION_DATA_MPU_REGION (1+TASK_MPU_REGION_OFFSET)
#define TASK_APPLICATION_DATA_USER_REGION (0+TASK_MPU_REGION_OFFSET)

#endif
