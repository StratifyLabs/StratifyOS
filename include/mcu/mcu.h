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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

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

typedef struct MCU_PACK {
  u16 irq_total /* Total number of interrupts */;
  u16 irq_middle_prio /* Middle priority value */;
  u16 usb_logical_endpoint_count /* Number of logical endpoints */;
  u16 delay_factor /* factor to multiply by when calculating delays (depends on memory
                      performance) */
    ;
  const char *git_hash /* Pointer to git hash string */;
} mcu_config_t;

extern const mcu_config_t mcu_config;

void sos_handle_event(int event, void *args);

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
