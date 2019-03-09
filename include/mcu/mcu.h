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



/*! \file
 * \brief MCU Header file (includes all other header files)
 */

/*! \addtogroup MCU
 *
 * @{
 *
 *
 */


#ifndef MCU_H_
#define MCU_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "sos/dev/pio.h"

/*! @} */

/*! \addtogroup IFACE_DEV
 * @{
 */


/*! @} */

/*! \addtogroup MCU
 * @{
 */

#include "mcu/types.h"
#include "sos/dev/usb.h"
#include "sos/dev/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined __link

#include "sos/fs/types.h"


//These values are defined in the linker script
extern u32 _top_of_stack;
extern u32 _text;
extern u32 _etext;
extern u32 _data;
extern u32 _edata;
extern u32 _bss;
extern u32 _ebss;
extern u32 _sys;
extern u32 _esys;

#define MCU_SYNC_IO_FLAG_READ (1<<15)

int mcu_sync_io(const devfs_handle_t * handle,
					 int (*func)(const devfs_handle_t * handle, devfs_async_t * op),
					 int loc,
					 const void * buf,
					 int nbyte,
					 int flags);

typedef struct MCU_PACK {
	u16 irq_total /* Total number of interrupts */;
	u16 irq_middle_prio /* Middle priority value */;
	u16 usb_logical_endpoint_count /* Number of logical endpoints */;
	u16 delay_factor /* factor to multiply by when calculating delays (depends on memory performance) */;
	const char * git_hash /* Pointer to git hash string */;
} mcu_config_t;

extern const mcu_config_t mcu_config;

void mcu_board_execute_event_handler(int event, void * args);

//execute an event callback -- callback is null'd if result is 0 -- deprecated
int mcu_execute_event_handler(mcu_event_handler_t * handler, u32 o_events, void * data);

static inline const mcu_pin_t * mcu_pin_at(const void * start, int i){
	const mcu_pin_t * p = (const mcu_pin_t *)start;
	return p+i;
}

const void * mcu_select_attr(const devfs_handle_t * handle, void * ctl);

int mcu_set_pin_assignment(const void * attr_pin_assignment,
									const void * config_pin_assignment,
									int count,
									int periph,
									int periph_port,
									void (*pre_configure_pin)(const mcu_pin_t *, void *),
									void (*post_configure_pin)(const mcu_pin_t *, void*),
									void * arg) MCU_ROOT_CODE;

const void * mcu_select_pin_assignment(const void * attr_pin_assignment,
													const void * config_pin_assignment,
													int count) MCU_ROOT_CODE;
#endif

enum {
	MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH /*! When set, the board LED is active high */ = (1<<0),
	MCU_BOARD_CONFIG_FLAG_SAFE_SVCALL /*! When set, the svcall can only be used by the kernel */ = (1<<1),
	MCU_BOARD_CONFIG_FLAG_ENABLE_CACHE /*! When set, the svcall can only be used by the kernel */ = (1<<2)
};

enum {
	MCU_BOARD_CONFIG_EVENT_ROOT_RESET /*! Event is called after a reset while still in root mode. Args is null */,
	MCU_BOARD_CONFIG_EVENT_ROOT_FATAL /*! Event is called when a fatal error occurs (called in priviledged mode).  Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_FATAL /*! Event is called when a fatal error occurs (called in unpriviledged mode).  Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_CRITICAL /*! Event is called when a critical error occurs (e.g. a filesystem fails to start).  Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_WARNING /*! Event is called when something isn't quite right but things should basically still work. Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_MESSAGE /*! Event is called when a message is available. Args is a c string */,

	MCU_BOARD_CONFIG_EVENT_ROOT_INITIALIZE_CLOCK /*! Called at the beginning of mcu_core_initclock() so that the BSP can initialize the clock configuration */,
	MCU_BOARD_CONFIG_EVENT_START_INIT /*! Called with sos_default_thread(). Args is a pointer to u8 which is the reset type */,
	MCU_BOARD_CONFIG_EVENT_START_FILESYSTEM /*! Called with sos_default_thread() when filesystem starts. Args is a pointer to u32 which says how many processes started */,
	MCU_BOARD_CONFIG_EVENT_START_LINK /*! Called with sos_default_thread(), when link starts.  Args is null */,
	MCU_BOARD_CONFIG_EVENT_HIBERNATE /*! MCU is about to enter hibernate mode */,
	MCU_BOARD_CONFIG_EVENT_POWERDOWN /*! MCU is about to enter powerdown mode */,
	MCU_BOARD_CONFIG_EVENT_WAKEUP_FROM_HIBERNATE /*! MCU wokeup from hibernation */,
	MCU_BOARD_CONFIG_EVENT_ROOT_TASK_INIT /*! MCU is about to enter context switching mode */,

	MCU_BOARD_CONFIG_EVENT_SERVICE_CALL_PERMISSION_DENIED /*! MCU tried to execute a service call that was not permitted */,

	MCU_BOARD_CONFIG_EVENT_ROOT_WDT_TIMEOUT /*! MCU had a WDT timeout */,
	MCU_BOARD_CONFIG_EVENT_SCHEDULER_IDLE /*! All tasks are idle (this can implement an alternate sleep implementation) */,
	MCU_BOARD_CONFIG_EVENT_TOTAL
};


/*! \brief MCU Board Configuration Type
 * \details The MCU Board configuration gives data
 * that tells Stratify OS how the MCU is used on the board.
 *
 */
typedef struct MCU_PACK {
	u32 core_osc_freq;
	u32 core_cpu_freq;
	u32 core_periph_freq;
	u32 usb_max_packet_zero;
	u32 o_flags /*! MCU flags such as MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH */;
	void (*event_handler)(int, void*) /*! A callback to an event handler that gets, for example, MCU_BOARD_CONFIG_EVENT_FATAL on a fatal event */;
	mcu_pin_t led /*! A pin on the board that drives an LED. Use {0xff, 0xff} if not available. */;
	u8 debug_uart_port /*! The port used for the UART debugger. This is only used for _debug builds */;
	u8 resd;
	uart_attr_t debug_uart_attr /*! The UART attributes for the UART debugger. */;
	const void * arch_config /*! A pointer to MCU architecture specific data, for example, stm32_arch_config_t */;
	u32 o_mcu_debug /*! Debugging flags (only used when linking to debug libraries */;
	u32 os_mpu_text_mask /*! Mask to apply to _text when setting the kernel memory protection 0x0000ffff to ignore bottom 16-bits */;
} mcu_board_config_t;

/*! \brief MCU Board configuration variable
 * \details This variable must be provided by the board support package.
 */
extern const mcu_board_config_t mcu_board_config;



#ifdef __cplusplus
}
#endif



#endif /* MCU_H_ */

/*! @} */


