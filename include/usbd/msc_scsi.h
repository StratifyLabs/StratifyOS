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
#define USBD_MSC_SCSI_READ_CAPACITY 0x25
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

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u8 page_code;
	u8 allocation_length_high;
	u8 allocation_length_low;
	u8 control;
} usbd_msc_scsi_inquiry_t;

typedef struct MCU_PACK {
	u8 opcode;
	u8 o_flags;
	u8 vendor_specific;
	u8 reserved;
	u8 ffmt;
	u8 control;
} usbd_msc_scsi_foram_unit_t;

typedef struct MCU_PACK {

} usbd_msc_scsi_command_t;


#endif /* USBD_MSC_SCSI_H_ */

/*! @} */
