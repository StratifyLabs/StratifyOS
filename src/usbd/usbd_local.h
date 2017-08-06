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

#ifndef USBD_LOCAL_H_
#define USBD_LOCAL_H_

#include "usbd/typedefs.h"

int usbd_control_reset(const devfs_handle_t * handle);
int usbd_control_attach(const devfs_handle_t * handle);
int usbd_control_detach(const devfs_handle_t * handle);
int usbd_control_configure(const devfs_handle_t * handle);
int usbd_control_unconfigure(const devfs_handle_t * handle);
int usbd_control_set_address(const devfs_handle_t * handle, u8 address);
int usbd_control_reset_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_enable_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_disable_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_stall_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_unstall_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_configure_endpoint(const devfs_handle_t * handle, const usbd_endpoint_descriptor_t * endpoint_descriptor);



#endif /* USBD_LOCAL_H_ */
