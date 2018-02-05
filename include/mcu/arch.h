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

#include <stdlib.h>
#include "types.h"

#if (defined __lpc17xx) || (defined __lpc177x_8x) || (defined __lpc407x_8x) || (defined __lpc43xx)
#undef __FPU_USED
#include "arch/lpc/lpc_arch.h"
#define ARCH_DEFINED
#elif (defined __stm32f446xx) || (defined __stm32f401xc)
#undef __FPU_USED
#include "arch/stm32/stm32_arch.h"
#define ARCH_DEFINED
#elif defined __armv7em
#define __CHECK_DEVICE_DEFINES
#define __FPU_PRESENT 1
#define ARM_MATH_CM4 1
#undef __FPU_USED
#define ARCH_DEFINED
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
#include "arch/cmsis/core_cm4.h"
#elif defined __armv7m
#define __CHECK_DEVICE_DEFINES
#define __FPU_PRESENT 0
#define ARM_MATH_CM3 1
#undef __FPU_USED
#define ARCH_DEFINED
typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
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
#include "arch/cmsis/core_cm3.h"
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
