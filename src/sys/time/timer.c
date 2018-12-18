#include <time.h>
#include <signal.h>
#include <limits.h>
#include <malloc.h>
#include <errno.h>
#include <sys/timespec.h>

#include "sos/sos.h"

#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "../scheduler/scheduler_timing.h"



int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid){
	//need to have a linked list way of managing these
	struct sigevent event;

	if( clock_id != CLOCK_REALTIME ){
		errno = EINVAL;
		return -1;
	}


	timer_t t = scheduler_timing_process_create_timer(evp);
	if( t == (timer_t)(-1) ){
		errno = EAGAIN;
		return -1;
	}

	*timerid = t;
	return 0;
}




int timer_delete(timer_t timerid){
	if( scheduler_timing_process_delete_timer(timerid) < 0 ){
		errno = EINVAL;
		return -1;
	}
	return 0;
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue){
	struct mcu_timeval mcu_value;
	struct mcu_timeval interval;
	struct mcu_timeval o_value;
	struct mcu_timeval o_interval;
	int result;

	scheduler_timing_convert_timespec(&mcu_value, &value->it_value);
	scheduler_timing_convert_timespec(&interval, &value->it_interval);

	result = scheduler_timing_process_set_timer(timerid, flags,
															  &mcu_value, &interval,
															  &o_value, &o_interval);

	if( (result == 0) && ovalue ){
		scheduler_timing_convert_mcu_timeval(&ovalue->it_value, &o_value);
		scheduler_timing_convert_mcu_timeval(&ovalue->it_interval, &o_interval);
	}

	return result;
}


int timer_gettime(timer_t timerid, struct itimerspec *value){
	struct mcu_timeval mcu_value;
	struct mcu_timeval interval;
	struct mcu_timeval now;
	//value->it_value is amount of time until timer expires

	int result = scheduler_timing_process_get_timer(timerid, &mcu_value, &interval, &now);
	if( result < 0 ){
		errno = EINVAL;
		return result;
	}

	//if timer value is in the past, difference will be 0 (disarmed)
	struct mcu_timeval difference;
	difference = scheduler_timing_subtract_mcu_timeval(&mcu_value, &now);

	//value->it_interval is current interval
	scheduler_timing_convert_mcu_timeval(&value->it_value, &difference);
	scheduler_timing_convert_mcu_timeval(&value->it_interval, &interval);


	return 0;
}

int timer_getoverrun(timer_t timerid){
	MCU_UNUSED_ARGUMENT(timerid);
	errno = ENOTSUP;
	return -1;
}
