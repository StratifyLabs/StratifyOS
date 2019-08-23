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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#include <errno.h>

#include "cortexm/cortexm.h"
#include "sos/sos.h"
#include "sos/link/transport_usb.h"
#include "cortexm/task.h"
#include "mcu/core.h"
#include "sos/link.h"
#include "cortexm/mpu.h"

static void sos_trace_event_addr(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len, u32 addr);
static void sos_trace_build_event(link_trace_event_t * event, link_trace_event_id_t event_id, const void * data_ptr, size_t data_len, u32 addr, int tid, const struct timespec * spec);
static void svcall_trace_event(void * args);

void sos_trace_root_trace_event(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len){
	register u32 lr asm("lr");
	link_trace_event_t event;
	if( sos_board_config.trace_event ){
		sos_trace_build_event(&event, event_id, data_ptr, data_len, lr, task_get_current(), 0);
		sos_board_config.trace_event(&event);
	}
}

void sos_trace_build_event(
		link_trace_event_t * event,
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len,
		u32 addr,
		int tid,
		const struct timespec * spec
		){
	event->header.size = sizeof(link_trace_event_t);
	event->header.id = LINK_NOTIFY_ID_POSIX_TRACE_EVENT;
	event->posix_trace_event.posix_event_id = event_id;
	event->posix_trace_event.posix_pid = task_get_pid( tid );
	event->posix_trace_event.posix_prog_address = addr; //grab the value of the caller
	event->posix_trace_event.posix_thread_id = tid;

	if( data_len > LINK_POSIX_TRACE_DATA_SIZE-1  ){
		data_len = LINK_POSIX_TRACE_DATA_SIZE-1;
		event->posix_trace_event.posix_truncation_status = 1;
	} else {
		event->posix_trace_event.posix_truncation_status = 0;
	}

	memset(event->posix_trace_event.data, 0, LINK_POSIX_TRACE_DATA_SIZE);
	memcpy(event->posix_trace_event.data, data_ptr, data_len);
	if (spec){
		event->posix_trace_event.posix_timestamp_tv_sec = spec->tv_sec;
		event->posix_trace_event.posix_timestamp_tv_nsec = spec->tv_nsec;
	} else {
		event->posix_trace_event.posix_timestamp_tv_sec = 0;
		event->posix_trace_event.posix_timestamp_tv_nsec = 0;
	}
	cortexm_assign_zero_sum32(&event, CORTEXM_ZERO_SUM32_COUNT(event));
}

void sos_trace_event(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len){
	register u32 lr asm("lr");
	sos_trace_event_addr(event_id, data_ptr, data_len, lr);
}

void sos_trace_event_addr(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len, u32 addr){
	sos_trace_event_addr_tid(event_id, data_ptr, data_len, addr, task_get_current());
}

void svcall_trace_event(void * args){
	CORTEXM_SVCALL_ENTER();
	sos_board_config.trace_event(args);
}

void sos_trace_event_addr_tid(
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len,
		u32 addr,
		int tid
		){
	//record event id and in-calling processes trace stream
	struct timespec spec;
	link_trace_event_t event;

	if( sos_board_config.trace_event ){
		//convert the address using the task memory location
		//check if addr is part of kernel or app
		if( ((addr >= (u32)&_text) && (addr < (u32)&_etext)) || (addr == 1) ){
			//kernel
			addr = addr - 1;
		} else {
			//app
			addr = addr - (u32)sos_task_table[tid].mem.code.address - 1 + 0xDE000000;
		}

		clock_gettime(CLOCK_REALTIME, &spec);

		sos_trace_build_event(&event, event_id, data_ptr, data_len, addr, tid, &spec);


		/*
		event.header.size = sizeof(link_trace_event_t);
		event.header.id = LINK_NOTIFY_ID_POSIX_TRACE_EVENT;
		event.posix_trace_event.posix_event_id = event_id;
		event.posix_trace_event.posix_pid = task_get_pid( tid );
		event.posix_trace_event.posix_prog_address = addr; //grab the value of the caller
		event.posix_trace_event.posix_thread_id = tid;

		if( data_len > LINK_POSIX_TRACE_DATA_SIZE-1  ){
			data_len = LINK_POSIX_TRACE_DATA_SIZE-1;
			event.posix_trace_event.posix_truncation_status = 1;
		} else {
			event.posix_trace_event.posix_truncation_status = 0;
		}

		memset(event.posix_trace_event.data, 0, LINK_POSIX_TRACE_DATA_SIZE);
		memcpy(event.posix_trace_event.data, data_ptr, data_len);
		event.posix_trace_event.posix_timestamp_tv_sec = spec.tv_sec;
		event.posix_trace_event.posix_timestamp_tv_nsec = spec.tv_nsec;

		cortexm_assign_zero_sum32(&event, CORTEXM_ZERO_SUM32_COUNT(event));
		*/

		cortexm_svcall(svcall_trace_event, &event);
	}

}



