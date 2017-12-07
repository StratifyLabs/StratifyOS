/* Copyright 2011-2016 Tyler Gilbert; 
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

/*! \addtogroup CORE Core Access
 * @{
 *
 *
 *
 * \details The Core module allows access to the core CPU functionality such as clock speed, power, and
 * interrupts.  In general, the OS should make extensive use of the Core access API while the device
 * drivers use the \ref IFACE_DEV API.  To maximize portability, applications should rely on the
 * OS abstraction of all hardware.
 *
 *
 */

/*! \file
 * \brief Core Functionality Header File
 *
 */

#ifndef _MCU_CORE_H_
#define _MCU_CORE_H_

#include "mcu/types.h"
#include "mcu/mcu.h"
#include "sos/dev/core.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif


int mcu_core_open(const devfs_handle_t * cfg) MCU_ROOT_CODE;
int mcu_core_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_core_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_core_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_ROOT_CODE;
int mcu_core_close(const devfs_handle_t * cfg) MCU_ROOT_CODE;

int mcu_core_getinfo(const devfs_handle_t * handle, void * arg) MCU_ROOT_CODE;
int mcu_core_setattr(const devfs_handle_t * handle, void * arg) MCU_ROOT_CODE;
int mcu_core_setaction(const devfs_handle_t * handle, void * arg) MCU_ROOT_CODE;
int mcu_core_setpinfunc(const devfs_handle_t * handle, void * arg) MCU_ROOT_CODE;
int mcu_core_setclkout(const devfs_handle_t * handle, void * arg) MCU_ROOT_CODE;
int mcu_core_setclkdivide(const devfs_handle_t * handle, void * arg) MCU_ROOT_CODE;
int mcu_core_getmcuboardconfig(const devfs_handle_t * handle, void * arg) MCU_ROOT_CODE;

//below are undocumented calls that can be made by BSPs but aren't accessible to applications
static inline int mcu_core_getclock() MCU_ALWAYS_INLINE;
int mcu_core_getclock(){ return mcu_board_config.core_cpu_freq; }

void mcu_core_get_bootloader_api(void * args) MCU_ROOT_CODE;
void mcu_core_set_nvic_priority(int irq, int prio) MCU_ROOT_CODE;
void mcu_core_setclock(int fclk, int fosc) MCU_ROOT_CODE;
int mcu_core_initclock(int div) MCU_ROOT_CODE;
void mcu_core_setclockinternal(int fclk) MCU_ROOT_CODE;
void mcu_core_setclock_main_12mhz_72mhz() MCU_ROOT_CODE;
int mcu_core_setusbclock(int fosc /*! The oscillator frequency */) MCU_ROOT_CODE;
int mcu_core_invokebootloader(int port, void * arg) MCU_ROOT_CODE;


typedef enum {
	CORE_SLEEP /*! Sleep mode */,
	CORE_DEEPSLEEP /*! Deep sleep (preserve SRAM) */,
	CORE_DEEPSLEEP_STOP /*! Deep sleep (preserve SRAM, stop clocks) */,
	CORE_DEEPSLEEP_STANDBY /*! Turn the device off (lose SRAM) */
} core_sleep_t;


int mcu_core_user_sleep(core_sleep_t level);
void mcu_set_sleep_mode(int * level) MCU_ROOT_CODE;
int mcu_core_execsleep(int port, void * arg) MCU_ROOT_CODE;
int mcu_core_reset(int port, void * arg) MCU_ROOT_CODE;
int mcu_core_set_pinsel_func(const mcu_pin_t * pin, core_periph_t function, int periph_port) MCU_ROOT_CODE;
void mcu_core_getserialno(mcu_sn_t * serialno) MCU_ROOT_CODE;



#ifdef __cplusplus
}
#endif

#endif // _MCU_CORE_H_

/*! @} */
