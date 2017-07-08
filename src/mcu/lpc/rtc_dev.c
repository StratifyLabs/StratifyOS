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
#include "mcu/cortexm.h"
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

#if MCU_RTC_API == 1

#else
#define RTC_ISDST_REG (regs->GPREG0)
#endif

static LPC_RTC_Type * const rtc_regs[MCU_RTC_PORTS] = MCU_RTC_REGS;
static const u8 const rtc_irqs[MCU_RTC_PORTS] = MCU_RTC_IRQS;

typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} rtc_local_t;
static rtc_local_t rtc_local MCU_SYS_MEM;


void _mcu_rtc_dev_power_on(int port){
	LPC_RTC_Type * regs = rtc_regs[port];
	if ( rtc_local.ref_count == 0 ){
		rtc_local.handler.callback = NULL;
		regs->CCR = (CLKEN);
	}
	rtc_local.ref_count++;
}

void _mcu_rtc_dev_power_off(int port){
	//LPC_RTC_Type * regs = rtc_regs[port];
	if ( rtc_local.ref_count > 0 ){
		if ( rtc_local.ref_count == 1 ){
			//core_priv_disable_irq((void*)RTC_IRQn);
			//regs->CCR = 0;
		}
		rtc_local.ref_count--;
	}
}

int _mcu_rtc_dev_powered_on(int port){
	LPC_RTC_Type * regs = rtc_regs[port];

	if ( regs->CCR & (CLKEN) ){
		return 1;
	} else {
		return 0;
	}
}


int mcu_rtc_getinfo(int port, void * ctl){
	rtc_attr_t * ctlp;
	ctlp = (rtc_attr_t *)ctl;
	ctlp->pin_assign = 0;
	return 0;
}

int mcu_rtc_setattr(int port, void * ctl){
	//LPC_RTC_Type * regs = rtc_regs[port];
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

	if( _mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	rtc_local.handler.callback = action->handler.callback;
	rtc_local.handler.context = action->handler.context;

	_mcu_cortexm_set_irq_prio(rtc_irqs[port], action->prio);


	//Set the event
	return mcu_rtc_setcountevent(port, (void*)action->o_events);
}

int _mcu_rtc_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return 0;
}

int mcu_rtc_setalarm(int port, void * ctl){
	LPC_RTC_Type * regs = rtc_regs[port];

	rtc_alarm_t * alarmp;

	alarmp = (rtc_alarm_t *)ctl;
	_mcu_cortexm_priv_enable_irq((void*)RTC_IRQn);

	//elevate prio to come out of hibernate
	_mcu_cortexm_set_irq_prio(RTC_IRQn, 3);


	regs->ASEC = alarmp->time.time.tm_sec;
	regs->AMIN = alarmp->time.time.tm_min;
	regs->AHRS = alarmp->time.time.tm_hour;
	regs->ADOM = alarmp->time.time.tm_mday;
	regs->ADOW = alarmp->time.time.tm_wday;
	regs->ADOY = alarmp->time.time.tm_yday + 1;
	regs->AMON = alarmp->time.time.tm_mon + 1;
	regs->AYRS = alarmp->time.time.tm_year;


	switch(alarmp->type){
	case RTC_ALARM_ONCE:
		regs->AMR = AMRDOY|AMRDOW; //don't compare the day of the year or day of week
		break;
	case RTC_ALARM_MINUTE:
		regs->AMR = (~(AMRSEC)) & 0xFF; //only compare seconds
		break;
	case RTC_ALARM_HOURLY:
		regs->AMR = (~(AMRSEC|AMRMIN))  & 0xFF;
		break;
	case RTC_ALARM_DAILY:
		regs->AMR =  (~(AMRSEC|AMRMIN|AMRHOUR))  & 0xFF;
		break;
	case RTC_ALARM_WEEKLY:
		regs->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOW)) & 0xFF;
		break;
	case RTC_ALARM_MONTHLY:
		regs->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOM)) & 0xFF;
		break;
	case RTC_ALARM_YEARLY:
		regs->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOM|AMRMON)) & 0xFF;
		break;
	}
	return 0;
}

