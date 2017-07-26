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

#include "mcu/arch.h"
#include "mcu/mcu.h"
#include "core_flags.h"

//int mcu_board_config.core_cpu_freq MCU_SYS_MEM;



#define PCLKSEL0_ALL_1 0x55515155
#define PCLKSEL1_ALL_1 0x54555455
#define PCLKSEL0_ALL_2 0xAAA2A2AA
#define PCLKSEL1_ALL_2 0xA8AAA8AA
#define PCLKSEL0_ALL_4 0x00000000
#define PCLKSEL1_ALL_4 0x00000000
#define PCLKSEL0_ALL_8 0xFFF3F3FF
#define PCLKSEL1_ALL_8 0xFCFFFCFF


#ifdef LPCXX7X_8X
static void mcu_core_initclock_dev(int fclk, int fosc, uint8_t clk_src, int pdiv){
	uint16_t clk_div;
	uint8_t msel;
	uint8_t psel;

	LPC_SC->CLKSRCSEL = 0;
	LPC_SC->CCLKSEL = 1; //Use sysclk with no divider
	LPC_SC->PCLKSEL = pdiv;

	switch(clk_src){
	case MAIN_OSC:
		if ( fosc > 18000000 ){  //If fosc is > 18M set to high speed oscillator mode
			MCU_SET_BIT(LPC_SC->SCS, OSCRANGE);
		}
		MCU_SET_BIT(LPC_SC->SCS, OSCEN); //enable the main oscillator
		while ( !MCU_TEST_BIT(LPC_SC->SCS, OSCSTAT ));
		LPC_SC->CLKSRCSEL = MAIN_OSC;
		break;
	}

	if( fclk <= fosc ){
		clk_div = (fosc + fclk/2) / (fclk);
		LPC_SC->CCLKSEL = (clk_div);  //use main clock -- not PLL
		fclk = fosc / clk_div;
		return;
	}

	//fclk = fosc * M -- calculate M
	msel = (fclk + (fosc>>1)) / fosc;
	if ( msel > 32 ) {
		msel = 32;
	}

	fclk = msel * fosc;

	msel--; //msel is m minus 1

	//fcco = fclk * 2 * p (fcc must be 156MHz to 320MHz)

	//P is 1, 2, 4, 8 and psel is 0, 1, 2, 3
	psel = 0;
	if( fclk < 80000000 ){ //fcco must be at least 156MHz -- 80MHz * 2 = 160MHz below 80 fcco will be at least 80
		psel = 1;
	}

	if( fclk < 40000000 ){
		psel = 2;
	}

	if( fclk < 20000000 ){
		psel = 3;
	}

	if( fclk <= 100000000 ){
		//disable PBOOST
		LPC_SC->PBOOST = 0;
	}

	LPC_SC->PLL0CFG = (msel) | (psel<<5);

	//Enable PLL0
	MCU_SET_BIT(LPC_SC->PLL0CON, PLLE0);
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;

	//Wait for PLL to lock
	while( !MCU_TEST_BIT(LPC_SC->PLL0STAT, 10) );

	//Update clock divider
	LPC_SC->CCLKSEL = (1) | (1<<8);

}
#endif


