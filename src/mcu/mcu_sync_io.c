/* Copyright 2011-2018 Tyler Gilbert; 
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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#include <fcntl.h>
#include "mcu/mcu.h"
#include "mcu/debug.h"
#include "mcu/boot_debug.h"

static int mcu_sync_io_complete(void * context, const mcu_event_t * data);

int mcu_sync_io_complete(void * context, const mcu_event_t * data){
	MCU_UNUSED_ARGUMENT(data);
	int * done = context;
	*done = 1234567;
	//return 0 to delete the callback
	return 0;
}

int mcu_sync_io(const devfs_handle_t * handle,
					 int (*func)(const devfs_handle_t *, devfs_async_t *),
					 int loc,
					 const void * buf,
					 int nbyte,
					 int flags){
	devfs_async_t op;
	volatile int done;
	int ret;

	if( nbyte == 0 ){
		return 0;
	}

	done = 0;
	op.buf_const = buf;
	op.loc = loc;
	op.flags = flags | O_RDWR;
	op.nbyte = nbyte;
	op.handler.context = (void*)&done;
	op.handler.callback = mcu_sync_io_complete;
	op.tid = 0;
	ret = func(handle, &op);

	if( ret == 0 ){

		while( done != 1234567 ){
			;
		}
		return op.nbyte;

	}
	return ret;

}
