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

/*! \addtogroup LPC17XXDEV LPC17xx Devices
 * @{
 *
 * \ingroup ARCHITECTURES
 *
 * \details This contains the device specific definitions for the LPC17xx microcontroller series.
 *
 * The following devices are supported:
 * - lpc1754
 * - lpc1756
 * - lpc1758
 * - lpc1759
 * - lpc1764
 * - lpc1765
 * - lpc1766
 * - lpc1768
 * - lpc1769
 *
 *
 * Pin assignement values specify how to map peripherals to IO pins.  The following code
 * snippet shows how to specify pins 0.19 and 0.20 for use with I2C port 1.  After
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
 * attr.bitrate = 100000;
 * ioctl(fd, I_SETATTR, &attr);
 * \endcode
 *
 * The following pin assignments are available on the lpc17xx architecture:
 * - ADC0
 * 	- 0:  0.23, 0.24, 0.25, 0.26, 1.30, 1.31
 * - CAN0 (Not Yet Supported)
 * - DAC0
 * 	- 0:  0.26
 * - EINT0
 * 	- 0:  2.10
 * - EINT1
 * 	- 0:  2.11
 * - EINT2
 * 	- 0:  2.12
 * - EINT3
 * 	- 0:  2.13
 * - I2C0
 * 	- 0:  0.27, 0.28
 * - I2C1
 * 	- 0:  0.0, 0.1
 * 	- 1:  0.19, 0.20
 * - I2C2
 * 	- 0:  0.10, 0.11
 * - I2S (Not Yet Supported)
 * - PWM1
 *  - 0:  1.18, 1.20, 1.21, 1.23, 1.24, 1.26
 *  - 1:  2.0, 2.1, 2.2, 2.3, 2.4, 2.5
 * - QEI0
 * 	- 0:  1.20, 1.23, 1.24
 * - SPI0 (SSP0 in the datasheet)
 * 	- 0:  0.15, 0.17, 0.18
 * 	- 1:  1.20, 1.23, 1.24
 * - SPI1 (SSP1 in the datasheet)
 * 	- 0:  0.7, 0.8, 0.9
 * - SPI2 (SPI0 in the datasheet)
 * 	- 0:  0.15, 0.17, 0.18
 * - TMR0
 * 	- 0:  1.28, 1.29
 * 	- 1:  3.25, 3.26
 * - TMR1
 * 	- 0:  1.22, 1.25
 * - TMR2
 * 	- 0:  0.4, 0.5, 0.6, 0.7, 0.8, 0.9
 * 	- 1:  4.28, 4.29
 * - TMR3
 * 	- 0:  0.10, 0.11
 * - UART0
 * 	- 0:  0.2, 0.3
 * - UART1
 * 	- 0:  0.15, 0.16
 * 	- 1:  2.0, 2.1
 * - UART2
 * 	- 0:  0.10, 0.11
 * 	- 1:  2.8, 2.9
 * - UART3
 * 	- 0:  0.0, 0.1
 * 	- 1:  0.25, 0.26
 * 	- 2:  4.28, 4.29
 * - USB0
 * 	- 0:  0.29, 0.30, 1.30
 *
 * The following is an example that shows how to use the pin assign values
 * when opening the I2C.  The same approach applies to other peripherals as well.
 * \code
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <dev/i2c.h>
 *
 * int open_i2c(){
 * 	int fd;
 * 	i2c_attr_t attr;  //for other devices this is spi_attr_t, uart_attr_t, etc
 * 	fd = open("/dev/i2c1", O_RDWR);
 *	if ( fd < 0 ){
 *		return -1;
 *	}
 * 	attr.pin_assign = 1; //use pins 0.19 and 0.20 -- to use pins 0.0 and 0.1 set this to 0 (See I2C1 above)
 * 	attr.bitrate = 100000; //100KHz
 * 	ioctl(fd, I_I2C_SETATTR, &attr); //or I_SPI_SETATTR, I_UART_SETATTR, etc
 *	return fd;
 * }
 * \endcode
 *
 */


/*! \file
 * \brief LPC17xx Header File
 *
 */


#ifndef LPC17XX_DEVICES_H_
#define LPC17XX_DEVICES_H_

#include <string.h>
#include "mcu/types.h"

/*! \brief PWM Channel 1 (PWM1[1])
 */
#define PWM_CHANNEL1 (0)
/*! \brief PWM Channel 2 (PWM1[2])
 */
