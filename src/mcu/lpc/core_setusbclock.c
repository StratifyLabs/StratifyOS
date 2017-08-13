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
#include "mcu/mcu.h"
#include "core_local.h"


int mcu_core_setusbclock(int fosc /*! The oscillator frequency (between 10 and 25MHz) */){
#ifdef __lpc17xx
	uint16_t m_mult;
	uint32_t fcco;
	//see if PLL0 can be used instead of PLL1
	if ( LPC_SC->PLL0CON & (1<<PLLC0) ){
		m_mult = LPC_SC->PLL0STAT + 1; //Assume N = 1
		fcco = fosc * (uint32_t)m_mult * 2;
		switch(fcco){
		case 288000000UL:
			LPC_SC->USBCLKCFG = 5;
			return 0;
		case 384000000UL:
			LPC_SC->USBCLKCFG = 7;
			return 0;
		case 480000000UL:
			LPC_SC->USBCLKCFG = 9;
			return 0;
		}

	}

	//Configure the USB PLL system to produce 48MHz
	switch(fosc){
	case 24000000UL:
		m_mult = 2;
		break;
	case 16000000UL:
		m_mult = 3;
		break;
	case 12000000UL:
		m_mult = 4;
		break;
	default:
		//USB is not supported
		return -1;
	}

	//If connected to PLL1, disconnect with a feed sequence
	if ( MCU_TEST_BIT(LPC_SC->PLL1STAT, PLLC1_STAT)){
		MCU_CLR_BIT(LPC_SC->PLL1CON, PLLC1);
		//PLL1 is connected
		LPC_SC->PLL1FEED = 0xAA;
		LPC_SC->PLL1FEED = 0x55;
	}

	//disable PLL1
	MCU_CLR_BIT(LPC_SC->PLL1CON, PLLE1);
	LPC_SC->PLL1FEED = 0xAA;
	LPC_SC->PLL1FEED = 0x55;

	//If PLL1 is enabled, it is automatically used for the USB clock source
	//Update PLL1CFG and execute feed
	LPC_SC->PLL1CFG = (m_mult-1)|(1<<5);
	LPC_SC->PLL1FEED = 0xAA;
	LPC_SC->PLL1FEED = 0x55;

	//Enable PLL1
	MCU_SET_BIT(LPC_SC->PLL1CON, PLLE1);
	LPC_SC->PLL1FEED = 0xAA;
	LPC_SC->PLL1FEED = 0x55;

	//Wait for the PLL to lock
	while( !MCU_TEST_BIT(LPC_SC->PLL1STAT, PLOCK1) );

	//Connect the PLL
	MCU_SET_BIT(LPC_SC->PLL1CON, PLLC1);
	LPC_SC->PLL1FEED = 0xAA;
	LPC_SC->PLL1FEED = 0x55;
#endif

#ifdef LPCXX7X_8X
	uint8_t p;
	uint8_t m;

	//Configure the USB PLL system to produce 48MHz
	switch(fosc){
	case 24000000UL:
		m = 1; //msel is m - 1 (24 * (1+1) = 48)
		p = 2;
		break;
	case 16000000UL:
		m = 2;
		p = 3;
		break;
	case 12000000UL:
		m = 3;
		p = 3;
		break;
	default:
		//USB is not supported
		return -1;
	}

	//If PLL1 is on, turn it off
	if ( MCU_TEST_BIT(LPC_SC->PLL1STAT, 8)){
		LPC_SC->PLL1CON = 0;
		//PLL0 is connected
		LPC_SC->PLL1FEED = 0xAA;
		LPC_SC->PLL1FEED = 0x55;
	}


	//If PLL1 is enabled, it is automatically used for the USB clock source
	//Update PLL1CFG and execute feed
	LPC_SC->PLL1CFG = m | (p<<5);

	//Enable PLL1
	LPC_SC->PLL1CON = 1;
	LPC_SC->PLL1FEED = 0xAA;
	LPC_SC->PLL1FEED = 0x55;

	//Wait for the PLL to lock
	while( !MCU_TEST_BIT(LPC_SC->PLL1STAT, 10) );

	LPC_SC->USBCLKSEL = (2<<8) | 1; //Use PLL1 with no clock divider
#endif

	return 0;

}


/*! @} */
