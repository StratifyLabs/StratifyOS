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

/*! \addtogroup CORE Core Microcontroller Access
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The Core peripheral gives access to the MCU clock and other core features.
 *
 */

/*! \file
 * \brief Analog to Digital Converter Header File
 *
 */

#ifndef SOS_DEV_CORE_H_
#define SOS_DEV_CORE_H_

#include <stdint.h>

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CORE_VERSION (0x030000)


#define CORE_IOC_IDENT_CHAR 'c'


typedef enum {
	CORE_FLAG_NONE,
	CORE_FLAG_IS_RESET_SOFTWARE /*! Software Reset (default if hardware reset cannot be determined) */ = (1<<0),
	CORE_FLAG_IS_RESET_POR /*! Power on Reset */ = (1<<1),
	CORE_FLAG_IS_RESET_EXTERNAL /*! External Reset signal */ = (1<<2),
	CORE_FLAG_IS_RESET_WDT /*! Watchdog Timer Reset */ = (1<<3),
	CORE_FLAG_IS_RESET_BOR /*! Brown Out Reset */ = (1<<4),
	CORE_FLAG_IS_RESET_SYSTEM /*! Software System Reset */ = (1<<5),
	CORE_FLAG_SET_CLKOUT /*! Use the CPU Clock */ = (1<<6),
	CORE_FLAG_IS_CLKOUT_CPU /*! Use the CPU Clock */ = (1<<7),
	CORE_FLAG_IS_CLKOUT_MAIN_OSC /*! Use the Main Oscillator */ = (1<<8),
	CORE_FLAG_IS_CLKOUT_INTERNAL_OSC /*! Use the Internal Oscillator */ = (1<<9),
	CORE_FLAG_IS_CLKOUT_USB /*! Use the USB Clock */ = (1<<10),
	CORE_FLAG_IS_CLKOUT_RTC /*! Use the RTC Clock */ = (1<<11),
	CORE_FLAG_EXEC_SLEEP /*! Sleep */ = (1<<12),
	CORE_FLAG_EXEC_DEEPSLEEP /*! Deep sleep (preserve SRAM) */ = (1<<13),
	CORE_FLAG_EXEC_DEEPSLEEP_STOP /*! Deep sleep (preserve SRAM, stop clocks) */ = (1<<14),
	CORE_FLAG_EXEC_DEEPSLEEP_STANDBY /*! Turn the device off (lose SRAM) */ = (1<<15),
	CORE_FLAG_EXEC_RESET /*! Set this flag with I_CORE_SETATTR to reset the device */ = (1<<16),
	CORE_FLAG_EXEC_INVOKE_BOOTLOADER /*! Set this flag with I_CORE_SETATTR to reset the device and start the bootloader */ = (1<<17)
} core_flag_t;


/*! \details This enumerates the valid peripherals supported
 * by Stratify OS.
 */
typedef enum {
	CORE_PERIPH_RESERVED /*! RESERVED */,
	CORE_PERIPH_CORE /*! Core Functionality */,
	CORE_PERIPH_ADC /*! Analog to Digital Converter */,
	CORE_PERIPH_DAC /*! Digital to Analog Converter */,
	CORE_PERIPH_UART /*! UART */,
	CORE_PERIPH_SPI /*! SPI */,
	CORE_PERIPH_USB /*! USB */,
	CORE_PERIPH_CAN /*! CAN */,
	CORE_PERIPH_ENET /*! ENET */,
	CORE_PERIPH_I2C /*! I2C */,
	CORE_PERIPH_I2S /*! I2S */,
	CORE_PERIPH_MEM /*! External memory interface */,
	CORE_PERIPH_RTC /*! RTC */,
	CORE_PERIPH_CEC /*! Consumer Electronic Control (Part of HDMI) */,
	CORE_PERIPH_QEI /*! Quadrature Encoder Interface */,
	CORE_PERIPH_PWM /*! PWM */,
	CORE_PERIPH_PIO /*! GPIO */,
	CORE_PERIPH_TMR /*! Timer (output compare and input capture) */,
	CORE_PERIPH_EINT /*! External interrupts */,
	CORE_PERIPH_WDT /*! Watch dog timer */,
	CORE_PERIPH_BOD /*! Brown out detection */,
	CORE_PERIPH_DMA /*! Direct Memory Access */,
	CORE_PERIPH_JTAG /*! JTAG */,
	CORE_PERIPH_RESET /*! Reset */,
	CORE_PERIPH_CLKOUT /*! Clockout */,
	CORE_PERIPH_LCD /*! LCD */,
	CORE_PERIPH_LCD1 /*! LCD */,
	CORE_PERIPH_LCD2 /*! LCD */,
	CORE_PERIPH_LCD3 /*! LCD */,
	CORE_PERIPH_EMC /*! External Memory Controller */,
	CORE_PERIPH_MCI /*! Multimedia Card Interface */,
	CORE_PERIPH_SSP /*! SSP */,
	CORE_PERIPH_MCPWM /*! Motor Control PWM */,
	CORE_PERIPH_NMI /*! Non-maskable Interrupt */,
	CORE_PERIPH_TRACE /*! Trace data */,
	CORE_PERIPH_SYS /*! System data */,
	CORE_PERIPH_QSPI /*! Quad SPI */,
	CORE_PERIPH_USART /*! USART */,
	CORE_PERIPH_SDIO /*! SD Input/output */,
	CORE_PERIPH_SPDIF /*! SPDIF */,
	CORE_PERIPH_HDMI /*! HDMI CEC */,
	CORE_PERIPH_MCO /*! MCO Pins */,
	CORE_PERIPH_DFSDM /*! DFSDM Pins */,
	CORE_PERIPH_FMP_I2C /*! Fast Mode Plus I2C */,
	CORE_PERIPH_DCMI /*! Digital Camera interface */,
	CORE_PERIPH_TOTAL
} core_periph_t;