#define PWM_CHANNEL2 (1)
/*! \brief PWM Channel 3 (PWM1[3])
 */
#define PWM_CHANNEL3 (2)
/*! \brief PWM Channel 4 (PWM1[4])
 */
#define PWM_CHANNEL4 (3)
/*! \brief PWM Channel 5 (PWM1[5])
 */
#define PWM_CHANNEL5 (4)
/*! \brief PWM Channel 6 (PWM1[6])
 */
#define PWM_CHANNEL6 (5)

#ifdef __cplusplus
extern "C" {
#endif



/*! \brief The maximum ADC value on the LPC17xx.
 */
#define ADC_MAX 0x0000FFF0

/*! \brief Minimum ADC set value
 */
#define ADC_MIN 0x01

/*! \brief The maximum ADC sampling frequency.
 */
#define ADC_MAX_FREQ 200000
#define ADC_SAMPLE_T

/*! \brief The maximum DAC output frequency.
 */
#define DAC_MAX_FREQ 1000000

/*! \brief This defines the type of an ADC sample (see adc_read() and adc_write()).
 */
typedef uint16_t adc_sample_t;

#define GPIO_SAMPLE_T
/*! \brief This defines the type of a GPIO sample for reading/writing the GPIO (see gpio_read() and gpio_write()).
 */
typedef u32 gpio_sample_t;

#define PIO_SAMPLE_T
/*! \brief This defines the type of a PIO sample for
 * reading/writing the PIO.
 */
typedef u32 pio_sample_t;

#define PWM_DUTY_T
/*! \brief This defines the type for the PWM duty cycle for reading/writing the PWM.
 */
typedef u32 pwm_duty_t;


/*! \brief This is the minimum flash write size. */
#define FLASH_MIN_WRITE_SIZE 256

/*! \brief This is the maximum flash write size. */
#define FLASH_MAX_WRITE_SIZE 1024

/*! \brief This is the minimum flash write size.
 */
#define MEM_MIN_FLASH_WRITE_SIZE 256

/*! \brief This is the maximum flash write size.
 */
#define MEM_MAX_FLASH_WRITE_SIZE 1024

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
} lpc17xx_tmr_action_channel_t;

typedef lpc17xx_tmr_action_channel_t tmr_action_channel_t;


#define MCU_CORE_BOOTLOADER_LOC 0x10002000
#define MCU_CORE_BOOTLOADER_VALUE 0x55AA55AA

int _mcu_lpc_flash_write_page(int page, void * addr, const void * src, int size);
int _mcu_lpc_flash_erase_page(int page);
int _mcu_lpc_flash_get_serialno(uint32_t * dest);

#define IAP_ADDRESS 0x1FFF1FF1

#include "cmsis/LPC17xx.h"

#define DEV_LAST_IRQ SW7_IRQn
#define DEV_MIDDLE_PRIORITY 16

#ifdef __cplusplus
}
#endif


#define MCU_DMA_CHANNELS 8

#define MCU_DMA_CHANNEL_REGS { LPC_GPDMACH0, LPC_GPDMACH1, LPC_GPDMACH2, LPC_GPDMACH3, LPC_GPDMACH4, LPC_GPDMACH5, LPC_GPDMACH6, LPC_GPDMACH7 }

#include "mcu_lpc_dma.h"

#define MPU_ACCESS_T
#define MPU_SIZE_T

#include "mcu_lpc_core.h"
#include "../cortex_m.h"

#define MCU_CORE_PORTS 1
#define MCU_EEPROM_PORTS 0
#define MCU_SPI_API 0
#define MCU_SPI_PORTS 1
#define MCU_SPI_REGS { LPC_SPI }
#define MCU_SPI_IRQS { SPI_IRQn }

#define MCU_SSP_PORTS 2


#define MCU_SSP_API 0
#define MCU_SSP_PORTS 2
#define MCU_SSP_REGS { LPC_SSP0, LPC_SSP1 }
#define MCU_SSP_IRQS { SSP0_IRQn, SSP1_IRQn }

#define MCU_GPIO_PORTS 5
#define MCU_PIO_PORTS 5

#define MCU_I2C_API 0
#define MCU_I2C_PORTS 3
#define MCU_I2C_REGS { LPC_I2C0, LPC_I2C1, LPC_I2C2 }
#define MCU_I2C_IRQS { I2C0_IRQn, I2C1_IRQn , I2C2_IRQn }

