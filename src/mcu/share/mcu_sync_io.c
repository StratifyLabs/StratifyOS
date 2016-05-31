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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#include <fcntl.h>
#include "mcu/mcu.h"
#include "mcu/debug.h"

void mcu_board_event(int event, void * args){
	if( mcu_board_config.event != 0 ){
		mcu_board_config.event(event, args);
	}
}

static int mcu_sync_io_complete(void * context, mcu_event_t data);

int mcu_sync_io_complete(void * context, mcu_event_t data){
	int * done = context;
	*done = 1;

	//return 0 to delete the callback
	return 0;
}

int mcu_sync_io(const device_cfg_t * cfg,
		int (*func)(const device_cfg_t * cfg, device_transfer_t * op),
		int loc,
		const void * buf,
		int nbyte,
		int flags){
	device_transfer_t op;
	volatile int done;
	int ret;

	if( nbyte == 0 ){
		return 0;
	}

	done = 0;
	op.cbuf = buf;
	op.loc = loc;
	op.flags = flags | O_RDWR;
	op.nbyte = nbyte;
	op.context = (void*)&done;
	op.callback = mcu_sync_io_complete;
	op.tid = 0;

	ret = func(cfg, &op);

	if( ret == 0 ){

		while( done == 0 ){
			//_mcu_core_sleep(CORE_SLEEP);
		}
		return op.nbyte;

	}
	return ret;

}