int mcu_rtc_getalarm(int port, void * ctl){
	LPC_RTC_Type * regs = rtc_regs[port];

	rtc_alarm_t * alarmp;
	alarmp = (rtc_alarm_t *)ctl;
	alarmp->time.time.tm_sec = regs->ASEC;
	alarmp->time.time.tm_min = regs->AMIN;
	alarmp->time.time.tm_hour = regs->AHRS;
	alarmp->time.time.tm_mday = regs->ADOM;
	alarmp->time.time.tm_wday = regs->ADOW;
	alarmp->time.time.tm_yday = regs->ADOY - 1;
	alarmp->time.time.tm_mon = regs->AMON - 1;
	alarmp->time.time.tm_year = regs->AYRS;
	return 0;
}

int mcu_rtc_disablealarm(int port, void * ctl){
	LPC_RTC_Type * regs = rtc_regs[port];

	regs->AMR = 0xFF;
	return 0;
}

int mcu_rtc_set(int port, void * ctl){
	LPC_RTC_Type * regs = rtc_regs[port];

	rtc_time_t * timep;
	timep = (rtc_time_t*)ctl;
	regs->SEC = timep->time.tm_sec;
	regs->MIN = timep->time.tm_min;
	regs->HRS = timep->time.tm_hour;
	regs->DOW = timep->time.tm_wday;
	regs->DOM = timep->time.tm_mday;
	regs->DOY = timep->time.tm_yday + 1;
	regs->MONTH = timep->time.tm_mon + 1;
	regs->YEAR = timep->time.tm_year;
#if defined RTC_ISDST_REG
	RTC_ISDST_REG = timep->time.tm_isdst;
#endif
	return 0;
}

int mcu_rtc_get(int port, void * ctl){
	LPC_RTC_Type * regs = rtc_regs[port];

	rtc_time_t * timep;
	timep = (rtc_time_t*)ctl;
	timep->time.tm_sec = regs->SEC;
	timep->time.tm_min = regs->MIN;
	timep->time.tm_hour = regs->HRS;
	timep->time.tm_wday = regs->DOW;
	timep->time.tm_mday = regs->DOM;
	timep->time.tm_yday = regs->DOY - 1;
	timep->time.tm_mon = regs->MONTH - 1;
	timep->time.tm_year = regs->YEAR;
#if defined RTC_ISDST_REG
	timep->time.tm_isdst = RTC_ISDST_REG;
#endif
	timep->useconds = 0;
	return 0;
}

int mcu_rtc_setcountevent(int port, void * ctl){
	LPC_RTC_Type * regs = rtc_regs[port];

	rtc_event_count_t event = (rtc_event_count_t)ctl;
	_mcu_cortexm_priv_enable_irq((void*)RTC_IRQn);
	switch(event){
	case RTC_EVENT_COUNT_NONE:
		regs->CIIR = 0;
		break;
	case RTC_EVENT_COUNT_SECOND:
		regs->CIIR = 1;
		break;
	case RTC_EVENT_COUNT_MINUTE:
		regs->CIIR = 2;
		break;
	case RTC_EVENT_COUNT_HOUR:
		regs->CIIR = 4;
		break;
	case RTC_EVENT_COUNT_DAY:
		regs->CIIR = 8;
		break;
	case RTC_EVENT_COUNT_WEEK:
		regs->CIIR = 16;
		break;
	case RTC_EVENT_COUNT_MONTH:
		regs->CIIR = 64;
		break;
	case RTC_EVENT_COUNT_YEAR:
		regs->CIIR = 128;
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}

void _mcu_core_rtc0_isr(){
	const int port = 0;
	LPC_RTC_Type * regs = rtc_regs[port];
	rtc_event_t event;
	int flags;
	flags = regs->ILR & 0x03;
	regs->ILR = flags; //clear the flags
	event = 0;
	if ( flags & RTCALF ){
		event |= (1<<RTC_EVENT_ALARM);
	}
	if ( flags & RTCCIF ){
		event |= (1<<RTC_EVENT_COUNT);
	}

	_mcu_cortexm_execute_event_handler(&(rtc_local.handler), (mcu_event_t)event);
}

#endif




