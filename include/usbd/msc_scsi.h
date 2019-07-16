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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS. If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

/*! \addtogroup USBD_MSC Mass Storage Class
 *
 *
 * @{
 * \ingroup USB_DEV
 *
 * \details This is not yet implemented.
 *
 */


/*! \file */

#ifndef USBD_MSC_SCSI_H_
#define USBD_MSC_SCSI_H_

#include "msc.h"

#define USBD_MSC_SCSI_TEST_UNIT_READY 0x00
#define USBD_MSC_SCSI_REQUEST_SENSE 0x03
#define USBD_MSC_SCSI_FORMAT_UNIT 0x04
#define USBD_MSC_SCSI_INQUIRY 0x12
#define USBD_MSC_SCSI_MODE_SELECT6 0x15
#define USBD_MSC_SCSI_MODE_SENSE6 0x1A
#define USBD_MSC_SCSI_START_STOP_UNIT 0x1B
#define USBD_MSC_SCSI_MEDIA_REMOVAL 0x1E
#define USBD_MSC_SCSI_READ_FORMAT_CAPACITIES 0x23
#define USBD_MSC_SCSI_READ_CAPACITY10 0x25
#define USBD_MSC_SCSI_READ10 0x28
#define USBD_MSC_SCSI_WRITE10 0x2A
#define USBD_MSC_SCSI_VERIFY10 0x2F
#define USBD_MSC_SCSI_SYNC_CACHE10 0x35
#define USBD_MSC_SCSI_READ12 0xA8
#define USBD_MSC_SCSI_WRITE12 0xAA
#define USBD_MSC_SCSI_MODE_SELECT10 0x55
#define USBD_MSC_SCSI_MODE_SENSE10 0x5A
#define USBD_MSC_SCSI_SYNC_CACHE16 0x91
#define USBD_MSC_SCSI_ATA_COMMAND_PASS_THROUGH12 0xA1
#define USBD_MSC_SCSI_ATA_COMMAND_PASS_THROUGH16 0x85
#define USBD_MSC_SCSI_SERVICE_ACTION_IN12 0xAB
#define USBD_MSC_SCSI_SERVICE_ACTION_IN16 0x9E
#define USBD_MSC_SCSI_SERVICE_ACTION_OUT12 0xA9
#define USBD_MSC_SCSI_SERVICE_ACTION_OUT16 0x9F
#define USBD_MSC_SCSI_REPORT_ID_INFO 0xA3

#define USBD_MSC_SCSI_SENSE_INFORMATION 0x00
#define USBD_MSC_SCSI_SENSE_COMMAND_SPECIFIC_INFORMATION 0x02
#define USBD_MSC_SCSI_SENSE_SENSE_KEY_SPECIFIC 0x02
#define USBD_MSC_SCSI_SENSE_FIELD_REPLACEABLE_UNIT 0x03
#define USBD_MSC_SCSI_SENSE_STREAM_COMMANDS 0x04
#define USBD_MSC_SCSI_SENSE_BLOCK_COMMANDS 0x05
#define USBD_MSC_SCSI_SENSE_OSD_OBJECT_IDENTIFICATION 0x06
#define USBD_MSC_SCSI_SENSE_OSD_RESPONSE_INTEGRITY_CHECK_VALUE 0x07
#define USBD_MSC_SCSI_SENSE_OSD_ATTRIBUTE_IDENTIFICATION 0x08
#define USBD_MSC_SCSI_SENSE_ATA_RETURN 0x09
#define USBD_MSC_SCSI_SENSE_ANOTHER_PROGRESS_INDICATION 0x0A



typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u8 page_code;
	u16 allocation_length; //big endian
	u8 control;
} usbd_msc_scsi_inquiry_t;

#define USBD_MSC_SCSI_INQUIRY_PERIPHERAL_TYPE_DIRECT_ACCESS 0x00
#define USBD_MSC_SCSI_INQUIRY_VERSION_NO_STANDARD 0x00
#define USBD_MSC_SCSI_INQUIRY_VERSION_OBSOLETE 0x02
#define USBD_MSC_SCSI_INQUIRY_VERSION_ANSI_301_1997 0x03
#define USBD_MSC_SCSI_INQUIRY_VERSION_ANSI_351_2001 0x04
#define USBD_MSC_SCSI_INQUIRY_VERSION_ANSI_408_2005 0x05
#define USBD_MSC_SCSI_INQUIRY_VERSION_ANSI_513_2015 0x06
#define USBD_MSC_SCSI_INQUIRY_REMOVABLE_MEDIUM 0x80
#define USBD_MSC_SCSI_INQUIRY_NONREMOVABLE_MEDIUM 0x00

