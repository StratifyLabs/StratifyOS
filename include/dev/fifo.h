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
#include "iface/dev/fifo.h"
#include "iface/device_config.h"

typedef struct {
	int head;
	int tail;
	int rop_len;
	int wop_len;
	int o_flags;
	device_transfer_t * rop;
	device_transfer_t * wop;
} fifo_state_t;

typedef struct {
	char * buffer;
	int size;
} fifo_cfg_t;

int fifo_open(const device_cfg_t * cfg);
int fifo_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int fifo_read(const device_cfg_t * cfg, device_transfer_t * rop);
int fifo_write(const device_cfg_t * cfg, device_transfer_t * wop);
int fifo_close(const device_cfg_t * cfg);



//helper functions for implementing FIFO's on other devices
void fifo_inc_head(fifo_state_t * state, int size);
int fifo_read_buffer(const fifo_cfg_t * cfgp, fifo_state_t * state, device_transfer_t * rop);
int fifo_write_buffer(const fifo_cfg_t * cfgp, fifo_state_t * state, device_transfer_t * wop);

int fifo_getattr(fifo_attr_t * attr, fifo_state_t * state, const fifo_cfg_t * cfg);
void fifo_data_transmitted(const fifo_cfg_t * cfgp, fifo_state_t * state);
void fifo_data_received(const fifo_cfg_t * cfgp, fifo_state_t * state);
void fifo_flush(fifo_state_t * state);

#endif /* DEV_FIFO_H_ */

