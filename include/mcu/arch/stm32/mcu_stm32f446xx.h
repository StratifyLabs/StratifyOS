/*
 * mcu_stm32f446xx.h
 *
 *  Created on: Jun 29, 2017
 *      Author: tgil
 */

#ifndef MCU_ARCH_STM32_MCU_STM32F446XX_H_
#define MCU_ARCH_STM32_MCU_STM32F446XX_H_


#include <string.h>
#include "mcu/types.h"


#include "cmsis/stm32f4xx.h"
#include "cmsis/stm32f446xx.h"


#define MCU_NO_HARD_FAULT 1

#define MCU_ENET_PORTS 1
#define MCU_FLASH_PORTS 1
#define MCU_MEM_PORTS 1

#define MCU_CORE_PORTS 1
#define MCU_EEPROM_PORTS 0
#define MCU_SPI_API 0
#define MCU_SPI_PORTS 1
#define MCU_SPI_REGS { LPC_SPI }
#define MCU_SPI_IRQS { SPI_IRQn }


#define MCU_TMR_PORTS 14
#define MCU_TMR_REGS { TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8, TIM9, TIM10, TIM11, TIM12, TIM13, TIM14 }
#define MCU_TMR_IRQS { -1, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_DAC_IRQn, TIM7_IRQn, -1, TIM1_BRK_TIM9_IRQn, TIM1_UP_TIM10_IRQn, TIM1_TRG_COM_TIM11_IRQn, \
	TIM8_BRK_TIM12_IRQn, TIM8_UP_TIM13_IRQn, TIM8_TRG_COM_TIM14_IRQn }

#define MCU_PIO_PORTS 8
#define MCU_PIO_REGS { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH }
#define MCU_PIO_IRQS { 0 }

#define DEV_LAST_IRQ FMPI2C1_ER_IRQn
#define DEV_MIDDLE_PRIORITY 16


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif /* MCU_ARCH_STM32_MCU_STM32F446XX_H_ */