#define USBD_MSC_SCSI_INQUIRY_O_FLAGS0_NORMACA (1<<5)
#define USBD_MSC_SCSI_INQUIRY_O_FLAGS0_HISUP (1<<4)
#define USBD_MSC_SCSI_INQUIRY_O_FLAGS0_RESPONSE_DATA_FORMAT 0x02

#define USBD_MSC_SCSI_INQUIRY_O_FLAGS1_PROTECT (1<<0)
#define USBD_MSC_SCSI_INQUIRY_O_FLAGS1_3PC (1<<3)
#define USBD_MSC_SCSI_INQUIRY_O_FLAGS1_TPGS(x) (x<<4)
#define USBD_MSC_SCSI_INQUIRY_O_FLAGS1_ACC (1<<6)
#define USBD_MSC_SCSI_INQUIRY_O_FLAGS1_SCCS (1<<7)

typedef struct MCU_PACK {
	u8 vendor_identification[8];
	u8 product_identification[16];
	u8 product_revision_level[4];
} usbd_msc_scsi_standard_inquiry_indentification_t;

typedef struct MCU_PACK {
	u8 peripheral;
	u8 removable_medium;
	u8 version;
	u8 o_flags0;
	u8 additional_length;
	u8 o_flags1;
	u8 o_flags2;
	u8 o_flags3;
	usbd_msc_scsi_standard_inquiry_indentification_t identification;
} usbd_msc_scsi_standard_inquiry_data_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u8 vendor_specific;
	u8 reserved;
	u8 ffmt;
	u8 control;
} usbd_msc_scsi_format_unit_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 immediately;
	u8 reserved;
	u8 power_condition_modifier;
	u8 o_flags;
	u8 control;
} usbd_msc_scsi_start_stop_unit_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u32 logical_block_address; //big endian
	u8 group_number;
	u16 transfer_length;
	u8 control;
} usbd_msc_scsi_transfer_10_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u32 logical_block_address; //big endian
	u8 group_number;
	u32 transfer_length;
	u8 control;
} usbd_msc_scsi_transfer_12_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u32 logical_block_address; //big endian
	u8 group_number;
	u16 verification_length;
	u8 control;
} usbd_msc_scsi_verify10_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 reserved0;
	u32 logical_block_address; //big endian
	u16 reserved1;
	u8 reserved2;
	u8 control;
} usbd_msc_scsi_read_capacity10_t;

typedef struct MCU_PACK {
	u32 returned_logical_block_address;
	u32 block_length_in_bytes;
} usbd_msc_scsi_read_capacity10_data_t;

typedef struct MCU_PACK {
	u8 reserved1;
	u8 reserved2;
	u8 reserved3;
	u8 capacity_list_length;
	u32 block_count;
	u8 descriptor_code;
	u8 block_size_high;
	u8 block_size_medium;
	u8 block_size_low;
} usbd_msc_scsi_read_format_capacities_data_t;


#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_NO_SENSE (0x00)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_RECOVERED_ERROR (0x01)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_NOT_READY (0x02)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_MEDIUM_ERROR (0x03)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_HARDWARE_ERROR (0x04)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_ILLEGAL_REQUEST (0x05)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_UNIT_ATTENTION (0x06)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_DATA_PROTECT (0x07)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_FIRMWARE_ERROR (0x09)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_ABORTED_COMMAND (0x0B)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_EQUAL (0x0C)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_VOLUME_OVERFLOW (0x0D)
#define USBD_MSC_SCSI_REQUEST_SENSE_KEY_MISCOMPARE (0x0E)

typedef struct MCU_PACK {
	u8 opcode;
	u8 description;
	u16 reserved;
	u8 allocation_length;
	u8 control;
} usbd_msc_scsi_request_sense_t;

typedef struct MCU_PACK {
	u8 error_code_valid;
	u8 segment_number;
	u8 sense_key_flags;
	u8 information[4];
	u8 additional_sense_length;
	u8 command_specific_information[4];
	u8 additional_sense_code;
	u8 additional_sense_code_qualifier;
	u8 field_replaceable_unit_code;
	u8 sense_key_specific[3];
} usbd_msc_scsi_request_sense_data_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u32 logical_block_address; //big endian
	u8 group_number;
	u16 number_of_blocks;
	u8 control;
} usbd_msc_scsi_sync_cache10_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u8 page_code;
	u8 subpage_code;
	u8 allocation_length;
	u8 control;
} usbd_msc_scsi_mode_sense6_t;

typedef struct MCU_PACK {
	u16 reserved;
	u16 information_length;
	u8 information[64];
} usbd_msc_scsi_report_id_data_t;

typedef struct MCU_PACK {

} usbd_msc_scsi_command_t;


#endif /* USBD_MSC_SCSI_H_ */

/*! @} */
