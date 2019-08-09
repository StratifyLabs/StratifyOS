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
#include <string.h>

#include "mcu/mcu.h"
#include "cortexm/task.h"
#include "cortexm/mpu.h"
#include "mcu/core.h"
#include <fcntl.h>
#include "../scheduler/scheduler_local.h"
#include "trace.h"

typedef struct {
	trace_id_handle_t trace;
	void * next;
} trace_list_t;

static trace_list_t * trace_first = 0;


static void trace_cleanup(){
	trace_list_t * entry;
	int tid;
	for(entry = trace_first; entry != 0; entry = entry->next){
		tid = entry->trace.tid;
		if( (task_enabled(tid) == 0) ||
				(task_get_pid(tid) != entry->trace.pid) ){
			//delete the trace
			posix_trace_shutdown(&(entry->trace));
		}
	}
}


static trace_id_handle_t * trace_find_free(){
	trace_list_t * entry;
	trace_list_t * new_entry;
	trace_list_t * last_entry;
	last_entry = 0;
	for(entry = trace_first; entry != 0; entry = entry->next){
		last_entry = entry;
		if( entry->trace.tid == 0 ){
			return &entry->trace;
		}
	}

	//no free message queues
	new_entry = _malloc_r(sos_task_table[0].global_reent, sizeof(trace_list_t));
	if( new_entry == 0 ){
		return 0;
	}

	if( last_entry == 0 ){
		trace_first = new_entry;
	} else {
		last_entry->next = new_entry;
	}

	new_entry->next = 0;
	return &new_entry->trace;
}

static trace_id_handle_t * trace_get_ptr(trace_id_t id){
	trace_id_handle_t * ptr = (trace_id_handle_t*)id;
	trace_list_t * entry;
	for(entry = trace_first; entry != 0; entry = entry->next){
		if( ptr == &(entry->trace) ){
			return ptr;
		}
	}
	errno = EINVAL;
	return 0;
}


static uint32_t calc_checksum(trace_id_t id){
	uint32_t check;
	uint32_t * ptr = (uint32_t*)id;
	int i;
	check = 0xAA55AA55;
	//go over handle excluding checksum value
	for(i=0; i < sizeof(link_trace_id_handle_t) - 4; i+=4){
		check ^= *ptr++;
	}
	return check;
}

static int is_invalid(trace_id_t id){
	if( id == 0 ){
		errno = EINVAL;
		return 1;
	}
	if( calc_checksum(id) == id->checksum ){ return 0; }
	errno = EINVAL;
	return 1;
}

static void update_checksum(trace_id_t id){
	id->checksum = calc_checksum(id);
}

static int exec_trace_event(mqd_t mqdes, struct posix_trace_event_info * info, const void * data_ptr, size_t data_len);

typedef struct {
	trace_id_t id;
	int result;
} root_trace_id_t;
static void svcall_set_trace_id(void * args);
static void svcall_shutdown_trace_id(void * args);

static int trace_timedgetnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable,
		const struct timespec * abs_timeout);

int posix_trace_clear(trace_id_t id){
	//clear all events in the trace stream
	errno = ENOTSUP;
	return -1;
}

int posix_trace_close(trace_id_t id){
	//deallocate trace log
	errno = ENOTSUP;
	return -1;
}


void svcall_count_trace_id(void * args){
	CORTEXM_SVCALL_ENTER();
	pid_t * pid = (pid_t*)args;
	int i;
	int ret = 0;
	for(i=0; i < task_get_total(); i++){
		if( task_enabled(i) ){
			if( task_get_pid(i) == *pid ){
				ret++; //found at least on task with pid
			}
		}
	}
	*pid = ret;
}

void svcall_set_trace_id(void * args){
	CORTEXM_SVCALL_ENTER();
	root_trace_id_t * p = args;
	int i;
	p->result = 0;
	for(i=0; i < task_get_total(); i++){
		if( task_enabled(i) ){
			if( task_get_pid(i) == p->id->pid ){
				scheduler_root_set_trace_id(i, p->id);
				p->result++; //found the pid -- otherwise return ESRCH
			}
		}
	}
}

