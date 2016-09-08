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


//#include "config.h"
#include <errno.h>
#include <fcntl.h>
#include "iface/dev/bootloader.h"
#include "mcu/core.h"

#include "mcu/debug.h"

static core_reset_src_t _mcu_core_get_reset_src();
static int _mcu_core_enable_clkout(int clk_source, int div);
void _mcu_set_sleep_mode(int * level);
static u8 mcu_core_reset_source = CORE_RESET_SRC_SOFTWARE;

int mcu_core_setpinfunc(int port, void * arg){
	core_pinfunc_t * argp = arg;
	return _mcu_core_set_pinsel_func(argp->io.port,
			argp->io.pin,
			argp->periph_func,
			argp->periph_port);
}

void _mcu_core_dev_power_on(int port){}
void _mcu_core_dev_power_off(int port){}
int _mcu_core_dev_powered_on(int port){ return 1; }


void _mcu_core_exec_event_handler(mcu_event_handler_t * event, mcu_event_t arg){
	if( event->callback != 0 ){
		if( event->callback(event->context, arg) == 0 ){
			event->callback = 0;
		}
	}
}

int mcu_core_getattr(int port, void * arg){
	core_attr_t * attrp = arg;
	attrp->clock = mcu_board_config.core_cpu_freq;
	if( mcu_core_reset_source == CORE_RESET_SRC_SOFTWARE ){
		mcu_core_reset_source = _mcu_core_get_reset_src();
	}
	attrp->reset_type = mcu_core_reset_source;
	attrp->signature = _mcu_core_getsignature(port, arg);
	return _mcu_lpc_flash_get_serialno(attrp->serial_number);
}

int mcu_core_setattr(int port, void * arg){
	errno = ENOTSUP;
	return -1;
}

int mcu_core_setaction(int port, void * arg){
	errno = ENOTSUP;
	return -1;
}
int mcu_core_sleep(int port, void * arg){
	int level;
	level = (int)arg;

	_mcu_set_sleep_mode(&level);
	if ( level < 0 ){
		return level;
	}

	//Wait for an interrupts
	__WFI();
	return 0;
}

int mcu_core_reset(int port, void * arg){
	//delay first
	_mcu_core_delay_us(20*1000);
	_mcu_core_priv_reset(NULL);
	//doesn't arrive here
	return 0;
}

int mcu_core_invokebootloader(int port, void * arg){
	_mcu_core_delay_us(500*1000);
	bootloader_api_t api;
	_mcu_core_priv_bootloader_api(&api);
	api.exec(0);
	return 0;
}


int mcu_core_setclkout(int port, void * arg){
	core_clkout_t * clkout = arg;
	return _mcu_core_enable_clkout(clkout->src, clkout->div);
}

int mcu_core_setclkdivide(int port, void * arg){

#ifdef __lpc17xx
	//the errata on the LPC17xx chips prevent this from working correctly
	errno = ENOTSUP;
	return -1;
#endif

#ifdef LPCXX7X_8X
	uint32_t div = (int)arg;
	uint32_t clksel;

	if( (div >= 1) && (div <= 31) ){
		clksel = LPC_SC->CCLKSEL & ~0x1F;
		clksel |= div;
		LPC_SC->CCLKSEL = clksel;
	} else {
		errno = EINVAL;
		return -1;
	}
#endif

	return 0;
}

int mcu_core_getmcuboardconfig(int port, void * arg){
	memcpy(arg, &mcu_board_config, sizeof(mcu_board_config));
	return 0;
}


void _mcu_set_sleep_mode(int * level){
	SCB->SCR &= ~(1<<SCB_SCR_SLEEPDEEP_Pos);

#if defined LPC_SC
	LPC_SC->PCON = 0;
	switch(*level){
	case CORE_DEEPSLEEP_STOP:
		LPC_SC->PCON = 1; //turn off the flash as well
		//no break
	case CORE_DEEPSLEEP:
		SCB->SCR |= (1<<SCB_SCR_SLEEPDEEP_Pos);
		break;
	case CORE_SLEEP:
		break;
	case CORE_DEEPSLEEP_STANDBY:
		SCB->SCR |= (1<<SCB_SCR_SLEEPDEEP_Pos);
		LPC_SC->PCON = 3;
		break;
	default:
		*level = -1;
		return;
	}
	*level = 0;
#endif
}

core_reset_src_t _mcu_core_get_reset_src(){
	core_reset_src_t src = CORE_RESET_SRC_SOFTWARE;
	uint32_t src_reg;

#if defined LPC_SC
	src_reg = LPC_SC->RSID;
	LPC_SC->RSID = 0x0F;
#endif

	if ( src_reg & (1<<3) ){
		return CORE_RESET_SRC_BOR;
	}

	if ( src_reg & (1<<2) ){
		return CORE_RESET_SRC_WDT;
	}

	if ( src_reg & (1<<0) ){
		return CORE_RESET_SRC_POR;
	}

	if ( src_reg & (1<<1) ){
		return CORE_RESET_SRC_EXTERNAL;
	}

	return src;
}

int _mcu_core_enable_clkout(int clk_source, int div){
	div = (div & 0xF);
	if ( div != 0 ){
		div = div - 1;
	}

#if defined LPC_SC
	switch(clk_source){
	case CORE_CLKOUT_CPU:
		LPC_SC->CLKOUTCFG = (1<<8)|(div<<4)|(0<<0);
		break;
	case CORE_CLKOUT_MAIN_OSC:
		LPC_SC->CLKOUTCFG = (1<<8)|(div<<4)|(1<<0);
		break;
	case CORE_CLKOUT_INTERNAL_OSC:
		LPC_SC->CLKOUTCFG = (1<<8)|(div<<4)|(2<<0);
		break;
	case CORE_CLKOUT_USB:
		LPC_SC->CLKOUTCFG = (1<<8)|(div<<4)|(3<<0);
		break;
	case CORE_CLKOUT_RTC:
		LPC_SC->CLKOUTCFG = (1<<8)|(div<<4)|(4<<0);
		break;
	default:
		return -1;
	}
#endif

	_mcu_core_set_pinsel_func(1, 27, CORE_PERIPH_CORE, 0);
	return 0;

}


int _mcu_core_setirqprio(int irq, int prio){

	prio = DEV_MIDDLE_PRIORITY - prio;
	if( prio < 1 ){
		prio = 1;
	}

	if( prio > (DEV_MIDDLE_PRIORITY*2-1)){
		prio = DEV_MIDDLE_PRIORITY*2-1;
	}

	NVIC_SetPriority( (IRQn_Type)irq, prio );

	return 0;
}
