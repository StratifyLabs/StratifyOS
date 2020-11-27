#ifndef SOS_SOS_CONFIG_H
#define SOS_SOS_CONFIG_H

#include <sdk/types.h>

#include "cortexm/task_types.h"
#include "fs/devfs.h"
#include "sys/socket.h"
#include "trace.h"

#define SOS_SCHEDULER_TIMEVAL_SECONDS 2048
#define SOS_USECOND_PERIOD (1000000UL * SOS_SCHEDULER_TIMEVAL_SECONDS)
#define SOS_PROCESS_TIMER_COUNT 4

typedef struct {
  u32 o_flags;
  struct mcu_timeval value;
  struct mcu_timeval interval;
  struct sigevent sigevent;
} sos_process_timer_t;

typedef struct {
  pthread_attr_t attr /*! This holds the task's pthread attributes */;
  volatile void *block_object /*! The blocking object */;
  union {
    volatile int exit_status /*! The task's exit status */;
    void *(*init)(void *)/*! Task 0 init routine */;
  };
  pthread_mutex_t
    *signal_delay_mutex /*! The mutex to lock if the task cannot be interrupted */;
  volatile struct mcu_timeval wake /*! When to wake the task */;
  volatile u16 flags /*! This indicates whether the process is active or not */;
  trace_id_t trace_id /*! Trace ID is PID is being traced (0 otherwise) */;
  sos_process_timer_t timer[SOS_PROCESS_TIMER_COUNT];
} sched_task_t;

typedef struct {
  const devfs_device_t *devfs_list;
  const sysfs_t *rootfs_list;
  const char *stdin_dev;
  const char *stdout_dev;
  const char *stderr_dev;
  const char *trace_dev;
} sos_fs_config_t;

typedef struct {
  void (*write)(const void *buf, int nbyte);
  void (*trace_event)(void *event);
  u32 flags;
} sos_debug_config_t;

typedef struct {
  void (*initialize)();
  void (*enable)();
  u32 (*disable)();
  void (*set_channel)(const mcu_channel_t *channel);
  void (*get_channel)(mcu_channel_t *channel);
  u32 (*microseconds)();
  u32 (*nanoseconds)();
  u32 frequency;
} sos_clock_config_t;

typedef struct {
  u8 task_total;
  u16 start_stack_size;
  void *(*start)(void *);
  void *start_args;
} sos_task_config_t;

typedef struct {
  u32 memory_size;
  u32 os_mpu_text_mask;
  u32 flags;
  u32 secret_key_size;
#if defined __link
  u32 name;
  u32 version;
  u32 git_hash;
  u32 id;
  u32 team_id;
  u32 secret_key_address;
  u32 event_handler;
#else
  const char *name;
  const char *version;
  const char *git_hash;
  const char *id;
  const char *team_id;
  const void *secret_key_address;
  int (*kernel_request)(int request, void *data);
  const void *(*kernel_request_api)(u32 request);
#endif
} sos_sys_config_t;

typedef struct {
  void (*enable)();
  void (*disable)();
  void (*invalidate_instruction)();
  void (*invalidate_instruction_block)(u32 address, u32 size);
  void (*clean_instruction)();
  void (*clean_instruction_block)(u32 address, u32 size);
  void (*invalidate_data)();
  void (*invalidate_data_block)(u32 address, u32 size);
  void (*clean_data)();
  void (*clean_data_block)(u32 address, u32 size);
} sos_cache_config_t;

/*! \brief Stratify Board Configuration Structure
 * \details This structure holds the compiler-link time
 * configuration data.
 */
typedef struct MCU_PACK {
  sos_fs_config_t fs;
  sos_clock_config_t clock;
  sos_task_config_t task;
  sos_sys_config_t sys;
  sos_debug_config_t debug;
  sos_cache_config_t cache;
  const sos_socket_api_t *socket_api;
  void (*event_handler)(int, void *);
} sos_config_t;

#define SOS_DEFAULT_START_STACK_SIZE 2048
#if !defined __link

// must be provided by board support package
extern volatile sched_task_t sos_sched_table[];
extern volatile task_t sos_task_table[];
extern const sos_config_t sos_config;

#define SOS_DECLARE_TASK_TABLE(task_count)                                               \
  volatile sched_task_t sos_sched_table[task_count] MCU_SYS_MEM;                         \
  volatile task_t sos_task_table[task_count] MCU_SYS_MEM
#endif

#define SOS_USER_ROOT 0
#define SOS_USER 1

#define SOS_DECLARE_SECRET_KEY_32(x)                                                     \
  static void x() MCU_ALIGN(32);                                                         \
  void x() {                                                                             \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
    asm volatile("NOP");                                                                 \
  }

#include "events.h"

#endif // SOS_CONFIG_H
