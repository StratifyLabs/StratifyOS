// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \file
 * \brief MCU Header file (includes all other header files)
 */

/*! \addtogroup MCU
 *
 * @{
 *
 *
 */

#ifndef MCU_MCU_H_
#define MCU_MCU_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sos/config.h"
#include "sos/dev/pio.h"

/*! @} */

/*! \addtogroup IFACE_DEV
 * @{
 */

/*! @} */

/*! \addtogroup MCU
 * @{
 */

#include <sdk/types.h>
#include "sos/dev/uart.h"
#include "sos/dev/usb.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined __link

#include "sos/fs/types.h"

// These values are defined in the linker script
extern u32 _top_of_stack;
extern u32 _text;
extern u32 _etext;
extern u32 _tcim;
extern u32 _etcim;
extern u32 _data;
extern u32 _edata;
extern u32 _bss;
extern u32 _ebss;
extern u32 _sys;
extern u32 _esys;

#define MCU_SYNC_IO_FLAG_READ (1 << 15)

int mcu_sync_io(
  const devfs_handle_t *handle,
  int (*func)(const devfs_handle_t *handle, devfs_async_t *op),
  int loc,
  const void *buf,
  int nbyte,
  int flags);

// execute an event callback -- callback is null'd if result is 0 -- deprecated
int mcu_execute_event_handler(mcu_event_handler_t *handler, u32 o_events, void *data);

static inline const mcu_pin_t *mcu_pin_at(const void *start, int i) {
  const mcu_pin_t *p = (const mcu_pin_t *)start;
  return p + i;
}

const void *mcu_select_attr(const devfs_handle_t *handle, void *ctl);

int mcu_set_pin_assignment(
  const void *attr_pin_assignment,
  const void *config_pin_assignment,
  int count,
  int periph,
  int periph_port,
  void (*pre_configure_pin)(const mcu_pin_t *, void *),
  void (*post_configure_pin)(const mcu_pin_t *, void *),
  void *arg) MCU_ROOT_CODE;

bootloader_api_t *mcu_core_get_bootloader_api();

const void *mcu_select_pin_assignment(
  const void *attr_pin_assignment,
  const void *config_pin_assignment,
  int count) MCU_ROOT_CODE;
#endif

#define MCU_DECLARE_SECRET_KEY_32(x)                                                     \
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

#ifdef __cplusplus
}
#endif

#endif /* MCU_MCU_H_ */

/*! @} */
