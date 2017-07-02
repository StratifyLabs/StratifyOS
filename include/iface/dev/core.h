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

#ifndef IFACE_DEV_CORE_H_
#define IFACE_DEV_CORE_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/mcu.h"
#include "mcu/types.h"
#include "iface/dev/pio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CORE_IOC_IDENT_CHAR 'c'

typedef mcu_action_t core_action_t;

/*! \brief See below for details.
 * \details These are the possible values for \a reset_type
 * in \ref core_attr_t.
 */
typedef enum {
	CORE_RESET_SRC_SOFTWARE /*! Software Reset (default if hardware reset cannot be determined) */,
	CORE_RESET_SRC_POR /*! Power on Reset */,
	CORE_RESET_SRC_EXTERNAL /*! External Reset signal */,
	CORE_RESET_SRC_WDT /*! Watchdog Timer Reset */,
	CORE_RESET_SRC_BOR /*! Brown Out Reset */,
	CORE_RESET_SRC_SYSTEM /*! Software System Reset */
} core_reset_src_t;


/*! \details THis lists the valid values for the clock output
 * source clock.
 */
typedef enum {
	CORE_CLKOUT_CPU /*! Use the CPU Clock */,
	CORE_CLKOUT_MAIN_OSC /*! Use the Main Oscillator */,
	CORE_CLKOUT_INTERNAL_OSC /*! Use the Internal Oscillator */,
	CORE_CLKOUT_USB /*! Use the USB Clock */,
	CORE_CLKOUT_RTC /*! Use the RTC Clock */
} core_clkout_src_t;

/*! \brief Used with I_CORE_SETCLKOUT
 * \details This structure is used to set
 * the clock out attributes (see \ref I_CORE_SETCLKOUT).
 */
typedef struct MCU_PACK {
	u32 src /*! \brief The clock output source (see \ref core_clkout_src_t) */;
	u32 div /*! \brief The clock divide value applied to src */;
} core_clkout_t;


enum {
	CORE_FAULT_NONE,
	CORE_FAULT_HARD,
	CORE_FAULT_MEM,
	CORE_FAULT_BUS,
	CORE_FAULT_USAGE,
	CORE_FAULT_ISR
};

/*! \details This enumerates the valid peripherals supported
 * by HWPL.
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
	CORE_PERIPH_TOTAL
} core_periph_t;

#define CORE_PERIPH_GPIO CORE_PERIPH_PIO
#define CORE_PERIPH_SDC CORE_PERIPH_MCI
#define CORE_PERIPH_ETH CORE_PERIPH_ENET

/*! \details This lists the sleep modes supported by HWPL
 *
 */
typedef enum {
	CORE_SLEEP /*! Sleep mode */,
	CORE_DEEPSLEEP /*! Deep sleep (preserve SRAM) */,
	CORE_DEEPSLEEP_STOP /*! Deep sleep (preserve SRAM, stop clocks) */,
	CORE_DEEPSLEEP_STANDBY /*! Turn the device off (lose SRAM) */
} core_sleep_t;


/*! \brief Core IO Attributes
 * \details This structure defines the attributes structure
 * for configuring the Core port.
 */
typedef struct MCU_PACK {
	u32 serial_number[4] /*! \brief The serial number of the device (from the silicon) */;
	u32 clock /*! \brief The current clock speed */;
	u32 signature /*! \brief The software signature */;
	u8 reset_type /*! \brief The source of the last reset */;
} core_attr_t;

/*!  \brief This requests reads the core attributes.
 */
#define I_CORE_ATTR _IOCTLR(CORE_IOC_IDENT_CHAR, I_GLOBAL_ATTR, core_attr_t)
#define I_CORE_GETATTR I_CORE_ATTR

/*! \brief This requests writes the core attributes.
 */
#define I_CORE_SETATTR _IOCTLW(CORE_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, core_attr_t)
#define I_CORE_SETACTION _IOCTLW(CORE_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, core_action_t)


/*! \brief Data structure for setting the pin functionality.
 * \details This structure is used with I_CORE_SETPINFUNC to
 * set the functionality of pins.
 */
typedef struct MCU_PACK {
	u8 periph_port /*! \brief Peripheral port value (e.g. 0 for I2C0) */;
	u8 periph_func /*! \brief Peripheral port function (see \ref core_periph_t) */;
	pio_t io /*! \brief PIO port and pin (see \ref pio_t) */;
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
#define I_CORE_SETPINFUNC _IOCTLW(CORE_IOC_IDENT_CHAR, I_GLOBAL_TOTAL, core_pinfunc_t)

/*! \brief This request powersdown the device.
 */
#define I_CORE_SLEEP _IOCTL(CORE_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1)
/*! \brief This request resets the device.
 */
#define I_CORE_RESET _IOCTL(CORE_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2)
/*! \brief This request invokes the bootloader.
 */
#define I_CORE_INVOKEBOOTLOADER _IOCTL(CORE_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 3)

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
#define I_CORE_SETCLKOUT _IOCTLW(CORE_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 4, core_clkout_t)

/*! \brief See below for details.
 * \details This configures the main clock divide functionality.  This can be used to
 * reduce power consumption.  This main clock is divided but the peripheral clock stays
 * constant.  The CPU clock can't be divided below the peripheral clock.
 * \code
 * ioctl(core_fd, I_CORE_SETCLKDIVIDE, 4);
 * \endcode
 *
 */
#define I_CORE_SETCLKDIVIDE _IOCTL(CORE_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 5)

/*! \brief See below for details.
 * \details This copies the mcu_board_config_t data that is set by the
 * board support package.
 * \code
 * mcu_board_config_t config;
 * ioctl(core_fd, I_CORE_GETMCUBOARDCONFIG, &config);
 * \endcode
 *
 */
#define I_CORE_GETMCUBOARDCONFIG _IOCTLR(CORE_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 6, mcu_board_config_t)

#define I_CORE_TOTAL 7

#ifdef __cplusplus
}
#endif


#endif // IFACE_DEV_CORE_H_

/*! @} */