//This is setup by the system or another process that wants to trace the target pid
int posix_trace_create(pid_t pid, const trace_attr_t * attr, trace_id_t * id){
	trace_id_handle_t trace_handle;
	struct mq_attr mattr;
	size_t msg_size;
	root_trace_id_t args;
	trace_attr_t tmp_attr;
	int oflag;
	//create a new trace stream for pid -- create a new message queue and tell the target pid it is being traced



	//check for any existing traces on processes that no longer exist
	trace_cleanup();

	if( attr == 0 ){
		posix_trace_attr_init(&tmp_attr);
	} else {
		memcpy(&tmp_attr, attr, (sizeof(trace_attr_t)));
	}


	pid_t tmp_pid;
	tmp_pid = pid;
	//check to see if there are any tasks with the target ID -- if not, don't create the trace
	cortexm_svcall(svcall_count_trace_id, &tmp_pid);
	if( tmp_pid == 0 ){
		errno = ESRCH;
		return -1;
	}

	mattr.mq_maxmsg = tmp_attr.stream_size;
	posix_trace_attr_getmaxsystemeventsize(&tmp_attr, &msg_size);
	mattr.mq_msgsize = msg_size;

	oflag = O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK;
	if( tmp_attr.stream_policy == POSIX_TRACE_LOOP ){
		oflag |= MQ_FLAGS_LOOP;
	}

	trace_handle.mq = mq_open(tmp_attr.name, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0666, &mattr);
	if( trace_handle.mq < 0 ){
		return -1;
	}

	trace_handle.filter = POSIX_TRACE_ALL_EVENTS_MASK;
	trace_handle.pid = pid;
	trace_handle.status = 0; //trace is suspended on start
	trace_handle.tid = task_get_current();
	memcpy(&(trace_handle.attr), &tmp_attr, sizeof(trace_attr_t));

	//populate the timestamp
	clock_gettime(CLOCK_REALTIME, (struct timespec *)&(trace_handle.attr.create_time));

	update_checksum(&trace_handle);

	*id = trace_find_free();
	if( *id == 0 ){
		//discard the message queue if creation of the id memory fails
		mq_discard(trace_handle.mq);
		return -1;
	}

	//copy the data over to the id area
	memcpy(*id, &trace_handle, sizeof(trace_id_handle_t));

	args.id = *id;
	args.result = 0;
	cortexm_svcall(svcall_set_trace_id, &args);

	return 0;
}

int posix_trace_create_withlog(pid_t pid, const trace_attr_t * attr, int fd, trace_id_t * id){
	//create a new trace stream and associated log
	errno = ENOTSUP;
	return -1;
}

void posix_trace_event_addr_tid(trace_event_id_t event_id, const void * data_ptr, size_t data_len, uint32_t addr, int tid){
	//record event id and in-calling processes trace stream
	trace_id_handle_t trace_handle;
	int ret;
	struct posix_trace_event_info event_info;
	//check for an active trace stream

	trace_id_t trace_id = scheduler_trace_id( tid );


	if( trace_id == 0 ){
		return;
	}

	memcpy(&trace_handle, trace_id, sizeof(trace_id_handle_t));

	//check to see if trace is running
	if( (trace_handle.status & POSIX_STREAM_STATUS_MASK) == 0 ){
		return;
	}

	//check to see if event is filtered out
	if( (trace_handle.filter & (1<<event_id)) == 0 ){
		return;
	}

	//convert the address using the task memory location
	//check if addr is part of kernel or app
	if( (addr > (uint32_t)&_text) && (addr < (uint32_t)&_etext) ){
		//kernel
		addr = addr - 1;
	} else {
		//app
		addr = addr - (u32)sos_task_table[tid].mem.code.address - 1 + 0xDE000000;
	}

	event_info.posix_event_id = event_id;
	event_info.posix_pid = task_get_pid( tid );
	event_info.posix_prog_address = (void*)addr; //grab the value of the caller
	event_info.posix_thread_id = tid;
	clock_gettime(CLOCK_REALTIME, &event_info.posix_timestamp);

	//if data_len is too big for the data in the trace - set the trunc status to true
	if( trace_handle.attr.data_size < data_len ){
		event_info.posix_truncation_status = 1;
		data_len = trace_handle.attr.data_size;
	} else {
		event_info.posix_truncation_status = 0;
	}

	//now send a message on the associated message queue
	if( (ret = exec_trace_event(trace_handle.mq, &event_info, data_ptr, data_len)) != 0 ){
		//trace overrun condition
		if( ret == EAGAIN ){
			//overrun
			trace_handle.status |= POSIX_STREAM_OVERRUN_STATUS_MASK;
		} else {
			//some other error writing to the queue
		}
	}
	return;
}

