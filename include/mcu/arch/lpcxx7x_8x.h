

#ifndef LPCXX7X_8X_H_
#define LPCXX7X_8X_H_


#define LPCXX7X_8X

#include "mcu/types.h"

typedef uint32_t pwm_duty_t;

#define PIO_SAMPLE_T
/*! \brief This defines the type of a PIO sample for
 * reading/writing the PIO.
 */
typedef uint32_t pio_sample_t;



/*! \brief The maximum ADC value on the LPC17xx.
 */
#define ADC_MAX 0x0000FFF0

/*! \brief Minimum ADC set value
 */
#define ADC_MIN 0x01

/*! \brief The maximum ADC sampling frequency.
 */
#define ADC_MAX_FREQ 400000
#define ADC_SAMPLE_T

/*! \brief The maximum DAC output frequency.
 */
#define DAC_MAX_FREQ 1000000

/*! \brief This defines the type of an ADC sample (see adc_read() and adc_write()).
 */
typedef uint16_t adc_sample_t;

#define MCU_CORE_PORTS 1
#define MCU_EEPROM_PORTS 1
#define MCU_EEPROM_SIZE (4*1024)
#define MCU_EEPROM_PAGE_SIZE 64
#define MCU_SPI_PORTS 0
#define MCU_SSP_PORTS 3
#define MCU_SSP_REGS { LPC_SSP0, LPC_SSP1, LPC_SSP2 }
#define MCU_MCI_PORTS 1
#define MCU_MCI_REGS { LPC_MCI }
#define MCU_GPIO_PORTS 5
#define MCU_PIO_PORTS 6
#define MCU_PIO_REGS { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4, LPC_GPIO5 }
#define MCU_I2C_PORTS 3
#define MCU_I2C_REGS { LPC_I2C0, LPC_I2C1, LPC_I2C2 }
#define MCU_I2S_PORTS 1
#define MCU_I2S_REGS { LPC_I2S }
#define MCU_UART_PORTS 5
#define MCU_UART_REGS { (LPC_UART_TypeDef *)LPC_UART0, (LPC_UART_TypeDef *)LPC_UART1, LPC_UART2, LPC_UART3, (LPC_UART_TypeDef *)LPC_UART4 }


#define MCU_TMR_PORTS 4
#define MCU_EINT_PORTS 4
#define MCU_FLASH_PORTS 1
#define MCU_MEM_PORTS 1
#define MCU_ADC_PORTS 1
#define MCU_DAC_PORTS 1
#define MCU_QEI_PORTS 1
#define MCU_RTC_PORTS 1
#define MCU_USB_PORTS 1
#define MCU_PWM_PORTS 2
#define MCU_PWM_REGS { LPC_PWM0, LPC_PWM1 }


#define MCU_ENET_PORTS 1

typedef struct MCU_PACK {
	  __IO uint32_t StaticConfig;
	  __IO uint32_t StaticWaitWen;
	  __IO uint32_t StaticWaitOen;
	  __IO uint32_t StaticWaitRd;
	  __IO uint32_t StaticWaitPage;
	  __IO uint32_t StaticWaitWr;
	  __IO uint32_t StaticWaitTurn;
} LPC_EMC_Static_TypeDef;

typedef struct MCU_PACK {
	  __IO uint32_t DynamicConfig;
	  __IO uint32_t DynamicRasCas;
} LPC_EMC_Dynamic_TypeDef;

#define MCU_EMC_PORTS 1
#define MCU_EMC_CHANS 4
#define MCU_EMC_STATIC_REGS { (LPC_EMC_Static_TypeDef*)&LPC_EMC->StaticConfig0, (LPC_EMC_Static_TypeDef*)&LPC_EMC->StaticConfig1, (LPC_EMC_Static_TypeDef*)&LPC_EMC->StaticConfig2, (LPC_EMC_Static_TypeDef*)&LPC_EMC->StaticConfig3 }
#define MCU_EMC_DYNAMIC_REGS { (LPC_EMC_Dynamic_TypeDef*)&LPC_EMC->DynamicConfig0, (LPC_EMC_Dynamic_TypeDef*)&LPC_EMC->DynamicConfig1, (LPC_EMC_Dynamic_TypeDef*)&LPC_EMC->DynamicConfig2, (LPC_EMC_Dynamic_TypeDef*)&LPC_EMC->DynamicConfig3 }



#define MCU_START_OF_SRAM 0x10000000
#define MCU_START_OF_AHB_SRAM 0x20000000

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


#define DEV_LAST_IRQ EEPROM_IRQn
#define DEV_MIDDLE_PRIORITY 16
#define IAP_ADDRESS 0x1FFF1FF1
#define MCU_CORE_BOOTLOADER_LOC 0x10002000
#define MCU_CORE_BOOTLOADER_VALUE 0x55AA55AA

#define MCU_LAST_BOOTLOADER_PAGE 5

//Pin mapping for lpc17xx vs lpc177x_8x

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

int _mcu_lpc_flash_write_page(int page, void * addr, const void * src, int size);
int _mcu_lpc_flash_erase_page(int page);
int _mcu_lpc_flash_get_serialno(uint32_t * dest);

#endif /* LPCXX7X_8X_H_ */
