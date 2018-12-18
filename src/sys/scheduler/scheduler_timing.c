/* Copyright 2011-2017 Tyler Gilbert;
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


#include "config.h"

#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "scheduler_local.h"
#include "../signal/sig_local.h"

#include "mcu/tmr.h"
#include "mcu/rtc.h"
#include "mcu/debug.h"

static volatile u32 sched_usecond_counter MCU_SYS_MEM;

static int open_usecond_tmr();
static int root_handle_usecond_overflow_event(void * context, const mcu_event_t * data);
static int root_handle_usecond_match_event(void * context, const mcu_event_t * data);
static int root_handle_usecond_process_timer_match_event(void * context, const mcu_event_t * data);
static int send_and_reload_timer(volatile sos_process_timer_t * timer, u8 task_id, u32 now);
static u8 scheduler_timing_process_timer_task_id(timer_t timer_id){ return timer_id >> 8; }
static u8 scheduler_timing_process_timer_id_offset(timer_t timer_id){ return timer_id & 0xFF; }
static u8 scheduler_timing_process_timer_count(){ return SOS_PROCESS_TIMER_COUNT; }
static void update_tmr_for_process_timer_match(volatile sos_process_timer_t * timer);

int scheduler_timing_init(){
	if ( open_usecond_tmr() < 0 ){
		return -1;
	}
	return 0;
}

volatile sos_process_timer_t * scheduler_timing_process_timer(timer_t timer_id){
	u8 task_id = scheduler_timing_process_timer_task_id(timer_id);
	u8 id_offset = scheduler_timing_process_timer_id_offset(timer_id);

	if( (task_id < task_get_total()) && (id_offset < scheduler_timing_process_timer_count())){
		return sos_sched_table[task_id].timer + id_offset;
	}
	return 0;
}

typedef struct {
	timer_t timer_id;
	const struct sigevent * event;
	int result;
} root_allocate_timer_t;

static void root_allocate_timer(void * args){
	root_allocate_timer_t * p = args;
	volatile sos_process_timer_t * timer = scheduler_timing_process_timer(p->timer_id);
	if( timer->o_flags == 0 ){
		if( p->event ){
			memcpy((void*)&timer->sigevent, p->event, sizeof(struct sigevent));
		} else {
			memset((void*)&timer->sigevent, 0, sizeof(struct sigevent));
			timer->sigevent.sigev_notify = SIGEV_SIGNAL;
			timer->sigevent.sigev_signo = SIGALRM;
			timer->sigevent.sigev_value.sival_int = p->timer_id;
		}
		timer->value.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
		timer->value.tv_usec = 0;
		timer->interval.tv_sec = 0;
		timer->interval.tv_usec = 0;
		timer->o_flags = SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_INITIALIZED;

		cortexm_assign_zero_sum32((void*)timer, sizeof(sos_process_timer_t)/sizeof(u32));
		p->result = 0;
	} else {
		p->result = -1;
	}
}

void scheduler_timing_root_process_timer_initialize(u16 task_id){
	memset((void*)sos_sched_table[task_id].timer, 0, sizeof(sos_process_timer_t)*SOS_PROCESS_TIMER_COUNT);

	//the first available timer slot is reserved for alarm/ualarm
	if( task_get_parent(task_id) == task_id ){
		root_allocate_timer_t args;
		args.timer_id = SCHEDULER_TIMING_PROCESS_TIMER(task_id, 0);
		args.event = 0;
		root_allocate_timer(&args);
	}
}

timer_t scheduler_timing_process_create_timer(const struct sigevent * evp){
	s32 count = scheduler_timing_process_timer_count();
	int pid = task_get_pid(task_get_current());
	root_allocate_timer_t args;
	args.event = evp;
	args.result = -1;
	for(u8 task_id = 0; task_id < task_get_total(); task_id++){
		if( task_get_pid(task_id) == pid ){
			for(u8 id_offset = 0; id_offset < count; id_offset++){
				args.timer_id = SCHEDULER_TIMING_PROCESS_TIMER(task_id, id_offset);
				cortexm_svcall(root_allocate_timer, &args);
				if( args.result == 0 ){
					return args.timer_id;
				}
			}
		}
	}
	return (timer_t)(-1);
}

typedef struct {
	timer_t timer_id;
	int result;
} root_delete_timer_t;

static void root_delete_timer(void * args) MCU_ROOT_EXEC_CODE;
void root_delete_timer(void * args){
	root_delete_timer_t * p = args;
	volatile sos_process_timer_t * timer = scheduler_timing_process_timer(p->timer_id);
	if( timer == 0 ){
		p->result = -1;
		return;
	}

	memset((void*)timer, 0, sizeof(sos_process_timer_t));
	cortexm_assign_zero_sum32((void*)timer, sizeof(sos_process_timer_t)/sizeof(u32));
	p->result = 0;
}

int scheduler_timing_process_delete_timer(timer_t timer_id){
	root_delete_timer_t args;
	args.timer_id = timer_id;
	args.result = -202020;
	cortexm_svcall(root_delete_timer, &args);
	return args.result;
}

typedef struct {
	timer_t timer_id;
	int flags;
	const struct mcu_timeval * value;
	const struct mcu_timeval * interval;
	struct mcu_timeval * o_value;
	struct mcu_timeval * o_interval;
	int result;
} root_settime_t;

static void root_settime(void * args) MCU_ROOT_EXEC_CODE;
void root_settime(void * args){
	root_settime_t * p = args;
	struct mcu_timeval abs_time;



	volatile sos_process_timer_t * timer = scheduler_timing_process_timer(p->timer_id);
	if( timer == 0 ){
		errno = EINVAL;
		p->result = -1;
		return;
	}

	if( timer->value.tv_sec == SCHEDULER_TIMEVAL_SEC_INVALID ){
		p->o_value->tv_sec = 0;
		p->o_value->tv_usec = 0;
	} else {
		//ovalue is the amount of time before the timer would have expired timer->timeout - current_time
		*p->o_value = scheduler_timing_subtract_mcu_timeval((struct mcu_timeval*)&timer->value, &abs_time);
	}

	scheduler_timing_root_get_realtime(&abs_time);
	if( (p->flags & TIMER_ABSTIME) == 0 ){
		//value is a relative time -- convert to absolute time
		timer->value = scheduler_timing_add_mcu_timeval(&abs_time, p->value);
	} else {
		timer->value = *p->value;
	}

	//update the interval output value
	*p->o_interval = timer->interval;
	timer->interval = *p->interval;
	if( (timer->interval.tv_sec == 0) &&
		 (timer->interval.tv_usec > 0) &&
		 (timer->interval.tv_usec < SCHED_USECOND_TMR_MINIMUM_PROCESS_TIMER_INTERVAL) ){
		timer->interval.tv_usec = SCHED_USECOND_TMR_MINIMUM_PROCESS_TIMER_INTERVAL;
	}

	//stop the timer -- see if event is in past, assign the values, start the timer
	update_tmr_for_process_timer_match(timer);

	cortexm_assign_zero_sum32((void*)timer, sizeof(sos_process_timer_t)/sizeof(u32));
	p->result = 0;
}

int scheduler_timing_process_set_timer(timer_t timerid, int flags,
													const struct mcu_timeval * value,
													const struct mcu_timeval * interval,
													struct mcu_timeval * o_value,
													struct mcu_timeval * o_interval){
	root_settime_t args;
	args.timer_id = timerid;
	args.flags = flags;
	args.value = value;
	args.interval = interval;
	args.o_value = o_value;
	args.o_interval = o_interval;
	args.result = -202020;
	cortexm_svcall(root_settime, &args);
	return args.result;
}

typedef struct {
	timer_t timer_id;
	struct mcu_timeval * value;
	struct mcu_timeval * interval;
	struct mcu_timeval * now;
	int result;
} root_gettime_t;

static void root_gettime(void * args) MCU_ROOT_EXEC_CODE;
void root_gettime(void * args){
	root_gettime_t * p = args;

	volatile sos_process_timer_t * timer = scheduler_timing_process_timer(p->timer_id);
	if( timer == 0 ){
		p->result = -1;
		return;
	}

	scheduler_timing_root_get_realtime(p->now);
	if( timer->value.tv_sec == SCHEDULER_TIMEVAL_SEC_INVALID ){
		p->value->tv_sec = 0;
		p->value->tv_usec = 0;
	} else {
		//value is absolute time but gettime want's relative time
		*(p->value) = scheduler_timing_subtract_mcu_timeval((struct mcu_timeval*)&timer->value, p->now);
	}
	*p->interval = timer->interval;
	p->result = 0;
}


int scheduler_timing_process_get_timer(timer_t timerid, struct mcu_timeval * value, struct mcu_timeval * interval, struct mcu_timeval * now){
	root_gettime_t args;
	args.timer_id = timerid;
	args.result = -202020;
	args.value = value;
	args.interval = interval;
	args.now = now;

	cortexm_svcall(root_gettime, &args);
	return args.result;
}


u32 scheduler_timing_seconds_to_clocks(int seconds){
	return (u32)mcu_core_getclock() * (u32)seconds;
}

u32 scheduler_timing_useconds_to_clocks(int useconds){
	return (u32)(SCHEDULER_CLOCK_USEC_MULT * useconds);
}

void update_tmr_for_process_timer_match(volatile sos_process_timer_t * timer){
	mcu_channel_t chan_req;
	u32 now;
	devfs_handle_t tmr_handle;
	int is_time_to_send = 1;
	tmr_handle.port = sos_board_config.clk_usecond_tmr;

	//Initialization

	if (timer->value.tv_sec >= sched_usecond_counter){

		if(timer->value.tv_sec == sched_usecond_counter){

			mcu_tmr_disable(&tmr_handle, 0);
			//See if abs_time is in the past
			mcu_tmr_get(&tmr_handle, &now);

			//Read the current OC value to see if it needs to be updated
			chan_req.loc = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;

			mcu_tmr_getchannel(&tmr_handle, &chan_req);
			if ( timer->value.tv_usec < chan_req.value ){ //this means the signal needs to happen sooner than currently set
				chan_req.value = timer->value.tv_usec;
			}

			if( (timer->value.tv_usec > now) && //needs to be enough in the future to allow the OC to be set before the timer passes it
				 (chan_req.value == timer->value.tv_usec) ){
				mcu_tmr_setchannel(&tmr_handle, &chan_req);
				is_time_to_send = 0;
			}
		}

		mcu_tmr_enable(&tmr_handle, 0);

		if( is_time_to_send ){
			//send it now and reload if needed
			send_and_reload_timer(timer, task_get_current(), now);

			//if interval is non-zero -- this needs to be called again
			if( timer->interval.tv_sec + timer->interval.tv_usec ){
				update_tmr_for_process_timer_match(timer);
			}

		}

	} else {
		//signal happens in the future
		is_time_to_send = 0;
	}


}

typedef struct {
	int si_signo;
	int sig_value;
} root_unqueue_timer_t;

static void root_unqueue_timer(void * args){
	root_unqueue_timer_t * p = args;
	timer_t timer_id;
	for(u8 j=0; j < SOS_PROCESS_TIMER_COUNT; j++){
		timer_id = SCHEDULER_TIMING_PROCESS_TIMER(task_get_current(), j);
		volatile sos_process_timer_t * timer = scheduler_timing_process_timer(timer_id);
		if( (timer->sigevent.sigev_signo == p->si_signo) &&
			 (timer->sigevent.sigev_value.sival_int == p->sig_value)){
			//unqueue this timer
			timer->o_flags &= ~SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED;
		}
	}

}

void scheduler_timing_process_unqueue_timer(int tid,
														  int si_signo,
														  union sigval sig_value){
	root_unqueue_timer_t args;
	args.si_signo = si_signo;
	args.sig_value = sig_value.sival_int;
	cortexm_svcall(root_unqueue_timer, &args);
}

int send_and_reload_timer(volatile sos_process_timer_t * timer, u8 task_id, u32 now){

	//check to see if a signal has already been queued
	if( ((timer->o_flags & SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED) == 0) && (timer->sigevent.sigev_notify == SIGEV_SIGNAL) ){
		int result = signal_root_send(0,
												task_id,
												timer->sigevent.sigev_signo,
												SI_TIMER,
												timer->sigevent.sigev_value.sival_int,
												task_get_current() == task_id);
		if( result < 0 ){
			mcu_debug_printf("failed to fire %d\n", SYSFS_GET_RETURN(result));
		} else {
			timer->o_flags |= SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED;
		}

	} else {
		mcu_debug_printf("pending 0x%X %d %d 0x%X\n",
							  timer->o_flags,
							  timer->sigevent.sigev_notify,
							  timer->sigevent.sigev_signo,
							  timer->sigevent.sigev_value.sival_int);
	}

	//reload the timer if interval is valid
	if( timer->interval.tv_sec + timer->interval.tv_usec ){
		struct mcu_timeval current;
		current.tv_sec = sched_usecond_counter;
		current.tv_usec = now;
		timer->value = scheduler_timing_add_mcu_timeval(&current, (struct mcu_timeval*)&timer->interval);
	} else {
		timer->value.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
	}
	return 0;
}

void scheduler_timing_root_timedblock(void * block_object, struct mcu_timeval * abs_time){
	int id;
	mcu_channel_t chan_req;
	u32 now;
	devfs_handle_t tmr_handle;
	int is_time_to_sleep;
	tmr_handle.port = sos_board_config.clk_usecond_tmr;

	//Initialization
	id = task_get_current();
	sos_sched_table[id].block_object = block_object;
	is_time_to_sleep = 0;

	if (abs_time->tv_sec >= sched_usecond_counter){

		sos_sched_table[id].wake.tv_sec = abs_time->tv_sec;
		sos_sched_table[id].wake.tv_usec = abs_time->tv_usec;

		if(abs_time->tv_sec == sched_usecond_counter){

			mcu_tmr_disable(&tmr_handle, 0);
			//See if abs_time is in the past
			mcu_tmr_get(&tmr_handle, &now);

			//Read the current OC value to see if it needs to be updated
			chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;

			mcu_tmr_getchannel(&tmr_handle, &chan_req);
			if ( abs_time->tv_usec < chan_req.value ){ //this means the interrupt needs to happen sooner than currently set
				chan_req.value = abs_time->tv_usec;
			}

			//Is it necessary to look ahead since the timer is stopped? -- Issue #62
			if( abs_time->tv_usec > now ){ //needs to be enough in the future to allow the OC to be set before the timer passes it
				if( chan_req.value == abs_time->tv_usec ){
					mcu_tmr_setchannel(&tmr_handle, &chan_req);
				}
				is_time_to_sleep = 1;
			}

			mcu_tmr_enable(&tmr_handle, 0);

		} else {
			is_time_to_sleep = 1;
		}
	}

	//only sleep if the time hasn't already passed
	if( is_time_to_sleep ){
		scheduler_root_update_on_sleep();
	}

}

void scheduler_timing_convert_timespec(struct mcu_timeval * tv, const struct timespec * ts){
	div_t d;
	if ( ts == NULL ){
		tv->tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
		tv->tv_usec = 0;
	} else {
		d = div(ts->tv_sec, SOS_SCHEDULER_TIMEVAL_SECONDS);
		tv->tv_sec = d.quot;
		tv->tv_usec = d.rem * 1000000UL + (ts->tv_nsec + 999UL) / 1000UL;
	}
}

void scheduler_timing_convert_mcu_timeval(struct timespec * ts, const struct mcu_timeval * mcu_tv){
	div_t d;
	d = div(mcu_tv->tv_usec, 1000000UL);
	ts->tv_sec = mcu_tv->tv_sec * SCHEDULER_TIMEVAL_SECONDS + d.quot;
	ts->tv_nsec = d.rem * 1000UL;
}

//never used
u32 scheduler_timing_get_realtime(){
	struct mcu_timeval tv;
	cortexm_svcall((cortexm_svcall_t)scheduler_timing_root_get_realtime, &tv);
	return tv.tv_usec;
}

struct mcu_timeval scheduler_timing_add_mcu_timeval(const struct mcu_timeval * a, const struct mcu_timeval * b){
	struct mcu_timeval result;
	result.tv_sec = a->tv_sec + b->tv_sec;
	result.tv_usec = a->tv_usec + b->tv_usec;
	if( result.tv_usec > SOS_USECOND_PERIOD ){
		result.tv_sec++;
		result.tv_usec -= SOS_USECOND_PERIOD;
	}
	return result;
}

struct mcu_timeval scheduler_timing_subtract_mcu_timeval(const struct mcu_timeval * a, const struct mcu_timeval * b){
	struct mcu_timeval result;
	result.tv_sec = 0;
	result.tv_usec = 0;
	if( a->tv_sec > b->tv_sec ){
		result.tv_sec = a->tv_sec - b->tv_sec;
		if( a->tv_usec >= b->tv_usec ){
			result.tv_usec = a->tv_usec - b->tv_usec;
		} else {
			result.tv_sec--;
			result.tv_usec = SOS_USECOND_PERIOD - (b->tv_usec - a->tv_usec);
		}
	} else if( a->tv_sec == b->tv_sec ){
		//tv_sec is already 0
		if( a->tv_usec >= b->tv_usec ){
			result.tv_usec = a->tv_usec - b->tv_usec;
		}
	}

	return result;
}



void scheduler_timing_root_get_realtime(struct mcu_timeval * tv){
	devfs_handle_t tmr_handle;
	tmr_handle.port = sos_board_config.clk_usecond_tmr;
	mcu_tmr_disable(&tmr_handle, 0);
	tv->tv_sec = sched_usecond_counter;
	mcu_tmr_get(&tmr_handle, &(tv->tv_usec));
	mcu_tmr_enable(&tmr_handle, 0);
}

int root_handle_usecond_overflow_event(void * context, const mcu_event_t * data){
	sched_usecond_counter++;
	root_handle_usecond_match_event(0, 0);
	root_handle_usecond_process_timer_match_event(0, 0);
	return 1; //do not clear callback
}

int root_handle_usecond_match_event(void * context, const mcu_event_t * data){
	int i;
	u32 next;
	u32 tmp;
	int new_priority;
	mcu_channel_t chan_req;
	u32 now;
	devfs_handle_t tmr_handle;
	tmr_handle.port = sos_board_config.clk_usecond_tmr;
	tmr_handle.config = 0;
	tmr_handle.state = 0;

	//Initialize variables
	chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
	chan_req.value = SOS_USECOND_PERIOD + 1;
	new_priority = SCHED_LOWEST_PRIORITY - 1;
	next = SOS_USECOND_PERIOD;

	mcu_tmr_disable(&tmr_handle, 0);
	mcu_tmr_get(&tmr_handle, &now);

	for(i=1; i < task_get_total(); i++){

		if( task_enabled_not_active(i) ){
			tmp = sos_sched_table[i].wake.tv_usec;

			//compare the current clock to the wake time
			if ( (sos_sched_table[i].wake.tv_sec < sched_usecond_counter) ||
				  ( (sos_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp <= now) )
				  ){
				//wake this task
				scheduler_root_assert_active(i, SCHEDULER_UNBLOCK_SLEEP);
				if( !task_stopped_asserted(i) && (scheduler_priority(i) > new_priority) ){
					new_priority = scheduler_priority(i);
				}

			} else if ( (sos_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp < next) ) {
				//see if this is the next event to wake up
				next = tmp;
			}
		}
	}
	if ( next < SOS_USECOND_PERIOD ){
		chan_req.value = next;
	}
	mcu_tmr_setchannel(&tmr_handle, &chan_req);

	scheduler_root_update_on_wake(-1, new_priority);
	mcu_tmr_enable(&tmr_handle, 0);

	return 1;
}

int root_handle_usecond_process_timer_match_event(void * context, const mcu_event_t * data){
	//a system timer expired
	int i;
	int j;
	u32 next;
	u32 tmp;
	mcu_channel_t chan_req;
	u32 now;
	devfs_handle_t tmr_handle;
	tmr_handle.port = sos_board_config.clk_usecond_tmr;
	tmr_handle.config = 0;
	tmr_handle.state = 0;

	//Initialize variables
	chan_req.loc = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;
	chan_req.value = SOS_USECOND_PERIOD + 1;
	next = SOS_USECOND_PERIOD;

	mcu_tmr_disable(&tmr_handle, 0);
	mcu_tmr_get(&tmr_handle, &now);


	for(i=1; i < task_get_total(); i++){
		//look for the next signal

		if( task_enabled(i) ){

			for(j=0; j < SOS_PROCESS_TIMER_COUNT; j++){
				volatile sos_process_timer_t * timer = sos_sched_table[i].timer + j;

				if( timer->o_flags & SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_INITIALIZED ){
					tmp = timer->value.tv_usec;
					if ( (timer->value.tv_sec < sched_usecond_counter) ||
						  ( (timer->value.tv_sec == sched_usecond_counter) && (tmp <= now) )
						  ){

						//reload the timer if interval is valid
						send_and_reload_timer(timer, i, now);

						//if interval value is non-zero, need to check if this is less than next
						if( timer->interval.tv_sec + timer->interval.tv_usec ){
							tmp = timer->value.tv_usec;
							if( (timer->value.tv_sec == sched_usecond_counter) && (tmp < next) ){
								next = tmp;
							}
						}

					} else if ( (timer->value.tv_sec == sched_usecond_counter) && (tmp < next) ) {
						//see if this is the next event to wake up
						next = tmp;
					}
				}
			}
		}
	}

	if ( next < SOS_USECOND_PERIOD ){
		chan_req.value = next;
	}
	mcu_tmr_setchannel(&tmr_handle, &chan_req);
	mcu_tmr_enable(&tmr_handle, 0);

	return 1;
}


int open_usecond_tmr(){
	int err;
	tmr_attr_t attr;
	tmr_info_t info;
	mcu_action_t action;
	mcu_channel_t chan_req;
	devfs_handle_t tmr;
	tmr.port = sos_board_config.clk_usecond_tmr;
	tmr.config = 0;
	tmr.state = 0;

	//Open the microsecond timer
	err = mcu_tmr_open(&tmr);
	if(err){ return err; }

	err = mcu_tmr_getinfo(&tmr, &info);
	if(err){ return err; }


	memset(&attr, 0, sizeof(tmr_attr_t));
	attr.freq = 1000000UL;
	attr.o_flags = TMR_FLAG_SET_TIMER | TMR_FLAG_IS_SOURCE_CPU | TMR_FLAG_IS_AUTO_RELOAD;
	attr.period = SOS_USECOND_PERIOD; //only works if TMR_FLAG_IS_AUTO_RELOAD is supported
	memset(&attr.pin_assignment, 0xff, sizeof(tmr_pin_assignment_t));

	err = mcu_tmr_setattr(&tmr, &attr);
	if ( err ){
		return err;
	}


	//Initialize the value of the timer to zero
	err = mcu_tmr_set(&tmr, (void*)0);
	if (err){ return err; }

	if( (info.o_flags & TMR_FLAG_IS_AUTO_RELOAD) == 0 ){
		//The reset OC is only needed if TMR_FLAG_IS_AUTO_RELOAD is not supported
		//Set the reset output compare value to reset the clock every STFY_USECOND_PERIOD

		attr.channel.loc = SCHED_USECOND_TMR_RESET_OC;
		attr.channel.value = SOS_USECOND_PERIOD;
		attr.o_flags = TMR_FLAG_SET_CHANNEL | TMR_FLAG_IS_CHANNEL_RESET_ON_MATCH;
		err = mcu_tmr_setattr(&tmr, &attr);
		if ( err ){
			return err;
		}

		action.prio = 0;
		action.channel = SCHED_USECOND_TMR_RESET_OC;
		action.o_events = MCU_EVENT_FLAG_MATCH;
		action.handler.callback = root_handle_usecond_overflow_event;
		action.handler.context = 0;
		err = mcu_tmr_setaction(&tmr, &action);
		if (err ){
			return -1;
		}
	} else {
		action.prio = 0;
		action.channel = 0; //doesn't matter
		action.o_events = MCU_EVENT_FLAG_OVERFLOW;
		action.handler.callback = root_handle_usecond_overflow_event;
		action.handler.context = 0;
		err = mcu_tmr_setaction(&tmr, &action);
		if (err ){
			return -1;
		}
	}

	//Turn the timer on
	err = mcu_tmr_enable(&tmr, 0);
	if (err){ return -1; }

	//This sets up the output compare unit used with the usleep() function
	chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
	chan_req.value = SOS_USECOND_PERIOD + 1;
	err = mcu_tmr_setchannel(&tmr, &chan_req);
	if ( err ){ return -1; }

	action.channel = SCHED_USECOND_TMR_SLEEP_OC;
	action.o_events = MCU_EVENT_FLAG_MATCH;
	action.handler.callback = root_handle_usecond_match_event;
	action.handler.context = 0;
	err = mcu_tmr_setaction(&tmr, &action);
	if ( err ){
		return -1;
	}

#if SOS_PROCESS_TIMER_COUNT > 0
	chan_req.loc = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;
	err = mcu_tmr_setchannel(&tmr, &chan_req);
	if ( err ){ return -1; }

	action.channel = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;
	action.o_events = MCU_EVENT_FLAG_MATCH;
	action.handler.callback = root_handle_usecond_process_timer_match_event;
	action.handler.context = 0;
	err = mcu_tmr_setaction(&tmr, &action);
	if ( err ){
		return -1;
	}
#endif

	return 0;
}