void posix_trace_event_addr(trace_event_id_t event_id, const void * data_ptr, size_t data_len, uint32_t addr){
	posix_trace_event_addr_tid(event_id, data_ptr, data_len, addr, task_get_current());
}

//This is used for applications -- trace an event
void posix_trace_event(trace_event_id_t event_id, const void * data_ptr, size_t data_len){
	//MCU_CORE_DECLARE_CALLER_REGISTER(lr);
	//posix_trace_event_addr(event_id, data_ptr, data_len, lr);
}

int exec_trace_event(mqd_t mqdes, struct posix_trace_event_info * info, const void * data_ptr, size_t data_len){
	int tmp_errno;
	int ret;
	size_t len = sizeof(struct posix_trace_event_info) + data_len;
	//create a buffer on the stack large enough to hold the data and the header
	char buffer[len];
	memcpy(buffer, info, sizeof(struct posix_trace_event_info));
	memcpy(buffer + sizeof(struct posix_trace_event_info), data_ptr, data_len);

	//send the message on the queue
	tmp_errno = errno;
	errno = 0;
	mq_trysend(mqdes, buffer, len, 0);
	ret = errno;
	errno = tmp_errno;
	return ret;
}

int posix_trace_eventid_equal(trace_id_t id, trace_event_id_t event1, trace_event_id_t event2){
	if( event1 == event2 ){
		return 1;
	}
	return 0;
}

int posix_trace_eventid_get_name(trace_id_t id, trace_event_id_t event, char * event_name){
	//copy event id to event name
	id = trace_get_ptr(id);
	if( is_invalid(id) ){ return -1; }

	switch(event){
	case POSIX_TRACE_OVERFLOW:
		strcpy(event_name, "trace overflow");
		break;
	case POSIX_TRACE_RESUME:
		strcpy(event_name, "trace resume");
		break;
	case POSIX_TRACE_FLUSH_START:
		strcpy(event_name, "trace flush start");
		break;
	case POSIX_TRACE_FLUSH_STOP:
		strcpy(event_name, "trace flush stop");
		break;
	case POSIX_TRACE_START:
		strcpy(event_name, "trace start");
		break;
	case POSIX_TRACE_STOP:
		strcpy(event_name, "trace stop");
		break;
	case POSIX_TRACE_FILTER:
		strcpy(event_name, "trace filter");
		break;
	case POSIX_TRACE_ERROR:
		strcpy(event_name, "error");
		break;
	case POSIX_TRACE_UNNAMED_USER_EVENT:
		strcpy(event_name, "unnamed");
		break;
	case POSIX_TRACE_MESSAGE:
		strcpy(event_name, "message");
		break;
	case POSIX_TRACE_WARNING:
		strcpy(event_name, "warning");
		break;
	case POSIX_TRACE_CRITICAL:
		strcpy(event_name, "critical");
		break;
	case POSIX_TRACE_FATAL:
		strcpy(event_name, "fatal");
		break;
	default:
		strcpy(event_name, "unknown");
		break;
	}
	return 0;
}

int posix_trace_eventid_open(const char * event_name, trace_event_id_t * event_id){
	errno = ENOTSUP;
	return -1;
}

int posix_trace_eventset_add(trace_event_id_t event_id, trace_event_set_t * event_set){
	*event_set |= (1<<event_id);
	return 0;
}

