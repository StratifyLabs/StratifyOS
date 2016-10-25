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


#include <errno.h>

#include "stratify/stratify.h"
#include "iface/stratify_link_transport_usb.h"
#include "mcu/task.h"
#include "mcu/core.h"
#include "iface/link.h"
#include "mcu/mpu.h"

static void stratify_trace_event_addr(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len, u32 addr);

void stratify_priv_trace_event(void * info){
	link_notify_posix_trace_event_t notify;
	memcpy(&(notify.info), info, sizeof(link_posix_trace_event_info_t));
	notify.id = LINK_NOTIFY_ID_POSIX_TRACE_EVENT;
	stratify_board_config.notify_write(&notify, sizeof(link_notify_posix_trace_event_t));
}

void stratify_trace_event(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len){
	register u32 lr asm("lr");
	stratify_trace_event_addr(event_id, data_ptr, data_len, lr);
}

void stratify_trace_event_addr(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len, u32 addr){
	if( stratify_board_config.notify_write != 0 ){
		stratify_trace_event_addr_tid(event_id, data_ptr, data_len, addr, task_get_current());
	}
}

void stratify_trace_event_addr_tid(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len, u32 addr, int tid){
	//record event id and in-calling processes trace stream
	struct timespec spec;
	link_posix_trace_event_info_t event_info;

	//convert the address using the task memory location
	//check if addr is part of kernel or app
	if( (addr > (u32)&_text) && (addr < (u32)&_etext) ){
		//kernel
		addr = addr - 1;
	} else {
		//app
		addr = addr - (u32)mpu_addr((u32)stratify_task_table[tid].mem.code.addr) - 1 + 0xDE000000;
	}

	event_info.posix_event_id = event_id;
	event_info.posix_pid = task_get_pid( tid );
	event_info.posix_prog_address = addr; //grab the value of the caller
	event_info.posix_thread_id = tid;

	if( data_len > LINK_POSIX_TRACE_DATA_SIZE-1  ){
		data_len = LINK_POSIX_TRACE_DATA_SIZE;
	}
	memset(event_info.data, 0, LINK_POSIX_TRACE_DATA_SIZE);
	memcpy(event_info.data, data_ptr, data_len);
	clock_gettime(CLOCK_REALTIME, &spec);
	event_info.posix_timestamp_tv_sec = spec.tv_sec;
	event_info.posix_timestamp_tv_nsec = spec.tv_nsec;

	mcu_core_privcall(stratify_priv_trace_event, &event_info);

}



