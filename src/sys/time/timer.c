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

unsigned int process_alarm(unsigned int seconds, useconds_t useconds, useconds_t interval){
	timer_t timer_id = task_get_parent( task_get_current() );

	//if timer is already set, get the number of seconds until it expires and reset
	struct itimerspec timer;
	struct itimerspec o_timer;

	timer.it_value.tv_sec = seconds;
	timer.it_value.tv_nsec = useconds*1000UL;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = interval*1000UL;

	timer_settime(timer_id, 0, &timer, &o_timer);

	return o_timer.it_value.tv_sec;
}

/*!
 * \brief alarm
 * \param seconds
 * \return
 */
unsigned int alarm(unsigned int seconds){
	//timer id is from task zero and timer 0 -- it is pre-initialized
	return process_alarm(seconds, 0, 0) / 1000000UL;
}

/*!
 * \brief ualarm
 * \param seconds
 * \return
 */
unsigned int ualarm(useconds_t useconds, useconds_t interval){
	return process_alarm(0, useconds, interval);
}

/*!
 * \brief timer_create
 * \param clock_id
 * \param evp
 * \param timerid
 * \return
 */
int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid){
	//need to have a linked list way of managing these
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



/*!
 * \brief timer_delete
 * \param timerid
 * \return
 */
int timer_delete(timer_t timerid){
	if( scheduler_timing_process_delete_timer(timerid) < 0 ){
		errno = EINVAL;
		return -1;
	}
	return 0;
}

/*!
 * \brief timer_settime
 * \param timerid
 * \param flags
 * \param value
 * \param ovalue
 * \return
 */
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

/*!
 * \brief timer_gettime
 * \param timerid
 * \param value
 * \return
 */
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

	//value->it_interval is current interval
	scheduler_timing_convert_mcu_timeval(&value->it_value, &mcu_value);
	scheduler_timing_convert_mcu_timeval(&value->it_interval, &interval);

	return 0;
}

/*!
 * \brief timer_getoverrun
 * \param timerid
 * \return
 */
int timer_getoverrun(timer_t timerid){
	MCU_UNUSED_ARGUMENT(timerid);
	errno = ENOTSUP;
	return -1;
}
