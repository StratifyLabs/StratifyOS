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
#include <errno.h>
#include "mcu/rtc.h"
#include "mcu/debug.h"
#include "mcu/core.h"

#define CLKEN (1<<0)
#define CTCRST (1<<1)
#define CCALEN (1<<4)

#define RTCCIF (1<<0)
#define RTCALF (1<<1)

#define AMRSEC (1<<0)
#define AMRMIN (1<<1)
#define AMRHOUR (1<<2)
#define AMRDOM (1<<3)
#define AMRDOW (1<<4)
#define AMRDOY (1<<5)
#define AMRMON (1<<6)
#define AMRYEAR (1<<7)

#if MCU_RTC_PORTS > 0

#define RTC_ISDST_REG (LPC_RTC->GPREG0)

typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} rtc_local_t;
static rtc_local_t rtc_local MCU_SYS_MEM;


void _mcu_rtc_dev_power_on(int port){
	if ( rtc_local.ref_count == 0 ){
		rtc_local.handler.callback = NULL;
		LPC_RTC->CCR = (CLKEN);
	}
	rtc_local.ref_count++;
}

void _mcu_rtc_dev_power_off(int port){
	if ( rtc_local.ref_count > 0 ){
		if ( rtc_local.ref_count == 1 ){
			mcu_priv_debug("RTC OFF\n");
			//core_priv_disable_irq((void*)RTC_IRQn);
			//LPC_RTC->CCR = 0;
		}
		rtc_local.ref_count--;
	}
}

int _mcu_rtc_dev_powered_on(int port){
	if ( LPC_RTC->CCR & (CLKEN) ){
		return 1;
	} else {
		return 0;
	}
}


int mcu_rtc_getattr(int port, void * ctl){
	rtc_attr_t * ctlp;
	ctlp = (rtc_attr_t *)ctl;
	ctlp->pin_assign = 0;
	return 0;
}

int mcu_rtc_setattr(int port, void * ctl){
	rtc_attr_t * ctlp;
	ctlp = (rtc_attr_t *)ctl;
	if ( ctlp->pin_assign != 0 ){
		errno = EINVAL;
		return -1 - offsetof(rtc_attr_t, pin_assign);
	}
	return 0;
}

int mcu_rtc_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;

	if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	rtc_local.handler.callback = action->callback;
	rtc_local.handler.context = action->context;

	//Set the event
	return mcu_rtc_setcountevent(port, (void*)action->event);
}

int _mcu_rtc_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	return 0;
}

int mcu_rtc_setalarm(int port, void * ctl){
	rtc_alarm_t * alarmp;

	alarmp = (rtc_alarm_t *)ctl;
	_mcu_core_priv_enable_irq((void*)RTC_IRQn);

	LPC_RTC->ALSEC = alarmp->time.time.tm_sec;
	LPC_RTC->ALMIN = alarmp->time.time.tm_min;
	LPC_RTC->ALHOUR = alarmp->time.time.tm_hour;
	LPC_RTC->ALDOM = alarmp->time.time.tm_mday;
	LPC_RTC->ALDOW = alarmp->time.time.tm_wday;
	LPC_RTC->ALDOY = alarmp->time.time.tm_yday + 1;
	LPC_RTC->ALMON = alarmp->time.time.tm_mon + 1;
	LPC_RTC->ALYEAR = alarmp->time.time.tm_year;

	switch(alarmp->type){
	case RTC_ALARM_ONCE:
		LPC_RTC->AMR = AMRDOY|AMRDOW; //don't compare the day of the year or day of week
		break;
	case RTC_ALARM_MINUTE:
		LPC_RTC->AMR = (~(AMRSEC)) & 0xFF; //only compare seconds
		break;
	case RTC_ALARM_HOURLY:
		LPC_RTC->AMR = (~(AMRSEC|AMRMIN))  & 0xFF;
		break;
	case RTC_ALARM_DAILY:
		LPC_RTC->AMR =  (~(AMRSEC|AMRMIN|AMRHOUR))  & 0xFF;
		break;
	case RTC_ALARM_WEEKLY:
		LPC_RTC->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOW)) & 0xFF;
		break;
	case RTC_ALARM_MONTHLY:
		LPC_RTC->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOM)) & 0xFF;
		break;
	case RTC_ALARM_YEARLY:
		LPC_RTC->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOM|AMRMON)) & 0xFF;
		break;
	}
	return 0;
}

