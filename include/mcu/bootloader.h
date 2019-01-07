/* Copyright 2011-2017 Tyler Gilbert;
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

#ifndef MCU_BOOTLOADER_H_
#define MCU_BOOTLOADER_H_

#include "../sos/dev/bootloader.h"


/*! \brief Bootloader attributes.
 * \details This structure contains the attributes for the bootloader.
 */
typedef struct MCU_PACK {
	u32 code_size /*! The number of bytes occupied by the bootloader (so the OS knows not to erase the bootloader) */;
	void (*exec)(void * args) /*! Execute the bootlaoder */;
	void (*usbd_control_root_init)(void * context) /*! Use this to handle standard USB packet is using same USB setup as bootloader */;
	void (*event)(int, void*) /*! Invoke the board events (encryption, decryption, custom code) */;
} bootloader_api_t;


enum {
	BOOT_BOARD_CONFIG_FLAG_HW_REQ_ACTIVE_HIGH = (1<<0),
	BOOT_BOARD_CONFIG_FLAG_HW_REQ_PULLUP = (1<<1),
	BOOT_BOARD_CONFIG_FLAG_HW_REQ_PULLDOWN = (1<<2)
};


enum {
	BOOT_EVENT_FATAL /*! Called on a Fatal Error */,
	BOOT_EVENT_CRITICAL /*! Called on a critical error */,
	BOOT_EVENT_START /*! Called when the device starts executing code and before checking for a bootloader request */,
	BOOT_EVENT_INIT_CLOCK /*! Called just before the bootloader updates the clock configuration */,
	BOOT_EVENT_INIT /*! Called when the bootloader is initializing */,
	BOOT_EVENT_RESET /*! Called when the link protocol calls for a reset */,
	BOOT_EVENT_RESET_BOOTLOADER /*! Called when the link protocol calls for a bootloader reset */,
	BOOT_EVENT_READ_SERIALNO /*! Called when the link protocol reads the serial number */,
	BOOT_EVENT_FLASH_READ /*! Called when the link protocol reads the flash */,
	BOOT_EVENT_FLASH_WRITE /*! Called when the link protocol writes the flash */,
	BOOT_EVENT_FLASH_ERASE /*! Called when the link protocol erases the flash */,
	BOOT_EVENT_ENCRYPT /*! Called when the bootloader needs something encrypted */,
	BOOT_EVENT_DECRYPT /*! Called when the bootloader needs something decrypted */,
	BOOT_EVENT_RUN_APP /*! Called just before the bootloader runs the user code */,
	BOOT_EVENT_RUN_BOOTLOADER /*! Called just before the bootloader runs the bootloader code (no app or bootloader was requested)*/,
	BOOT_EVENT_CHECK_APP_EXISTS,
	BOOT_EVENT_CHECK_SOFTWARE_BOOT_REQUEST,
	BOOT_EVENT_CHECK_HARDWARE_BOOT_REQUEST,
	BOOT_EVENT_TOTAL
};


typedef struct {
	int abort;
	int bytes;
	int increment;
	int total;
} boot_event_flash_t;

typedef struct {
	void * dest;
	const void * src;
	const char * key;
} boot_event_crypt_t;



typedef struct MCU_PACK {
	u32 sw_req_loc;
	u32 sw_req_value;
	u32 program_start_addr;
	mcu_pin_t hw_req;
	u16 o_flags;
	link_transport_driver_t * link_transport_driver;
	u32 id;
} bootloader_board_config_t;




#endif /* MCU_BOOTLOADER_H_ */
