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


#include "config.h"

#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "sched_flags.h"

#include "mcu/tmr.h"
#include "mcu/rtc.h"
#include "mcu/debug.h"

static volatile u32 sched_usecond_counter MCU_SYS_MEM;

static int usecond_overflow_event(void * context, mcu_event_t * data);
static int open_usecond_tmr();

static int priv_usecond_match_event(void * context, mcu_event_t * data);

int sched_timing_init(){
	if ( open_usecond_tmr() < 0 ){
		return -1;
	}
	return 0;
}

u32 sched_seconds_to_clocks(int seconds){
	return (u32)_mcu_core_getclock() * (u32)seconds;
}

u32 sched_useconds_to_clocks(int useconds){
	return (u32)(sos_board_config.clk_usec_mult * useconds);
}

u32 sched_nanoseconds_to_clocks(int nanoseconds){
	return (u32)nanoseconds * 1024 / sos_board_config.clk_nsec_div;
}

void sched_priv_timedblock(void * block_object, struct sched_timeval * abs_time){
	int id;
	mcu_channel_t chan_req;
	u32 now;
	bool time_sleep;

	//Initialization
	id = task_get_current();
	sos_sched_table[id].block_object = block_object;
	time_sleep = false;

	if (abs_time->tv_sec >= sched_usecond_counter){

		sos_sched_table[id].wake.tv_sec = abs_time->tv_sec;
		sos_sched_table[id].wake.tv_usec = abs_time->tv_usec;

		if(abs_time->tv_sec == sched_usecond_counter){

			mcu_tmr_disable(sos_board_config.clk_usecond_tmr, 0);

			//Read the current OC value to see if it needs to be updated
			chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
			mcu_tmr_getoc(sos_board_config.clk_usecond_tmr, &chan_req);
			if ( abs_time->tv_usec < chan_req.value ){
				chan_req.value = abs_time->tv_usec;
			}

			//See if abs_time is in the past
			now = (u32)mcu_tmr_get(sos_board_config.clk_usecond_tmr, NULL);
			if( abs_time->tv_usec > (now+40) ){ //needs to be enough in the future to allow the OC to be set before the timer passes it
				mcu_tmr_setoc(sos_board_config.clk_usecond_tmr, &chan_req);
				time_sleep = true;
			}

			mcu_tmr_enable(sos_board_config.clk_usecond_tmr, 0);


		} else {
			time_sleep = true;
		}
	}

	if ( (block_object == NULL) && (time_sleep == false) ){
		//Do not sleep
		return;
	}

	sched_priv_update_on_sleep();
}

void sched_convert_timespec(struct sched_timeval * tv, const struct timespec * ts){
#if SINGLE_TASK == 0
	div_t d;
	if ( ts == NULL ){
		tv->tv_sec = SCHED_TIMEVAL_SEC_INVALID;
		tv->tv_usec = 0;
	} else {
		d = div(ts->tv_sec, STFY_SCHED_TIMEVAL_SECONDS);
		tv->tv_sec = d.quot;
		tv->tv_usec = d.rem * 1000000 + (ts->tv_nsec + 500) / 1000;
	}
#endif
}

void sched_convert_timeval(struct timeval * t, const struct sched_timeval * tv){
#if SINGLE_TASK == 0

#endif
}

void sched_priv_get_realtime(struct sched_timeval * tv){
	tv->tv_sec = sched_usecond_counter;
	tv->tv_usec = (u32)mcu_tmr_get(sos_board_config.clk_usecond_tmr, NULL);
}

int usecond_overflow_event(void * context, mcu_event_t * data){
	sched_usecond_counter++;
	priv_usecond_match_event(NULL, 0);
	return 1; //do not clear callback
}

