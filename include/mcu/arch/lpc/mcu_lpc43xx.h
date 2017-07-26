/*
 * mcu_lpc43xx.h
 *
 *  Created on: May 16, 2016
 *      Author: tgil
 */

#ifndef MCU_ARCH_MCU_LPC43XX_H_
#define MCU_ARCH_MCU_LPC43XX_H_

#include <string.h>
#include "mcu/types.h"

#if !defined CORE_M4
#define CORE_M4
#endif

#if defined __SOFTFP__
#undef __SOFTFP__
#endif

#include "cmsis/LPC43xx.h"
#include "../cortex_m.h"
#include "mcu_lpc_core.h"

/*! \details This defines the timer channels.
 * It is assigned to the channel member of \ref mcu_action_t
 * and \ref mcu_channel_t.
 */
typedef enum {
	TMR_ACTION_CHANNEL_OC0 /*! Output compare channel 0 */,
	TMR_ACTION_CHANNEL_OC1 /*! Output compare channel 1 */,
	TMR_ACTION_CHANNEL_OC2 /*! Output compare channel 2 */,
	TMR_ACTION_CHANNEL_OC3 /*! Output compare channel 3 */,
	TMR_ACTION_CHANNEL_IC0 /*! Input capture channel 0 */,
	TMR_ACTION_CHANNEL_IC1 /*! Input capture channel 1 */,
	TMR_ACTION_CHANNEL_IC2 /*! Input capture channel 2 */,
	TMR_ACTION_CHANNEL_IC3 /*! Input capture channel 3 */,
} lpc43xx_tmr_action_channel_t;

typedef lpc43xx_tmr_action_channel_t tmr_action_channel_t;

typedef u32 pwm_duty_t;

#define PIO_SAMPLE_T
/*! \brief This defines the type of a PIO sample for
 * reading/writing the PIO.
 */
typedef u32 pio_sample_t;



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

#define MCU_EEPROM_API 1
#define MCU_EEPROM_PORTS 1
#define MCU_EEPROM_REGS { LPC_EEPROM }
#define MCU_EEPROM_IRQS { 0xFF }

#define MCU_EEPROM_SIZE (16*1024)
#define MCU_EEPROM_PAGE_SIZE 64


#define MCU_SPI_API 0
#define MCU_SPI_PORTS 1
#define MCU_SPI_REGS { LPC_SPI }
#define MCU_SPI_IRQS { SPI_INT_IRQn }

#define MCU_SSP_API 0
#define MCU_SSP_PORTS 2
typedef LPC_SSPn_Type LPC_SSP_Type;
#define MCU_SSP_REGS { LPC_SSP0, LPC_SSP1 }
#define MCU_SSP_IRQS { SSP0_IRQn, SSP1_IRQn }

#define MCU_MCI_PORTS 0
#define MCU_MCI_REGS { LPC_MCI }

#define MCU_PIO_API 1
#define MCU_PIO_PORTS 1
#define MCU_PIO_REGS { LPC_GPIO_PORT }

#define MCU_I2C_API 1
#define MCU_I2C_PORTS 2
typedef LPC_I2Cn_Type LPC_I2C_Type;
#define MCU_I2C_REGS { LPC_I2C0, LPC_I2C1 }
#define MCU_I2C_IRQS { I2C0_IRQn, I2C1_IRQn }

#define MCU_I2S_API 0
#define MCU_I2S_PORTS 2
typedef LPC_I2Sn_Type LPC_I2S_Type;
#define MCU_I2S_REGS { LPC_I2S0, LPC_I2S1 }
#define MCU_I2S_IRQS { I2S0_IRQn, I2S1_IRQn }

#define MCU_UART_API 0
#define MCU_UART_PORTS 4
typedef LPC_USARTn_Type LPC_UART_Type;
#define MCU_UART_REGS { (LPC_UART_Type*)LPC_USART0, (LPC_UART_Type*)LPC_UART1, (LPC_UART_Type*)LPC_USART2, (LPC_UART_Type*)LPC_USART3  }
#define MCU_UART_IRQS { USART0_IRQn, UART1_IRQn, USART2_IRQn, USART3_IRQn }

