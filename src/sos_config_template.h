
#ifndef CONFIG_SOS_CONFIG_H
#define CONFIG_SOS_CONFIG_H

//This file should be copied to another folder and renamed sos_config.h
//The folder should assigned to SOS_CONFIG_PATH by the super project


// PTHREAD CONFIGURATION OPTIONS
#define CONFIG_PTHREAD_MAX_LOCKS 1024
#define CONFIG_PTHREAD_MUTEX_PRIO_CEILING 0
#define CONFIG_PTHREAD_STACK_MIN 128
#define CONFIG_PTHREAD_DEFAULT_STACK_SIZE 1536

// SCHED CONFIGURATION OPTIONS
#define CONFIG_SCHED_LOWEST_PRIORITY 0
#define CONFIG_SCHED_HIGHEST_PRIORITY 31
#define CONFIG_SCHED_DEFAULT_PRIORITY 0
// duration is in milliseconds
#define CONFIG_SCHED_RR_DURATION 10

// Task options
// total number of threads (system and application)
// more tasks use require more sysmem
#define CONFIG_TASK_TOTAL 16
#define CONFIG_TASK_PROCESS_TIMER_COUNT 2
#define CONFIG_TASK_DEFAULT_STACKGUARD_SIZE 128

#define CONFIG_TASK_NUM_SIGNALS 32

#define CONFIG_MALLOC_CHUNK_SIZE 32
#define CONFIG_MALLOC_SBRK_JUMP_SIZE 128

// require a valid digital signature when installing applications
#define CONFIG_APPFS_IS_VERIFY_SIGNATURE 1
// require the OS to be digitally signed
#define CONFIG_BOOT_IS_VERIFY_SIGNATURE 1

// Bootloader provides AES cryptography using a device unique secret key
#define CONFIG_BOOT_IS_AES_CRYPTO 1

#define CONFIG_USE_STDIO 1

#endif /* CONFIG_SOS_CONFIG_H */
