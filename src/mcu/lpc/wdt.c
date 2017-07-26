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

#include "mcu/arch.h"
#include "mcu/wdt.h"
#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "cortexm/fault.h"


#define WDEN (1<<0)
#define WDRESET (1<<1)
#define WDTOF (1<<2)
#define WDINT (1<<3)

#define WDLOCK (1<<31)

int mcu_wdt_init(int mode, int interval){

	//Set the clock source
#ifdef __lpc17xx
	int clk_src;
	clk_src = mode & WDT_MODE_CLK_SRC_MASK;
	LPC_WDT->CLKSEL = 0;
	switch(clk_src){
	case WDT_MODE_CLK_SRC_INTERNAL_RC:
		LPC_WDT->CLKSEL = 0;
		break;
	case WDT_MODE_CLK_SRC_MAIN:
		LPC_WDT->CLKSEL = 1;
		break;
	case WDT_MODE_CLK_SRC_RTC:
		LPC_WDT->CLKSEL = 2;
		break;
	default:
		return -1;
	}

	//Lock the clock source
	LPC_WDT->CLKSEL |= WDLOCK;
#endif

	LPC_WDT->MOD = 0;

	//Set Reset mode
	if ( mode & WDT_MODE_RESET ){ LPC_WDT->MOD |= (WDRESET); }

	if ( mode & WDT_MODE_INTERRUPT ){
#if defined LPCXX7X_8X
		LPC_WDT->WARNINT = 500;
		LPC_WDT->MOD |= WDINT;
#endif

		NVIC_SetPriority(WDT_IRQn, 2);
		cortexm_enable_irq((void*)WDT_IRQn);
	}

	//Set the interval
	mcu_wdt_setinterval(interval);

	//Start the watchdog timer
	LPC_WDT->MOD |= WDEN;


	//feed sequence is required to start the timer
	LPC_WDT->FEED = 0xAA;
	LPC_WDT->FEED = 0x55;

	return 0;
}

int mcu_wdt_setinterval(int interval){
	uint32_t counts;

#ifdef LPCXX7X_8X
	//WDT oscillator is 500KHz with a 4 count pre-divider
	counts = interval*500/4;
#else
	int clk_src;

#if LPC_WDT_API == 1
	clk_src = 0;
#else
	clk_src = LPC_WDT->CLKSEL & 0x03;
#endif
	//Set the clock source
	counts = 0xFFFFFFFF;
	switch(clk_src){
	case 0:
		counts = (1000 * interval);
		break;
	case 1:
		counts = ( (mcu_core_getclock() / 1000) * interval + 2 ) / 4;
		break;
	case 2:
		counts = ( 8 * interval );
		break;
	}
#endif

	//Set the interval
	LPC_WDT->TC = (uint32_t)counts;

	mcu_wdt_reset();

	return 0;
}

void mcu_wdt_priv_reset(void * args){
	LPC_WDT->FEED = 0xAA;
	LPC_WDT->FEED = 0x55;
}

void mcu_wdt_reset(){
	LPC_WDT->FEED = 0xAA;
	LPC_WDT->FEED = 0x55;
}


//ISR handler is in the cortex_m/fault.c source file
