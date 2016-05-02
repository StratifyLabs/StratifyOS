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

#include "mcu/mcu.h"

#include "core_flags.h"

int mcu_board_config.core_cpu_freq;

static void _mcu_update_clock_source(int source){
	LPC_SYSCON->MAINCLKSEL = source;
#if defined __lpc13xx
	LPC_SYSCON->MAINCLKUEN = 0x01;  // Update MCLK Clock Source
	LPC_SYSCON->MAINCLKUEN = 0x00;  // Toggle Update Register
	LPC_SYSCON->MAINCLKUEN = 0x01;
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));       // Wait Until Updated
#endif
}

static void _mcu_update_pllclock_source(int source){
	LPC_SYSCON->SYSPLLCLKSEL  = source;
#if defined __lpc13xx
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
	LPC_SYSCON->SYSPLLCLKUEN  = 0x00;
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01; //this sequence updates the PLL clock source
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01)); //wait for the signal to go low
#endif
#if defined __lpc13uxx
	  while (!(LPC_SYSCON->SYSPLLSTAT & 0x01)){
		  ; //wait for PLL lock
	  }
#endif

}

static void cfg_pll(uint8_t msel, uint8_t psel);
static void cfg_pll_irc(uint8_t msel, uint8_t psel);

void _mcu_core_setclock(int fclk /*! The new clock frequency */,
		int fosc /*! The oscillator frequency (between 10 and 25MHz) */){
	uint8_t psel;
	uint8_t msel;
	int div;
	//use approx 260MHz for Fcco


	LPC_SYSCON->PDRUNCFG &= ~(1 << 5);          // Power-up System Osc

	if( fosc > 18000000 ){
		LPC_SYSCON->SYSOSCCTRL = 1<<1; //use a high frequency oscillator
	}

	if( fclk == 0 ){
		fclk = fosc;
	}

	if( fclk <= fosc ){
		//don't use the PLL
		div = fosc / fclk;
		div &= 0xFF; //255 is the max value
		LPC_SYSCON->SYSAHBCLKDIV  = div;
		mcu_board_config.core_cpu_freq = fosc / div;
		_mcu_update_pllclock_source(1); //use system clock for PLL input
		_mcu_update_clock_source(1); //use the PLL input as the main clock
		return;
	}


	msel = (fclk + (fosc>>1)) / fosc;
	if ( msel > 32 ) {
		msel = 32;
	}
	psel = (260000000UL + fclk) / (fclk<<1);
	if ( psel > 8 ){
		psel = 8;
	}
	if ( psel < 2 ){
		psel = 2;
	}
	psel >>= 1;
	cfg_pll(msel, psel);
	mcu_board_config.core_cpu_freq = (uint32_t)fclk;

	//power down the IRC
	//LPC_SYSCON->PDRUNCFG |= (1 << 1);  //Power down the IRC

}

void cfg_pll(uint8_t msel, uint8_t psel){
	// set PLL multiplier & divisor.
	// values computed from config.h
	_mcu_update_pllclock_source(1); //use main for PLL input

	LPC_SYSCON->SYSPLLCTRL = PLLCFG_MSEL(msel) | PLLCFG_PSEL(psel/2);
	LPC_SYSCON->PDRUNCFG &= ~(1 << 7);          // Power-up SYSPLL
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));	// Wait Until PLL Locked

	_mcu_update_clock_source(3); //use the PLL output
}


void _mcu_core_setclockinternal(uint32_t fclk /*! The new clock frequency */){
	uint8_t psel;
	uint8_t msel;
	uint32_t div;
	//use approx 260MHz for Fcco

	if( fclk == 0 ){
		mcu_board_config.core_cpu_freq = 12000000;
		return;
	}

	if( fclk <= 12000000 ){
		div = 12000000 / fclk;
		div &= 0xFF; //255 is the max value
		LPC_SYSCON->SYSAHBCLKDIV  = div;
		mcu_board_config.core_cpu_freq = 12000000 / div;
		return;
	}

	//calculate PLL values
	msel = (fclk + (12000000UL>>1)) / 12000000UL;
	if ( msel > 32 ) {
		msel = 32;
	}
	psel = (260000000UL + fclk) / (fclk<<1);
	if ( psel > 8 ){
		psel = 8;
	}
	if ( psel < 2 ){
		psel = 2;
	}
	psel >>= 1;
	cfg_pll_irc(msel, psel);
	mcu_board_config.core_cpu_freq = fclk;
}

void cfg_pll_irc(uint8_t msel, uint8_t psel){
	// set PLL multiplier & divisor.
	// values computed from config.h

	_mcu_update_pllclock_source(0); //use IRC for PLL input

	LPC_SYSCON->SYSPLLCTRL = PLLCFG_MSEL(msel) | PLLCFG_PSEL(psel/2);
	LPC_SYSCON->PDRUNCFG &= ~(1 << 7);          // Power-up SYSPLL
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));	      // Wait Until PLL Locked

	_mcu_update_clock_source(3); //use the PLL output
}


/*! @} */
