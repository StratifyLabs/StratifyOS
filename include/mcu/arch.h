/* Copyright 2011-2018 Tyler Gilbert;
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

#include <stdlib.h>
#include "types.h"

#define SCHED_USECOND_TMR_RESET_OC 0
#define SCHED_USECOND_TMR_SLEEP_OC 1
#define SCHED_USECOND_TMR_SYSTEM_TIMER_OC 2
#define SCHED_USECOND_TMR_MINIMUM_PROCESS_TIMER_INTERVAL 100

#define PTHREAD_DEFAULT_STACK_SIZE 1536
#define MALLOC_CHUNK_SIZE 64
#define MALLOC_SBRK_JUMP_SIZE 128
#define SCHED_FIRST_THREAD_STACK_SIZE 2048
#define SCHED_DEFAULT_STACKGUARD_SIZE 128



#if (defined __lpc17xx) || (defined __lpc177x_8x) || (defined __lpc407x_8x) || (defined __lpc43xx)
#undef __FPU_USED
#include "arch/lpc/lpc_arch.h"
#define ARCH_DEFINED
#elif (defined __stm32f446xx) || (defined __stm32f401xc) || (defined __stm32f411xe) \
	|| (defined __stm32f417xx) || (defined __stm32f412zx) || (defined __stm32f429xx) \
	|| (defined __stm32f746xx) || (defined __stm32f722xx) || (defined __stm32f723xx) || (defined __stm32f767xx) \
	|| (defined __stm32l475xx) || (defined __stm32l432xx) ||(defined __stm32f401xe) \
	|| (defined __stm32h743xx) || (defined __stm32h750xx) \
	|| (defined __stm32f205xx)	|| (defined __stm32f207xx)  || (defined __stm32f215xx)  || (defined __stm32f217xx)
#undef __FPU_USED
#include "arch/stm32/stm32_arch.h"
#define ARCH_DEFINED
#elif (defined __imxrt1052)
#include "arch/imxrt/imxrt_arch.h"
#define ARCH_DEFINED
#else

typedef enum IRQn
{
	/******  Cortex-M4 Processor Exceptions Numbers ***************************************************/
	NonMaskableInt_IRQn           = -14,      /*!< 2 Non Maskable Interrupt                         */
	HardFault_IRQn                = -13,              /*!<   3  Hard Fault, all classes of Fault                                 */
	MemoryManagement_IRQn         = -12,      /*!< 4 Cortex-M3 Memory Management Interrupt          */
	BusFault_IRQn                 = -11,      /*!< 5 Cortex-M3 Bus Fault Interrupt                  */
	UsageFault_IRQn               = -10,      /*!< 6 Cortex-M3 Usage Fault Interrupt                */
	SVCall_IRQn                   = -5,       /*!< 11 Cortex-M3 SV Call Interrupt                   */
	DebugMonitor_IRQn             = -4,       /*!< 12 Cortex-M3 Debug Monitor Interrupt             */
	PendSV_IRQn                   = -2,       /*!< 14 Cortex-M3 Pend SV Interrupt                   */
	SysTick_IRQn                  = -1,       /*!< 15 Cortex-M3 System Tick Interrupt               */
} IRQn_Type;

#define __MPU_PRESENT 1
#define ARCH_DEFINED 1
#define __NVIC_PRIO_BITS 8

#if defined __v7em_f5ss || defined __v7em_f5sh || defined __v7em_f5ds || defined __v7em_f5dh

#define __ICACHE_PRESENT 0
#define __DCACHE_PRESENT 0
#define __DTCM_PRESENT 0
#define __CM7_REV 1
#define __Vendor_SysTickConfig 0
#endif

#if defined __v7m
#define ARCH "v7m"

#define __FPU_PRESENT 0
#if !defined ARM_MATH_CM3
#define ARM_MATH_CM3 1
#endif
#undef __FPU_USED
#include "arch/cmsis/core_cm3.h"

#elif defined __v7em
#define ARCH "v7em"
#define __FPU_PRESENT 0
#if !defined ARM_MATH_CM4
#define ARM_MATH_CM4 1
#endif
#undef __FPU_USED
#include "arch/cmsis/core_cm4.h"

#elif defined __v7em_f4ss
#define ARCH "v7em_f4ss"

#define __FPU_PRESENT 1
#if !defined ARM_MATH_CM4
#define ARM_MATH_CM4 1
#endif
#undef __FPU_USED
#include "arch/cmsis/core_cm4.h"

#elif defined __v7em_f4sh
#define ARCH "v7em_f4sh"

#define __FPU_PRESENT 1
#if !defined ARM_MATH_CM4
#define ARM_MATH_CM4 1
#endif
#undef __FPU_USED
#include "arch/cmsis/core_cm4.h"

#elif defined __v7em_f5ss
#define ARCH "v7em_f5ss"

#define __CHECK_DEVICE_DEFINES
#define __FPU_PRESENT 1
#if !defined ARM_MATH_CM7
#define ARM_MATH_CM7 1
#endif
#undef __FPU_USED
#include "arch/cmsis/core_cm7.h"

#elif defined __v7em_f5sh
#define ARCH "v7em_f5sh"

#define __CHECK_DEVICE_DEFINES
#define __FPU_PRESENT 1
#if !defined ARM_MATH_CM7
#define ARM_MATH_CM7 1
#endif
#undef __FPU_USED
#include "arch/cmsis/core_cm7.h"

#elif defined __v7em_f5ds
#define ARCH "v7em_f5ds"

#define __CHECK_DEVICE_DEFINES
#define __FPU_PRESENT 1
#if !defined ARM_MATH_CM7
#define ARM_MATH_CM7 1
#endif
#undef __FPU_USED
#include "arch/cmsis/core_cm7.h"

#elif defined __v7em_f5dh
#define ARCH "v7em_f5dh"

#define __CHECK_DEVICE_DEFINES
#define __FPU_PRESENT 1
#define ARM_MATH_CM7 1
#undef __FPU_USED
#include "arch/cmsis/core_cm7.h"

#else
#error "No ARM Arch is defined"
#endif

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
#define ARCH_DEFINED

#else
#include <sys/syslimits.h>
typedef void * mcu_target_ptr_t;
#endif


#endif /* ARCH_H_ */
