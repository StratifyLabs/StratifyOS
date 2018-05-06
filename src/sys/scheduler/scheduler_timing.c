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

#include "mcu/tmr.h"
#include "mcu/rtc.h"
#include "mcu/debug.h"

static volatile u32 sched_usecond_counter MCU_SYS_MEM;

static int open_usecond_tmr();
static int root_handle_usecond_overflow_event(void * context, const mcu_event_t * data);
static int root_handle_usecond_match_event(void * context, const mcu_event_t * data);

int scheduler_timing_init(){
	if ( open_usecond_tmr() < 0 ){
		return -1;
	}
	return 0;
}

u32 scheduler_timing_seconds_to_clocks(int seconds){
	return (u32)mcu_core_getclock() * (u32)seconds;
}

u32 scheduler_timing_useconds_to_clocks(int useconds){
	return (u32)(SCHEDULER_CLOCK_USEC_MULT * useconds);
}

u32 scheduler_timing_nanoseconds_to_clocks(int nanoseconds){
	return (u32)nanoseconds * 1024 / SCHEDULER_CLOCK_NSEC_DIV;
}

void scheduler_timing_root_timedblock(void * block_object, struct mcu_timeval * abs_time){
	int id;
	mcu_channel_t chan_req;
	u32 now;
	devfs_handle_t tmr_handle;
	bool time_sleep;
	tmr_handle.port = sos_board_config.clk_usecond_tmr;

	//Initialization
	id = task_get_current();
	sos_sched_table[id].block_object = block_object;
	time_sleep = false;

	if (abs_time->tv_sec >= sched_usecond_counter){

		sos_sched_table[id].wake.tv_sec = abs_time->tv_sec;
		sos_sched_table[id].wake.tv_usec = abs_time->tv_usec;

		if(abs_time->tv_sec == sched_usecond_counter){

			mcu_tmr_disable(&tmr_handle, 0);

			//Read the current OC value to see if it needs to be updated
			chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
			mcu_tmr_getchannel(&tmr_handle, &chan_req);
			if ( abs_time->tv_usec < chan_req.value ){
				chan_req.value = abs_time->tv_usec;
			}

			//See if abs_time is in the past
            mcu_tmr_get(&tmr_handle, &now);
			if( abs_time->tv_usec > (now+40) ){ //needs to be enough in the future to allow the OC to be set before the timer passes it
				mcu_tmr_setchannel(&tmr_handle, &chan_req);
				time_sleep = true;
			}

			mcu_tmr_enable(&tmr_handle, 0);


		} else {
			time_sleep = true;
		}
	}

	if ( (block_object == NULL) && (time_sleep == false) ){
		//Do not sleep
		return;
	}

	scheduler_root_update_on_sleep();
}

void scheduler_timing_convert_timespec(struct mcu_timeval * tv, const struct timespec * ts){
	div_t d;
	if ( ts == NULL ){
		tv->tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
		tv->tv_usec = 0;
	} else {
		d = div(ts->tv_sec, STFY_SCHEDULER_TIMEVAL_SECONDS);
		tv->tv_sec = d.quot;
		tv->tv_usec = d.rem * 1000000 + (ts->tv_nsec + 500) / 1000;
	}
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
	root_handle_usecond_match_event(NULL, 0);
	return 1; //do not clear callback
}

int root_handle_usecond_match_event(void * context, const mcu_event_t * data){
	int i;
	u32 next;
	u32 tmp;
	int new_priority;
	mcu_channel_t chan_req;
	u32 current_match;
	devfs_handle_t tmr_handle;
	tmr_handle.port = sos_board_config.clk_usecond_tmr;
	tmr_handle.config = 0;
	tmr_handle.state = 0;

	//Initialize variables
	chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
	chan_req.value = STFY_USECOND_PERIOD + 1;
	new_priority = SCHED_LOWEST_PRIORITY - 1;
	next = STFY_USECOND_PERIOD;

	mcu_tmr_disable(&tmr_handle, 0);
    mcu_tmr_get(&tmr_handle, &current_match);

	for(i=1; i < task_get_total(); i++){
        if ( task_enabled(i) && !task_active_asserted(i) ){ //enabled and inactive tasks only
			tmp = sos_sched_table[i].wake.tv_usec;

			//compare the current clock to the wake time
			if ( (sos_sched_table[i].wake.tv_sec < sched_usecond_counter) ||
					( (sos_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp <= current_match) )
			){
				//wake this task
                sos_sched_table[i].wake.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID; //this line is duplicated in scheduler_root_assert_active()
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
	if ( next < STFY_USECOND_PERIOD ){
		chan_req.value = next;
	}
	mcu_tmr_setchannel(&tmr_handle, &chan_req);

    scheduler_root_update_on_wake(-1, new_priority);

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
	attr.freq = 1000000;
	attr.o_flags = TMR_FLAG_SET_TIMER | TMR_FLAG_IS_SOURCE_CPU | TMR_FLAG_IS_AUTO_RELOAD;
	attr.period = STFY_USECOND_PERIOD; //only works if TMR_FLAG_IS_AUTO_RELOAD is supported
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
		attr.channel.value = STFY_USECOND_PERIOD;
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
	chan_req.value = STFY_USECOND_PERIOD + 1;
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

	return 0;
}