int mcu_rtc_getalarm(int port, void * ctl){
	rtc_alarm_t * alarmp;
	alarmp = (rtc_alarm_t *)ctl;
	alarmp->time.time.tm_sec = LPC_RTC->ALSEC;
	alarmp->time.time.tm_min = LPC_RTC->ALMIN;
	alarmp->time.time.tm_hour = LPC_RTC->ALHOUR;
	alarmp->time.time.tm_mday = LPC_RTC->ALDOM;
	alarmp->time.time.tm_wday = LPC_RTC->ALDOW;
	alarmp->time.time.tm_yday = LPC_RTC->ALDOY - 1;
	alarmp->time.time.tm_mon = LPC_RTC->ALMON - 1;
	alarmp->time.time.tm_year = LPC_RTC->ALYEAR;
	return 0;
}

int mcu_rtc_disablealarm(int port, void * ctl){
	LPC_RTC->AMR = 0xFF;
	return 0;
}

int mcu_rtc_set(int port, void * ctl){
	rtc_time_t * timep;
	timep = (rtc_time_t*)ctl;
	LPC_RTC->SEC = timep->time.tm_sec;
	LPC_RTC->MIN = timep->time.tm_min;
	LPC_RTC->HOUR = timep->time.tm_hour;
	LPC_RTC->DOW = timep->time.tm_wday;
	LPC_RTC->DOM = timep->time.tm_mday;
	LPC_RTC->DOY = timep->time.tm_yday + 1;
	LPC_RTC->MONTH = timep->time.tm_mon + 1;
	LPC_RTC->YEAR = timep->time.tm_year;
	RTC_ISDST_REG = timep->time.tm_isdst;
	return 0;
}

int mcu_rtc_get(int port, void * ctl){
	rtc_time_t * timep;
	timep = (rtc_time_t*)ctl;
	timep->time.tm_sec = LPC_RTC->SEC;
	timep->time.tm_min = LPC_RTC->MIN;
	timep->time.tm_hour = LPC_RTC->HOUR;
	timep->time.tm_wday = LPC_RTC->DOW;
	timep->time.tm_mday = LPC_RTC->DOM;
	timep->time.tm_yday = LPC_RTC->DOY - 1;
	timep->time.tm_mon = LPC_RTC->MONTH - 1;
	timep->time.tm_year = LPC_RTC->YEAR;
	timep->time.tm_isdst = RTC_ISDST_REG;
	timep->useconds = 0;
	return 0;
}

int mcu_rtc_setcountevent(int port, void * ctl){
	rtc_event_count_t event = (rtc_event_count_t)ctl;
	_mcu_core_priv_enable_irq((void*)RTC_IRQn);
	switch(event){
	case RTC_EVENT_COUNT_NONE:
		LPC_RTC->CIIR = 0;
		break;
	case RTC_EVENT_COUNT_SECOND:
		LPC_RTC->CIIR = 1;
		break;
	case RTC_EVENT_COUNT_MINUTE:
		LPC_RTC->CIIR = 2;
		break;
	case RTC_EVENT_COUNT_HOUR:
		LPC_RTC->CIIR = 4;
		break;
	case RTC_EVENT_COUNT_DAY:
		LPC_RTC->CIIR = 8;
		break;
	case RTC_EVENT_COUNT_WEEK:
		LPC_RTC->CIIR = 16;
		break;
	case RTC_EVENT_COUNT_MONTH:
		LPC_RTC->CIIR = 64;
		break;
	case RTC_EVENT_COUNT_YEAR:
		LPC_RTC->CIIR = 128;
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}

void _mcu_core_rtc_isr(void){
	rtc_event_t event;
	int flags;
	flags = LPC_RTC->ILR & 0x03;
	LPC_RTC->ILR = flags; //clear the flags
	event = 0;
	if ( flags & RTCALF ){
		event |= (1<<RTC_EVENT_ALARM);
	}
	if ( flags & RTCCIF ){
		event |= (1<<RTC_EVENT_COUNT);
	}

	_mcu_core_exec_event_handler(&(rtc_local.handler), (mcu_event_t)event);
}

#endif




