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


/*! @} */

/*! \addtogroup IFACE_DEV
 * @{
 */


/*! @} */

/*! \addtogroup HWPL
 * @{
 */

#include "mcu/fault.h"
#include "mcu/arch.h"
#include "mcu/types.h"
#include "iface/dev/pio.h"
#include "iface/device_config.h"

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

int mcu_check_adc_port(const device_cfg_t * cfgp);
int mcu_check_spi_port(const device_cfg_t * cfgp);
int mcu_check_i2c_port(const device_cfg_t * cfgp);
int mcu_check_pwm_port(const device_cfg_t * cfgp);
int mcu_check_uart_port(const device_cfg_t * cfgp);

#define MCU_SYNC_IO_FLAG_READ (1<<15)

int mcu_sync_io(const device_cfg_t * cfg,
		int (*func)(const device_cfg_t * cfg, device_transfer_t * op),
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
	u32 flags;
	void (*event)(int, void*);
	pio_t led;
	u16 resd;
} mcu_board_config_t;

extern const mcu_board_config_t mcu_board_config;

void mcu_board_event(int event, void * args);

#ifdef __cplusplus
}
#endif



#endif /* MCU_H_ */

/*! @} */