#ifdef __lpc17xx
static int mcu_core_initclock_dev(int fclk, int fosc, uint8_t clk_src, int pdiv){
	uint16_t m_mult;
	uint16_t clk_div;
	uint32_t fcco;
	uint32_t sel0, sel1;

	switch(pdiv){
	case 1:
		sel0 = PCLKSEL0_ALL_1;
		sel1 = PCLKSEL1_ALL_1;
		break;
	case 2:
		sel0 = PCLKSEL0_ALL_2;
		sel1 = PCLKSEL1_ALL_2;
		break;
	case 4:
		sel0 = PCLKSEL0_ALL_4;
		sel1 = PCLKSEL1_ALL_4;
		break;
	case 8:
		sel0= PCLKSEL0_ALL_8;
		sel1 = PCLKSEL1_ALL_8;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	//If connected to PLL0, disconnect with a feed sequence
	if ( MCU_TEST_BIT(LPC_SC->PLL0STAT, PLLC0_STAT)){
		MCU_CLR_BIT(LPC_SC->PLL0CON, PLLC0);
		//PLL0 is connected
		LPC_SC->PLL0FEED = 0xAA;
		LPC_SC->PLL0FEED = 0x55;
	}

	//disable PLL0
	MCU_CLR_BIT(LPC_SC->PLL0CON, PLLE0);
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;

	//-- Errata won't let PCLK be changed after PLL is running
#ifdef __lpc17xx
	LPC_SC->PCLKSEL0 = sel0;
	LPC_SC->PCLKSEL1 = sel1;
#endif

#ifdef LPCXX7X_8X
	LPC_SC->PCLKSEL = pdiv;
#endif

	switch(clk_src){
	case MAIN_OSC:
		MCU_SET_BIT(LPC_SC->SCS, OSCEN);
		while ( !MCU_TEST_BIT(LPC_SC->SCS, OSCSTAT ));
		if ( MCU_TEST_BIT(LPC_SC->SCS, OSCSTAT) ){
			//Enable the main oscillator
			MCU_SET_BIT(LPC_SC->SCS, OSCEN);
			if ( fosc > 18000000 ){  //If mcu_board_config.core_cpu_freq is > 18M set to high speed oscillator mode
				MCU_SET_BIT(LPC_SC->SCS, OSCRANGE);
			}
		}
		break;
	}


	if ( fclk > 96000000UL ){
		fcco = 480000000UL;
	} else {
		fcco = 288000000UL;
	}


	LPC_SC->CLKSRCSEL = clk_src;

	if ( fclk > fosc ){
		//FCCO must be between 275 MHZ and 550MHz
		//Calcute the divisors and multiplier for the PLL
		m_mult = (fcco + fosc)/(fosc*2);
		m_mult = ( m_mult > 512 ) ? 512 : m_mult;
		m_mult =  ( m_mult < 6 ) ? 6 : m_mult;

		clk_div = (fcco + fclk/2) / (fclk);
		clk_div = ( clk_div < 3 ) ? 3 : clk_div;
		clk_div = ( clk_div > 256 ) ? 256 : clk_div;
		//mcu_board_config.core_cpu_freq = fosc * 2 * m_mult / clk_div;
	} else if ( fclk <= fosc ){
		clk_div = (fosc + fclk/2) / (fclk);
#ifdef __lpc17xx
		LPC_SC->CCLKCFG = (clk_div - 1);
#endif

#ifdef LPCXX7X_8X
		LPC_SC->CCLKSEL = clk_div; //fclk < fosc so don't use the PLL
#endif

		//mcu_board_config.core_cpu_freq = fosc / clk_div;
		return 0;
	}


	//Update PLL0CFG and execute feed
	LPC_SC->PLL0CFG = (m_mult-1);  //Assume N = 1
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;

	//Enable PLL0
	MCU_SET_BIT(LPC_SC->PLL0CON, PLLE0);
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;

	//Update clock divider
#ifdef __lpc17xx
	LPC_SC->CCLKCFG = (clk_div - 1);
#endif

#ifdef LPCXX7X_8X
	LPC_SC->CCLKSEL = clk_div | (1<<8); //fclk < fosc so use the PLL
#endif

	//Wait for the PLL to lock
	while( !MCU_TEST_BIT(LPC_SC->PLL0STAT, PLOCK0) );

	//Connect the PLL
	MCU_SET_BIT(LPC_SC->PLL0CON, PLLC0);
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;

	return 0;
}
#endif


#if defined __lpc13uxx || defined __lpc13xx

void cfg_pll(int source, uint8_t msel, uint8_t psel){
	// set PLL multiplier & divisor.
	// values computed from config.h
	LPC_SYSCON->SYSPLLCLKSEL  = source;

#if defined __lpc13xx
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
	LPC_SYSCON->SYSPLLCLKUEN  = 0x00;
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01; //this sequence updates the PLL clock source
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01)); //wait for the signal to go low
#endif

	LPC_SYSCON->SYSPLLCTRL = PLLCFG_MSEL(msel) | PLLCFG_PSEL(psel/2);
	LPC_SYSCON->PDRUNCFG &= ~(1 << 7);          // Power-up SYSPLL
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));	// Wait Until PLL Locked

	LPC_SYSCON->MAINCLKSEL = 3; //use PLL output
#if defined __lpc13xx
	LPC_SYSCON->MAINCLKUEN = 0x01;  // Update MCLK Clock Source
	LPC_SYSCON->MAINCLKUEN = 0x00;  // Toggle Update Register
	LPC_SYSCON->MAINCLKUEN = 0x01;
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));       // Wait Until Updated
#endif
}

#endif

#ifdef __lpc13uxx
static int mcu_core_initclock_dev(int fclk, int fosc, uint8_t clk_src, int pdiv){
	int i;
	uint8_t psel;
	uint8_t msel;

	if( clk_src == MAIN_OSC ){
		LPC_SYSCON->PDRUNCFG     &= ~(1 << 5);          /* Power-up System Osc      */
		if( fosc > 18000000 ){
			LPC_SYSCON->SYSOSCCTRL = 1;
		} else {
			LPC_SYSCON->SYSOSCCTRL = 0;
		}
		for (i = 0; i < 200; i++){
			__NOP();
		}
	}

	if( (fclk <= 12000000) && (clk_src = IRC_OSC) ){
		LPC_SYSCON->MAINCLKSEL = IRC_OSC; //use IRC oscillator
		LPC_SYSCON->SYSAHBCLKDIV  = 12000000 / fclk;
	} else if( (fclk <= fosc) && (clk_src == MAIN_OSC) ){
		LPC_SYSCON->MAINCLKSEL = MAIN_OSC; //use PLL input (MAIN_OSC)
		LPC_SYSCON->SYSAHBCLKDIV  = fosc / fclk;
	} else if( fclk > fosc ){

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
		cfg_pll(clk_src, msel, psel);

		LPC_SYSCON->MAINCLKSEL = 3; //use PLL output
	}

	return 0;
}

#endif




//requires mcu_core_osc_freq, mcu_board_config.core_cpu_freq, and mcu_board_config.core_periph_freq to be defined ext
int mcu_core_initclock(int div){
	uint8_t clk_src = 0;
	uint32_t fosc = mcu_board_config.core_osc_freq;
	int pdiv = mcu_board_config.core_cpu_freq / mcu_board_config.core_periph_freq;
	uint32_t fclk = mcu_board_config.core_cpu_freq / div;

	//validate div
	switch(div){
	case 1:
	case 2:
#ifdef LPCXX7X_8X
	case 3:
#endif
	case 4:
#ifdef __lpc17xx
	case 8:
#endif
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	//validate pdiv
	switch(pdiv){
	case 1:
	case 2:
#ifdef LPCXX7X_8X
	case 3:
#endif
	case 4:
#ifdef __lpc17xx
	case 8:
#endif
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	if( fosc == 0 ){
		fosc = 4000000UL;
		clk_src = IRC_OSC;
	} else {
		clk_src = MAIN_OSC;
	}

	mcu_core_initclock_dev(fclk, fosc, clk_src, pdiv);


	return 0;
}

/*! @} */
