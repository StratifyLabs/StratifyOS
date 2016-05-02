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


/*! \addtogroup LPC13XXDEV LPC13xx Devices
 * @{
 *
 * \ingroup ARCHITECTURES
 *
 * \details This contains the device specific definitions for the LPC13xx microcontroller series.
 *
 * The following devices are supported:
 * - lpc1342
 * - lpc1343
 *
 *
 * GPIO Configurations used with pin_assign member of the *_attr_t data structures:
 * - TBD
 */

/*! \file
 * \brief LPC13xx Header File
 *
 */

#ifndef LPC13UXX_H_
#define LPC13UXX_H_

#include "mcu/types.h"
#include <string.h>
#include <stdint.h>


/*! \brief The maximum ADC value on the LPC13xx */
#define ADC_MAX 0x0000FFF0

/*! \brief The minimum ADC step value on the LPC13xx */
#define ADC_MIN 0x01

/*! \brief The maximum ADC sampling frequency */
#define ADC_MAX_FREQ 400000
#define ADC_SAMPLE_T

/*! \brief ADC Sample type (when reading ADC) */
typedef uint16_t adc_sample_t;

#define GPIO_SAMPLE_T
/*! \brief This defines the type of a GPIO sample for
 * reading/writing the GPIO (see gpio_read() and gpio_write()).
 *
 */
typedef uint32_t gpio_sample_t;


#define PWM_DUTY_T
/*! \brief This defines the type for the PWM duty cycle for reading/writing the PWM */
typedef uint32_t pwm_duty_t;


/*! \brief The minimum flash write size */
#define FLASH_MIN_WRITE_SIZE 256

/*! \brief The maximum flash write size */
#define FLASH_MAX_WRITE_SIZE 1024


/*! \details This lists the timer action channels.
 *
 */
typedef enum {
	TMR_ACTION_CHANNEL_OC0 /*! Output compare channel 0 */,
	TMR_ACTION_CHANNEL_OC1 /*! Output compare channel 1 */,
	TMR_ACTION_CHANNEL_OC2 /*! Output compare channel 2 */,
	TMR_ACTION_CHANNEL_OC3 /*! Output compare channel 3 */,
	TMR_ACTION_CHANNEL_IC0 /*! Input capture channel 0 */,
} lpc13xx_tmr_action_channel_t;

typedef lpc13xx_tmr_action_channel_t tmr_action_channel_t;


#define MCU_PERIPH_MEMORY_BASE
#define MCU_PERIPH_MEMORY_SIZE

int _mcu_lpc_flash_write_page(int page, void * addr, const void * src, int size);
int _mcu_lpc_flash_erase_page(int page);
int _mcu_lpc_flash_get_serialno(uint32_t * dest);

#define IAP_ADDRESS 0x1FFF1FF1

#include "mcu/arch/LPC13Uxx_cm3.h"
#include "mcu/arch/delay_cm3.h"

#define MPU_ACCESS_T
#define MPU_SIZE_T

#include "mcu/arch/lpc13xx_flags.h"
#include "mcu/arch/common_cm3.h"

#define DEV_LAST_IRQ Reserved5_IRQn
#define DEV_MIDDLE_PRIORITY 4

#define SYSAHBCLKCTRL_SYS (1<<0)
#define SYSAHBCLKCTRL_ROM (1<<1)
#define SYSAHBCLKCTRL_RAM (1<<2)
#define SYSAHBCLKCTRL_FLASH1 (1<<3)
#define SYSAHBCLKCTRL_FLASH2 (1<<4)
#define SYSAHBCLKCTRL_I2C (1<<5)
#define SYSAHBCLKCTRL_GPIO (1<<6)
#define SYSAHBCLKCTRL_CT16B0 (1<<7)
#define SYSAHBCLKCTRL_CT16B1 (1<<8)
#define SYSAHBCLKCTRL_CT32B0 (1<<9)
#define SYSAHBCLKCTRL_CT32B1 (1<<10)
#define SYSAHBCLKCTRL_SSP (1<<11)
#define SYSAHBCLKCTRL_UART (1<<12)
#define SYSAHBCLKCTRL_ADC (1<<13)
#define SYSAHBCLKCTRL_USB_REG (1<<14)
#define SYSAHBCLKCTRL_WDT (1<<15)
#define SYSAHBCLKCTRL_IOCON (1<<16)

#include <string.h>

#define WDMOD_WDEN 0x01
#define WDMOD_WDRESET (1<<1)
#define WDMOD_WDTOF (1<<2)
#define WDMOD_WDINT (1<<3)

enum {
	CLKOUT_IRC_OSC,
	CLKOUT_SYS_OSC,
	CLKOUT_WDT_OSC,
	CLKOUT_MAIN
};

#include "lpc13xx_flags.h"

