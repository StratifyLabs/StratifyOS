/* Copyright 2011; 
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
 * Please see http://www.coactionos.com/license.html for
 * licensing information.
 */

/*! \addtogroup LINK
 *
 * @{
 *
 * \details This module provides the interface to Stratify OS Link as a USB device.  The interface can be
 * customized by adding new initialization values for \ref link_dev_desc and \ref link_string_desc in the
 * \a devices.c file.  Only the following values can be changed by the user:
 * - USB VID
 * - USB PID
 * - Manufacturer String
 * - Product String
 * - Serial Number String
 *
 * Changing any of the above values will require custom USB drivers to make software applications work with
 * Stratify OS.
 *
 */

/*! \file */


#ifndef BOOT_LINK_H_
#define BOOT_LINK_H_

#include "sos/boot/boot_debug.h"
#include "sos/fs/devfs.h"
#include "sos/link.h"
#include "usbd/types.h"

void * boot_link_update(void * arg);

#endif /* BOOT_LINK_H_ */


/*! @} */
