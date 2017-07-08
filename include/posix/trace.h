

#ifndef TRACE_H_
#define TRACE_H_

#include <sys/types.h>
#include <pthread.h>
#include <limits.h>

#include "sos/link/link.h"
#include "mqueue.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef link_trace_attr_t trace_attr_t;
typedef link_trace_event_id_t trace_event_id_t;
typedef link_trace_event_set_t trace_event_set_t;
typedef link_trace_id_handle_t trace_id_handle_t;
typedef link_trace_id_t trace_id_t;

#define POSIX_TRACE_CLOSE_FOR_CHILD

/*! \brief Trace will overwrite oldest event */
#define POSIX_TRACE_LOOP 0
/*! \brief Trace will discard new events if the trace is full */
#define POSIX_TRACE_UNTIL_FULL 1
/*! \brief Trace will FLUSH events to the log when full */
#define POSIX_TRACE_FLUSH 2
/*! \brief Trace will append to the log */
#define POSIX_TRACE_APPEND 3


//System trace event types

#define POSIX_TRACE_INHERITED
#define POSIX_TRACE_NOT_TRUNCATED

#define POSIX_TRACE_TRUNCATED_READ
#define POSIX_TRACE_TRUNCATED_RECORD



#define POSIX_TRACE_RUNNING 1 //posix_stream_status
#define POSIX_TRACE_SUSPENDED 0 //posix_stream_status

#define POSIX_TRACE_OVERRUN 1 //posix_log_overrun_status
#define POSIX_TRACE_NO_OVERRUN 0  //posix_log_overrun_status

#define POSIX_TRACE_FULL 1 //posix_log_full_status
#define POSIX_TRACE_NOT_FULL 0 //posix_log_full_status

#define POSIX_TRACE_FLUSHING 1 //posix_stream_flush_status
#define POSIX_TRACE_NOT_FLUSHING 0  //posix_stream_flush_status


/*! \brief Include all process independent, implementation defined events */
#define POSIX_TRACE_WOPID_EVENTS 1
#define POSIX_TRACE_WOPID_EVENTS_MASK (0x0000FF00)

/*! \brief Include all process independent, implementation defined events */
#define POSIX_TRACE_SYSTEM_EVENTS 2
#define POSIX_TRACE_SYSTEM_EVENTS_MASK (0x000000FF)


typedef struct {
	pid_t pid /*! \brief Trace target PID */;
	pthread_t pthread_id /*! \brief Trace ID owner */;
	trace_attr_t attr /*! \brief Trace attributes */;
} trace_event_data_t;

/*! \brief Beginning of trace overflow */
#define POSIX_TRACE_OVERFLOW (LINK_POSIX_TRACE_OVERFLOW)

/*! \brief End of trace overflow */
#define POSIX_TRACE_RESUME (LINK_POSIX_TRACE_RESUME)

/*! \brief Flush start event */
#define POSIX_TRACE_FLUSH_START (LINK_POSIX_TRACE_FLUSH_START)

/*! \brief Flush stop event */
#define POSIX_TRACE_FLUSH_STOP (LINK_POSIX_TRACE_FLUSH_STOP)

/*! \brief Trace started */
#define POSIX_TRACE_START (LINK_POSIX_TRACE_START)

/*! \brief Trace stopped */
#define POSIX_TRACE_STOP (LINK_POSIX_TRACE_STOP)

/*! \brief Trace filter changed */
#define POSIX_TRACE_FILTER (LINK_POSIX_TRACE_FILTER)

/*! \brief Trace filter changed */
#define POSIX_TRACE_ERROR (LINK_POSIX_TRACE_ERROR)

/*! \brief Unname user event */
#define POSIX_TRACE_UNNAMED_USER_EVENT (LINK_POSIX_TRACE_UNNAMED_USER_EVENT)

/*! \brief Message event -- data is a string */
#define POSIX_TRACE_MESSAGE (LINK_POSIX_TRACE_MESSAGE)

/*! \brief Warning event -- data is a string */
#define POSIX_TRACE_WARNING (LINK_POSIX_TRACE_WARNING)

/*! \brief Critical/Error event -- data is a string */
#define POSIX_TRACE_CRITICAL (LINK_POSIX_TRACE_CRITICAL)

/*! \brief Fatal event -- data is a string (associate with a crash) */
#define POSIX_TRACE_FATAL (LINK_POSIX_TRACE_FATAL)


/*! \brief Include all events */
#define POSIX_TRACE_ALL_EVENTS 3
#define POSIX_TRACE_ALL_EVENTS_MASK (0xFFFFFFFF)

#define POSIX_TRACE_SET_EVENTSET 1
#define POSIX_TRACE_ADD_EVENTSET 2
#define POSIX_TRACE_SUB_EVENTSET 3

struct posix_trace_event_info {
	trace_event_id_t posix_event_id;
	pid_t posix_pid;
	void *posix_prog_address;
	int posix_truncation_status;
	struct timespec posix_timestamp;
	pthread_t posix_thread_id;
};

struct posix_trace_status_info {
	int posix_stream_status; //POSIX_TRACE_RUNNING or POSIX_TRACE_SUSPENDED
	int posix_stream_full_status; //POSIX_TRACE_FULL or POSIX_TRACE_NOT_FULL
	int posix_stream_overrun_status; //POSIX_TRACE_OVERRUN or POSIX_TRACE_NO_OVERRUN
	int posix_stream_flush_status; //POSIX_TRACE_FLUSHING or POSIX_TRACE_NOT_FLUSHING
	int posix_stream_flush_error; //set if an error occured on flush since last read
	int posix_log_overrun_status; //POSIX_TRACE_OVERRUN or POSIX_TRACE_NO_OVERRUN
	int posix_log_full_status; //POSIX_TRACE_FULL or POSIX_TRACE_NOT_FULL
};