int priv_usecond_match_event(void * context, mcu_event_t * data){
#if SINGLE_TASK == 0
	int i;
	u32 next;
	u32 tmp;
	int new_priority;
	mcu_channel_t chan_req;
	static const u32 overflow = (STFY_USECOND_PERIOD);
	u32 current_match;

	//Initialize variables
	chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
	chan_req.value = STFY_USECOND_PERIOD + 1;
	new_priority = SCHED_LOWEST_PRIORITY - 1;
	next = overflow;

	mcu_tmr_disable(sos_board_config.clk_usecond_tmr, 0);
	current_match = mcu_tmr_get(sos_board_config.clk_usecond_tmr, NULL);

	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) && !sched_active_asserted(i) ){ //enabled and inactive tasks only
			tmp = sos_sched_table[i].wake.tv_usec;
			//compare the current clock to the wake time
			if ( (sos_sched_table[i].wake.tv_sec < sched_usecond_counter) ||
					( (sos_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp <= current_match) )
			){
				//wake this task
				sos_sched_table[i].wake.tv_sec = SCHED_TIMEVAL_SEC_INVALID;
				sched_priv_assert_active(i, SCHED_UNBLOCK_SLEEP);
				if( !sched_stopped_asserted(i) && (sched_get_priority(i) > new_priority) ){
					new_priority = sched_get_priority(i);
				}

			} else if ( (sos_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp < next) ) {
				//see if this is the next event to wake up
				next = tmp;
			}
		}
	}
	if ( next < overflow ){
		chan_req.value = next;
	}
	mcu_tmr_setoc(sos_board_config.clk_usecond_tmr, &chan_req);

	sched_priv_update_on_wake(new_priority);

	mcu_tmr_enable(sos_board_config.clk_usecond_tmr, 0);

#endif
	return 1;
}

int open_usecond_tmr(){
	int err;
	tmr_attr_t attr;
	mcu_action_t action;
	mcu_channel_t chan_req;
	devfs_handle_t tmr;


	tmr.port = sos_board_config.clk_usecond_tmr;
	//Open the microsecond timer
	err = mcu_tmr_open(&tmr);
	if (err){
		return err;
	}

	memset(&attr, 0, sizeof(tmr_attr_t));
	attr.freq = 1000000;
	attr.o_flags = TMR_FLAG_SET_TIMER | TMR_FLAG_IS_CLKSRC_CPU;
	memset(&attr.pin_assignment, 0xff, sizeof(tmr_pin_assignment_t));

	err = mcu_tmr_setattr(tmr.port, &attr);
	if ( err ){
		return err;
	}


	//Initialize the value of the timer to zero
	err = mcu_tmr_set(tmr.port, (void*)0);
	if (err){
		return err;
	}

	//Set the reset output compare value to reset the clock every 32 seconds
	chan_req.loc = SCHED_USECOND_TMR_RESET_OC;
	chan_req.value = STFY_USECOND_PERIOD; //overflow every SCHED_TIMEVAL_SECONDS seconds
	err = mcu_tmr_setoc(tmr.port, &chan_req);
	if (err){
		return -1;
	}

	attr.channel.loc = SCHED_USECOND_TMR_RESET_OC;
	attr.o_flags = TMR_FLAG_SET_CHANNEL | TMR_FLAG_IS_CHANNEL_RESET_ON_MATCH;
	err = mcu_tmr_setattr(tmr.port, &attr);
	if ( err ){
		return err;
	}

	action.prio = 0;
	action.channel = SCHED_USECOND_TMR_RESET_OC;
	action.o_events = MCU_EVENT_FLAG_MATCH;
	action.handler.callback = usecond_overflow_event;
	err = mcu_tmr_setaction(tmr.port, &action);
	if (err ){
		return -1;
	}

	//Turn the timer on
	err = mcu_tmr_enable(tmr.port, 0);
	if (err){ return -1; }

	//This sets up the output compare unit used with the usleep() function
	chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
	chan_req.value = STFY_USECOND_PERIOD + 1;
	err = mcu_tmr_setoc(tmr.port, &chan_req);
	if ( err ){
		return -1;
	}

	action.channel = SCHED_USECOND_TMR_SLEEP_OC;
	action.o_events = MCU_EVENT_FLAG_MATCH;
	action.handler.callback = priv_usecond_match_event;
	err = mcu_tmr_setaction(tmr.port, &action);
	if ( err ){
		return -1;
	}

	return 0;
}




