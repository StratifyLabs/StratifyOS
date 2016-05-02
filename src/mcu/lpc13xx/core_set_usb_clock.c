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

/*! \addtogroup CORE
 *
 * \ingroup LPC1XXX
 *
 * @{
 */


#include "mcu/mcu.h"
#include "core_flags.h"


/*! \brief sets the USB clock based on the system oscillator input.
 * \details This function sets the USB clock based on the input oscillator.
 * The input oscillator should be an integral divisor of 48Mhz such as
 * 12Mhz, 16MHz, 24Mhz, etc.
 *
 */
int _mcu_core_setusbclock(int fosc){
	uint8_t psel;
	LPC_SYSCON->PDRUNCFG &= ~(1<<10); // Power-up USB PHY
	LPC_SYSCON->PDRUNCFG &= ~(1<<8); // Power-up USB PLL
	LPC_SYSCON->USBPLLCLKSEL = 0x01; // Select System clock as PLL Input
#if defined __lpc13xx
	LPC_SYSCON->USBPLLCLKUEN = 0x01; // Update Clock Source
	LPC_SYSCON->USBPLLCLKUEN = 0x00; // Toggle Update Register
	LPC_SYSCON->USBPLLCLKUEN = 0x01;
	while (!(LPC_SYSCON->USBPLLCLKUEN & 0x01)); // Wait Until Updated
#endif

	psel = (260000000UL + 48000000UL) / (48000000UL<<1);
	LPC_SYSCON->USBPLLCTRL = PLLCFG_MSEL(48000000UL/fosc) | PLLCFG_PSEL(psel/2);
	while (!(LPC_SYSCON->USBPLLSTAT   & 0x01)); // Wait Until PLL Locked
	LPC_SYSCON->USBCLKSEL = 0x00; // Select USB PLL
#ifdef __lpc13uxx
	LPC_SYSCON->USBCLKDIV = 1;
#endif
	return 0;
}


/*! @} */