#define MCU_TMR_API 1
#define MCU_TMR_PORTS 4
typedef LPC_TIMERn_Type LPC_TIM_Type;
#define MCU_TMR_REGS { LPC_TIMER0, LPC_TIMER1, LPC_TIMER2, LPC_TIMER3 }
#define MCU_TMR_IRQS { TIMER0_IRQn, TIMER1_IRQn, TIMER2_IRQn, TIMER3_IRQn }

#define MCU_EINT_API 1
#define MCU_EINT_PORTS 8
#define MCU_EINT_REGS 0
#define MCU_EING_IRQS { PIN_INT0_IRQn, PIN_INT1_IRQn, PIN_INT2_IRQn, PIN_INT3_IRQn, PIN_INT4_IRQn, PIN_INT5_IRQn, PIN_INT6_IRQn, PIN_INT7_IRQn }

#define MCU_FLASH_PORTS 1
#define MCU_MEM_PORTS 1

#define MCU_ADC_PORTS 2
typedef LPC_ADCn_Type LPC_ADC_Type;
#define MCU_ADC_REGS { LPC_ADC0, LPC_ADC1 }
#define MCU_ADC_IRQS { ADC0_IRQn, ADC1_IRQn }

#define MCU_DAC_PORTS 1
#define MCU_DAC_REGS { LPC_DAC }
#define MCU_DAC_IRQS { DAC_IRQn }

#define MCU_QEI_PORTS 1
#define MCU_QEI_REGS { LPC_QEI }
#define MCU_QEI_IRQS { QEI_IRQn }

#define MCU_RTC_API 1
#define MCU_RTC_PORTS 1
#define MCU_RTC_REGS { LPC_RTC }
#define MCU_RTC_IRQS { RTC_IRQn }

#define MCU_USB_PORTS 2
#define MCU_USB_REGS { LPC_USB0, LPC_USB1 }
#define MCU_USB_IRQS { USB0_IRQn, USB1_IRQn }
//typedef LPC_USBn_Type LPC_USB_TypeDef;

#define MCU_PWM_PORTS 0

#define MCU_ENET_API 1
#define MCU_ENET_PORTS 1
#define MCU_ENET_REGS { LPC_ETHERNET }
#define MCU_ENET_IRQS { ETHERNET_IRQn }

#define MCU_WDT_API 0
#define WDT_IRQn WWDT_IRQn

/*
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
*/

#define MCU_EMC_PORTS 0
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


#define DEV_LAST_IRQ QEI_IRQn
#define DEV_MIDDLE_PRIORITY 16
#define DEV_USB_LOGICAL_ENDPOINT_COUNT 16
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

#define LPC_WDT_API 1
#define LPC_WDT LPC_WWDT

int mcu_lpc_flash_write_page(int page, void * addr, const void * src, int size);
int mcu_lpc_flash_erase_page(int page);
int mcu_lpc_flash_get_serialno(uint32_t * dest);



typedef struct                          /* Channel Registers                  */
{
  __IO uint32_t CSrcAddr;
  __IO uint32_t CDestAddr;
  __IO uint32_t CLLI;
  __IO uint32_t CControl;
  __IO uint32_t CConfig;
} LPC_GPDMACH_Type;


#define MCU_DMA_API 1
#define MCU_DMA_CHANNELS 8
#define MCU_DMA_CHANNEL_REGS { (LPC_GPDMACH_Type*)0x40002100, (LPC_GPDMACH_Type*)0x40002120, (LPC_GPDMACH_Type*)0x40002140, (LPC_GPDMACH_Type*)0x40002160, (LPC_GPDMACH_Type*)0x40002180, (LPC_GPDMACH_Type*)0x400021A0, (LPC_GPDMACH_Type*)0x400021C0, (LPC_GPDMACH_Type*)0x400021E0 }

#include "mcu_lpc_dma.h"



#endif /* MCU_ARCH_MCU_LPC43XX_H_ */
