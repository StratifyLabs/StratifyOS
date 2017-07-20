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
#include "sos/fs/devfs.h"

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

#ifdef __cplusplus
extern "C" {
#endif


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

int mcu_sync_io(const devfs_handle_t * cfg,
		int (*func)(const devfs_handle_t * cfg, devfs_async_t * op),
		int loc,
		const void * buf,
		int nbyte,
		int flags);

enum {
	MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH = (1<<0)
};

enum {
	MCU_BOARD_CONFIG_EVENT_PRIV_RESET /*! Event is called after a reset while still in priviledged mode. Args is null */,
	MCU_BOARD_CONFIG_EVENT_PRIV_FATAL /*! Event is called when a fatal error occurs (called in priviledged mode).  Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_FATAL /*! Event is called when a fatal error occurs (called in unpriviledged mode).  Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_CRITICAL /*! Event is called when a critical error occurs (e.g. a filesystem fails to start).  Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_WARNING /*! Event is called when something isn't quite right but things should basically still work. Args is a c string */,
	MCU_BOARD_CONFIG_EVENT_MESSAGE /*! Event is called when a message is available. Args is a c string */,

	//These are used with stratify_default_thread
	MCU_BOARD_CONFIG_EVENT_START_INIT /*! Called with stratify_default_thread(). Args is a pointer to u8 which is the reset type */,
	MCU_BOARD_CONFIG_EVENT_START_FILESYSTEM /*! Called with stratify_default_thread() when filesystem starts. Args is a pointer to u32 which says how many processes started */,
	MCU_BOARD_CONFIG_EVENT_START_LINK /*! Called with stratify_default_thread(), when link starts.  Args is null */,
	MCU_BOARD_CONFIG_EVENT_TOTAL
};

typedef struct MCU_PACK {
	u32 core_osc_freq;
	u32 core_cpu_freq;
	u32 core_periph_freq;
	u32 usb_max_packet_zero;
	u32 o_flags;
	void (*event)(int, void*);
	mcu_pin_t led;
	mcu_pin_t debug_uart_pin_assignment[2];
	mcu_pin_t usb_pin_assignment[USB_PIN_ASSIGNMENT_COUNT];
	u8 debug_uart_port;
	u8 resd;
} mcu_board_config_t;

extern const mcu_board_config_t mcu_board_config;

typedef struct MCU_PACK {
	u16 irq_total;
	u16 irq_middle_prio;
	u16 usb_logical_endpoint_count;
} mcu_config_t;

extern const mcu_config_t mcu_config;

void mcu_board_event(int event, void * args);

int mcu_execute_event_handler(mcu_event_handler_t * handler, u32 o_events, void * data);

#ifdef __cplusplus
}
#endif



#endif /* MCU_H_ */

/*! @} */


