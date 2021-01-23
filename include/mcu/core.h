// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup CORE Core Access
 * @{
 *
 *
 *
 * \details The Core module allows access to the core CPU functionality such as clock
 * speed, power, and interrupts.  In general, the OS should make extensive use of the Core
 * access API while the device drivers use the \ref IFACE_DEV API.  To maximize
 * portability, applications should rely on the OS abstraction of all hardware.
 *
 *
 */

/*! \file
 * \brief Core Functionality Header File
 *
 */

#ifndef _MCU_CORE_H_
#define _MCU_CORE_H_

#include <sdk/types.h>

#include "sos/dev/core.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int mcu_core_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_core_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_core_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_core_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_core_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_core_getinfo(const devfs_handle_t *handle, void *arg) MCU_ROOT_CODE;
int mcu_core_setattr(const devfs_handle_t *handle, void *arg) MCU_ROOT_CODE;
int mcu_core_setaction(const devfs_handle_t *handle, void *arg) MCU_ROOT_CODE;

// below are undocumented calls that can be made by BSPs but aren't accessible to
// applications

int mcu_core_invokebootloader(int port, void *arg) MCU_ROOT_CODE;

typedef enum {
  CORE_SLEEP /*! Sleep mode */,
  CORE_DEEPSLEEP /*! Deep sleep (preserve SRAM) */,
  CORE_DEEPSLEEP_STOP /*! Deep sleep (preserve SRAM, stop clocks) */,
  CORE_DEEPSLEEP_STANDBY /*! Turn the device off (lose SRAM) */
} core_sleep_t;

void mcu_core_prepare_deepsleep(int level);
void mcu_core_recover_deepsleep(int level);

int mcu_core_user_sleep(core_sleep_t level);
void mcu_set_sleep_mode(int *level) MCU_ROOT_CODE;
int mcu_core_execsleep(int port, void *arg) MCU_ROOT_CODE;
int mcu_core_reset(int port, void *arg) MCU_ROOT_CODE;
int mcu_core_set_pinsel_func(
  const mcu_pin_t *pin,
  core_periph_t function,
  int periph_port) MCU_ROOT_CODE;
void mcu_core_getserialno(mcu_sn_t *serialno) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif // _MCU_CORE_H_

/*! @} */
