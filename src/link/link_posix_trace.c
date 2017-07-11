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


#include "link_flags.h"

int link_posix_trace_create(link_transport_mdriver_t * driver, uint32_t pid, link_trace_id_t * id){
	link_op_t op;
	link_reply_t reply;
	int err;

	//link_debug(LINK_DEBUG_MESSAGE, "posix trace create %s", attr->name);

	op.posix_trace_create.cmd = LINK_CMD_POSIX_TRACE_CREATE;
	op.posix_trace_create.pid = pid;

	link_debug(LINK_DEBUG_MESSAGE, "Write open op");
	err = link_transport_masterwrite(driver, &op, sizeof(link_posix_trace_create_t));
	if ( err < 0 ){
		link_error("failed to write open op with handle %d", (int)driver->dev.handle);
		return link_handle_err(driver, err);
	}

	//read the reply to see if the trace opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read the reply");
		return link_handle_err(driver, err);
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Operation failed (%d)", link_errno);
	} else {
		*id = (link_trace_id_t)reply.err_number; //assign ID the value of the new ID
	}
	return reply.err;
}

int link_posix_trace_tryget_events(link_transport_mdriver_t * driver,
		link_trace_id_t id,
		void * data,
		size_t num_bytes){

	link_op_t op;
	link_reply_t reply;
	int err;
	int len;

	op.posix_trace_tryget_events.cmd = LINK_CMD_POSIX_TRACE_TRY_GET_EVENTS;
	op.posix_trace_tryget_events.num_bytes = num_bytes;
	op.posix_trace_tryget_events.trace_id = id;

	link_debug(LINK_DEBUG_MESSAGE, "Write op");
	err = link_transport_masterwrite(driver, &op, sizeof(link_posix_trace_create_t));
	if ( err < 0 ){
		link_error("failed to write open op with handle %d", (int)driver->dev.handle);
		return link_handle_err(driver, err);
	}

	//read the reply
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read the reply");
		return link_handle_err(driver, err);
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Operation failed (%d)", link_errno);
		return -1;
	}

	if( reply.err == 0 ){
		return 0; //no bytes read
	}

	//now reply.err has the number of bytes that are about to be sent
	len = link_transport_masterread(driver, data, reply.err);
	if( len < 0 ){
		link_error("Failed to read trace events %d", reply.err);
	}

	//the number of bytes read
	return len;
}

int link_posix_trace_shutdown(link_transport_mdriver_t * driver,
		link_trace_id_t id){
	link_op_t op;
	link_reply_t reply;
	int err;


	op.posix_trace_shutdown.cmd = LINK_CMD_POSIX_TRACE_SHUTDOWN;
	op.posix_trace_shutdown.trace_id = id;

	link_debug(LINK_DEBUG_MESSAGE, "Write shutdown op");
	err = link_transport_masterwrite(driver, &op, sizeof(link_posix_trace_shutdown_t));
	if ( err < 0 ){
		link_error("failed to write op with handle %d", (int)driver->dev.handle);
		return link_handle_err(driver, err);
	}

	//read the reply to see if the trace shutdown correctly
	link_debug(LINK_DEBUG_MESSAGE, "Wait for shutdown reply");
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read the reply");
		return link_handle_err(driver, err);
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Operation failed (%d)", link_errno);
	}

	return reply.err;
}

const char * link_posix_trace_getname(int trace_number){
	switch(trace_number){

	case LINK_POSIX_TRACE_OVERFLOW:
		return "trace overflow";
	case LINK_POSIX_TRACE_RESUME:
		return "trace resume";
	case LINK_POSIX_TRACE_FLUSH_START:
		return "trace flush start";
	case LINK_POSIX_TRACE_FLUSH_STOP:
		return "trace flush stop";
	case LINK_POSIX_TRACE_START:
		return "trace start";
	case LINK_POSIX_TRACE_STOP:
		return "trace stop";
	case LINK_POSIX_TRACE_FILTER:
		return "trace filter";
	case LINK_POSIX_TRACE_ERROR:
		return "trace error";
	case LINK_POSIX_TRACE_UNNAMED_USER_EVENT:
		return "unnamed user event";
	case LINK_POSIX_TRACE_MESSAGE:
		return "message";
	case LINK_POSIX_TRACE_WARNING:
		return "warning";
	case LINK_POSIX_TRACE_CRITICAL:
		return "critical";
	case LINK_POSIX_TRACE_FATAL:
		return "fatal";
	default:
		return "unknown";
	}
}


