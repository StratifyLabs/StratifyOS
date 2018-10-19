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

#ifndef SOS_DEV_MMC_H_
#define SOS_DEV_MMC_H_

#include "mcu/types.h"

#define MMC_VERSION (0x030000)
#define MMC_IOC_IDENT_CHAR 'Q'

enum {
	MMC_FLAG_SET_INTERFACE = (1<<0),
	MMC_FLAG_SEND_STATUS = (1<<1),
	MMC_FLAG_GET_CARD_STATE = (1<<2),
	MMC_FLAG_IS_CLOCK_RISING = (1<<3),
	MMC_FLAG_IS_CLOCK_FALLING = (1<<4),
	MMC_FLAG_IS_CLOCK_BYPASS_ENABLED = (1<<5),
	MMC_FLAG_IS_CLOCK_POWER_SAVE_ENABLED = (1<<6),
	MMC_FLAG_IS_HARDWARE_FLOW_CONTROL_ENABLED = (1<<7),
	MMC_FLAG_IS_BUS_WIDTH_1 = (1<<8),
	MMC_FLAG_IS_BUS_WIDTH_4 = (1<<9),
	MMC_FLAG_ERASE_BLOCKS = (1<<10),
	MMC_FLAG_IS_BUS_WIDTH_8 = (1<<11),
	MMC_FLAG_IS_BYTE_ADDRESSING = (1<<12),
	MMC_FLAG_RESET = (1<<13)
};

enum {
	MMC_CARD_STATE_NONE,
	MMC_CARD_STATE_READY,
	MMC_CARD_STATE_IDENTIFICATION,
	MMC_CARD_STATE_STANDBY,
	MMC_CARD_STATE_TRANSFER,
	MMC_CARD_STATE_SENDING,
	MMC_CARD_STATE_RECEIVING,
	MMC_CARD_STATE_PROGRAMMING,
	MMC_CARD_STATE_DISCONNECTED,
	MMC_CARD_STATE_ERROR = 255
};

typedef struct {
	u32 o_flags;
	u32 o_events;
	u32 freq;
	u32 type;
	u32 version;
	u32 card_class;
	u32 relative_address;
	u32 block_count;
	u32 block_size;
	u32 logical_block_count;
	u32 logical_block_size;
} mmc_info_t;

typedef struct {
	mcu_pin_t clock;
	mcu_pin_t command;
	mcu_pin_t data[8];
} mmc_pin_assignment_t;

typedef struct {
	u32 o_flags;
	u32 freq /*! Maximum frequency is 25MHz */;
	mmc_pin_assignment_t pin_assignment;
	u32 start /*! Used with MMC_FLAG_ERASE_BLOCKS as the first block to erase */;
	u32 end/*! Used with MMC_FLAG_ERASE_BLOCKS as the last block to erase */;
} mmc_attr_t;


typedef struct {
	u8 manufacturer_id;
	u16 oem_application_id;
	u32 product_name_1;
	u8 product_name_2;
	u32 product_revision;
	u32 product_serial_number;
	u8 resd0;
	u16 manufacturer_date;
	u8 crc;
	u8 resd1;
} mmc_cid_t;

typedef struct MCU_PACK {
	u8  csd_structure;
	u8  system_specification_version;
	u8  resd0;
	u8  taac;
	u8  nsac;
	u8  maximum_bus_clock_frequency;
	u16 card_command_classes;
	u8  read_block_length;
	u8  partial_block_read_allowed;
	u8  write_block_misalignment;
	u8  read_block_misalignment;
	u8  dsr_implemnted;
	u8  resd1;
	u32 device_size;
	u8  max_read_current_at_minimum_vdd;
	u8  max_read_current_at_maximum_vdd;
	u8  max_write_current_at_minimum_vdd;
	u8  max_write_current_at_maximum_vdd;
	u8  device_size_multiplier;
	u8  erase_group_size;
	u8  erase_group_multiplier;
	u8  write_protect_group_size;
	u8  write_protect_group_enable;
	u8  manufacturer_default_ecc;
	u8  write_speed_factor;
	u8  max_write_block_length;
	u8  partial_block_write_allowed;
	u8  resd2;
	u8  content_protection_application;
	u8  file_format_group;
	u8  copy_flag;
	u8  permanent_write_protection;
	u8  temporary_write_protection;
	u8  file_format;
	u8  ecc;
	u8  crc;
	u8  resd3;
} mmc_csd_t;

typedef struct MCU_PACK {
	u8  data_bus_width;
	u8  secured_mode;
	u16 card_type;
	u32 protected_area_size;
	u8  speed_class;
	u8  performance_move;
	u8  allocation_unit_size;
	u16 erase_size;
	u8  erase_timeout;
	u8  erase_offset;
} mmc_status_t;



#define I_MMC_GETVERSION _IOCTL(MMC_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief IOCTL request to get the attributes.
 * \hideinitializer
 */
#define I_MMC_GETINFO _IOCTLR(MMC_IOC_IDENT_CHAR, I_MCU_GETINFO, mmc_info_t)

/*! \brief IOCTL request to set the attributes.
 * \hideinitializer
 */
#define I_MMC_SETATTR _IOCTLW(MMC_IOC_IDENT_CHAR, I_MCU_SETATTR, mmc_attr_t)

/*! \brief IOCTL request to set the action.
 * \hideinitializer
 */
#define I_MMC_SETACTION _IOCTLW(MMC_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

#define I_MMC_GETCID _IOCTLR(MMC_IOC_IDENT_CHAR, I_MCU_TOTAL, mmc_cid_t)
#define I_MMC_GETCSD _IOCTLR(MMC_IOC_IDENT_CHAR, I_MCU_TOTAL+1, mmc_csd_t)
#define I_MMC_GETSTATUS _IOCTLR(MMC_IOC_IDENT_CHAR, I_MCU_TOTAL+1, mmc_status_t)


#define I_MMC_TOTAL 3

#endif /* SOS_DEV_MMC_H_ */
