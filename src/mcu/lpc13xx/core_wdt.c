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

#include "mcu/wdt.h"
#include "mcu/core.h"
#include "core_wdt_flags.h"

int mcu_wdt_init(int mode, int interval){
	int clk_src;

	if ( mode & WDT_MODE_INTERRUPT ){
		//! \todo Enable the interrupt -- make it the highest priority
		return -1; //Don't support interrupt -- if WDT resets, it is because interrupts are disabled for too long
	}

	//Set the clock source
	clk_src = mode & WDT_MODE_CLK_SRC_MASK;
	LPC_SYSCON->WDTCLKDIV = 1; //divide the clock by 1
	LPC_SYSCON->WDTCLKSEL = 0;
	switch(clk_src){
	case WDT_MODE_CLK_SRC_INTERNAL_RC:
		LPC_SYSCON->WDTCLKSEL = 0;
		break;
	case WDT_MODE_CLK_SRC_MAIN:
		LPC_SYSCON->WDTCLKSEL = 1;
		break;
	case WDT_MODE_CLK_SRC_WDT_OSC:
		LPC_SYSCON->WDTCLKSEL = 2;
		break;
	case WDT_MODE_CLK_SRC_RTC:
		return -1;
	default:
		return -1;
	}

	//Update the clock source
	LPC_SYSCON->WDTCLKUEN |= (1<<0);

	LPC_WDT->MOD = 0;

	//Set Reset mode
	if ( mode & WDT_MODE_RESET ){
		LPC_WDT->MOD |= (WDRESET);
	}

	//Set the interval
	mcu_wdt_setinterval(interval);

	//Start the watchdog timer
	LPC_WDT->MOD |= WDEN;

	return 0;
}

int mcu_wdt_setinterval(int interval){
	uint32_t counts;
	int clk_src;

	clk_src = LPC_SYSCON->WDTCLKSEL & 0x03;
	//Set the clock source
	counts = 0xFFFFFFFF;
	switch(clk_src){
	case 0:
		//IRC oscillator
		counts = (12000 * interval);
		break;
	case 1:
		//main oscillator
		counts = ( (_mcu_core_getclock() / 1000) * interval );
		break;
	case 2:
		//WDT oscillator -- this clock resets to 1.6MHz divided by 2 = 0.8MHz
		counts = 800 * interval;
		break;
	}

	//Set the interval
	LPC_WDT->TC = (uint32_t)counts;
	mcu_wdt_reset();

	return 0;
}

void mcu_wdt_reset(){
	LPC_WDT->FEED = 0xAA;
	LPC_WDT->FEED = 0x55;
}
