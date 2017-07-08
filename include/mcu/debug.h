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

#ifndef _MCU_DEBUG_H_
#define _MCU_DEBUG_H_

#include <stdio.h>
#include <string.h>
#include "mcu/cortexm.h"
#include "mcu/core.h"

#include "sos/fs/devfs.h"
#include "uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __link

#define MCU_DEBUG_PORT 0

#if !defined ___debug
#define mcu_debug_init() 0
#define mcu_priv_write_debug_uart(x)
#define mcu_debug(...)
#define mcu_priv_debug(...)
#define mcu_debug_irq(...)
#define USB_DEV_DEBUG_INIT()
#define USB_DEV_STAT() 0
#else
#define MCU_DEBUG 1
int mcu_debug_init();
void mcu_priv_write_debug_uart(void * args);

#define MCU_DEBUG_BUFFER_SIZE 256
extern char mcu_debug_buffer[MCU_DEBUG_BUFFER_SIZE];

//UART debugging
#define mcu_debug(...) do { \
	siprintf(mcu_debug_buffer, __VA_ARGS__); \
	mcu_core_privcall(mcu_priv_write_debug_uart, NULL); \
} while(0)

#define mcu_priv_debug(...) do { \
	siprintf(mcu_debug_buffer, __VA_ARGS__); \
	mcu_priv_write_debug_uart(NULL); \
} while(0)

#define mcu_debug_irq(...) do { \
	int bytes; \
	devfs_async_t op; \
	device_periph_t periph; \
	siprintf(mcu_debug_buffer, __VA_ARGS__); \
	bytes = strlen(mcu_debug_buffer); \
	periph.port = MCU_DEBUG_PORT; \
	op.buf = mcu_debug_buffer; \
	op.nbyte = bytes; \
	op.handler.callback = NULL; \
	mcu_uart_write((const devfs_handle_t *)&periph, &op); \
} while(0)

extern volatile int usbdev_stat;
#define USB_DEV_STAT() (usbdev_stat)

#endif


#endif

#ifdef __cplusplus
}
#endif


#endif /* DEBUG_H_ */