typedef struct MCU_PACK {
	u32 o_flags /*! Setting certain flags will configure the core as described */;
	u32 o_events;
	u32 freq /*! The current clock speed */;
	mcu_sn_t serial /*! The serial number of the device (from the silicon) */;
	u32 resd[8];
} core_info_t;

/*! \brief Core IO Attributes
 * \details This structure defines the attributes structure
 * for configuring the Core port.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Setting certain flags will configure the core as described */;
	u32 freq /*! This is used with the clock out flags to set the output frequency */;
	u32 resd[8];
} core_attr_t;

#define I_CORE_GETVERSION _IOCTL(CORE_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*!  \brief This requests reads the core attributes.
 */
#define I_CORE_GETINFO _IOCTLR(CORE_IOC_IDENT_CHAR, I_MCU_GETINFO, core_info_t)

/*! \brief This requests writes the core attributes.
 */
#define I_CORE_SETATTR _IOCTLW(CORE_IOC_IDENT_CHAR, I_MCU_SETATTR, core_attr_t)
#define I_CORE_SETACTION _IOCTLW(CORE_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


/*! \brief Data structure for setting the pin functionality.
 * \details This structure is used with I_CORE_SETPINFUNC to
 * set the functionality of pins.
 */
typedef struct MCU_PACK {
	u8 periph_port /*! Peripheral port value (e.g. 0 for I2C0) */;
	u8 periph_func /*! Peripheral port function (see \ref core_periph_t) */;
	mcu_pin_t io /*! PIO port and pin (see \ref mcu_pin_t) */;
} core_pinfunc_t;


/*! \brief Data structure used for setting interrupt priorities
 * \details This data structure is used with the I_CORE_SETIRQPRIO
 * request to set the interrupt priority for peripherals.
 */
typedef struct MCU_PACK {
	u8 periph /*! \brief The peripheral type  (see \ref core_periph_t) */;
	s8 prio /*! \brief Relative priority value -- less than zero to decrease priority */;
	u8 port /*! \brief The peripheral port number (e.g. 0 for I2C0) */;
} core_irqprio_t;

/*! \brief See below for details.
 * \details This request sets the functionality of the specified pin.  The
 * following example shows how to set P0.15 to be used with I2C0.  If
 * the port/pin combo does not match the peripheral function, the request
 * will fail.  This call is not necessary when using the set attribute
 * request.  For example, I_I2C_SETATTR will configure the pins according
 * to the pin_assign member.
 * \code
 * core_pinfunc_t pinfunc;
 * pinfunc.periph_port = 0;
 * pinfunc.periph_func = CORE_PERIPH_I2C;
 * pinfunc.io.port = 0;
 * pinfunc.io.pin = 15;
 * ioctl(core_fd, I_CORE_SETPINFUNC, &pinfunc);
 * \endcode
 *
 */
#define I_CORE_SETPINFUNC _IOCTLW(CORE_IOC_IDENT_CHAR, I_MCU_TOTAL, core_pinfunc_t)


/*! \brief See below for details.
 * \details This configures the clkout functionality.
 * \code
 * core_clkout_t clkout;
 * clkout.src = CORE_CLKOUT_MAIN_OSC;
 * clkout.div = 8; //output will be the src divided by this value
 * ioctl(core_fd, I_CORE_SETCLKOUT, &clkout);
 * \endcode
 *
 */
#define I_CORE_SETCLKOUT _IOCTLW(CORE_IOC_IDENT_CHAR, I_MCU_TOTAL + 1, core_clkout_t)

/*! \brief See below for details.
 * \details This configures the main clock divide functionality.  This can be used to
 * reduce power consumption.  This main clock is divided but the peripheral clock stays
 * constant.  The CPU clock can't be divided below the peripheral clock.
 * \code
 * ioctl(core_fd, I_CORE_SETCLKDIVIDE, 4);
 * \endcode
 *
 */
#define I_CORE_SETCLKDIVIDE _IOCTL(CORE_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)

/*! \brief See below for details.
 * \details This copies the mcu_board_config_t data that is set by the
 * board support package.
 * \code
 * mcu_board_config_t config;
 * ioctl(core_fd, I_CORE_GETMCUBOARDCONFIG, &config);
 * \endcode
 *
 */
#define I_CORE_GETMCUBOARDCONFIG _IOCTLR(CORE_IOC_IDENT_CHAR, I_MCU_TOTAL + 3, mcu_board_config_t)

//invoke bootloader for version 2
#define I_CORE_INVOKEBOOTLOADER_2 _IOCTL('c', 6)

#define I_CORE_TOTAL 7

#ifdef __cplusplus
}
#endif


#endif // SOS_DEV_CORE_H_

/*! @} */

