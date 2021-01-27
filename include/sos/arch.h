// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \file
 * \brief Architecture Header File includes correct architecture.
 */

#ifndef ARCH_H_
#define ARCH_H_

#include <sdk/types.h>
#include <stdlib.h>

#define SCHED_USECOND_TMR_RESET_OC 2
#define SCHED_USECOND_TMR_SLEEP_OC 0
#define SCHED_USECOND_TMR_SYSTEM_TIMER_OC 1
#define SCHED_USECOND_TMR_MINIMUM_PROCESS_TIMER_INTERVAL 100

#define PTHREAD_DEFAULT_STACK_SIZE 1536
#define MALLOC_CHUNK_SIZE 32
#define MALLOC_SBRK_JUMP_SIZE 128
#define SCHED_FIRST_THREAD_STACK_SIZE 2048
#define SCHED_DEFAULT_STACKGUARD_SIZE 128

#if defined MCU_ARCH_IMXRT
#include <imxrt_arch.h>
#define ARCH_DEFINED
#endif

#if defined MCU_ARCH_LPC
#include <lpc_arch.h>
#define ARCH_DEFINED
#endif

#if defined MCU_ARCH_STM32
#include <stm32_arch.h>
#define ARCH_DEFINED
#endif

#if !defined ARCH_DEFINED
#define ARCH_DEFINED 1

typedef enum IRQn {
  /******  Cortex-M4 Processor Exceptions Numbers
   ***************************************************/
  NonMaskableInt_IRQn = -14,   /*!< 2 Non Maskable Interrupt                         */
  HardFault_IRQn = -13,        /*!<   3  Hard Fault, all classes of Fault        */
  MemoryManagement_IRQn = -12, /*!< 4 Cortex-M3 Memory Management Interrupt          */
  BusFault_IRQn = -11,         /*!< 5 Cortex-M3 Bus Fault Interrupt                  */
  UsageFault_IRQn = -10,       /*!< 6 Cortex-M3 Usage Fault Interrupt                */
  SVCall_IRQn = -5,            /*!< 11 Cortex-M3 SV Call Interrupt                   */
  DebugMonitor_IRQn = -4,      /*!< 12 Cortex-M3 Debug Monitor Interrupt             */
  PendSV_IRQn = -2,            /*!< 14 Cortex-M3 Pend SV Interrupt                   */
  SysTick_IRQn = -1,           /*!< 15 Cortex-M3 System Tick Interrupt               */
} IRQn_Type;

#define __MPU_PRESENT 1
#define __NVIC_PRIO_BITS 8

#if defined __v7em_f5ss || defined __v7em_f5sh || defined __v7em_f5ds                    \
  || defined __v7em_f5dh

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
#include <limits.h>
typedef void *mcu_target_ptr_t;
#endif

#endif /* ARCH_H_ */
