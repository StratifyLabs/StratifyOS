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

/*! \addtogroup STM32FXXDEV STM32F Devices
 * @{
 *
 * \ingroup ARCHITECTURES
 *
 * \details This contains the device specific definitions for the STM32F microcontroller series.
 *
 * The following devices are supported:
 * - TBD
 *
 *
 * GPIO configurations specify how to map peripherals to GPIO pins.  The following code
 * snippet shows how to specify pins XXX and XXX for use with I2C port 1.  After
 * the ioctl() call, the port is ready to be read/written.
 *
 * \code
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include "mcu/mcu.h"
 * int fd;
 * i2c_attr_t attr;
 * fd = open("/dev/i2c1", O_RDWR);
 * attr.pin_assign = 1; //Use pins 0.19 and 0.20 for the I2C Bus
 * ioctl(fd, I_SETATTR, &attr);
 * \endcode
 *
 * The following GPIO configurations are available on the STM32F architecture:

 *
 * 	The following is an example that shows how to use the GPIO configuration values
 * 	when opening the I2C.  The same approach applies to other peripherals as well.
 * 	\code
 * 	#include <unistd.h>
 * 	#include <fcntl.h>
 * 	#include <dev/i2c.h>
 *
 * 	int open_i2c(void){
 * 		int fd;
 * 		i2c_attr_t attr;  //for other devices this is spi_attr_t, uart_attr_t, etc
 * 		fd = open("/dev/i2c1", O_RDWR);
 *		if ( fd < 0 ){
 *			return -1;
 *		}
 * 		attr.pin_assign = 1; //use pins XXX and XXX-- to use pins 0.0 and 0.1 set this to 0 (See I2C1 above)
 * 		attr.bitrate = 100000; //100KHz
 * 		ioctl(fd, I_I2C_SETATTR, &attr); //or I_SPI_SETATTR, I_UART_SETATTR, etc
 *
 * 		return fd;
 * 	}
 * 	\endcode
 *
 */


/*! \file
 * \brief STM32F Header File
 *
 */

#ifndef MCU_ARCH_STM32F_H_
#define MCU_ARCH_STM32F_H_

#include <string.h>
#include "mcu/types.h"
#include "stm32f10x.h"


/*! \details The maximum ADC value on the Stm32f.
 *
 */
#define ADC_MAX ((1<<12)-1)

/*! \details The minimum ADC step value.
 *
 */
#define ADC_MIN 0x01

/*! \details The maximum ADC sampling frequency.
 *
 */
#define ADC_MAX_FREQ 200000
#define ADC_SAMPLE_T

/*! \details This defines the type of an ADC sample (see
 * adc_read()).
 *
 */
typedef uint16_t adc_sample_t;

/*! \details The maximum DAC output frequency.
 *
 */
#define DAC_MAX_FREQ 1000000

#define GPIO_SAMPLE_T
/*! \details This defines the type of a GPIO sample for
 * reading/writing the GPIO (see gpio_read() and gpio_write()).
 *
 */
typedef uint16_t gpio_sample_t;

#define PIO_SAMPLE_T
/*! \details This defines the type of a PIO sample for
 * reading/writing the PIO.
 *
 */
typedef uint16_t pio_sample_t;

enum {
	GPIO_PORTA,
	GPIO_PORTB,
	GPIO_PORTC,
	GPIO_PORTD,
	GPIO_PORTE,
	GPIO_PORTF,
	GPIO_PORTG,
	GPIO_PORTH
};

enum {
	PIO_PORTA,
	PIO_PORTB,
	PIO_PORTC,
	PIO_PORTD,
	PIO_PORTE,
	PIO_PORTF,
	PIO_PORTG,
	PIO_PORTH
};



/*! \details This defines the timer channels.
 * It is assigned to the channel member of \ref tmr_action_t
 * and \ref tmr_reqattr_t.
 */
typedef enum {
	TMR_ACTION_CHANNEL_OC0 /*! Output compare channel 0 */,
	TMR_ACTION_CHANNEL_OC1 /*! Output compare channel 1 */,
	TMR_ACTION_CHANNEL_OC2 /*! Output compare channel 2 */,
	TMR_ACTION_CHANNEL_OC3 /*! Output compare channel 3 */,
	TMR_ACTION_CHANNEL_IC0 /*! Input capture channel 0 */,
	TMR_ACTION_CHANNEL_IC1 /*! Input capture channel 1 */,
	TMR_ACTION_CHANNEL_IC2 /*! Input capture channel 2 */,
	TMR_ACTION_CHANNEL_IC3 /*! Input capture channel 3 */
} stm32f_tmr_action_channel_t;

