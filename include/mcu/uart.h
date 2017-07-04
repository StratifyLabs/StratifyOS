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

#include "iface/device_config.h"
#include "iface/dev/uart.h"

#ifdef __cplusplus
extern "C" {
#endif


int mcu_uart_open(const device_cfg_t * cfg) MCU_PRIV_CODE;
int mcu_uart_read(const device_cfg_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
int mcu_uart_write(const device_cfg_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
int mcu_uart_ioctl(const device_cfg_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_uart_close(const device_cfg_t * cfg) MCU_PRIV_CODE;

int mcu_3_uart_open(const dev_handle_t * cfg) MCU_PRIV_CODE;
int mcu_3_uart_read(const dev_handle_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
int mcu_3_uart_write(const dev_handle_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
int mcu_3_uart_ioctl(const dev_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_3_uart_close(const dev_handle_t * cfg) MCU_PRIV_CODE;

int mcu_uart_getattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_uart_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_uart_setaction(int port, void * ctl) MCU_PRIV_CODE;

int mcu_uart_clear(int port, void * ctl) MCU_PRIV_CODE;
int mcu_uart_getbyte(int port, void * ctl) MCU_PRIV_CODE;
int mcu_uart_flush(int port, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif // _MCU_UART_H_

/*! @} */

