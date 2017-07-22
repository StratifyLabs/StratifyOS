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

#ifndef DEV_UARTFIFO_H_
#define DEV_UARTFIFO_H_

#include "sos/dev/uartfifo.h"

#include <stdbool.h>
#include "fifo.h"



/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct {
	fifo_state_t fifo;
} uartfifo_state_t;



/*! \details This is used for the configuration of the device.
 *
 */
typedef struct {
	fifo_config_t fifo;
} uartfifo_config_t;

#ifdef __cplusplus
extern "C" {
#endif

int uartfifo_open(const devfs_handle_t * cfg);
int uartfifo_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int uartfifo_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int uartfifo_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int uartfifo_close(const devfs_handle_t * cfg);

#ifdef __cplusplus
}
#endif


#endif /* DEV_UARTFIFO_H_ */

