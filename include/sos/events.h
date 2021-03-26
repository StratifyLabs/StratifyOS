// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_SOS_EVENTS_H
#define SOS_SOS_EVENTS_H

#include <sdk/types.h>

enum {
  SOS_EVENT_ROOT_RESET,
  SOS_EVENT_ROOT_FATAL,
  SOS_EVENT_ROOT_TASK_INITIALIZED,
  SOS_EVENT_ROOT_INITIALIZED,
  SOS_EVENT_ROOT_WDT_TIMEDOUT,
  SOS_EVENT_ROOT_DEBUG_INITIALIZED,
  SOS_EVENT_ROOT_MPU_INITIALIZED,
  SOS_EVENT_ROOT_ENABLE_CACHE = SOS_EVENT_ROOT_MPU_INITIALIZED,
  SOS_EVENT_ROOT_PREPARE_DEEPSLEEP,
  SOS_EVENT_ROOT_RECOVER_DEEPSLEEP,
  SOS_EVENT_ROOT_INVALID_PIN_ASSIGNMENT,
  SOS_EVENT_ROOT_APPLICATION_FAULT,
  SOS_EVENT_FATAL,
  SOS_EVENT_ERROR,
  SOS_EVENT_ENTER_FIRST_THREAD,
  SOS_EVENT_RESET_SOURCE,
  SOS_EVENT_FILESYSTEM_INITIALIZED,
  SOS_EVENT_START_LINK,
  SOS_EVENT_HIBERNATE,
  SOS_EVENT_POWERDOWN,
  SOS_EVENT_MALLOC_FAILED,
  SOS_EVENT_WAKEUP_FROM_HIBERNATE,
  SOS_EVENT_SERVICE_CALL_PERMISSION_DENIED,
  SOS_EVENT_FIFO_INIT_REQUESTED,
  SOS_EVENT_FIFO_DATA_RECEIVED,
  SOS_EVENT_FIFO_DATA_TRANSMITTED,
  SOS_EVENT_FIFO_WRITE,
  SOS_EVENT_FIFO_READ,
  SOS_EVENT_BOOT_RESET,
  SOS_EVENT_BOOT_RESET_BOOTLOADER,
  SOS_EVENT_BOOT_RUN_APPLICATION,
  SOS_EVENT_BOOT_RUN_BOOTLOADER,
  SOS_EVENT_BOOT_READ_SERIAL_NUMBER,
  SOS_EVENT_BOOT_WRITE_FLASH,
  SOS_EVENT_BOOT_ERASE_FLASH,
  SOS_EVENT_ASSERT
};

#if defined __cplusplus
extern "C" {
#endif

void sos_handle_event(int event, void *args);

#if defined __cplusplus
}
#endif

#endif // SOS_EVENTS_H