int posix_trace_eventset_del(trace_event_id_t event_id, trace_event_set_t * event_set){
	*event_set &= ~(1<<event_id);
	return 0;
}

int posix_trace_eventset_empty(trace_event_set_t * event_set){
	*event_set = 0;
	return 0;
}

int posix_trace_eventset_fill(trace_event_set_t * event_set, int what){
	switch(what){
	case POSIX_TRACE_WOPID_EVENTS:
		*event_set |= POSIX_TRACE_WOPID_EVENTS_MASK;
		break;
	case POSIX_TRACE_SYSTEM_EVENTS:
		*event_set |= POSIX_TRACE_SYSTEM_EVENTS_MASK;
		break;
	case POSIX_TRACE_ALL_EVENTS:
		*event_set = POSIX_TRACE_ALL_EVENTS_MASK;
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}

int posix_trace_eventset_ismember(trace_event_id_t event_id,
		const trace_event_set_t * event_set,
		int * ismember){
	if( (*event_set & (1<<event_id)) != 0 ){
		*ismember = 1;
	} else {
		*ismember = 0;
	}
	return 0;
}

int posix_trace_flush(trace_id_t id){
	//copy trace events to the log if available -- otherwise return an error
	errno = EINVAL;
	return -1;
}

int posix_trace_get_attr(trace_id_t id, trace_attr_t * attr){
	memcpy(attr, &(id->attr), sizeof(trace_attr_t));
	return 0;
}

int posix_trace_get_filter(trace_id_t id, trace_event_set_t * event_set){
	*event_set = id->filter;
	return 0;
}

int posix_trace_get_status(trace_id_t id, struct posix_trace_status_info * info){
	struct mq_attr attr;

	mq_getattr(id->mq, &attr);
	if( attr.mq_msgsize == attr.mq_maxmsg ){
		id->status |= POSIX_STREAM_FULL_STATUS_MASK;
	}

	info->posix_stream_status = ((id->status & POSIX_STREAM_STATUS_MASK) == POSIX_STREAM_STATUS_MASK);
	info->posix_stream_full_status= ((id->status & POSIX_STREAM_FULL_STATUS_MASK) == POSIX_STREAM_FULL_STATUS_MASK);
	info->posix_stream_overrun_status = ((id->status & POSIX_STREAM_OVERRUN_STATUS_MASK) == POSIX_STREAM_OVERRUN_STATUS_MASK);
	info->posix_stream_flush_status = ((id->status & POSIX_STREAM_FLUSH_STATUS_MASK) == POSIX_STREAM_FLUSH_STATUS_MASK);
	info->posix_stream_flush_error = ((id->status & POSIX_STREAM_FLUSH_ERROR_MASK) == POSIX_STREAM_FLUSH_ERROR_MASK);
	info->posix_log_overrun_status = ((id->status & POSIX_STREAM_LOG_OVERRUN_STATUS_MASK) == POSIX_STREAM_LOG_OVERRUN_STATUS_MASK);
	info->posix_log_full_status = ((id->status & POSIX_STREAM_LOG_FULL_STATUS_MASK) == POSIX_STREAM_LOG_FULL_STATUS_MASK);

	id->status &= ~POSIX_STREAM_OVERRUN_STATUS_MASK; //clear the overrun
	return 0;
}

int posix_trace_open(int fd, trace_id_t * id){
	//associate id with an open trace log
	errno = ENOTSUP;
	return -1;
}

int posix_trace_rewind(trace_id_t id){
	//set timestamp to oldest in the log
	errno = ENOTSUP;
	return -1;
}

int posix_trace_set_filter(trace_id_t id, const trace_event_set_t * event_set, int how){
	id = trace_get_ptr(id);
	if( is_invalid(id) ){ return -1; }

	switch(how){
	case POSIX_TRACE_SET_EVENTSET:
		id->filter = *event_set;
		break;
	case POSIX_TRACE_ADD_EVENTSET:
		id->filter |= *event_set;
		break;
	case POSIX_TRACE_SUB_EVENTSET:
		id->filter &= ~(*event_set);
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	update_checksum(id);
	return 0;
}

void svcall_shutdown_trace_id(void * args){
	CORTEXM_SVCALL_ENTER();
	int i;
	root_trace_id_t * p = args;
	for(i=0; i < task_get_total(); i++){
		if( scheduler_trace_id(i) == p->id ){
			scheduler_root_set_trace_id(i, 0);
		}
	}
}

int posix_trace_shutdown(trace_id_t id){
	//free all resources associated with this stream id
	root_trace_id_t args;

	id = trace_get_ptr(id);
	if( is_invalid(id) ){ return -1; }

	args.id = id;
	cortexm_svcall(svcall_shutdown_trace_id, &args);
	mq_discard(id->mq);
	memset(id, 0, sizeof(trace_id_handle_t));
	return 0;
}

int posix_trace_start(trace_id_t id){
	id = trace_get_ptr(id);
	if( is_invalid(id) ){ return -1; }
	//set status to POSIX_TRACE_RUNNING
	id->status |= POSIX_STREAM_STATUS_MASK;
	update_checksum(id);
	return 0;
}

int posix_trace_stop(trace_id_t id){
	id = trace_get_ptr(id);
	if( is_invalid(id) ){ return -1; }
	//set status to POSIX_TRACE_RUNNING
	id->status &= ~POSIX_STREAM_STATUS_MASK;
	update_checksum(id);
	return 0;
}

int posix_trace_trid_eventid_open(trace_id_t id,
		const char * event_name,
		trace_event_id_t * event_id){
	//associate id with name for this stream
	errno = ENOTSUP;
	return -1;
}



int posix_trace_eventtypelist_getnext_id(trace_id_t id, trace_event_id_t * event_id, int * unavailable){
	//iterates through the items in the trace stream -- set unavail to non-zero when done
	return 0;
}

int posix_trace_eventtypelist_rewind(trace_id_t id){
	//rewind iterator
	return 0;
}


//THE FOLLOWING ARE FOR READING A TRACE
int posix_trace_getnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable){
	return trace_timedgetnext_event(id, event, data, num_bytes, data_len, unavailable, 0);
}

int posix_trace_timedgetnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable,
		const struct timespec * abs_timeout){
	//report a recorded trace event from an active stream without log -- with a timeout
	return trace_timedgetnext_event(id, event, data, num_bytes, data_len, unavailable, abs_timeout);
}

