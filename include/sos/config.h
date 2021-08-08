// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef STRATIFYOS_SOS_CONFIG_H
#define STRATIFYOS_SOS_CONFIG_H

#include <sdk/types.h>

#include "dev/bootloader.h"
#include "dev/pio.h"
#include "fs/devfs.h"
#include "dev/appfs.h"
#include "trace.h"

typedef struct MCU_PACK {
  // pointer to device filesystem, required
  const devfs_device_t *devfs_list;
  // pointer to device filesystem, required
  const sysfs_t *rootfs_list;
  // name of device in /dev for standard input
  const char *stdin_dev;
  // name of device in /dev for standard output
  const char *stdout_dev;
  // name of device in /dev for standard error
  // this can be the same as stdout
  const char *stderr_dev;
  // device name for system trace (ie /dev/trace)
  const char *trace_dev;
} sos_fs_config_t;

typedef struct MCU_PACK {
  // OR SOS_DEBUG_* flags
  u32 flags;
  // initialize serial tracing
  void (*initialize)();
  // write to serial trace output
  void (*write)(const void *buf, int nbyte);
  // write to trace buffer (root call)
  void (*trace_event)(void *event);
  // enable the board LED (root call)
  void (*enable_led)();
  // disable the board LED (root call)
  void (*disable_led)();
} sos_debug_config_t;

// Stratify OS needs a 32-bit microsecond timer for all timing
// of the OS. You can optionally provide submicrosecond
// precision with the nanoseconds() callback
typedef struct MCU_PACK {
  // initialize timing
  void (*initialize)(
    int (*handle_match_channel0)(void *context, const mcu_event_t *data),
    int (*handle_match_channel1)(void *context, const mcu_event_t *data),
    int (*handle_overflow)(void *context, const mcu_event_t *data));
  // turn the timer on
  void (*enable)();
  // turn the timer off and return current value
  u32 (*disable)();
  // set the value of the compare channel
  void (*set_channel)(const mcu_channel_t *channel);
  // get the value of the compare channel
  void (*get_channel)(mcu_channel_t *channel);
  // return the clock's microsecond cound
  u32 (*microseconds)();
  // use for sub-microsecond precision
  // just return 0 if not available
  u32 (*nanoseconds)();
} sos_clock_config_t;

typedef struct MCU_PACK {
  // total number of tasks to support
  // u8 task_total;
  // stack size of the first thread
  u16 start_stack_size;
  // function of the first thread
  void *(*start)(void *);
  // arguments passed to start()
  void *start_args;
} sos_task_config_t;

// This is only needed if you are using USB
typedef struct MCU_PACK {
  // size of control EP (usually 64)
  u32 control_endpoint_max_size;
  // number of logical endpoints
  u16 logical_endpoint_count;
  // set the USB attributes -- see the STM32 example
  int (*set_attributes)(const devfs_handle_t *handle, void *ctl);
  // set a callback for the spec'd action -- see the STM32 example
  int (*set_action)(const devfs_handle_t *handle, mcu_action_t *action);
  // root write to the specified endpoint
  void (*write_endpoint)(
    const devfs_handle_t *handle,
    u32 endpoint_num,
    const void *src,
    u32 size);
  // root read from the specified endpoint
  int (*read_endpoint)(const devfs_handle_t *handle, u32 endpoint_num, void *dest);
} sos_usb_config_t;

