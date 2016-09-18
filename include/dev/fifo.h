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


#ifndef DEV_FIFO_H_
#define DEV_FIFO_H_

#include <stdbool.h>
#include "mcu/types.h"
#include "iface/dev/fifo.h"
#include "iface/device_config.h"

typedef struct MCU_PACK {
	u16 head;
	u16 tail;
	u16 rop_len;
	u16 wop_len;
	device_transfer_t * rop;
	device_transfer_t * wop;
	u32 o_flags;
} fifo_state_t;

int fifo_open(const device_cfg_t * cfg);
int fifo_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int fifo_read(const device_cfg_t * cfg, device_transfer_t * rop);
int fifo_write(const device_cfg_t * cfg, device_transfer_t * wop);
int fifo_close(const device_cfg_t * cfg);

int fifo_is_writeblock(fifo_state_t * state);
void fifo_set_writeblock(fifo_state_t * state, int value);

int fifo_is_overflow(fifo_state_t * state);
void fifo_set_overflow(fifo_state_t * state, int value);


#endif /* DEV_FIFO_H_ */