#define MCU_I2S_PORTS 1
#define MCU_I2S_REGS { LPC_I2S }
#define MCU_I2S_IRQS { I2S_IRQn }

#define MCU_UART_API 0
#define MCU_UART_PORTS 4
#define MCU_UART_REGS { (LPC_UART_Type *)LPC_UART0, (LPC_UART_Type *)LPC_UART1, LPC_UART2, LPC_UART3 }
#define MCU_UART_IRQS { UART0_IRQn, UART1_IRQn, UART2_IRQn, UART3_IRQn }

#define MCU_TMR_API 0
#define MCU_TMR_PORTS 4
#define MCU_TMR_REGS { LPC_TIM0, LPC_TIM1, LPC_TIM2, LPC_TIM3 }
#define MCU_TMR_IRQS { TIMER0_IRQn, TIMER1_IRQn, TIMER2_IRQn, TIMER3_IRQn }


#define MCU_EINT_PORTS 4


#define MCU_ENET_PORTS 1
#define MCU_FLASH_PORTS 1
#define MCU_MEM_PORTS 1

#define MCU_ADC_PORTS 1
#define MCU_ADC_REGS { LPC_ADC }
#define MCU_ADC_IRQS { ADC_IRQn }

#define MCU_DAC_PORTS 1
#define MCU_DAC_REGS { LPC_DAC }
#define MCU_DAC_IRQS { DAC_IRQn }


#define MCU_QEI_PORTS 1
#define MCU_QEI_REGS { LPC_QEI }
#define MCU_QEI_IRQS { QEI_IRQn }

#define MCU_RTC_API 0
#define MCU_RTC_PORTS 1
#define MCU_RTC_REGS { LPC_RTC }
#define MCU_RTC_IRQS { RTC_IRQn }

#define MCU_USB_PORTS 1

#define MCU_PWM_PORTS 2
#define MCU_PWM_REGS { 0, LPC_PWM1 }
#define MCU_PWM_IRQS { PWM1_IRQn, PWM1_IRQn }

#define MCU_ENET_API 0
#define MCU_ENET_PORTS 1
#define MCU_ENET_REGS { LPC_EMAC }
#define MCU_ENET_IRQS { ENET_IRQn }

#define MCU_WDT_API 0

#define MCU_START_OF_SRAM 0x10000000
#define MCU_START_OF_AHB_SRAM 0x2007C000

 /*! \brief ADC0 pin assignment 0 is port 0 pins 23, 24, 25, 26, 30, and 31 */
 #define __ADC0_PINASSIGN_PORT0_23_24_25_26_30_31 (0)

 /*! \brief DAC0 pin assignment 0 is port 0 pin 26 */
 #define __DAC0_PINASSIGN_PORT0_26 (0)

 /*! \brief EINT0 pin assignment 0 is port 2 pin 10 */
 #define __EINT0_PINASSIGN_PORT2_10 (0)
 /*! \brief EINT1 pin assignment 0 is port 2 pin 11 */
 #define __EINT1_PINASSIGN_PORT2_11 (0)
 /*! \brief EINT2 pin assignment 0 is port 2 pin 12 */
 #define __EINT2_PINASSIGN_PORT2_12 (0)
 /*! \brief EINT3 pin assignment 0 is port 2 pin 13 */
 #define __EINT3_PINASSIGN_PORT2_13 (0)

/*! \brief I2C0 pin assignment 0 is port 0 pins 27 and 28 */
#define __I2C0_PINASSIGN_PORT0_27_28 (0)
/*! \brief I2C1 pin assignment 0 is port 0 pins 0 and 1 */
#define __I2C1_PINASSIGN_PORT0_0_1 (0)
/*! \brief I2C1 pin assignment 1 is port 1 pins 19 and 20 */
#define __I2C1_PINASSIGN_PORT1_19_20 (1)
/*! \brief I2C2 pin assignment 0 is port 0 pins 10 and 11 */
#define __I2C2_PINASSIGN_PORT0_10_11 (0)

/*! \brief PWM1 pin assignment 0 is port 0 pins 18, 20, 21, 23, 24, and 26 */
#define __PWM1_PINASSIGN_PORT0_18_20_21_23_24_26 (0)
/*! \brief PWM1 pin assignment 1 is port 2 pins 0, 1, 2, 3, 4, and 5 */
#define __PWM1_PINASSIGN_PORT2_0_1_2_3_4_5 (1)

/*! \brief QEI0 pin assignment 0 is port 0 pins 20, 23, and 24 */
#define __QEI0_PINASSIGN_PORT0_20_23_24 (0)

