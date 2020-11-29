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

/*! \addtogroup WDT Watch Dog Timer (WDT)
 * @{
 *
 * \ingroup CORE
 *
 * \details The Watch Dog Timer (WDT) is used to reset the device if the timer expires.  It is
 * designed as a fail safe to prevent software from freezing the processor.  It should be
 * integrated at the OS level to ensure no application processes or threads disable
 * context switching.
 *
 * The following code example initializes the WDT using the internal RC oscillator.
 * \code
 * wdt_init(WDT_MODE_RESET|WDT_MODE_CLK_SRC_INTERNAL_RC, 3000);
 * \endcode
 *
 * If three seconds (3000ms) passes without a call to wdt_reset(), the MCU will reset.
 *
 */

/*! \file
 * \brief Watch Dog Timer Header File
 *
 */

#ifndef WDT_H_
#define WDT_H_

#include <sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \details This enum lists the valid values of
 * the WDT clock sources.  Not all sources
 * are supported on all devices.  Consult the device
 * documentation to see which sources are supported.
 *
 * The WDT can be an OR'd value of one clock source and one of
 * WDT_MODE_RESET or WDT_MODE_INTERRUPT.  For example:
 * \code
 * wdt_init(WDT_MODE_RESET|WDT_MODE_CLK_SRC_INTERNAL_RC, 3000);
 * \endcode
 *
 */
typedef enum {
	WDT_MODE_RESET /*! Reset the device when the WDT times out */ = (1<<0),
	WDT_MODE_INTERRUPT /*! Interrupt the processor on a WDT time out */ = (1<<1),
	WDT_MODE_CLK_SRC_INTERNAL_RC /*! WDT clock source is an internal RC oscillator */ = (0<<2),
	WDT_MODE_CLK_SRC_MAIN /*! WDT clock source is the same as the core CPU */ = (1<<2),
	WDT_MODE_CLK_SRC_RTC /*! WDT is clocked the same as the RTC (usually 32KHz) */ = (2<<2),
	WDT_MODE_CLK_SRC_WDT_OSC /*! The WDT is clock using the dedicated WDT oscillator */ = (3<<2)
} wdt_mode_t;

#define WDT_MODE_CLK_SRC_MASK (0x07 << 2)

int mcu_wdt_init(int mode, int interval) MCU_ROOT_CODE;
void mcu_wdt_reset();
void mcu_wdt_root_reset(void * args) MCU_ROOT_CODE;
int mcu_wdt_setaction(int (*action)(const void *, void *)) MCU_ROOT_CODE;
int mcu_wdt_setinterval(int interval) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif


#endif /* WDT_H_ */

/*! @} */
