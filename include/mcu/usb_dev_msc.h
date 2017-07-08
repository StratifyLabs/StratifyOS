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

/*! \addtogroup USB_DEV_MSC Mass Storage Class
 *
 *
 * @{
 * \ingroup USB_DEV
 *
 * \details This is not yet implemented.
 *
 */


/*! \file */

#ifndef USB_DEV_MSC_H_
#define USB_DEV_MSC_H_


//! \todo add an init function

//Subclasses
#define MSC_SUBCLASS_RBC 0x01
#define MSC_SUBCLASS_SFF 0x02
#define MSC_SUBCLASS_QIC 0x03
#define MSC_SUBCLASS_UFI 0x04
#define MSC_SUBCLASS_SFF 0x05
#define MSC_SUBCLASS_SCSI 0x06

//Protocol Codes
#define MSC_CBI_PROTOCOL 0x00
#define MSC_CBI_PROTOCOL_COMMAND_COMPLETION 0x01
#define MSC_BULK_ONLY_TRANSPORT 0x50




#endif /* USB_DEV_MSC_H_ */

/*! @} */