/*! \brief SPI0 pin assignment 0 is port 0 pins 15, 17, and 18 */
#define __SPI0_PINASSIGN_PORT0_15_17_18 (0)
/*! \brief SPI0 pin assignment 1 is port 1 pins 20, 23, and 24 */
#define __SPI0_PINASSIGN_PORT1_20_23_24 (1)
/*! \brief SPI1 pin assignment 0 is port 0 pins 7, 8, and 9 */
#define __SPI1_PINASSIGN_PORT0_7_8_9 (0)
/*! \brief SPI2 pin assignment 0 is port 0 pins 15, 16, and 18 */
#define __SPI2_PINASSIGN_PORT0_15_16_18 (0)

/*! \brief TMR0 pin assignment 0 is port 1 pins 26 (input0), 27 (input1), 28 (output0) and 29 (output1) */
#define __TMR0_PINASSIGN_PORT1_26_27_28_29 (0)
 /*! \brief TMR0 pin assignment 1 is port 3 pins 25 and 26 */
 #define __TMR0_PINASSIGN_PORT3_25_26 (1)
 /*! \brief TMR1 pin assignment 0 is port 1 pins 22 and 25 */
 #define __TMR1_PINASSIGN_PORT3_22_25 (0)
 /*! \brief TMR2 pin assignment 0 is port 0 pins 4, 5, 6, 7, 8, and/or 9 */
 #define __TMR2_PINASSIGN_PORT0_4_5_6_7_8_9 (0)
  /*! \brief TMR2 pin assignment 1 is port 4 pins 28 and 29 */
  #define __TMR2_PINASSIGN_PORT4_28_29 (1)
 /*! \brief TMR3 pin assignment 0 is port 0 pins 10 and 11 */
 #define __TMR3_PINASSIGN_PORT0_10_11 (0)

/*! \brief UART0 pin assignment 0 is port 0 pins 2 and 3 */
#define __UART0_PINASSIGN_PORT0_2_3 (0)
 /*! \brief UART0 pin assignment 1 is port 0 pins 15 and 16 */
 #define __UART1_PINASSIGN_PORT0_15_16 (0)
 /*! \brief UART1 pin assignment 1 is port 2 pins 0 and 1 */
 #define __UART1_PINASSIGN_PORT2_0_1 (1)
 /*! \brief UART2 pin assignment 0 is port 0 pins 10 and 11 */
 #define __UART2_PINASSIGN_PORT0_0_11 (0)
 /*! \brief UART2 pin assignment 1 is port 2 pins 8 and 9 */
 #define __UART2_PINASSIGN_PORT2_8_9 (1)
 /*! \brief UART3 pin assignment 0 is port 0 pins 0 and 1 */
 #define __UART3_PINASSIGN_PORT0_0_1 (0)
 /*! \brief UART3 pin assignment 1 is port 0 pins 25 and 26 */
 #define __UART3_PINASSIGN_PORT0_25_26 (1)
 /*! \brief UART3 pin assignment 2 is port 4 pins 28 and 29 */
 #define __UART3_PINASSIGN_PORT4_28_29 (2)


//Pin mapping differences between 17xx and 177x_8x
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
#define MCU_ADC_CHANNEL6_PIN 3
#define MCU_ADC_CHANNEL7_PORT 0
#define MCU_ADC_CHANNEL7_PIN 2

#define MCU_UART_PORT0_PINASSIGN0 0
#define MCU_UART_TXPIN0_PINASSIGN0 3
#define MCU_UART_RXPIN0_PINASSIGN0 2

#define MCU_I2C_PORT0_PINASSIGN0 0
#define MCU_I2C_SCLPIN0_PINASSIGN0 28
#define MCU_I2C_SDAPIN0_PINASSIGN0 27

#define MCU_SPI_PORT0_PINASSIGN0 0
#define MCU_SPI_MOSIPIN0_PINASSIGN0 18
#define MCU_SPI_MISOPIN0_PINASSIGN0 17
#define MCU_SPI_SCKPIN0_PINASSIGN0 15

#define MCU_SPI_PORT0_PINASSIGN1 1
#define MCU_SPI_MOSIPIN0_PINASSIGN1 24
#define MCU_SPI_MISOPIN0_PINASSIGN1 23
#define MCU_SPI_SCKPIN0_PINASSIGN1 20


#endif /* LPC17XX_DEVICES_H_ */

/*! @} */
