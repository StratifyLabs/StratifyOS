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

static volatile uint32_t sched_usecond_counter MCU_SYS_MEM;

static int usecond_overflow_event(void * context, mcu_event_t data);
static int open_usecond_tmr();

static int priv_usecond_match_event(void * context, mcu_event_t data);

int sched_timing_init(){
	if ( open_usecond_tmr() < 0 ){
		return -1;
	}
	return 0;
}

uint32_t sched_seconds_to_clocks(int seconds){
	return (uint32_t)_mcu_core_getclock() * (uint32_t)seconds;
}

uint32_t sched_useconds_to_clocks(int useconds){
	return (uint32_t)(stfy_board_config.clk_usec_mult * useconds);
}

uint32_t sched_nanoseconds_to_clocks(int nanoseconds){
	return (uint32_t)nanoseconds * 1024 / stfy_board_config.clk_nsec_div;
}

void sched_priv_timedblock(void * block_object, struct sched_timeval * abs_time){
#if SINGLE_TASK == 0
	int id;
	tmr_reqattr_t chan_req;
	uint32_t now;
	bool time_sleep;

	//Initialization
	id = task_get_current();
	stfy_sched_table[id].block_object = block_object;
	time_sleep = false;

	if (abs_time->tv_sec >= sched_usecond_counter){

		stfy_sched_table[id].wake.tv_sec = abs_time->tv_sec;
		stfy_sched_table[id].wake.tv_usec = abs_time->tv_usec;

		if(abs_time->tv_sec == sched_usecond_counter){

			mcu_tmr_off(stfy_board_config.clk_usecond_tmr, NULL); //stop the timer

			//Read the current OC value to see if it needs to be updated
			chan_req.channel = SCHED_USECOND_TMR_SLEEP_OC;
			mcu_tmr_getoc(stfy_board_config.clk_usecond_tmr, &chan_req);
			if ( abs_time->tv_usec < chan_req.value ){
				chan_req.value = abs_time->tv_usec;
			}

			//See if abs_time is in the past
			now = (uint32_t)mcu_tmr_get(stfy_board_config.clk_usecond_tmr, NULL);
			if( abs_time->tv_usec > (now+40) ){ //needs to be enough in the future to allow the OC to be set before the timer passes it
				mcu_tmr_setoc(stfy_board_config.clk_usecond_tmr, &chan_req);
				time_sleep = true;
			}

			mcu_tmr_on(stfy_board_config.clk_usecond_tmr, NULL); //start the timer


		} else {
			time_sleep = true;
		}
	}

	if ( (block_object == NULL) && (time_sleep == false) ){
		//Do not sleep
		return;
	}

	sched_priv_update_on_sleep();
#endif
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
	tv->tv_usec = (uint32_t)mcu_tmr_get(stfy_board_config.clk_usecond_tmr, NULL);
}

int usecond_overflow_event(void * context, mcu_event_t data){
	sched_usecond_counter++;
	priv_usecond_match_event(NULL, 0);
	return 1; //do not clear callback
}

int priv_usecond_match_event(void * context, mcu_event_t data){
#if SINGLE_TASK == 0
	int i;
	uint32_t next;
	uint32_t tmp;
	int new_priority;
	tmr_reqattr_t chan_req;
	static const uint32_t overflow = (STFY_USECOND_PERIOD);
	uint32_t current_match;

	//Initialize variables
	chan_req.channel = SCHED_USECOND_TMR_SLEEP_OC;
	chan_req.value = STFY_USECOND_PERIOD + 1;
	new_priority = SCHED_LOWEST_PRIORITY - 1;
	next = overflow;

	mcu_tmr_off(stfy_board_config.clk_usecond_tmr, NULL); //stop the timer
	current_match = mcu_tmr_get(stfy_board_config.clk_usecond_tmr, NULL);

	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) && !sched_active_asserted(i) ){ //enabled and inactive tasks only
			tmp = stfy_sched_table[i].wake.tv_usec;
			//compare the current clock to the wake time
			if ( (stfy_sched_table[i].wake.tv_sec < sched_usecond_counter) ||
					( (stfy_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp <= current_match) )
			){
				//wake this task
				stfy_sched_table[i].wake.tv_sec = SCHED_TIMEVAL_SEC_INVALID;
				sched_priv_assert_active(i, SCHED_UNBLOCK_SLEEP);

				if ( sched_get_priority(i) > new_priority ){
					new_priority = sched_get_priority(i);
				}

			} else if ( (stfy_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp < next) ) {
				//see if this is the next event to wake up
				next = tmp;
			}
		}
	}
	if ( next < overflow ){
		chan_req.value = next;
	}
	mcu_tmr_setoc(stfy_board_config.clk_usecond_tmr, &chan_req);

	sched_priv_update_on_wake(new_priority);

	mcu_tmr_on(stfy_board_config.clk_usecond_tmr, NULL);
#endif
	return 1;
}

int open_usecond_tmr(){
	int err;
	tmr_attr_t cfg;
	tmr_action_t action;
	tmr_reqattr_t chan_req;
	device_periph_t tmr;


	tmr.port = stfy_board_config.clk_usecond_tmr;
	//Open the microsecond timer
	err = mcu_tmr_open((device_cfg_t*)&tmr);
	if (err){
		return err;
	}

	memset(&cfg, 0, sizeof(tmr_attr_t));
	cfg.freq = 1000000;
	cfg.clksrc = TMR_CLKSRC_CPU;
	err = mcu_tmr_setattr(tmr.port, &cfg);
	if ( err ){
		return err;
	}

	//Initialize the value of the timer to zero
	err = mcu_tmr_set(tmr.port, (void*)0);
	if (err){
		return err;
	}

	//Set the reset output compare value to reset the clock every 32 seconds
	chan_req.channel = SCHED_USECOND_TMR_RESET_OC;
	chan_req.value = STFY_USECOND_PERIOD; //overflow every SCHED_TIMEVAL_SECONDS seconds
	err = mcu_tmr_setoc(tmr.port, &chan_req);
	if (err){
		return -1;
	}

	action.prio = 0;
	action.channel = SCHED_USECOND_TMR_RESET_OC;
	action.event = TMR_ACTION_EVENT_RESET|TMR_ACTION_EVENT_INTERRUPT;
	action.callback = usecond_overflow_event;
	err = mcu_tmr_setaction(tmr.port, &action);
	if (err){
		return -1;
	}

	//Turn the timer on
	err = mcu_tmr_on(tmr.port, NULL);
	if (err){
		return -1;
	}

	//This sets up the output compare unit used with the usleep() function
	chan_req.channel = SCHED_USECOND_TMR_SLEEP_OC;
	chan_req.value = STFY_USECOND_PERIOD + 1;
	err = mcu_tmr_setoc(tmr.port, &chan_req);
	if ( err ){
		return -1;
	}

	action.channel = SCHED_USECOND_TMR_SLEEP_OC;
	action.event = TMR_ACTION_EVENT_INTERRUPT;
	action.callback = priv_usecond_match_event;

	err = mcu_tmr_setaction(tmr.port, &action);
	if ( err ){
		return -1;
	}

	return 0;
}




