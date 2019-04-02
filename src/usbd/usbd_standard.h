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

#ifndef USBD_STANDARD_H_
#define USBD_STANDARD_H_

#include "usbd/control.h"

int usbd_standard_request_handle_setup(usbd_control_t * context);

u32 usbd_standard_request_set_interface(usbd_control_t * context);
u32 usbd_standard_request_get_interface(usbd_control_t * context);
void * usbd_control_add_ptr(usbd_control_t * context, void * ptr, u32 n);
u32 usbd_standard_request_set_clr_feature (usbd_control_t * context, u32 sc);
u32 usbd_standard_request_get_descriptor(usbd_control_t * context);
u32 usbd_standard_request_set_address (usbd_control_t * context);
u32 usbd_standard_request_set_config (usbd_control_t * context);
u32 usbd_standard_request_get_config (usbd_control_t * context);
u32 usbd_standard_request_get_status (usbd_control_t * context);


#endif /* USBD_STANDARD_H_ */
