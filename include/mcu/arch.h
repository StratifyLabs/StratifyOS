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


/*! \file
 * \brief Architecture Header File includes correct architecture.
 */

#ifndef ARCH_H_
#define ARCH_H_

#include <stdint.h>
#include <stdlib.h> //defines NULL
#include "types.h"

#define USB_CTRL_MAX_PACKET_SIZE 64


#ifdef __lpc82x
#define CORE_M0PLUS 1
#define ARM_MATH_CM0
#include "arch/mcu_lpc82x.h"
#endif

#ifdef __lpc13xx
#define ARM_MATH_CM3
#include "arch/mcu_lpc13xx.h"
#endif

#ifdef __lpc13uxx
#define ARM_MATH_CM3
#include "arch/mcu_lpc13uxx.h"
#endif

#ifdef __lpc17xx
#define ARM_MATH_CM3
#include "arch/mcu_lpc17xx.h"
#endif

#ifdef __lpc177x_8x
#define ARM_MATH_CM3
#include "arch/mcu_lpc177x_8x.h"
#endif

#ifdef __lpc407x_8x
#define CORE_M4 1
#define ARM_MATH_CM4 1
#include "arch/mcu_lpc407x_8x.h"
#endif

#ifdef __lpc43xx
#define CORE_M4 1
#define ARM_MATH_CM4 1
#include "arch/mcu_lpc43xx.h"
#endif

#ifdef __link
#ifdef __cplusplus
extern "C" {
#endif
typedef u8 tmr_action_channel_t;
typedef u32 mcu_target_ptr_t;
#define PWM_DUTY_T
typedef u32 pwm_duty_t;

#undef NAME_MAX
#define NAME_MAX 24
#ifdef __cplusplus
}
#endif

#define MCU_CORE_PORTS 32
#define MCU_SPI_PORTS 32
#define MCU_GPIO_PORTS 32
#define MCU_PIO_PORTS 32
#define MCU_I2C_PORTS 32
#define MCU_UART_PORTS 32
#define MCU_TMR_PORTS 32
#define MCU_EINT_PORTS 32
#define MCU_FLASH_PORTS 32
#define MCU_MEM_PORTS 32
#define MCU_ADC_PORTS 32
#define MCU_DAC_PORTS 32
#define MCU_QEI_PORTS 32
#define MCU_RTC_PORTS 32
#define MCU_USB_PORTS 32
#define MCU_PWM_PORTS 32

#else
#include <sys/syslimits.h>
typedef void * mcu_target_ptr_t;
#endif

#ifdef __stm32f
#include "arch/stm32f.h"
#endif

struct link_tm {
	s32	tm_sec;
	s32	tm_min;
	s32	tm_hour;
	s32	tm_mday;
	s32	tm_mon;
	s32	tm_year;
	s32	tm_wday;
	s32	tm_yday;
	s32	tm_isdst;
};

#endif /* ARCH_H_ */