enum {
	TIM2_PORT /*! Port for timer 2 */,
	TIM3_PORT  /*! Port for timer 3 */,
	TIM15_PORT  /*! Port for timer 15 */,
	TIM16_PORT  /*! Port for timer 16 */,
	TIM17_PORT /*! Port for timer 17 */,
	TIM4_PORT /*! Port for timer 4 */,
	TIM5_PORT /*! Port for timer 5 */,
	TIM6_PORT /*! Port for timer 6 */,
	TIM7_PORT /*! Port for timer 7 */
};


typedef stm32f_tmr_action_channel_t tmr_action_channel_t;

#define PWM_DUTY_T
/*! \details This defines the type for the PWM duty cycle for
 * reading/writing the PWM (see pwm_read() and pwm_write());
 */
typedef uint32_t pwm_duty_t;


/*! \details This is the minimum flash write size.
 *
 */
#define MEM_MIN_FLASH_WRITE_SIZE 256

/*! \details This is the maximum flash write size.
 *
 */
#define MEM_MAX_FLASH_WRITE_SIZE 1024


#define MCU_CORE_BOOTLOADER_LOC 0x10002000
#define MCU_CORE_BOOTLOADER_VALUE 0x55AA55AA

#include "common_cm3.h"

#define MCU_CORE_PORTS 1

#ifdef STM32F10X_LD
#define MCU_GPIO_PORTS 4
#define MCU_GPIO_REGS { GPIOA, GPIOB, GPIOC, GPIOD }
#define MCU_EINT_PORTS 16
#define MCU_UART_PORTS 2
#define MCU_UART_REGS { USART1, USART2 }
#define MCU_I2C_PORTS 1
#define MCU_I2C_REGS { I2C1 }
#define MCU_SPI_PORTS 1
#define MCU_SPI_REGS { SPI1 }
#define MCU_ADC_PORTS 2
#define MCU_ADC_REGS { ADC1, ADC2 }
#define MCU_ADC_CHANNELS 16
#define MCU_TMRADV_PORTS 1
#define MCU_TMRADV_REGS { TIM1 }
#define MCU_TMR_PORTS 2
#define MCU_TMR_REGS { TIM2, TIM3 }
#define MCU_RTC_PORTS 1
#define MCU_RTC_REGS { RTC }
#define MCU_USB_PORTS 0
#endif

#ifdef STM32F10X_LD_VL
#define MCU_GPIO_PORTS 4
#define MCU_GPIO_REGS { GPIOA, GPIOB, GPIOC, GPIOD }
#define MCU_EINT_PORTS 16
#define MCU_UART_PORTS 2
#define MCU_UART_REGS { USART1, USART2 }
#define MCU_I2C_PORTS 1
#define MCU_I2C_REGS { I2C1 }
#define MCU_SPI_PORTS 1
#define MCU_SPI_REGS { SPI1 }
#define MCU_ADC_PORTS 1
#define MCU_ADC_REGS { ADC1 }
#define MCU_ADC_CHANNELS 16
#define MCU_TMRADV_PORTS 1
#define MCU_TMRADV_REGS { TIM1 }
#define MCU_TMR_PORTS 5
#define MCU_TMR_REGS { TIM2, TIM3, TIM15, TIM16, TIM17 }
#define MCU_RTC_PORTS 1
#define MCU_RTC_REGS { RTC }
#define MCU_USB_PORTS 0
#endif

#ifdef STM32F10X_MD_VL
#define MCU_GPIO_PORTS 5
#define MCU_GPIO_REGS { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE }
#define MCU_UART_PORTS 3
#define MCU_UART_REGS { USART1, USART2, USART3 }
#define MCU_I2C_PORTS 2
#define MCU_I2C_REGS { I2C1, I2C2 }
#define MCU_SPI_PORTS 2
#define MCU_SPI_REGS { SPI1, SPI2 }
#define MCU_ADC_PORTS 1
#define MCU_ADC_REGS { ADC1 }
#define MCU_ADC_CHANNELS 16
#define MCU_TMRADV_PORTS 1
#define MCU_TMRADV_REGS { TIM1 }
#define MCU_TMR_PORTS 6
#define MCU_TMR_REGS { TIM2, TIM3, TIM15, TIM16, TIM17, TIM4 }
#define MCU_RTC_PORTS 1
#define MCU_RTC_REGS { RTC }
#define MCU_USB_PORTS 0
#endif


#define MCU_PIO_PORTS MCU_GPIO_PORTS
#define MCU_PIO_REGS MCU_GPIO_REGS

#define DEV_LAST_IRQ 67
#define DEV_MIDDLE_PRIORITY 16


#endif /* MCU_ARCH_STM32F_H_ */

/*! @} */