enum {
	POSIX_STREAM_STATUS_MASK = (1<<0),
	POSIX_STREAM_FULL_STATUS_MASK = (1<<1),
	POSIX_STREAM_OVERRUN_STATUS_MASK = (1<<2),
	POSIX_STREAM_FLUSH_STATUS_MASK = (1<<3),
	POSIX_STREAM_FLUSH_ERROR_MASK = (1<<4),
	POSIX_STREAM_LOG_OVERRUN_STATUS_MASK = (1<<5),
	POSIX_STREAM_LOG_FULL_STATUS_MASK = (1<<6),
};

int  posix_trace_attr_destroy(trace_attr_t *);
int  posix_trace_attr_getclockres(const trace_attr_t *,
		struct timespec *);
int  posix_trace_attr_getcreatetime(const trace_attr_t *,
		struct timespec *);
int  posix_trace_attr_getgenversion(const trace_attr_t *, char *);
int  posix_trace_attr_getinherited(const trace_attr_t *,
		int *);

int  posix_trace_attr_getlogfullpolicy(const trace_attr_t *,
		int *);
int  posix_trace_attr_getlogsize(const trace_attr_t *,
		size_t *);
int  posix_trace_attr_getmaxdatasize(const trace_attr_t *,
		size_t *);
int  posix_trace_attr_getmaxsystemeventsize(const trace_attr_t *,
		size_t *);
int  posix_trace_attr_getmaxusereventsize(const trace_attr_t *,
		size_t, size_t *);
int  posix_trace_attr_getname(const trace_attr_t *, char *);
int  posix_trace_attr_getstreamfullpolicy(const trace_attr_t *,
		int *);
int  posix_trace_attr_getstreamsize(const trace_attr_t *,
		size_t *);
int  posix_trace_attr_init(trace_attr_t *);
int  posix_trace_attr_setinherited(trace_attr_t *, int);
int  posix_trace_attr_setlogfullpolicy(trace_attr_t *, int);
int  posix_trace_attr_setlogsize(trace_attr_t *, size_t);
int  posix_trace_attr_setmaxdatasize(trace_attr_t *, size_t);
int  posix_trace_attr_setname(trace_attr_t *, const char *);
int  posix_trace_attr_setstreamsize(trace_attr_t *, size_t);
int  posix_trace_attr_setstreamfullpolicy(trace_attr_t *, int);
int  posix_trace_clear(trace_id_t id);
int  posix_trace_close(trace_id_t id);
int  posix_trace_create(pid_t pid, const trace_attr_t * attr, trace_id_t * id);
int  posix_trace_create_withlog(pid_t pid, const trace_attr_t * attr, int fd, trace_id_t * id);
void posix_trace_event(trace_event_id_t event_id, const void * data_ptr, size_t data_len);
int  posix_trace_eventid_equal(trace_id_t id, trace_event_id_t event1, trace_event_id_t event2);
int  posix_trace_eventid_get_name(trace_id_t id, trace_event_id_t event, char * event_name);
int  posix_trace_eventid_open(const char * event_name, trace_event_id_t * event_id);
int  posix_trace_eventset_add(trace_event_id_t event_id, trace_event_set_t * event_set);
int  posix_trace_eventset_del(trace_event_id_t event_id, trace_event_set_t * event_set);
int  posix_trace_eventset_empty(trace_event_set_t * event_set);
int  posix_trace_eventset_fill(trace_event_set_t * event_set, int what);
int  posix_trace_eventset_ismember(trace_event_id_t event_id, const trace_event_set_t * event_set, int * what);
int  posix_trace_eventtypelist_getnext_id(trace_id_t id, trace_event_id_t * event_id, int * what);
int  posix_trace_eventtypelist_rewind(trace_id_t id);
int  posix_trace_flush(trace_id_t id);
int  posix_trace_get_attr(trace_id_t id, trace_attr_t * attr);
int  posix_trace_get_filter(trace_id_t id, trace_event_set_t * event_set);
int  posix_trace_get_status(trace_id_t id, struct posix_trace_status_info * info);
int  posix_trace_getnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable);
int  posix_trace_open(int fd, trace_id_t * id);
int  posix_trace_rewind(trace_id_t id);
int  posix_trace_set_filter(trace_id_t id, const trace_event_set_t * event_set, int filter);
int  posix_trace_shutdown(trace_id_t id);
int  posix_trace_start(trace_id_t id);
int  posix_trace_stop(trace_id_t id);
int  posix_trace_timedgetnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable,
		const struct timespec * abs_timeout);
int  posix_trace_trid_eventid_open(trace_id_t id,
		const char * event_name,
		trace_event_id_t * event_id);
int  posix_trace_trygetnext_event(trace_id_t id,
		struct posix_trace_event_info * event,
		void * data,
		size_t num_bytes,
		size_t * data_len,
		int * unavailable);


//just a quick unformated message
int posix_trace_trygetnext_data(trace_id_t id, void * data, size_t num_bytes);

void posix_trace_event_addr(trace_event_id_t event_id, const void * data_ptr, size_t data_len, uint32_t addr);
void posix_trace_event_addr_tid(trace_event_id_t event_id, const void * data_ptr, size_t data_len, uint32_t addr, int tid);

#ifdef __cplusplus
}
#endif


#endif /* TRACE_H_ */
