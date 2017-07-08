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

#include "sos/dev/fifo.h"
#include "sos/fs/devfs.h"

typedef struct MCU_PACK {
	u16 head;
	u16 tail;
	u16 rop_len;
	u16 wop_len;
	devfs_async_t * rop;
	devfs_async_t * wop;
	u32 o_flags;
} fifo_state_t;

/*! \brief FIFO Configuration
 * \details This structure defines the static FIFO configuration.
 *
 */
typedef struct MCU_PACK {
	u32 size /*! \brief The size of the buffer (only size-1 is usable) */;
	char * buffer /*! \brief A pointer to the buffer */;
	void (*notify_on_read)(int nbyte) /*! Callback for when fifo is read */;
	void (*notify_on_write)(int nbyte) /*! Callback for when fifo is written */;
} fifo_cfg_t;

int fifo_open(const devfs_handle_t * cfg);
int fifo_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int fifo_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int fifo_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int fifo_close(const devfs_handle_t * cfg);


//helper functions for implementing FIFOs
void fifo_flush(fifo_state_t * state);
void fifo_getinfo(fifo_info_t * info, const fifo_cfg_t * cfgp, fifo_state_t * state);

void fifo_inc_head(fifo_state_t * state, int size);
void fifo_inc_tail(fifo_state_t * state, int size);
int fifo_is_write_ok(fifo_state_t * state, u16 size, int writeblock);

int fifo_is_writeblock(fifo_state_t * state);
void fifo_set_writeblock(fifo_state_t * state, int value);

int fifo_is_notify_read(fifo_state_t * state);
void fifo_set_notify_read(fifo_state_t * state, int value);

int fifo_is_notify_write(fifo_state_t * state);
void fifo_set_notify_write(fifo_state_t * state, int value);

int fifo_is_overflow(fifo_state_t * state);
void fifo_set_overflow(fifo_state_t * state, int value);

int fifo_read_buffer(const fifo_cfg_t * cfgp, fifo_state_t * state, char * buf);
int fifo_write_buffer(const fifo_cfg_t * cfgp, fifo_state_t * state, const char * buf);

void fifo_data_received(const fifo_cfg_t * cfgp, fifo_state_t * state);
int fifo_write_local(const fifo_cfg_t * cfgp, fifo_state_t * state, devfs_async_t * wop);
int fifo_read_local(const fifo_cfg_t * cfgp, fifo_state_t * state, devfs_async_t * rop);

void fifo_cancel_rop(fifo_state_t * state);

/*! \brief See below.
 * \details This defines the configuration values for a FIFO device.
 *
 * \param device_name The name of the device (e.g "fifo0")
 * \param cfg_ptr A pointer to the const \ref fifo_cfg_t data structure
 * \param state_ptr A pointer to the state structure (see \ref fifo_state_t)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 * Example:
 * \code
 * #include <sos/dev/fifo.h>
 * const fifo_cfg_t fifo_cfg0 = { .buffer = fifo_buf0, .size = FIFO_BUF_SIZE };
 * fifo_state_t fifo_state0;
 * const device_t devices[DEVICES_TOTAL+1] = {
 * 	...
 * 	FIFO_DEVICE("fifo0", &fifo_cfg0, &fifo_state0, 0666, USER_ROOT, GROUP_ROOT),
 * 	...
 * 	DEVICE_TERMINATOR
 * }
 * \endcode
 *
 */

#define FIFO_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(fifo), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}

#define FIFO_DEVICE_CFG(buf, buf_size, read_notify, write_notify) { .buffer = buf, .size = buf_size, .notify_on_read = read_notify, .notify_on_write = write_notify }



#endif /* DEV_FIFO_H_ */

