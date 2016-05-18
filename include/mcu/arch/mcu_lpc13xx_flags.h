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

/*! \addtogroup LPC13XXDEV
 * @{
 *
 *
 */

/*! \file */

#ifndef LPC13XX_FLAGS_H_
#define LPC13XX_FLAGS_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PCSYS = 0,
	PCROM = 1,
	PCRAM = 2,
	PCFLASHREG = 3,
	PCFLASHARRAY = 4,
	PCI2C = 5,
	PCI2C0 = 5,
	PCGPIO = 6,
	PCCT16B0 = 7,
	PCCT16B1 = 8,
	PCCT32B0 = 9,
	PCCT32B1 = 10,
	PCSSP = 11,
	PCSSP0 = 11,
	PCUART = 12,
	PCUART0 = 12,
	PCADC = 13,
	PCUSB_REG = 14,
	PCWDT = 15,
	PCIOCON = 16,
	PCSSP1 = 18,
	PC_TOTAL
};


static inline void _mcu_lpc_core_enable_pwr(int port) MCU_ALWAYS_INLINE;
static inline void _mcu_lpc_core_enable_pwr_mask(int mask) MCU_ALWAYS_INLINE;
static inline void _mcu_lpc_core_disable_pwr(int port) MCU_ALWAYS_INLINE;
static inline void _mcu_lpc_core_disable_pwr_mask(int mask) MCU_ALWAYS_INLINE;
static inline int _mcu_lpc_core_pwr_enabled(int periph) MCU_ALWAYS_INLINE;


void _mcu_lpc_core_enable_pwr(int port){ MCU_SET_BIT(LPC_SYSCON->SYSAHBCLKCTRL, port); }
void _mcu_lpc_core_enable_pwr_mask(int mask){ MCU_SET_MASK(LPC_SYSCON->SYSAHBCLKCTRL, mask); }
void _mcu_lpc_core_disable_pwr(int port){ MCU_CLR_BIT(LPC_SYSCON->SYSAHBCLKCTRL, port); }
void _mcu_lpc_core_disable_pwr_mask(int mask){ MCU_CLR_MASK(LPC_SYSCON->SYSAHBCLKCTRL, mask); }
int _mcu_lpc_core_pwr_enabled(int periph){ return (MCU_TEST_BIT(LPC_SYSCON->SYSAHBCLKCTRL, periph)); }


#define CORE_PINMODE_MASK ( (1<<4)|(1<<3)|(1<<5) )

void core_wr_pinmode(int port, int pin, int value);

#ifdef __cplusplus
}
#endif


#endif /* LPC13XX_FLAGS_H_ */

/*! @} */