typedef struct MCU_PACK {
  // pointer to secret_key_size-bytes used as a secret system key
  const void *secret_key_address;
  // size of secret key (usually 32 bytes)
  u32 secret_key_size;
  // number of bytes used for the system heap/stack space
  u32 memory_size;
  // mask for MPU is OS start is offset from boundary by a small amount
  u32 os_mpu_text_mask;
  u32 flags;
  // target core clock frequency
  u32 core_clock_frequency;
  // beginning of the bootloader (set to 0xffffffff) if no bootloader
  u32 bootloader_start_address;
  // null-terminated name of the board
  const char *name;
  // null-terminated version
  const char *version;
  // SOS_GIT_HASH
  const char *git_hash;
  // unused
  const char *mcu_git_hash;
  // OS package ID -- can be ""
  const char *id;
  // team ID -- can be ""
  const char *team_id;
  // location of the reset vector table
  void *vector_table;
  // root initialization routine
  void (*initialize)();
  // these are convenience functions for the board support package
  // they are not used by Stratify OS
  // root pin IO set attributes -- see STM32 example
  void (*pio_set_attributes)(int port, const pio_attr_t *attr);
  // root pin IO write
  void (*pio_write)(int port, u32 mask, int value);
  // root pin IO read
  u32 (*pio_read)(int port, u32 mask);
  // function to get the MCU serial number
  void (*get_serial_number)(mcu_sn_t *serial_number);
  // user implementation of the kernel_request() available to apps
  int (*kernel_request)(int request, void *data);
  // this is how "shared" libraries are implemented -- see STM32 example
  const void *(*kernel_request_api)(u32 request);
  //a function to provide the requested application key
  //This is run in root mode
  int (*get_public_key)(u32 index, appfs_public_key_t * dest);
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

// only needed on cortex M7
// can be left as do nothing functions
// if the cache is not used
typedef struct MCU_PACK {
  void (*enable)();
  void (*disable)();
  void (*invalidate_instruction)();
  void (*invalidate_data)();
  void (*invalidate_data_block)(void *address, size_t size);
  void (*clean_data)();
  void (*clean_data_block)(void *address, size_t size);
  u32 external_sram_policy;
  u32 sram_policy;
  u32 flash_policy;
  u32 peripherals_policy;
  u32 lcd_policy;
  u32 external_flash_policy;
  u32 tightly_coupled_data_policy;
  u32 tightly_coupled_instruction_policy;
  u32 os_code_mpu_type;
  u32 os_data_mpu_type;
  u32 os_system_data_mpu_type;
} sos_cache_config_t;

/* Cache policy bits -- others are zero
 * Bits 19, 20, 21 = TEX
 * Bit 18 = shareable (can't be cached?) (S)
 * Bit 17 = cacheable (C)
 * Bit 16 = write-back (B)
 *
 *
 */

#define SOS_CACHE_DEFAULT_POLICY                                                         \
  ((1 << 21) | (1 << 20) | (1 << 19) | (0 << 18) | (1 << 17) | (0 << 16))
#define SOS_CACHE_PERIPHERALS_POLICY (((1 << 16) | (1 << 18)))

#define SOS_CACHE_WRITE_BACK_NO_WRITE_ALLOCATE                                           \
  ((0 << 21) | (0 << 20) | (0 << 19) | (0 << 18) | (1 << 17) | (1 << 16))

#define SOS_CACHE_WRITE_BACK_READ_WRITE_ALLOCATE                                         \
  ((0 << 21) | (0 << 20) | (1 << 19) | (0 << 18) | (1 << 17) | (1 << 16))

typedef struct MCU_PACK {
  // called during RR scheduling when no tasks are active
  void (*idle)();
  // called when application calls hibernate()
  void (*hibernate)(int seconds);
  // called when appliation calls powerdown()
  void (*powerdown)();
} sos_sleep_config_t;

typedef struct MCU_PACK {
  u16 interrupt_request_total;
  u16 interrupt_middle_priority;
  void (*set_interrupt_priority)(int number, int priority);
  void (*reset_watchdog_timer)();
  int (*set_pin_function)(const mcu_pin_t *pin, int function, int periph_port);
} sos_mcu_config_t;

typedef struct MCU_PACK {
  bootloader_api_t api;
  u32 program_start_address;
  u32 software_bootloader_request_address;
  u32 software_bootloader_request_value;
  int (*is_bootloader_requested)();
  devfs_handle_t flash_handle;
  int (*flash_erase_page)(const devfs_handle_t *handle, void *ctl);
  int (*flash_write_page)(const devfs_handle_t *handle, void *ctl);
  link_transport_driver_t *link_transport_driver;
} sos_boot_config_t;


typedef struct MCU_PACK {
  sos_sys_config_t sys;
  sos_mcu_config_t mcu;
  sos_boot_config_t boot;
  sos_fs_config_t fs;
  sos_clock_config_t clock;
  sos_task_config_t task;
  sos_debug_config_t debug;
  sos_cache_config_t cache;
  sos_sleep_config_t sleep;
  sos_usb_config_t usb;
  const void *socket_api;
  void (*event_handler)(int, void *);
} sos_config_t;

#if !defined __link

#if defined __cplusplus
extern "C" {
#endif

// must be provided by board support package
extern const sos_config_t sos_config;

#if defined __cplusplus
}
#endif

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

#define SOS_DECLARE_PUBLIC_KEY_64(x)                                                     \
  static void x() MCU_ALIGN(64);                                                         \
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

#define SOS_SCHEDULER_TIMEVAL_SECONDS 2048
#define SOS_USECOND_PERIOD (1000000UL * SOS_SCHEDULER_TIMEVAL_SECONDS)

#endif // STRATIFYOS_SOS_CONFIG_H
