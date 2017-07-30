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

/*! \addtogroup UART_DEV UART
 * @{
 *
 * \ingroup DEV
 *
 */

#ifndef _MCU_UART_H_
#define _MCU_UART_H_

#include "sos/dev/uart.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	u32 value;
} uart_event_t;

typedef struct MCU_PACK {
	uart_attr_t attr; //default attributes
} uart_config_t;


int mcu_uart_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_uart_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_uart_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_uart_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_uart_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;

int mcu_uart_getinfo(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_uart_setattr(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_uart_setaction(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;

int mcu_uart_get(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_uart_put(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_uart_flush(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif // _MCU_UART_H_

/*! @} */

