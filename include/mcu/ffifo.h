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


#ifndef DEV_FFIFO_H_
#define DEV_FFIFO_H_

#include "sos/dev/ffifo.h"
#include "sos/fs/devfs.h"


/*! \details This is the state for a framed FIFO (ffifo)
 *
 */
typedef struct MCU_PACK {
	u16 head;
	u16 tail;
	u16 rop_len;
	u16 wop_len;
	devfs_async_t * rop;
	devfs_async_t * wop;
	u32 o_flags;
} ffifo_state_t;

/*! \details This is the configuration for a framed FIFO (ffifo)
 *
 */
typedef struct MCU_PACK {
	u16 count /*! \brief The number of frames in the buffer */;
	u16 frame_size /*! The size of each frame */;
	char * buffer /*! \brief A pointer to the buffer */;
	void (*notify_on_read)(int nbyte) /*! Callback for when frame is read */;
	void (*notify_on_write)(int nbyte) /*! Callback for when frame is written */;
} ffifo_cfg_t;

int ffifo_open(const devfs_handle_t * cfg);
int ffifo_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int ffifo_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int ffifo_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int ffifo_close(const devfs_handle_t * cfg);


char * ffifo_get_frame(const ffifo_cfg_t * cfgp, u16 frame);

//helper functions for implementing FIFOs
void ffifo_flush(ffifo_state_t * state);
void ffifo_getinfo(ffifo_info_t * info, const ffifo_cfg_t * cfgp, ffifo_state_t * state);

void ffifo_inc_head(ffifo_state_t * state, u16 count);
void ffifo_inc_tail(ffifo_state_t * state, u16 count);
int ffifo_is_write_ok(ffifo_state_t * state, u16 count, int writeblock);

int ffifo_is_writeblock(ffifo_state_t * state);
void ffifo_set_writeblock(ffifo_state_t * state, int value);

int ffifo_is_notify_read(ffifo_state_t * state);
void ffifo_set_notify_read(ffifo_state_t * state, int value);

int ffifo_is_notify_write(ffifo_state_t * state);
void ffifo_set_notify_write(ffifo_state_t * state, int value);

int ffifo_is_overflow(ffifo_state_t * state);
void ffifo_set_overflow(ffifo_state_t * state, int value);

int ffifo_read_buffer(const ffifo_cfg_t * cfgp, ffifo_state_t * state, char * buf, int len);
int ffifo_write_buffer(const ffifo_cfg_t * cfgp, ffifo_state_t * state, const char * buf, int len);

void ffifo_data_transmitted(const ffifo_cfg_t * cfgp, ffifo_state_t * state);
void ffifo_data_received(const ffifo_cfg_t * cfgp, ffifo_state_t * state);
int ffifo_write_local(const ffifo_cfg_t * cfgp, ffifo_state_t * state, devfs_async_t * wop);
int ffifo_read_local(const ffifo_cfg_t * cfgp, ffifo_state_t * state, devfs_async_t * rop);

void ffifo_cancel_rop(ffifo_state_t * state);

#define FFIFO_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(ffifo), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}

#define FFIFO_DEVICE_CFG(buf, fcount, fsize, read_notify, write_notify) { .buffer = buf, .count = fcount, .frame_size = fsize, .notify_on_read = read_notify, .notify_on_write = write_notify }




#endif /* DEV_FFIFO_H_ */