#define MCU_CORE_PORTS 1
#define MCU_SPI_PORTS 0
#define MCU_SSP_PORTS 2
#define MCU_SSP_REGS { LPC_SSP0, LPC_SSP1 }
#define MCU_PIO_PORTS 2
#define MCU_PIO_REGS { LPC_GPIO0, LPC_GPIO1 }
#define MCU_I2C_PORTS 1
#define MCU_I2C_REGS { LPC_I2C }
#define MCU_UART_PORTS 1
#define MCU_UART_REGS { LPC_USART }
#define MCU_TMR_PORTS 4
#define MCU_TMR_REGS { (LPC_TMR_TypeDef*)LPC_CT16B0, (LPC_TMR_TypeDef*)LPC_CT16B1, (LPC_TMR_TypeDef*)LPC_CT32B0, (LPC_TMR_TypeDef*)LPC_CT32B1 }
#define MCU_EINT_PORTS 8

#define MCU_FLASH_PORTS 1
#define MCU_MEM_PORTS 1
#define MCU_ADC_PORTS 1
#define MCU_ADC_REGS { LPC_ADC }
#define MCU_DAC_PORTS 0
#define MCU_QEI_PORTS 0
#define MCU_RTC_PORTS 0
#define MCU_USB_PORTS 1
#define MCU_USB_REGS { LPC_USB }
#define MCU_PWM_PORTS 0

#define MCU_EEPROM_PORTS 0
#define MCU_EMC_PORTS 0

#define MCU_ADC_CHANNEL0_PORT 0
#define MCU_ADC_CHANNEL0_PIN 23
#define MCU_ADC_CHANNEL1_PORT 0
#define MCU_ADC_CHANNEL1_PIN 24
#define MCU_ADC_CHANNEL2_PORT 0
#define MCU_ADC_CHANNEL2_PIN 25
#define MCU_ADC_CHANNEL3_PORT 0
#define MCU_ADC_CHANNEL3_PIN 26
#define MCU_ADC_CHANNEL4_PORT 1
#define MCU_ADC_CHANNEL4_PIN 30
#define MCU_ADC_CHANNEL5_PORT 1
#define MCU_ADC_CHANNEL5_PIN 31
#define MCU_ADC_CHANNEL6_PORT 0
#define MCU_ADC_CHANNEL6_PIN 12
#define MCU_ADC_CHANNEL7_PORT 0
#define MCU_ADC_CHANNEL7_PIN 13


#define MCU_UART_PORT0_PINASSIGN0 0
#define MCU_UART_TXPIN0_PINASSIGN0 18
#define MCU_UART_RXPIN0_PINASSIGN0 19

#define MCU_I2C_PORT0_PINASSIGN0 0
#define MCU_I2C_SCLPIN0_PINASSIGN0 4
#define MCU_I2C_SDAPIN0_PINASSIGN0 5

#define MCU_SPI_PORT0_PINASSIGN0 0
#define MCU_SPI_MOSIPIN0_PINASSIGN0 9
#define MCU_SPI_MISOPIN0_PINASSIGN0 8
#define MCU_SPI_SCKPIN0_PINASSIGN0 10
#define MCU_SPI_SSELPIN0_PINASSIGN0 2

#define MCU_SPI_PORT0_PINASSIGN1 0
#define MCU_SPI_MOSIPIN0_PINASSIGN1 9
#define MCU_SPI_MISOPIN0_PINASSIGN1 8
#define MCU_SPI_SCKPIN0_PINASSIGN1 6
#define MCU_SPI_SSELPIN0_PINASSIGN1 2

#define UART0_IRQn USART_IRQn
#define I2C0_IRQn I2C_IRQn
#define TIMER_16_0_IRQn CT16B0_IRQn

typedef LPC_I2C_Type LPC_I2C_TypeDef;
typedef LPC_USART_Type LPC_UART_TypeDef;
typedef LPC_SSP0_Type LPC_SSP_TypeDef;
typedef LPC_ADC_Type LPC_ADC_TypeDef;
typedef LPC_USB_Type LPC_USB_TypeDef;
typedef LPC_CT16B0_Type LPC_CT16B0_TypeDef;
typedef LPC_CT16B1_Type LPC_CT16B1_TypeDef;
typedef LPC_CT32B0_Type LPC_CT32B0_TypeDef;
typedef LPC_CT32B1_Type LPC_CT32B1_TypeDef;
typedef LPC_IOCON_Type LPC_IOCON_TypeDef;
typedef LPC_GPIO_Type LPC_GPIO_TypeDef;
typedef LPC_CT32B1_Type LPC_TMR_TypeDef;
typedef LPC_USB_Type LPC_USB_TypeDef;


#endif /* LPC13UXX_H_ */

/*! @} */
