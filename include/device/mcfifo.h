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


#ifndef DEV_MCFIFO_H_
#define DEV_MCFIFO_H_

#include "fifo.h"
#include "sos/dev/mcfifo.h"

typedef struct MCU_PACK {
	u32 * owner_array;
	fifo_state_t * fifo_state_array;
} mcfifo_state_t;

/*! \brief MCFIFO Configuration
 * \details This structure defines the static MCFIFO configuration.
 *
 */
typedef struct MCU_PACK {
	u16 count /*! The number of channels */;
	u16 size /*! The size of the buffer (only size-1 is usable) */;
	const fifo_config_t * fifo_config_array;
} mcfifo_config_t;

int mcfifo_open(const devfs_handle_t * handle);
int mcfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int mcfifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int mcfifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int mcfifo_close(const devfs_handle_t * handle);




#endif /* DEV_MCFIFO_H_ */

