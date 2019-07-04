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

#ifndef USBD_MSC_H_
#define USBD_MSC_H_

#include "types.h"


//Subclasses
#define USBD_MSC_SUBCLASS_RBC 0x01
#define USBD_MSC_SUBCLASS_SFF 0x02
#define USBD_MSC_SUBCLASS_QIC 0x03
#define USBD_MSC_SUBCLASS_UFI 0x04
#define USBD_MSC_SUBCLASS_SFF 0x05
#define USBD_MSC_SUBCLASS_SCSI 0x06

//Protocol Codes
#define USBD_MSC_INTERFACE_PROTOCOL_CBI 0x00
#define USBD_MSC_INTERFACE_PROTOCOL_CBI_COMMAND_COMPLETION 0x01
#define USBD_MSC_INTERFACE_PROTOCOL_BULK_ONLY_TRANSPORT 0x50

//Requests
#define USBD_MSC_REQUEST_RESET 0xFF
#define USBD_MSC_REQUEST_GET_MAX_LUN 0xFE

#define USBD_MSC_BULK_ONLY_COMMAND_BLOCK_WRAPPER 0
#define USBD_MSC_BULK_ONLY_DATA_OUT 1
#define USBD_MSC_BULK_ONLY_DATA_IN 2
#define USBD_MSC_BULK_ONLY_DATA_IN_LAST 3
#define USBD_MSC_BULK_ONLY_DATA_IN_LAST_STALL 4
#define USBD_MSC_BULK_ONLY_COMMAND_STATUS_WRAPPER 5
#define USBD_MSC_BULK_ONLY_ERROR 6
#define USBD_MSC_BULK_ONLY_RESET 7

#define USBD_MSC_BULK_ONLY_COMMAND_BLOCK_WRAPPER_SIGNATURE 0x43425355
#define USBD_MSC_BULK_ONLY_COMMAND_STATUS_WRAPPER_SIGNATURE 0x53425355

#define USBD_MSC_BULK_ONLY_COMMAND_STATUS_WRAPPER_COMMAND_PASSED 0
#define USBD_MSC_BULK_ONLY_COMMAND_STATUS_WRAPPER_COMMAND_FAILED 1
#define USBD_MSC_BULK_ONLY_COMMAND_STATUS_WRAPPER_PHASE_ERROR 2


typedef struct MCU_PACK {
	 u32 dCBWSignature;
	 u32 dCBWTag;
	 u32 dCBWDataTransferLength;
	 u8 bmCBWFlags;
	 u8 bCBWLUN;
	 u8 bCBWCBLength;
	 u8 CBWCB[16];
} usbd_msc_bulk_only_command_block_wrapper_t;

typedef struct MCU_PACK {
	u32 dCSWSignature;
	u32 dCSWTag;
	u32 dCSWDataResidue;
	u8 bCSWStatus;
} usbd_msc_bulk_only_command_status_wrapper_t;

typedef struct MCU_PACK {
	usbd_interface_descriptor_t interface_control /* The interface descriptor */;
	usbd_endpoint_descriptor_t bulk_in /* Endpoint:  Bulk in */;
	usbd_endpoint_descriptor_t bulk_out /* Endpoint:  Bulk out */;
} usbd_msc_configuration_descriptor_t;

#endif /* USBD_MSC_H_ */

/*! @} */