int posix_trace_trygetnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable){
	struct timespec abs_timeout;
	abs_timeout.tv_sec = 0;
	abs_timeout.tv_nsec = 0;
	return trace_timedgetnext_event(id, event, data, num_bytes, data_len, unavailable, &abs_timeout);
}

int posix_trace_trygetnext_data(trace_id_t id, void * data, size_t num_bytes){
	unsigned msg_prio = 0;
	return mq_receive(id->mq, data, num_bytes, &msg_prio);
}

int trace_timedgetnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable,
		const struct timespec * abs_timeout){
	size_t len = sizeof(struct posix_trace_event_info) + id->attr.data_size;
	size_t received_data_size;

	id = trace_get_ptr(id);
	if( is_invalid(id) ){ return -1; }

	//create a buffer on the stack large enough to hold the data and the header
	char buffer[len];
	int ret;
	unsigned msg_prio = 0;

	ret = mq_timedreceive(id->mq, buffer, len, &msg_prio, abs_timeout);
	if( ret < 0 ){
		return ret;
	}

	if( ret < sizeof(struct posix_trace_event_info) ){
		return -1;
	}

	received_data_size = ret - sizeof(struct posix_trace_event_info);
	if( received_data_size > num_bytes ){
		received_data_size = num_bytes;
	}

	*data_len = received_data_size;
	*unavailable = 0;

	//copy the message to the event/trace locations
	memcpy(event, buffer, sizeof(struct posix_trace_event_info));
	memcpy(data, buffer + sizeof(struct posix_trace_event_info), received_data_size);
	return 0;
}

