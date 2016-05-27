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

/*! \addtogroup CORE Core Access
 * @{
 *
 *
 *
 * \details The Core module allows access to the core CPU functionality such as clock speed, power, and
 * interrupts.  In general, the OS should make extensive use of the Core access API while the device
 * drivers use the \ref IFACE_DEV API.  To maximize portability, applications should rely on the
 * OS abstraction of all hardware.
 *
 *
 */

/*! \file
 * \brief Core Functionality Header File
 *
 */

#ifndef _MCU_CORE_H_
#define _MCU_CORE_H_

#include <stdint.h>
#include "mcu/mcu.h"
#include "iface/device_config.h"
#include "../iface/dev/core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*core_privcall_t)(void*);

/*! \details A privileged call.
 */
#ifndef __link
/*! \details This performs a privileged call.
 *
 */
void mcu_core_privcall(core_privcall_t call /*! the function to execute */,
		void * args /*! the pointer to pass to call */) __attribute__((optimize("1")));
#endif

/*! \details This function reads the OS signature value.
 *
 */
int _mcu_core_getsignature(int port, void * arg) MCU_PRIV_CODE;

/*! \details This function sets the main clock speed based on
 * the desired speed (fclk) and the given oscillator frequency (fosc).  This function rounds the target
 * frequency to the closest value achievable given any hardware limitations.
 *
 * \note This function is the de facto initialization routine for HWPL.  It should
 * be called before any other HWPL functions are called.
 *
 */
void _mcu_core_setclock(int fclk /*! The target clock frequency */,
		int fosc /*! The oscillator frequency (zero to use the internal oscillator) */ );


/*! \brief Init the clock.
 *
 * \details This function inits the clock.  It assumes the following variables are
 * linked externally:
 * \code
 * extern const int mcu_core_osc_freq; //oscillator freq (0 if not present)
 * extern const int mcu_board_config.core_cpu_freq; //max CPU freq (before device)
 * extern const int mcu_board_config.core_periph_freq; //constant periph clock freq
 * \endcode
 * @param div Divider value from max (1,2,4,8)
 * @return Zero on success
 *
 */
int _mcu_core_initclock(int div);


void _mcu_core_setclockinternal(int fclk) MCU_PRIV_CODE;

//int mcu_core_setclock(int port, void * arg);

/*! \details This sets the CPU block to 72Mhz assuming
 * a 12MHz clock is connected.
 */
void _mcu_core_setclock_main_12mhz_72mhz() MCU_PRIV_CODE;

/*! \details This function sets the USB clock speed to the required
 * value for USB operation on the device.  The target frequency is defined
 * by the MCU (usually 48MHz).  Most MCUs have limitations on what
 * oscillator frequencies are allowed to achieve the target frequency.
 *
 * \return Zero on success
 *
 */
int _mcu_core_setusbclock(int fosc /*! The oscillator frequency */) MCU_PRIV_CODE;

/*! \details This function puts the MCU in sleep mode.
 */
int _mcu_core_sleep(core_sleep_t level /*! The sleep type */);


/*! \details This function writes the pin function select value.  When a pin can
 * have multiple functions (such as GPIO or UART RX), this function selects
 * what function the pin is to have.  This function is portable; however
 * the arguments of the function will invariably be different between
 * MCU architectures.
 *
 *
 * \note  The respective periph_open()
 * function will configure the correct function for the pin without the need to call
 * core_set_pinsel_func().
 * It is only necessary to use this function if the pin_assign
 * value is set to \ref MCU_GPIO_CFG_USER.
 *
 */
int _mcu_core_set_pinsel_func(int gpio_port /*! The GPIO port number */,
		int pin /*! The GPIO pin number */,
		core_periph_t function /*! The peripheral to use */,
		int periph_port /*! The peripheral port to use (e.g. 1 for UART1) */) MCU_PRIV_CODE;

int mcu_core_setpinfunc(int port, void * arg) MCU_PRIV_CODE;

void _mcu_core_getserialno(sn_t * serialno) MCU_PRIV_CODE;

#define MCU_CORE_USB_MAX_PACKET_ZERO_VALUE 64

#ifndef __link
static inline int _mcu_core_getclock() MCU_ALWAYS_INLINE;
int _mcu_core_getclock(){ return mcu_board_config.core_cpu_freq; }
#endif

void _mcu_core_priv_enable_interrupts(void * args) MCU_PRIV_CODE;
void _mcu_core_priv_disable_interrupts(void * args) MCU_PRIV_CODE;
void _mcu_core_priv_enable_irq(void * x) MCU_PRIV_CODE;
void _mcu_core_priv_disable_irq(void * x) MCU_PRIV_CODE;
void _mcu_core_priv_reset(void * args) MCU_PRIV_CODE;
void _mcu_core_priv_get_stack_ptr(void * ptr) MCU_PRIV_CODE;
void _mcu_core_priv_set_stack_ptr(void * ptr) MCU_PRIV_CODE;
void _mcu_core_priv_get_thread_stack_ptr(void * ptr) MCU_PRIV_CODE;
void _mcu_core_priv_set_thread_stack_ptr(void * ptr) MCU_PRIV_CODE;

void _mcu_core_unprivileged_mode();
void _mcu_core_thread_mode();

void _mcu_core_delay_us(u32 us);
void _mcu_core_delay_ms(u32 ms);

void _mcu_core_priv_bootloader_api(void * args) MCU_PRIV_CODE;

int _mcu_core_priv_validate_callback(mcu_callback_t callback) MCU_PRIV_CODE;

void _mcu_core_exec_event_handler(mcu_event_handler_t * event, mcu_event_t arg) MCU_PRIV_CODE;
int _mcu_core_setirqprio(int irq, int prio) MCU_PRIV_CODE;


/*! \details
 * \sa periph_open()
 *
 */
int mcu_core_open(const device_cfg_t * cfg) MCU_PRIV_CODE;
/*! \details
 * \sa periph_read()
 *
 */
int mcu_core_read(const device_cfg_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_write()
 */
int mcu_core_write(const device_cfg_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_core_ioctl(const device_cfg_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_core_close(const device_cfg_t * cfg) MCU_PRIV_CODE;

int mcu_core_getattr(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_setattr(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_setaction(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_setpinfunc(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_sleep(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_reset(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_invokebootloader(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_setclkout(int port, void * arg) MCU_PRIV_CODE;
int mcu_core_setclkdivide(int port, void * arg) MCU_PRIV_CODE;





#ifdef __cplusplus
}
#endif

#endif // _MCU_CORE_H_

/*! @} */
