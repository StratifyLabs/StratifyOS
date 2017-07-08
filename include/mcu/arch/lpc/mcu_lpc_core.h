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
 * @{
 *
 * \ingroup LPC17XXDEV
 *
 *
 */

/*! \file */

#ifndef _MCU_ARCH_LPC_CORE_H_
#define _MCU_ARCH_LPC_CORE_H_

//Peripheral Clocks
enum {
	PCLK_WDT,
	PCLK_TIMER0,
	PCLK_TIMER1,
	PCLK_UART0,
	PCLK_UART1,
	PCLK_RESERVED0,
	PCLK_PWM1,
	PCLK_I2C0,
	PCLK_SPI,
	PCLK_RESERVED1,
	PCLK_SSP1,
	PCLK_DAC,
	PCLK_ADC,
	PCLK_CAN1,
	PCLK_CAN2,
	PCLK_ACF,
	PCLK_QEI,
	PCLK_GPIOINT,
	PCLK_PCB,
	PCLK_I2C1,
	PCLK_RESERVED3,
	PCLK_SSP0,
	PCLK_TIMER2,
	PCLK_TIMER3,
	PCLK_UART2,
	PCLK_UART3,
	PCLK_I2C2,
	PCLK_I2S,
	PCLK_RESERVED4,
	PCLK_RIT,
	PCLK_SYSCON,
	PCLK_MC
};

#define PCLK_MASK(x,v) (v<<(x*2))


#define PCLK_4 0
#define PCLK_1 1
#define PCLK_2 2
#define PCLK_8 3


//#define
enum {
	PCLCD,
	PCTIM0,
	PCTIM1,
	PCUART0,
	PCUART1,
	PCPWM0,
	PCPWM1,
	PCI2C0,
	PCSPI,
	PCRTC,
	PCSSP1, //bit 10
	PCEMC,
	PCADC,
	PCCAN1,
	PCCAN2,
	PCGPIO,
	PCRIT,
	PCMCPWM,
	PCQEI,
	PCI2C1,
	PCSSP2, //bit 20
	PCSSP0,
	PCTIM2,
	PCTIM3,
	PCUART2,
	PCUART3,
	PCI2C2,
	PCI2S,
	PCMCI,
	PCGPDMA,
	PCENET, //bit 30
	PCUSB
};

#define PCUART4 PCSPI



static inline void _mcu_lpc_core_enable_pwr(int periph) MCU_ALWAYS_INLINE;
static inline void _mcu_lpc_core_disable_pwr(int periph) MCU_ALWAYS_INLINE;
static inline int _mcu_lpc_core_pwr_enabled(int periph) MCU_ALWAYS_INLINE;

#if !defined __lpc43xx
void _mcu_lpc_core_enable_pwr(int periph){ LPC_SC->PCONP |= (1<<periph); }
void _mcu_lpc_core_disable_pwr(int periph){ LPC_SC->PCONP &= ~(1<<periph); }
int _mcu_lpc_core_pwr_enabled(int periph){ return (LPC_SC->PCONP & (1<<periph)); }
#else
void _mcu_lpc_core_enable_pwr(int periph){  }
void _mcu_lpc_core_disable_pwr(int periph){  }
int _mcu_lpc_core_pwr_enabled(int periph){ return 0; }
#endif



#endif /* _MCU_ARCH_LPC_CORE_H_ */

/*! @} */
