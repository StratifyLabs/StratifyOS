#ifndef SOS_CONFIG_H
#define SOS_CONFIG_H

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
  u32 flags;
  void (*initialize)();
  void (*write)(const void *buf, int nbyte);
  void (*trace_event)(void *event);
  void (*enable_led)();
  void (*disable_led)();
} sos_debug_config_t;

typedef struct {
  void (*initialize)(
    int (*handle_match_channel0)(void *context, const mcu_event_t *data),
    int (*handle_match_channel1)(void *context, const mcu_event_t *data),
    int (*handle_overflow)(void *context, const mcu_event_t *data));
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
  // low level USB API
  int (*set_attributes)(const devfs_handle_t *handle, void *ctl);
  int (*set_action)(const devfs_handle_t *handle, mcu_action_t *action);
  void (*write_endpoint)(
    const devfs_handle_t *handle,
    u32 endpoint_num,
    const void *src,
    u32 size);
  int (*read_endpoint)(const devfs_handle_t *handle, u32 endpoint_num, void *dest);
} sos_usb_config_t;

typedef struct {
  u32 memory_size;
  u32 os_mpu_text_mask;
  u32 flags;
  u32 secret_key_size;
  const char *name;
  const char *version;
  const char *git_hash;
  const char *mcu_git_hash;
  const char *id;
  const char *team_id;
  const void *secret_key_address;
  void (*initialize)();
  void (*get_serial_number)(mcu_sn_t *serial_number);
  int (*kernel_request)(int request, void *data);
  const void *(*kernel_request_api)(u32 request);
} sos_sys_config_t;

/*
 *
 * Clean -- write values in cache to memory
 * A cache clean operation ensures that updates made by an observer that controls
 * the cache are made visible to other observers that can access memory at the point
 * to which the operation is performed. Once the Clean has completed, the new memory
 * values are guaranteed to be visible to the point to which the operation is performed,
 * for example to the point of unification. The cleaning of a cache entry from a cache can
 * overwrite memory that has been written by another observer only if the entry
 * contains a location that has been written to by an observer in the shareability
 * domain of that memory location.
 *
 * Invalidate -- pull in values from memory to cache
 * A cache invalidate operation ensures that updates made visible by observers that
 *	access memory at the point to which the invalidate is defined are made visible
 * to an observer that controls the cache. This might result in the loss of updates
 * to the locations affected by the invalidate operation that have been written
 * by observers that access the cache. If the address of an entry on which the
 * invalidate operates does not have a Normal Cacheable attribute, or if the cache
 * is disabled, then an invalidate operation also ensures that this address is not
 * present in the cache.
 *
 */

typedef struct {
  void (*enable)();
  void (*disable)();
  void (*invalidate_instruction)();
  void (*invalidate_data)();
  void (*invalidate_data_block)(void *address, size_t size);
  void (*clean_data)();
  void (*clean_data_block)(void *address, size_t size);
} sos_cache_config_t;

typedef struct {
  void (*idle)();
  void (*hibernate)(int seconds);
  void (*powerdown)();
} sos_sleep_config_t;

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
  sos_sleep_config_t sleep;
  sos_usb_config_t usb;
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
